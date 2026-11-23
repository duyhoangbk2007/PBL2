#include "SeatMapView.h"
#include "CatalogView.h"
#include "../business/RoomService.h"
#include "../business/SeatService.h"
#include <QAbstractTableModel>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QShortcut>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>
#include <map>
#include <memory>

namespace {
QString rowName(int row)
{
    QString name;
    while (row > 0) {
        --row;
        name.prepend(QChar('A' + row % 26));
        row /= 26;
    }
    return name;
}

struct Cell {
    QString id;
    SeatType type = SeatType::STANDARD;
};
using Positions = std::map<std::pair<int, int>, Cell>;

const QStringList types = {"STANDARD", "VIP", "COUPLE"};

class SeatTile : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        const int type = index.data(Qt::UserRole).toInt();
        const QColor fills[] = {Qt::white, QColor("#e6eaf1"), QColor("#eceaff"), QColor("#fcecf4")};
        const QColor borders[] = {QColor("#cbd4e0"), QColor("#cbd4e0"), QColor("#c9c5fa"), QColor("#e9c5d7")};
        const int color = type + 1;
        QRect box = option.rect.adjusted(type == 2 ? 2 : 7, 4, type == 2 ? -2 : -7, -4);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        QPen pen(borders[color]);
        if (type < 0) {
            pen.setStyle(Qt::DashLine);
        }
        if (option.state & QStyle::State_Selected) {
            pen.setColor(QColor("#455fe1"));
            pen.setWidth(2);
        }
        painter->setPen(pen);
        painter->setBrush(fills[color]);
        painter->drawRoundedRect(box, 8, 8);
        painter->setPen(QColor("#43516f"));
        painter->drawText(box, Qt::AlignCenter, painter->fontMetrics().elidedText(
            index.data().toString(), Qt::ElideRight, box.width() - 4));
        painter->restore();
    }
};
}

class SeatGrid : public QAbstractTableModel
{
public:
    using QAbstractTableModel::QAbstractTableModel;
    int rows = 0;
    int columns = 0;
    Positions cells;
    void replace(int newRows, int newColumns, Positions positions)
    {
        beginResetModel();
        rows = newRows;
        columns = newColumns;
        cells = std::move(positions);
        endResetModel();
    }
    int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : rows; }
    int columnCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : columns; }
    QVariant headerData(int section, Qt::Orientation direction, int role) const override
    {
        if (role == Qt::DisplayRole && direction == Qt::Vertical) {
            return rowName(section + 1);
        }
        return QAbstractTableModel::headerData(section, direction, role);
    }
    QVariant data(const QModelIndex& index, int role) const override
    {
        const auto pos = cells.find({index.row() + 1, index.column() + 1});
        const bool empty = pos == cells.end();
        if (role == Qt::UserRole) {
            return empty ? -1 : static_cast<int>(pos->second.type);
        }
        if (role == Qt::DisplayRole) {
            return empty ? QString("+") : pos->second.id;
        }
        if (role == Qt::ToolTipRole || role == Qt::AccessibleTextRole) {
            return empty ? QString("Ô trống") : pos->second.id + " · " + types.at(static_cast<int>(pos->second.type));
        }
        return {};
    }
};

