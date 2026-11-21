#include "MainWindow.h"
#include "ApplicationContext.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("PBL2 Cinema");
    ApplicationContext context(qEnvironmentVariable("PBL2_DATA_DIR", "data"));
    MainWindow window(context, context.load());
    window.show();
    return app.exec();
}
