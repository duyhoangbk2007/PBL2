#include "MainWindow.h"
#include "LoginView.h"
#include "ApplicationContext.h"
#include "MovieManagementView.h"
#include "RoomManagementView.h"
#include "SeatMapView.h"
#include "BookingView.h"
#include "InvoiceHistoryView.h"
#include "TicketCancellationView.h"
#include "ReportView.h"
#include "PricingManagementView.h"
#include "ShowtimeManagementView.h"
#include "StaffManagementView.h"
#include "MemberManagementView.h"
#include "../business/AuthService.h"

#include <QButtonGroup>
#include <QFile>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
struct Page {
    QString permission;
    QString group;
    QString icon;
    QString title;
    QString description;
    QString panel;
    QStringList columns;
};

const Page navigation[] = {
    {"MOVIE_VIEW", "DANH MỤC NGHIỆP VỤ", "▤", "Quản lý phim", "Thông tin phim, lịch phát hành và trạng thái hoạt động.", "Danh sách phim", {"Mã phim", "Phim / Thể loại", "Thời lượng", "Độ tuổi", "Khoảng chiếu", "Trạng thái"}},
    {"ROOM_VIEW", "DANH MỤC NGHIỆP VỤ", "▣", "Quản lý phòng chiếu", "Danh mục phòng, kích thước và trạng thái.", "Danh sách phòng", {"Mã phòng", "Phòng chiếu", "Kích thước", "Trạng thái"}},
    {"SEAT_VIEW", "DANH MỤC NGHIỆP VỤ", "▦", "Quản lý ghế", "Chọn phòng, sau đó thao tác trực tiếp trên sơ đồ ghế.", "Sơ đồ ghế theo phòng", {}},
    {"SHOWTIME_VIEW", "DANH MỤC NGHIỆP VỤ", "◷", "Quản lý suất chiếu", "Danh sách toàn rạp, lọc theo phim, phòng, ngày và trạng thái.", "Danh sách suất chiếu", {}},
    {"STAFF_VIEW", "DANH MỤC NGHIỆP VỤ", "♙", "Quản lý nhân viên", "Quản lý tài khoản và trạng thái nhân viên.", "Danh sách nhân viên", {"Mã NV", "Username", "Họ tên", "Trạng thái"}},
    {"MEMBER_VIEW", "DANH MỤC NGHIỆP VỤ", "♧", "Quản lý thành viên", "Tra cứu, thêm và cập nhật thông tin liên hệ thành viên.", "Danh sách thành viên", {"Mã TV", "Họ tên", "Số điện thoại", "Điểm hiện có"}},
    {"TICKET_SELL", "VẬN HÀNH TẠI QUẦY", "▧", "Bán vé tại quầy", "Chọn phim, suất chiếu, ghế, loại vé, thành viên và xác nhận thu tiền.", "Chọn phim và suất chiếu", {}},
    {"INVOICE_VIEW_OWN", "VẬN HÀNH TẠI QUẦY", "▤", "Lịch sử hóa đơn", "Tra cứu hóa đơn và thông tin vé trong giao dịch.", "Tra cứu hóa đơn", {}},
    {"TICKET_CANCEL", "VẬN HÀNH TẠI QUẦY", "↶", "Hủy vé & hoàn tiền", "Hủy từng vé hợp lệ, giữ lịch sử và ghi nhận hoàn tiền.", "Tra cứu vé", {}},
    {"PRICING_VIEW", "CẤU HÌNH", "₫", "Giá vé & ưu đãi", "Cấu hình giá ghế, ưu đãi học sinh và thành viên.", "Giá vé theo loại ghế", {}},
    {"REPORT_VIEW", "PHÂN TÍCH", "▥", "Báo cáo & thống kê", "Doanh thu, tiền hoàn, số vé và xếp hạng phim.", "Tổng quan vận hành", {}}
};

bool canOpen(const AuthService& auth, const QString& permission)
{
    if (permission == "TICKET_CANCEL" && !auth.isAdmin()) {
        return false;
    }
    if (auth.hasPermission(permission.toStdString())) {
        return true;
    }
    return permission == "INVOICE_VIEW_OWN"
        && auth.hasPermission("INVOICE_VIEW_ALL");
}

