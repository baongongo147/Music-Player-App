#include <QApplication>
#include "view/mainwindow.hpp"

int main(int argc, char *argv[]) {
    qputenv("QT_MULTIMEDIA_PREFERRED_PLUGINS", QByteArray("windowsmediafoundation"));
    QApplication app(argc, argv);

    MainWindow w("songs.txt");
    w.resize(600, 400);
    w.show();

    return app.exec();
}
