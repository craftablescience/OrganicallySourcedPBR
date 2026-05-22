#include <QApplication>
#include <QPushButton>
#include "src/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::setApplicationName("organically_sourced_pbr");
    QApplication::setApplicationDisplayName(QObject::tr("Organically Sourced PBR"));
    QApplication::setOrganizationName("Project Collapse Studios");
    QApplication::setOrganizationDomain("ORG");
    QApplication::setApplicationVersion("0.1.0");
    QApplication::setWindowIcon(QIcon(QPixmap(":/logo.png")));

    auto main = new CMainWindow();
    main->show();

    return QApplication::exec();
}