QLabel* label(const QString& text, const QString& style)
{
    auto* result = new QLabel(text);
    result->setTextFormat(Qt::PlainText);
    result->setObjectName(style);
    return result;
}

QWidget* createPage(const Page& page, QWidget* content)
{
    auto* view = new QWidget;
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(38, 35, 38, 32);
    layout->setSpacing(8);
    layout->addWidget(label(page.group, "kicker"));
    layout->addWidget(label(page.title, "pageTitle"));
    auto* description = label(page.description, "description");
    description->setWordWrap(true);
    layout->addWidget(description);
    layout->addSpacing(17);

    if (content) {
        layout->addWidget(content, 1);
        auto* scroll = new QScrollArea;
        scroll->setObjectName("pageScroll");
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setWidgetResizable(true);
        scroll->setWidget(view);
        scroll->viewport()->setAutoFillBackground(false);
        view->setAutoFillBackground(false);
        return scroll;
    }
    auto* panel = new QFrame;
    panel->setObjectName("panel");
    auto* body = new QVBoxLayout(panel);
    body->setContentsMargins(1, 1, 1, 1);
    body->setSpacing(0);
    body->addWidget(label(page.panel, "panelHeading"));
    if (!page.columns.isEmpty()) {
        auto* table = new QTableWidget(0, page.columns.size());
        table->setHorizontalHeaderLabels(page.columns);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->hide();
        table->setFocusPolicy(Qt::NoFocus);
        table->setFrameShape(QFrame::NoFrame);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->horizontalHeader()->setFixedHeight(40);
        table->setFixedHeight(40);
        body->addWidget(table);
    }
    auto* empty = label("Chức năng đang chờ tích hợp.", "empty");
    empty->setAlignment(Qt::AlignCenter);
    empty->setMinimumHeight(140);
    body->addWidget(empty);
    layout->addWidget(panel, 0, Qt::AlignTop);
    layout->addStretch();
    return view;
}
}

MainWindow::MainWindow(ApplicationContext& context, const QString& startupError, QWidget* parent)
    : QMainWindow(parent), context(context), auth(context.auth), login(new LoginView(auth, startupError)),
      screens(new QStackedWidget)
{
    setWindowTitle("PBL2 Cinema");
    resize(1280, 820);
    QFile theme(":/gui/theme.qss");
    if (theme.open(QIODevice::ReadOnly)) {
        setStyleSheet(QString::fromUtf8(theme.readAll()));
    }

    setCentralWidget(screens);
    screens->addWidget(login);
    connect(login, &LoginView::authenticated, this, &MainWindow::openSession);
}

void MainWindow::openSession()
{
    if (!auth.getCurrentUser() || screens->currentWidget() != login) {
        return;
    }
    pages = new QStackedWidget;
    pages->setObjectName("pages");
    currentTitle = label({}, "crumbTitle");
    currentTitle->setWordWrap(true);
    auto* shell = new QWidget;
    shell->setObjectName("shell");
    screens->addWidget(shell);
    auto* row = new QHBoxLayout(shell);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(0);
    row->addWidget(createSidebar());
    auto* main = new QVBoxLayout;
    main->setSpacing(0);
    main->addWidget(createHeader());
    main->addWidget(pages, 1);
    row->addLayout(main, 1);
    screens->setCurrentWidget(shell);
}

void MainWindow::closeSession()
{
    auth.logout();
    QWidget* shell = screens->currentWidget();
    shell->setEnabled(false);
    screens->setCurrentWidget(login);
    screens->removeWidget(shell);
    shell->deleteLater();
    pages = nullptr;
    currentTitle = nullptr;
    login->reset();
}

