//Sistema Operativo = Windows 10
//IDE = Visual Studio Community Edition 2019
//compilador = Microsoft C++ Compiler (MSVC)
//lenguaje = C++17

#include <clocale>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include "gui_main.h"
#include "global.h"

int main(int argc, char **argv){
    QFile temaOscuro(":qdarkstyle/style.qss");
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    /* tema oscuro */
    if (temaOscuro.exists()){
        temaOscuro.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&temaOscuro);
        a.setStyleSheet(ts.readAll());
    }

    a.setApplicationName("backtracking search optimization over set covering problem");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("Jean Rodriguez H.");
    Gui_Main w;
    w.setWindowTitle("backtracking search optimization");
    w.show();
    return a.exec();
}
