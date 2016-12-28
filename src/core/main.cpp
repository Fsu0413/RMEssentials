#include "maindialog.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QTranslator>

#ifdef Q_OS_OSX
#include <QDir>
#endif

#ifdef Q_OS_ANDROID
#define CURRENTDIRPREFIX ""         \
                         "assets:/" \
                         ""
#else
#define CURRENTDIRPREFIX ""        \
                         "assets/" \
                         ""
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef Q_OS_OSX
#ifdef QT_NO_DEBUG
    QDir::setCurrent(qApp->applicationDirPath());
#endif
#endif

    QTranslator qt_translator;
    qt_translator.load(QStringLiteral(CURRENTDIRPREFIX "qt_zh_CN.qm"));
    qApp->installTranslator(&qt_translator);

    QTranslator translator;
    translator.load(QStringLiteral(CURRENTDIRPREFIX "changename.qm"));
    qApp->installTranslator(&translator);

#ifdef MOBILE_DEVICES
    QFile qss(QStringLiteral(CURRENTDIRPREFIX "RMEssentialsAndroid.qss"));
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
