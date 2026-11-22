#include "BookingView.h"
#include "ApplicationContext.h"
#include "BookingAdapter.h"
#include "CatalogView.h"
#include "PricingManagementView.h"
#include "../utils/DateTime.h"

#include <QAbstractTableModel>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <cmath>
#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <exception>

namespace {
class SeatMatrixView : public QTableView
{
public:
    QSize sizeHint() const override
    {
        const QSize headers(verticalHeader()->sizeHint().width(), horizontalHeader()->sizeHint().height());
        const QSize contents(horizontalHeader()->length(), verticalHeader()->length());
        return headers + contents + QSize(2 * frameWidth(), 2 * frameWidth());
    }
};

QString typeName(SeatType type)
{
    switch (type) {
        case SeatType::STANDARD: return "STANDARD";
        case SeatType::VIP: return "VIP";
        case SeatType::COUPLE: return "COUPLE";
    }
    return {};
}

bool upcoming(const Showtime& show)
{
    return show.getStatus() == ShowtimeStatus::SCHEDULED
        && DateTime::isBefore(DateTime::getCurrentDateTime(), show.getStartAt());
}

QTableWidget* makeTable(const QStringList& headers)
{
    auto* table = new QTableWidget(0, headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->verticalHeader()->setDefaultSectionSize(40);
    table->verticalHeader()->hide();
    table->setShowGrid(false);
    return table;
}

void writeLine(QTableWidget* table, int row, const QStringList& values)
{
    for (int i = 0; i < values.size(); ++i) {
        table->setItem(row, i, new QTableWidgetItem(values[i]));
    }
}
}

class BookingGrid : public QAbstractTableModel
{
public:
    struct Cell { QString id; SeatType kind; BookingAdapter::SeatStatus availability; };
    using Cells = std::map<std::pair<int, int>, Cell>;
    using QAbstractTableModel::QAbstractTableModel;
    Cells cells;
    QSet<QString> selected;
    int rows = 0, columns = 0;
    void replace(int height, int width, Cells physical)
    {
        beginResetModel();
        rows = height;
        columns = width;
        cells = std::move(physical);
        selected.clear();
        endResetModel();
    }
    int rowCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : rows; }
    int columnCount(const QModelIndex& parent = {}) const override { return parent.isValid() ? 0 : columns; }
    QVariant data(const QModelIndex& index, int role) const override
    {
        auto it = cells.find({index.row(), index.column()});
        if (it == cells.end()) {
            return role == Qt::DisplayRole ? QVariant("—") : QVariant();
        }
        const auto& seat = it->second;
        if (role == Qt::DisplayRole) {
            return seat.id + "\n" + typeName(seat.kind);
        }
        if (role == Qt::TextAlignmentRole) {
            return int(Qt::AlignCenter);
        }
        if (role == Qt::ToolTipRole) {
            switch (seat.availability) {
                case BookingAdapter::SeatStatus::Available:
                    return "Ghế còn trống";
                case BookingAdapter::SeatStatus::Sold:
                    return "Ghế đã bán";
                case BookingAdapter::SeatStatus::Unknown:
                    return "Chưa xác minh trạng thái ghế";
            }
        }
        if (role == Qt::BackgroundRole) {
            if (seat.availability ==
                BookingAdapter::SeatStatus::Sold) {
                return QColor("#cbd0d8");
            }

            if (seat.availability ==
                BookingAdapter::SeatStatus::Unknown) {
                return QColor("#f3d7a5");
            }

            if (selected.contains(seat.id)) {
                return QColor("#cbd6ff");
            }

            if (seat.kind == SeatType::COUPLE) {
                return QColor("#fcecf4");
            }

            if (seat.kind == SeatType::VIP) {
                return QColor("#eceaff");
            }

            return QColor("#e6eaf1");
        }
        return {};
    }
    void mark(const QString& id, bool checked)
    {
        if (checked) { selected.insert(id); } else { selected.remove(id); }
        notifySelectionChanged();
    }
    void unselectAll()
    {
        selected.clear();
        notifySelectionChanged();
    }
private:
    void notifySelectionChanged()
    {
        if (rows && columns) {
            emit dataChanged(index(0, 0), index(rows - 1, columns - 1), {Qt::BackgroundRole});
        }
    }
};

