#include "MainWindow.h"
#include "ApplicationContext.h"

#include <QApplication>
#include <QDir>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("PBL2 Cinema");
    const QString directory = qEnvironmentVariable("PBL2_DATA_DIR", "data");
    // Qt selects the Unicode directory. The backend can then open ASCII filenames
    // without converting a Windows path through the system ANSI code page.
    const bool located = !directory.isEmpty() && QDir::setCurrent(directory);
    ApplicationContext context(".");
    MainWindow window(context, located ? context.load()
                                       : QString("Không mở được thư mục dữ liệu: %1").arg(directory));
    window.show();
    return app.exec();
}