QWidget* MainWindow::createSidebar()
{
    auto* sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(246);
    auto* layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(12, 26, 12, 16);
    layout->setSpacing(0);

    auto* brand = new QHBoxLayout;
    brand->setContentsMargins(12, 0, 0, 0);
    auto* mark = label("▣", "logo");
    mark->setFixedSize(38, 38);
    mark->setAlignment(Qt::AlignCenter);
    brand->addWidget(mark);
    brand->addSpacing(10);
    auto* brandText = new QVBoxLayout;
    brandText->addWidget(label("PBL2 Cinema", "brand"));
    brandText->addWidget(label("MANAGEMENT", "brandCaption"));
    brand->addLayout(brandText, 1);
    layout->addLayout(brand);
    layout->addSpacing(16);

    auto* scroll = new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setWidgetResizable(true);
    auto* menu = new QWidget;
    auto* items = new QVBoxLayout(menu);
    items->setContentsMargins(0, 0, 2, 0);
    items->setSpacing(4);
    auto* buttons = new QButtonGroup(sidebar);
    QString group;
    int landing = 0;
    auto navigate = [this, buttons](const QString& permission) -> QWidget* {
        for (auto* target : buttons->buttons()) {
            if (target->property("permission").toString() == permission && canOpen(auth, permission)) {
                target->click();
                return pages->currentWidget();
            }
        }
        return nullptr;
    };
    for (const auto& page : navigation) {
        if (!canOpen(auth, page.permission)) {
            continue;
        }
        if (group != page.group) {
            group = page.group;
            items->addSpacing(9);
            items->addWidget(label(group, "group"));
        }
        auto* button = new QPushButton;
        button->setObjectName("nav");
        button->setAccessibleName(page.title);
        button->setToolTip(page.title);
        button->setCheckable(true);
        auto* line = new QHBoxLayout(button);
        line->setContentsMargins(12, 0, 6, 0);
        line->setSpacing(10);
        auto* icon = label(page.icon, "navIcon");
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedWidth(21);
        line->addWidget(icon);
        line->addWidget(label(page.title, "navTitle"), 1);
        items->addWidget(button);
        QWidget* content = nullptr;
        const bool catalog = page.permission == "MOVIE_VIEW" || page.permission == "ROOM_VIEW" || page.permission == "SEAT_VIEW" || page.permission == "SHOWTIME_VIEW";
        const bool transactionPage =
            page.permission == "TICKET_SELL" ||
            page.permission == "INVOICE_VIEW_OWN" ||
            page.permission == "TICKET_CANCEL" ||
            page.permission == "REPORT_VIEW";
        if (catalog && !context.catalogError.isEmpty()) {
            content = label(context.catalogError, "description");
        } else if (transactionPage &&
                !context.catalogError.isEmpty()) {
            content = label(context.catalogError, "description");
        } else if (page.permission == "TICKET_SELL" &&
                !context.memberError.isEmpty()) {
            content = label(context.memberError, "description");
        } else if (page.permission == "TICKET_SELL" &&
                !context.transactionError.isEmpty()) {
            content = label(context.transactionError, "description");
        } else if (page.permission == "TICKET_SELL" &&
                !context.pricingError.isEmpty()) {
            content = label(context.pricingError, "description");
        } else if (page.permission == "MOVIE_VIEW") {
            auto* movies = new MovieManagementView(context.movies, auth);
            content = movies;
            connect(movies, &CatalogView::openRelated, this, [navigate](const QString& id) {
                if (auto* target = navigate("SHOWTIME_VIEW")) {
                    if (auto* shows = target->findChild<ShowtimeManagementView*>()) {
                        shows->refresh(id);
                    }
                }
            });
        } else if (page.permission == "PRICING_VIEW") {
            if (context.pricingError.isEmpty()) {
                content = new PricingManagementView(context.pricing, auth);
            } else {
                content = label(context.pricingError, "description");
            }
        } else if (page.permission == "INVOICE_VIEW_OWN") {
            content = new InvoiceHistoryView(context.invoices, auth);
        } else if (page.permission == "TICKET_CANCEL") {
            content = new TicketCancellationView(context.booking, context.invoices, auth);
        } else if (page.permission == "REPORT_VIEW") {
            content = new ReportView(context.reports, auth);
        } else if (page.permission == "TICKET_SELL") {
            if (context.catalogError.isEmpty()) {
                content = new BookingView(context);
            } else {
                content = label(context.catalogError, "description");
            }
        } else if (page.permission == "SHOWTIME_VIEW") {
            content = new ShowtimeManagementView(context.showtimes, context.movies, context.rooms, auth);
        } else if (page.permission == "STAFF_VIEW") {
            content = new StaffManagementView(context.staff, auth);
        } else if (page.permission == "MEMBER_VIEW") {
            if (context.memberError.isEmpty()) {
                content = new MemberManagementView(context.members, auth);
            } else {
                content = label(context.memberError, "description");
            }
        } else if (page.permission == "ROOM_VIEW") {
            auto* rooms = new RoomManagementView(context.rooms, auth);
            content = rooms;
            connect(rooms, &CatalogView::openRelated, this, [navigate](const QString& id) {
                if (auto* target = navigate("SEAT_VIEW")) {
                    if (auto* map = target->findChild<SeatMapView*>("seatMap")) {
                        map->refresh(id);
                    }
                }
            });
        } else if (page.permission == "SEAT_VIEW") {
            auto* map = new SeatMapView(context.rooms, context.seats, auth);
            map->setObjectName("seatMap");
            content = map;
        }
        const int index = pages->addWidget(createPage(page, content));
        buttons->addButton(button, index);
        button->setProperty("permission", page.permission);
        if (auth.isStaff() && page.permission == "TICKET_SELL") {
            landing = index;
        }
    }
    items->addStretch();
    scroll->setWidget(menu);
    layout->addWidget(scroll, 1);
    if (buttons->buttons().isEmpty()) {
        pages->addWidget(label("Tài khoản chưa được cấp quyền truy cập màn hình nào.", "empty"));
        currentTitle->setText("Quyền truy cập");
    } else {
        connect(buttons, &QButtonGroup::idClicked, this, [this, buttons](int index) {
            auto* button = buttons->button(index);
            const auto permission = button->property("permission").toString();
            if (canOpen(auth, permission)) {
                pages->setCurrentIndex(index);
                currentTitle->setText(button->accessibleName());
                auto* current = pages->currentWidget();
                if (permission == "PRICING_VIEW") {
                    if (auto* prices = current->findChild<PricingManagementView*>()) {
                        prices->refresh();
                    }
                } else if (permission == "TICKET_SELL") {
                    if (auto* booking = current->findChild<BookingView*>()) {
                        booking->refresh();
                    }
                } else if (permission == "SEAT_VIEW") {
                    if (auto* map = current->findChild<SeatMapView*>("seatMap")) {
                        map->refresh();
                    }
                } else if (permission == "SHOWTIME_VIEW") {
                    if (auto* shows = current->findChild<ShowtimeManagementView*>()) {
                        shows->refresh();
                    }
                } else if (permission == "STAFF_VIEW") {
                    current->findChild<StaffManagementView*>()->refresh();
                } else if (permission == "MEMBER_VIEW") {
                    if (auto* members = current->findChild<MemberManagementView*>()) {
                        members->refresh();
                    }
                }
            }
        });
        buttons->button(landing)->click();
    }
    return sidebar;
}