BookingView::BookingView(ApplicationContext& context)
    : ctx(context), steps(new QStackedWidget), progress(new QLabel), notice(new QLabel),
      reviewTitle(new QLabel), total(new QLabel), estimate(new QLabel), search(new QLineEdit), memberQuery(new QLineEdit),
      movies(new QComboBox), shows(new QComboBox), members(new QComboBox), grid(new BookingGrid(this)),
      tickets(makeTable({"Ghế", "Loại ghế", "Loại vé"})),
      review(makeTable({"Ghế", "Loại ghế", "Loại vé", "Giá gốc", "Giảm", "Thực trả", "Điểm"})),
      age(new QCheckBox), student(new QCheckBox), cash(new QCheckBox("Đã thu tiền tại quầy")),
      back(new QPushButton("Quay lại")), next(new QPushButton), undo(new QPushButton("Hoàn tác chọn ghế")),
      clearSeats(new QPushButton("Bỏ chọn tất cả ghế")), cancel(new QPushButton("Bỏ giao dịch"))
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* demo = new QLabel("Chọn ghế còn trống, kiểm tra báo giá và xác nhận đã thu tiền trước khi hoàn tất giao dịch.");
    demo->setObjectName("demoNotice");
    demo->setWordWrap(true);
    layout->addWidget(demo);
    progress->setObjectName("kicker");
    layout->addWidget(progress);
    layout->addWidget(steps, 1);
    auto panel = [this] {
        auto* frame = new QFrame;
        frame->setObjectName("panel");
        steps->addWidget(frame);
        auto* body = new QVBoxLayout(frame);
        body->setContentsMargins(20, 16, 20, 16);
        return body;
    };
    auto* pick = panel();
    search->setPlaceholderText("Tìm phim theo tên hoặc mã…");
    movies->setObjectName("bookingMovie");
    shows->setObjectName("bookingShow");
    pick->addWidget(new QLabel("Chọn phim"));
    pick->addWidget(search);
    pick->addWidget(movies);
    pick->addWidget(new QLabel("Suất chiếu sắp tới"));
    pick->addWidget(shows);
    pick->addStretch();

    auto* map = panel();
    auto* screen = new QLabel("M À N   H Ì N H");
    screen->setAlignment(Qt::AlignCenter);
    screen->setObjectName("cinemaScreen");
    map->addWidget(screen);
    auto* matrix = new SeatMatrixView;
    matrix->setObjectName("bookingGrid");
    matrix->setModel(grid);
    matrix->setSelectionMode(QAbstractItemView::NoSelection);
    matrix->horizontalHeader()->setDefaultSectionSize(86);
    matrix->verticalHeader()->setDefaultSectionSize(50);
    connect(grid, &QAbstractItemModel::modelReset, matrix, [matrix] { matrix->updateGeometry(); });
    map->addWidget(matrix, 1, Qt::AlignCenter);
    map->addWidget(new QLabel("—  Không có ghế     •     COUPLE: một vé cho hai người"));
    auto* selectionActions = new QHBoxLayout;
    selectionActions->addWidget(undo);
    selectionActions->addWidget(clearSeats);
    selectionActions->addStretch();
    map->addLayout(selectionActions);
    auto* details = panel();
    details->addWidget(tickets, 1);
    details->addWidget(new QLabel("Thành viên"));
    memberQuery->setPlaceholderText("Nhập mã, số điện thoại hoặc họ tên");
    members->setObjectName("bookingMember");
    members->addItem("Khách vãng lai");
    details->addWidget(memberQuery);
    details->addWidget(members);
    details->addWidget(estimate);
    auto* summary = panel();
    review->setObjectName("quoteLines");
    tickets->setObjectName("ticketTypes");
    total->setObjectName("quoteTotal");
    memberQuery->setObjectName("memberLookup");
    reviewTitle->setWordWrap(true);
    reviewTitle->setTextFormat(Qt::PlainText);
    summary->addWidget(reviewTitle);
    summary->addWidget(review, 1);
    summary->addWidget(total);
    summary->addWidget(age);
    summary->addWidget(student);
    summary->addWidget(cash);
    notice->setTextFormat(Qt::PlainText);
    notice->setWordWrap(true);
    layout->addWidget(notice);
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(cancel);
    buttons->addStretch();
    buttons->addWidget(back);
    buttons->addWidget(next);
    layout->addLayout(buttons);
    connect(search, &QLineEdit::textChanged, this, [this] { CatalogUi::run(notice, [this] { populateMovies(); }); });
    connect(movies, &QComboBox::currentIndexChanged, this, [this] { CatalogUi::run(notice, [this] { populateShows(); }); });
    connect(shows, &QComboBox::currentIndexChanged, this, [this] { CatalogUi::run(notice, [this] { loadSeats(); }); });
    connect(matrix, &QTableView::clicked, this, [this](const QModelIndex& index) {
        CatalogUi::run(notice, [&] {
            if (!validShow()) { populateShows(); setStep(0); return; }
            auto it = grid->cells.find({index.row(), index.column()});

            if (it == grid->cells.end() ||
                grid->selected.contains(it->second.id)) {
                return;
            }

            if (it->second.availability !=
                BookingAdapter::SeatStatus::Available) {
                notice->setText(
                    "Ghế này đã được bán hoặc chưa xác minh "
                    "trạng thái. Vui lòng chọn ghế khác.");
                return;
            }
            choices.append({it->second.id, it->second.kind, TicketType::REGULAR});
            grid->mark(it->second.id, true);
            resetChecks();
            renderLines();
            setStep(1);
        });
    });
    connect(clearSeats, &QPushButton::clicked, this, [this] {
        choices.clear();
        grid->unselectAll();
        resetChecks();
        renderLines();
        setStep(1);
    });
    connect(undo, &QPushButton::clicked, this, [this] {
        if (!choices.isEmpty()) {
            grid->mark(choices.takeLast().id, false);
            resetChecks();
            renderLines();
            setStep(1);
        }
    });
    connect(memberQuery, &QLineEdit::textChanged, this, [this] { CatalogUi::run(notice, [this] { lookupMember(); }); });
    connect(members, &QComboBox::currentIndexChanged, this, [this] { resetChecks(); quote(); });
    for (QCheckBox* confirmation : {age, student, cash}) {
        connect(confirmation, &QCheckBox::toggled, this, [this] {
            if (steps->currentIndex() == 3) { setStep(3); }
        });
    }
    connect(next, &QPushButton::clicked, this, [this] { CatalogUi::run(notice, [this] { advance(); }); });
    connect(back, &QPushButton::clicked, this, [this] { setStep(qMax(0, steps->currentIndex() - 1)); });
    connect(cancel, &QPushButton::clicked, this, [this] {
        QSignalBlocker blocker(search);
        search->clear();
        CatalogUi::run(notice, [this] { populateMovies(); });
    });
    setStep(0);
}