SeatMapView::SeatMapView(RoomService& rooms, SeatService& seats, AuthService& auth, bool transactionReady)
    : rooms(rooms), seats(seats), auth(auth), transactionReady(transactionReady),
      picker(new QComboBox), grid(new SeatGrid(this)), status(new QLabel)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    picker->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    auto* toolbar = new QHBoxLayout;
    toolbar->addWidget(new QLabel("Phòng chiếu"));
    toolbar->addWidget(picker);
    toolbar->addStretch();
    layout->addLayout(toolbar);
    auto* panel = new QFrame;
    panel->setObjectName("panel");
    auto* body = new QVBoxLayout(panel);
    body->setContentsMargins(22, 20, 22, 18);
    auto* screen = new QLabel("M À N   H Ì N H");
    screen->setObjectName("cinemaScreen");
    screen->setAlignment(Qt::AlignCenter);
    body->addWidget(screen);
    auto* matrix = new QTableView;
    matrix->setObjectName("seatMatrix");
    matrix->setModel(grid);
    matrix->setItemDelegate(new SeatTile(matrix));
    matrix->setShowGrid(false);
    matrix->setFrameShape(QFrame::NoFrame);
    matrix->setSelectionMode(QAbstractItemView::SingleSelection);
    matrix->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    matrix->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    matrix->horizontalHeader()->setDefaultSectionSize(74);
    matrix->verticalHeader()->setDefaultSectionSize(55);
    auto* centered = new QHBoxLayout;
    centered->addStretch();
    centered->addWidget(matrix, 100);
    centered->addStretch();
    body->addLayout(centered, 1);
    connect(grid, &QAbstractItemModel::modelReset, matrix, [matrix, this] {
        const qint64 desired = 74LL * grid->columns + matrix->verticalHeader()->sizeHint().width() + 24;
        matrix->setMaximumWidth(static_cast<int>(qMin<qint64>(desired, QWIDGETSIZE_MAX)));
    });
    auto* legend = new QHBoxLayout;
    for (const auto& type : QStringList{"STANDARD", "VIP", "COUPLE", "Ô trống (+)"}) {
        auto* tag = new QLabel(type);
        tag->setObjectName("legend");
        tag->setProperty("kind", type);
        legend->addWidget(tag);
    }
    legend->addStretch();
    body->addLayout(legend);
    layout->addWidget(panel, 1);
    status->setWordWrap(true);
    status->setTextFormat(Qt::PlainText);
    layout->addWidget(status);
    connect(picker, &QComboBox::currentIndexChanged, this, [this] {
        status->clear();
        CatalogUi::run(status, [this] { loadRoom(); });
    });
    auto inspect = [this](const QModelIndex& index) {
        CatalogUi::run(status, [&] { openCell(index.row() + 1, index.column() + 1); });
    };
    connect(matrix, &QTableView::clicked, this, inspect);
    auto* enter = new QShortcut(QKeySequence(Qt::Key_Return), matrix);
    enter->setContext(Qt::WidgetShortcut);
    connect(enter, &QShortcut::activated, this, [matrix, inspect] {
        if (matrix->currentIndex().isValid()) {
            inspect(matrix->currentIndex());
        }
    });
}

void SeatMapView::refresh(const QString& preferredRoom)
{
    CatalogUi::run(status, [&] {
        const auto id = preferredRoom.isEmpty() ? picker->currentData().toString() : preferredRoom;
        QSignalBlocker blocker(picker);
        picker->clear();
        grid->replace(0, 0, {});
        const int count = rooms.getRoomCount();
        for (int i = 0; i < count; ++i) {
            const auto* room = rooms.getRoomAt(i);
            if (room) {
                const auto key = QString::fromStdString(room->getRoomId());
                picker->addItem(key + " · " + QString::fromStdString(room->getName()), key);
            }
        }
        const int selected = picker->findData(id);
        if (selected >= 0) {
            picker->setCurrentIndex(selected);
        }
        loadRoom();
    });
}

void SeatMapView::loadRoom()
{
    grid->replace(0, 0, {});
    const auto key = picker->currentData().toString().toStdString();
    const auto* room = rooms.findRoomById(key);
    if (!room) {
        status->setText("Chưa có phòng để hiển thị.");
        return;
    }
    Positions positions;
    const int count = seats.getSeatCount();
    for (int i = 0; i < count; ++i) {
        const auto* seat = seats.getSeatAt(i);
        if (seat && seat->getRoomId() == key) {
            positions[{seat->getRow(), seat->getColumn()}] = {QString::fromStdString(seat->getSeatId()), seat->getSeatType()};
        }
    }
    grid->replace(room->getRowCount(), room->getColumnCount(), std::move(positions));
}

