#include "maindialog.h"
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qt_translator;
    qt_translator.load(":/qt_zh_CN.qm");
    qApp->installTranslator(&qt_translator);

    QTranslator translator;
    translator.load(":/changename.qm");
    qApp->installTranslator(&translator);

    MainDialog w;
#ifdef Q_OS_ANDROID
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
