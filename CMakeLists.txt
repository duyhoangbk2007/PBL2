cmake_minimum_required(VERSION 3.21)
project(PBL2Cinema LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(pbl2_auth STATIC
    src/business/MovieService.cpp
    src/business/RoomService.cpp
    src/business/SeatService.cpp
    src/data_access/MovieRepository.cpp
    src/data_access/RoomRepository.cpp
    src/data_access/SeatRepository.cpp
    src/data_access/ShowtimeRepository.cpp
    src/utils/DateTime.cpp
    src/business/AuthService.cpp
    src/data_access/UserRepository.cpp
    src/data_access/RolePermissionRepository.cpp
    src/utils/FileIO.cpp
    src/utils/Validator.cpp
    src/models/User.cpp
    src/models/Invoice.cpp
    src/models/Member.cpp
    src/models/Movie.cpp
    src/models/Room.cpp
    src/models/Seat.cpp
    src/models/Showtime.cpp
    src/models/Ticket.cpp
)

set(gui_sources
    src/gui/CatalogView.cpp
    src/gui/MovieManagementView.cpp
    src/gui/RoomManagementView.cpp
    src/gui/SeatMapView.cpp
    src/gui/ApplicationContext.cpp
    src/gui/LoginView.cpp
    src/gui/LoginView.h
    src/gui/MainWindow.cpp
    src/gui/MainWindow.h
    src/gui/resources/gui.qrc
)
qt_add_executable(pbl2_gui WIN32 src/gui/main.cpp ${gui_sources})
target_link_libraries(pbl2_gui PRIVATE pbl2_auth Qt6::Widgets)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(test_gui_navigation tests/GUI/test_navigation.cpp tests/GUI/test_catalog.cpp ${gui_sources})
    target_include_directories(test_gui_navigation PRIVATE src/gui)
    target_link_libraries(test_gui_navigation PRIVATE pbl2_auth Qt6::Widgets Qt6::Test)
    target_compile_definitions(test_gui_navigation PRIVATE PBL2_GUI_FIXTURES="${CMAKE_CURRENT_SOURCE_DIR}/tests/GUI/data" PBL2_PERMISSIONS_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/role_permissions.txt")
    add_test(NAME gui_navigation COMMAND test_gui_navigation -platform offscreen)
endif()