void BookingView::resetChecks()
{
    age->setChecked(false);
    student->setChecked(false);
    cash->setChecked(false);
}

void BookingView::clearChoices()
{
    choices.clear();
    grid->replace(0, 0, {});
    roomId.clear();
    QSignalBlocker querySignals(memberQuery), memberSignals(members);
    memberQuery->clear();
    members->clear();
    members->addItem("Khách vãng lai");
    tickets->setRowCount(0);
    review->setRowCount(0);
    total->clear();
    estimate->clear();
    reviewTitle->clear();
    undo->setEnabled(false);
    clearSeats->setEnabled(false);
    resetChecks();
}

void BookingView::refresh()
{
    notice->clear();
    const auto film = movies->currentData();
    const auto show = shows->currentData();
    const auto selected = choices;
    const auto member = members->currentData();
    const auto query = memberQuery->text();
    const int step = steps->currentIndex();
    const bool allowLookup = ctx.auth.hasPermission("MEMBER_VIEW") && ctx.memberError.isEmpty();
    memberQuery->setEnabled(allowLookup);
    members->setEnabled(allowLookup);
    memberQuery->setToolTip(allowLookup ? QString() : "Tra cứu thành viên chưa khả dụng.");
    CatalogUi::run(notice, [&] {
        populateMovies();
        if (movies->findData(film) <= 0) { return; }
        movies->setCurrentIndex(movies->findData(film));
        if (shows->findData(show) <= 0) { return; }
        shows->setCurrentIndex(shows->findData(show));
        if (roomId.isEmpty()) { return; }
        for (const auto& line : selected) {
            for (const auto& entry : grid->cells) {
                if (entry.second.id == line.id &&
                    entry.second.kind == line.seat &&
                    entry.second.availability == BookingAdapter::SeatStatus::Available) {
                    choices.append(line);
                    grid->mark(line.id, true);
                    break;
                }
            }
        }
        if (allowLookup) {
            memberQuery->setText(query);
            members->setCurrentIndex(qMax(0, members->findData(member)));
        }
        renderLines();
        if (step == 3 && !choices.isEmpty()) {
            setStep(2);
            advance();
        } else {
            setStep(choices.isEmpty() ? qMin(step, 1) : step);
        }
    });
}