void SeatMapView::openCell(int row, int column)
{
    const auto roomId = picker->currentData().toString();
    const auto key = roomId.toStdString();
    const auto* room = rooms.findRoomById(key);
    if (!room) {
        status->setText("Không tìm thấy phòng.");
        return;
    }
    const auto pos = grid->cells.find({row, column});
    const bool empty = pos == grid->cells.end();
    // TODO: BACKEND INTEGRATION — SeatService cannot check historical ticket references.
    const bool editableLayout = transactionReady && room->getShowtimeCount() == 0;
    const bool edit = editableLayout && auth.hasPermission(empty ? "SEAT_ADD" : "SEAT_UPDATE");
    const bool remove = editableLayout && !empty && auth.hasPermission("SEAT_DELETE");
    if (empty && !edit) {
        status->setText(!transactionReady ? "Dữ liệu giao dịch chưa sẵn sàng; chỉ được xem sơ đồ ghế." :
                        editableLayout ? "Bạn chỉ được xem ô trống." :
                        "Phòng đã có suất chiếu. Đang chờ Service kiểm tra vé.");
        return;
    }
    const QString id = empty ? rowName(row) + QString("%1").arg(column, 2, 10, QChar('0')) : pos->second.id;
    const auto seatId = id.toStdString();
    const auto* existing = seats.findSeatById(key, seatId);
    if ((!empty && !existing) || (empty && existing)) {
        status->setText("Mã ghế hoặc vị trí không còn phù hợp. Kiểm tra lại sơ đồ.");
        loadRoom();
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(empty ? "Thêm ghế" : "Thông tin ghế");
    QFormLayout form(&dialog);
    form.addRow("Phòng", new QLabel(roomId));
    form.addRow("Mã ghế", new QLabel(id.toHtmlEscaped()));
    form.addRow("Hàng / Cột", new QLabel(QString("%1 / %2").arg(row).arg(column)));
    QComboBox type;
    type.addItems(types);
    if (existing) {
        type.setCurrentIndex(static_cast<int>(existing->getSeatType()));
    }
    type.setEnabled(edit);
    form.addRow("Loại ghế", &type);
    if (!editableLayout) {
        form.addRow(new QLabel(!transactionReady ? "Dữ liệu giao dịch chưa sẵn sàng; không thể sửa ghế." :
                               "Chờ kiểm tra vé của phòng đã có suất chiếu."));
    }
    QDialogButtonBox buttons(QDialogButtonBox::Close);
    if (edit) {
        buttons.addButton(QDialogButtonBox::Ok);
    }
    QPushButton* deleteButton = nullptr;
    bool deleting = false;
    if (remove) {
        deleteButton = buttons.addButton("Xóa ghế", QDialogButtonBox::DestructiveRole);
        connect(deleteButton, &QPushButton::clicked, &dialog, [&] {
            if (QMessageBox::question(&dialog, "Xóa ghế", "Xóa ghế " + id + "?", QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No) == QMessageBox::Yes) {
                deleting = true;
                dialog.accept();
            }
        });
    }
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const auto* currentRoom = rooms.findRoomById(key);
    if (!transactionReady || !currentRoom || currentRoom->getShowtimeCount() > 0) {
        status->setText("Phòng đã thay đổi hoặc đã có suất chiếu. Không thể sửa sơ đồ ghế.");
        loadRoom();
        return;
    }
    const auto newType = static_cast<SeatType>(type.currentIndex());
    bool changed = false;
    if (deleting && remove) {
        changed = seats.removeSeat(key, seatId);
    } else if (empty && edit) {
        auto seat = std::make_unique<Seat>(key, seatId, row, column, newType);
        changed = seats.addSeat(seat.get());
        if (changed) {
            seat.release();
        }
    } else if (edit) {
        changed = seats.updateSeat(Seat(key, seatId, row, column, newType));
    }
    CatalogUi::result(status, changed);
    loadRoom();
}
