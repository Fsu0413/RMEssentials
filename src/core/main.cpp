#include "maindialog.h"
#include <QTranslator>
#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qt_translator;
    qt_translator.load(":/qt_zh_CN.qm");
    qApp->installTranslator(&qt_translator);

    QTranslator translator;
    translator.load(":/changename.qm");
    qApp->installTranslator(&translator);

#ifdef MOBILE_DEVICES
    QFile qss(":/RMEssentialsAndroid.qss");
    if (qss.open(QIODevice::ReadOnly)) {
        QTextStream qssStream(&qss);
        qApp->setStyleSheet(qssStream.readAll());
        qss.close();
    }
#endif

    MainDialog w;
#ifdef MOBILE_DEVICES
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