void BookingView::populateMovies()
{
    QSignalBlocker movieSignals(movies), showSignals(shows);
    clearChoices();
    movies->clear();
    shows->clear();
    movies->addItem("Chọn phim…");
    shows->addItem("Chọn suất…");
    setStep(0);
    if (!ctx.auth.hasPermission("TICKET_SELL") || !ctx.auth.hasPermission("SHOWTIME_VIEW")
        || !ctx.auth.hasPermission("MOVIE_VIEW")) {
        notice->setText("Cần quyền bán vé, xem phim và suất chiếu.");
        return;
    }
    QSet<QString> seen;
    const int count = ctx.showtimes.getShowtimeCount();
    for (int i = 0; i < count; ++i) {
        const auto* show = ctx.showtimes.getShowtimeAt(i);
        if (!show || !upcoming(*show)) { continue; }
        const auto id = QString::fromStdString(show->getMovieId());
        if (seen.contains(id)) { continue; }
        seen.insert(id);
        const auto* movie = ctx.movies.findMovieById(show->getMovieId());
        if (!movie || !movie->isActive()) { continue; }
        const auto caption = QString::fromStdString(movie->getTitle()) + " · " + id;
        if (caption.contains(search->text().trimmed(), Qt::CaseInsensitive)) { movies->addItem(caption, id); }
    }
    notice->setText(movies->count() == 1 ? "Không có phim phù hợp với suất chiếu sắp tới." : QString());
}

void BookingView::populateShows()
{
    QSignalBlocker blocker(shows);
    clearChoices();
    shows->clear();
    shows->addItem("Chọn suất…");
    setStep(0);
    if (movies->currentIndex() <= 0) { return; }
    const auto id = movies->currentData().toString().toStdString();
    const int count = ctx.showtimes.getShowtimeCount();
    for (int i = 0; i < count; ++i) {
        const auto* show = ctx.showtimes.getShowtimeAt(i);
        if (show && show->getMovieId() == id && upcoming(*show)) {
            const auto key = QString::fromStdString(show->getShowtimeId());
            QString roomLabel = QString::fromStdString(show->getRoomId());
            if (ctx.auth.hasPermission("ROOM_VIEW")) {
                if (const auto* room = ctx.rooms.findRoomById(show->getRoomId())) {
                    roomLabel = QString::fromStdString(room->getName()) + " (" + roomLabel + ")";
                }
            }
            shows->addItem(QString::fromStdString(show->getStartAt()).replace('T', ' ') + " · " + roomLabel + " · " + key, key);
        }
    }
}

bool BookingView::validShow() const
{
    if (shows->currentIndex() <= 0 || !ctx.auth.hasPermission("TICKET_SELL")) { return false; }
    const auto filmId = movies->currentData().toString().toStdString();
    const auto* movie = ctx.movies.findMovieById(filmId);
    const auto* show = ctx.showtimes.findShowtimeById(shows->currentData().toString().toStdString());
    return movie && movie->isActive() && show && show->getMovieId() == filmId && upcoming(*show);
}