QWidget* MainWindow::createHeader()
{
    auto* header = new QFrame;
    header->setObjectName("header");
    header->setFixedHeight(78);
    auto* row = new QHBoxLayout(header);
    row->setContentsMargins(22, 0, 22, 0);
    row->setSpacing(10);
    row->addWidget(label("PBL2 Cinema  /", "crumb"));
    row->addWidget(currentTitle);
    row->addStretch();
    auto* account = new QVBoxLayout;
    account->setAlignment(Qt::AlignVCenter);
    account->setSpacing(1);
    const User* user = auth.getCurrentUser();
    if (user) {
        auto* name = label(QString::fromStdString(user->getFullName()), "account");
        name->setWordWrap(true);
        name->setToolTip(name->text());
        account->addWidget(name);
        const QString role = auth.isAdmin() ? "ADMIN" : "STAFF";
        auto* identity = label(QString::fromStdString(user->getUsername()) + " · " + role, "accountStatus");
        identity->setWordWrap(true);
        account->addWidget(identity);
    }
    row->addLayout(account);
    auto* avatar = label(auth.isAdmin() ? "A" : "S", "avatar");
    avatar->setFixedSize(36, 36);
    avatar->setAlignment(Qt::AlignCenter);
    row->addWidget(avatar);
    auto* logout = new QPushButton("Đăng xuất");
    logout->setObjectName("logout");
    connect(logout, &QPushButton::clicked, this, &MainWindow::closeSession);
    row->addWidget(logout);
    return header;
}