void BookingView::loadSeats()
{
    clearChoices();
    setStep(0);

    if (!validShow()) {
        return;
    }

    if (!ctx.auth.hasPermission("ROOM_VIEW") ||
        !ctx.auth.hasPermission("SEAT_VIEW")) {
        notice->setText(
            "Cần quyền xem phòng và sơ đồ ghế.");
        return;
    }

    // Không dùng dữ liệu giao dịch khi load lỗi.
    if (!ctx.catalogError.isEmpty() ||
        !ctx.transactionError.isEmpty()) {
        notice->setText(
            "Dữ liệu suất chiếu hoặc giao dịch "
            "chưa sẵn sàng.");
        return;
    }

    const auto showId =
        shows->currentData().toString().toStdString();

    const auto* show =
        ctx.showtimes.findShowtimeById(showId);

    if (!show) {
        notice->setText("Không tìm thấy suất chiếu.");
        return;
    }

    const auto* room =
        ctx.rooms.findRoomById(show->getRoomId());

    if (!room || !room->isActive()) {
        notice->setText(
            "Phòng chiếu không tồn tại "
            "hoặc đã ngừng hoạt động.");
        return;
    }

    // BookingService xác định ghế còn trống.
    const auto availability =
        BookingAdapter::loadAvailability(
            ctx.booking, showId);

    if (!availability.verified) {
        notice->setText(
            "Không thể xác minh trạng thái ghế.");
        return;
    }

    BookingGrid::Cells cells;

    const int count = ctx.seats.getSeatCount();

    for (int i = 0; i < count; ++i) {
        const auto* seat = ctx.seats.getSeatAt(i);

        if (!seat ||
            seat->getRoomId() != room->getRoomId()) {
            continue;
        }

        const auto status =
            availability.status(seat->getSeatId());

        cells[{seat->getRow() - 1,
               seat->getColumn() - 1}] = {
            QString::fromStdString(
                seat->getSeatId()),
            seat->getSeatType(),
            status
        };
    }

    roomId =
        QString::fromStdString(room->getRoomId());

    grid->replace(
        room->getRowCount(),
        room->getColumnCount(),
        std::move(cells));

    notice->clear();
    setStep(0);
}

void BookingView::lookupMember()
{
    QSignalBlocker blocker(members);

    members->clear();
    members->addItem("Khách vãng lai");

    resetChecks();

    const QString query =
        memberQuery->text().trimmed();

    if (!query.isEmpty() &&
        ctx.auth.hasPermission("MEMBER_VIEW") &&
        ctx.memberError.isEmpty()) {

        const int count =
            ctx.members.getMemberCount();

        for (int i = 0;
             i < count && members->count() <= 8;
             ++i) {

            const auto* member =
                ctx.members.getMemberAt(i);

            if (!member) {
                continue;
            }

            const QString id =
                QString::fromStdString(
                    member->getMemberId());

            const QString text =
                QString::fromStdString(
                    member->getFullName()) +
                " · " + id +
                " · " +
                QString::fromStdString(
                    member->getPhone());

            if (text.contains(
                    query,
                    Qt::CaseInsensitive)) {
                members->addItem(text, id);
            }
        }
    }

    // Tính lại sau khi danh sách đã cập nhật.
    quote();
}

void BookingView::renderLines()
{
    tickets->setRowCount(0);
    for (int row = 0; row < choices.size(); ++row) {
        tickets->insertRow(row);
        writeLine(tickets, row, {choices[row].id, typeName(choices[row].seat)});
        auto* type = new QComboBox;
        type->addItems({"REGULAR", "STUDENT"});
        type->setCurrentIndex(choices[row].ticket == TicketType::STUDENT ? 1 : 0);
        tickets->setCellWidget(row, 2, type);
        connect(type, &QComboBox::currentIndexChanged, this, [this, row](int index) {
            choices[row].ticket = index == 1 ? TicketType::STUDENT : TicketType::REGULAR;
            resetChecks();
            quote();
        });
    }
    undo->setEnabled(!choices.isEmpty());
    clearSeats->setEnabled(!choices.isEmpty());
    quote();
}

bool BookingView::quote()
{
    review->setRowCount(0);
    total->clear();
    estimate->clear();

    student->setVisible(false);

    if (choices.isEmpty()) {
        return false;
    }

    if (!validShow()) {
        notice->setText(
            "Suất chiếu không còn hợp lệ.");
        return false;
    }

    if (!ctx.pricingError.isEmpty() ||
        !ctx.memberError.isEmpty() ||
        !ctx.transactionError.isEmpty()) {
        notice->setText(
            "Dữ liệu giá, thành viên hoặc giao dịch "
            "chưa sẵn sàng.");
        return false;
    }

    const std::string showtimeId =
        shows->currentData().toString().toStdString();

    std::string memberId;

    if (members->currentIndex() > 0) {
        memberId =
            members->currentData().toString().toStdString();
    }

    std::vector<BookingItem> items;
    items.reserve(choices.size());

    bool hasStudent = false;
    bool hasStudentCouple = false;

    for (const auto& line : choices) {
        items.push_back({
            line.id.toStdString(),
            line.ticket
        });

        if (line.ticket == TicketType::STUDENT) {
            hasStudent = true;

            if (line.seat == SeatType::COUPLE) {
                hasStudentCouple = true;
            }
        }
    }

    try {
        BookingSummary summary;

        const bool success =
            ctx.booking.calculateBooking(
                showtimeId,
                items,
                memberId,
                summary
            );

        if (!success) {
            notice->setText(
                "Không thể tính giá vé. "
                "Vui lòng kiểm tra lại suất chiếu, "
                "ghế và thành viên.");

            return false;
        }

        if (summary.ticketPrices.size() !=
            items.size()) {
            notice->setText(
                "Dữ liệu báo giá không hợp lệ.");
            return false;
        }

        for (int i = 0;
             i < static_cast<int>(items.size());
             ++i) {

            const auto& line = choices[i];

            const double paid =
                summary.ticketPrices[i];

            const int row = review->rowCount();
            review->insertRow(row);

            writeLine(review, row, {
                line.id,
                typeName(line.seat),
                line.ticket == TicketType::STUDENT
                    ? "STUDENT"
                    : "REGULAR",
                "—",
                "—",
                formatVnd(paid),
                "—"
            });
        }

        total->setText(
            QString(
                "Tổng tạm tính: %1"
                "   ·   %2 điểm dự kiến"
            )
            .arg(formatVnd(summary.totalAmount))
            .arg(summary.earnedPoints)
        );

        estimate->setText(total->text());

        student->setVisible(hasStudent);

        student->setText(
            hasStudentCouple
                ? "Đã kiểm tra giấy tờ học sinh "
                  "cho từng người, gồm cả hai "
                  "người ở ghế COUPLE."
                : "Đã kiểm tra giấy tờ của từng "
                  "khách dùng vé học sinh."
        );

        notice->clear();
        return true;

    } catch (const std::exception& ex) {
        review->setRowCount(0);
        total->clear();
        estimate->clear();

        notice->setText(
            "Chưa thể tạm tính: " +
            QString::fromUtf8(ex.what())
        );

        return false;
    }
}

void BookingView::setStep(int step)
{
    steps->setCurrentIndex(step);

    const QStringList titles = {
        "1. Phim và suất chiếu",
        "2. Sơ đồ ghế",
        "3. Loại vé và thành viên",
        "4. Xem lại tại quầy"
    };

    progress->setText(titles.at(step));

    back->setEnabled(step > 0);

    next->setText(
        step == 3
            ? "Hoàn tất giao dịch"
            : "Tiếp tục →"
    );

    const bool dataReady =
        ctx.catalogError.isEmpty() &&
        ctx.memberError.isEmpty() &&
        ctx.pricingError.isEmpty() &&
        ctx.transactionError.isEmpty();

    const bool canContinue =
        step == 0
            ? !roomId.isEmpty()
            : !choices.isEmpty() &&
              (step < 3 || (age->isChecked() && cash->isChecked() &&
                            (!student->isVisible() || student->isChecked())));

    next->setEnabled(dataReady && canContinue && !bookingInProgress && !bookingStorageError);
}

void BookingView::advance()
{
    if (bookingInProgress || bookingStorageError) {
        return;
    }
    if (!ctx.catalogError.isEmpty() ||
        !ctx.memberError.isEmpty() ||
        !ctx.pricingError.isEmpty() ||
        !ctx.transactionError.isEmpty()) {
        notice->setText(
            "Dữ liệu chưa sẵn sàng để bán vé.");
        return;
    }

    if (!validShow()) {
        refresh();
        notice->setText(
            "Suất chiếu đã thay đổi hoặc bắt đầu. "
            "Vui lòng chọn lại.");
        return;
    }

    const int step = steps->currentIndex();

    if (step == 1 && choices.isEmpty()) {
        return;
    }

    // Bước 3: Kiểm tra và hiển thị báo giá.
    if (step == 2) {
        if (!ctx.auth.hasPermission("PRICING_VIEW")) {
            notice->setText(
                "Cần quyền xem giá để tiếp tục.");
            return;
        }

        if (!quote()) {
            return;
        }

        const auto* film =
            ctx.movies.findMovieById(
                movies->currentData()
                    .toString().toStdString());

        if (!film) {
            notice->setText(
                "Không tìm thấy thông tin phim.");
            return;
        }

        age->setText(
            QString(
                "Đã kiểm tra khách đủ tuổi "
                "theo phim (%1+)."
            ).arg(film->getMinAge())
        );

        reviewTitle->setText(
            movies->currentText() + "\n" +
            shows->currentText() + "\n" +
            members->currentText()
        );
    }

    if (step < 3) {
        setStep(step + 1);
        return;
    }

    // Bước 4: Hoàn tất giao dịch.
    if (step != 3) {
        return;
    }

    if (!ctx.auth.hasPermission("TICKET_SELL")) {
        notice->setText(
            "Bạn không có quyền bán vé.");
        return;
    }

    if (choices.isEmpty()) {
        notice->setText(
            "Vui lòng chọn ít nhất một ghế.");
        return;
    }

    if (!age->isChecked()) {
        notice->setText(
            "Vui lòng xác nhận đã kiểm tra "
            "độ tuổi của khách.");
        return;
    }

    bool hasStudentTicket = false;

    std::vector<BookingItem> items;
    items.reserve(choices.size());

    for (const auto& line : choices) {
        items.push_back({
            line.id.toStdString(),
            line.ticket
        });

        if (line.ticket == TicketType::STUDENT) {
            hasStudentTicket = true;
        }
    }

    if (hasStudentTicket && !student->isChecked()) {
        notice->setText(
            "Vui lòng xác nhận giấy tờ "
            "cho các vé học sinh.");
        return;
    }

    if (!cash->isChecked()) {
        notice->setText(
            "Vui lòng xác nhận đã thu tiền "
            "trước khi hoàn tất giao dịch.");
        return;
    }

    const std::string showtimeId =
        shows->currentData()
            .toString().toStdString();

    std::string memberId;

    if (members->currentIndex() > 0) {
        memberId =
            members->currentData()
                .toString().toStdString();
    }

    // Kiểm tra lại báo giá ngay trước khi bán.
    if (!quote()) {
        return;
    }

    // Tránh nhấn nút nhiều lần khi xử lý.
    bookingInProgress = true;
    next->setEnabled(false);

    try {
        std::string invoiceId;

        const bool success =
            ctx.booking.createBooking(
                showtimeId,
                items,
                memberId,
                age->isChecked(),
                cash->isChecked(),
                invoiceId
            );

        if (!success || invoiceId.empty()) {
            bookingInProgress = false;
            notice->setText(
                "Không thể hoàn tất giao dịch. "
                "Vui lòng kiểm tra lại thông tin.");

            setStep(3);
            return;
        }

        bookingInProgress = false;
        // Giao dịch đã hoàn thành.
        // Xóa lựa chọn cũ và tải lại danh sách.
        populateMovies();
        setStep(0);

        notice->setText(
            "Bán vé thành công. "
            "Mã hóa đơn: " +
            QString::fromStdString(invoiceId)
        );

    } catch (const std::exception& ex) {
        bookingInProgress = false;
        bookingStorageError = true;
        notice->setText(
            "Giao dịch gặp lỗi: " +
            QString::fromUtf8(ex.what()) +
            "\nNếu có lỗi ghi dữ liệu, "
            "không thực hiện lại giao dịch "
            "trước khi kiểm tra hóa đơn và vé."
        );

        // Không tự thử lại giao dịch.
        next->setEnabled(false);
    } catch (...) {
        bookingInProgress = false;
        bookingStorageError = true;
        notice->setText("Giao dịch gặp lỗi không xác định. Kiểm tra hóa đơn và vé trước khi thử lại.");
        next->setEnabled(false);
    }
}
