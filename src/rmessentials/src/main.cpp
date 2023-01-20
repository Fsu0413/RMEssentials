#include "main.h"

#include "changename.h"
#include "download.h"
#include "papasongclientedit.h"
#include "songclientedit.h"

#include <RMEss/RmeDownloader>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextStream>
#include <QTranslator>
#include <QVBoxLayout>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Rhythm Master Essensials  ") + QStringLiteral(RMEVERSION));

    QVBoxLayout *alllayout = new QVBoxLayout;

    QPushButton *changename = new QPushButton(tr("Filename Changer"));
    connect(changename, &QPushButton::clicked, this, &MainDialog::showChangeNameDialog);
    changename->setEnabled(false);

    QPushButton *download = new QPushButton(tr("File Downloader"));
    connect(download, &QPushButton::clicked, this, &MainDialog::showDownloadDialog);

    QPushButton *songclient = new QPushButton(tr("Song Client Editor"));
    connect(songclient, &QPushButton::clicked, this, &MainDialog::showSongClientEditDialog);

    QPushButton *papasongclient = new QPushButton(tr("Papa Song Client Editor"));
    connect(papasongclient, &QPushButton::clicked, this, &MainDialog::showPapaSongClientEditDialog);
    // 2023.1.15: Papa mode is not supported temporarily in Rhythm Master Remastered
    papasongclient->setEnabled(false);

    QHBoxLayout *aboutLayout = new QHBoxLayout;
    QPushButton *abouT = new QPushButton(tr("About..."));
    connect(abouT, &QPushButton::clicked, this, &MainDialog::about);
    QPushButton *aboutQt = new QPushButton(tr("About Qt..."));
    connect(aboutQt, &QPushButton::clicked, qApp, &QApplication::aboutQt);
    aboutLayout->addWidget(abouT);
    aboutLayout->addWidget(aboutQt);

    alllayout->addWidget(changename);
    alllayout->addWidget(download);
    alllayout->addWidget(songclient);
    alllayout->addWidget(papasongclient);
    alllayout->addLayout(aboutLayout);

    setLayout(alllayout);

    setMinimumWidth(200);

#if 1
    static const QString versioninfo = QStringLiteral("https://rmessentials.fsu0413.me/versioninfo");
#else
    static const QString versioninfo = QStringLiteral("https://rmessentials.fsu0413.me/versioninfotest");
#endif
    static const QString whatsnew = QStringLiteral("https://rmessentials.fsu0413.me/whatsnew");
    static const QString dlurl = QStringLiteral("https://rmessentials.fsu0413.me/dlurl");
    static const QString dlpasswd = QStringLiteral("https://rmessentials.fsu0413.me/dlpasswd");

    RmeDownloader *downloader = new RmeDownloader;
    downloader << versioninfo << whatsnew << dlurl << dlpasswd;

    downloader->setDownloadPath(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/RMESSENTIALS"));

    connect(downloader, &RmeDownloader::allCompleted, this, &MainDialog::checkForUpdate);
    connect(downloader, &RmeDownloader::allCompleted, downloader, &RmeDownloader::deleteLater);
    connect(downloader, &RmeDownloader::canceled, downloader, &RmeDownloader::deleteLater);
    connect(this, &MainDialog::finished, downloader, &RmeDownloader::cancel);

    downloader->start();
}

void MainDialog::showChangeNameDialog()
{
    ChangeNameDialog *dl = new ChangeNameDialog;
    connect(dl, &ChangeNameDialog::finished, dl, &ChangeNameDialog::deleteLater);
#ifndef MOBILE_DEVICES
    dl->show();
#else
    dl->showMaximized();
#endif
}

void MainDialog::showDownloadDialog()
{
    DownloadDialog *dl = new DownloadDialog;
    connect(dl, &DownloadDialog::finished, dl, &DownloadDialog::deleteLater);
#ifndef MOBILE_DEVICES
    dl->show();
#else
    dl->showMaximized();
#endif
}

void MainDialog::showSongClientEditDialog()
{
    SongClientEditDialog *dl = new SongClientEditDialog;
    connect(dl, &SongClientEditDialog::finished, dl, &SongClientEditDialog::deleteLater);
#ifndef MOBILE_DEVICES
    dl->show();
#else
    dl->showMaximized();
#endif
}

void MainDialog::showPapaSongClientEditDialog()
{
    PapaSongClientEditDialog *dl = new PapaSongClientEditDialog;
    connect(dl, &PapaSongClientEditDialog::finished, dl, &PapaSongClientEditDialog::deleteLater);
#ifndef MOBILE_DEVICES
    dl->show();
#else
    dl->showMaximized();
#endif
}

void MainDialog::about()
{
    QString aboutContent = tr("The version of RMEssentials frontend is: %1.\n\n"
                              "RMEssentials is a small software written by Fsu0413 (from Wings of Melody). \n"
                              "It is used to operate the files for a game by Tencent: Rhythm Master. \n"
                              "It now contains 4 main features: ChangeName, Download, "
                              "SongClientEdit, PapaSongClientEdit. \n\n"
                              "This Program is linked against Qt %2, and loads Qt %3 to run.\n"
                              "This Program is linked against libRmEss %1, and loads libRMEss %4 to run.")
                               .arg(QStringLiteral(RMEVERSION), QStringLiteral(QT_VERSION_STR), QString::fromUtf8(qVersion()), QString::fromUtf8(RmeVersion()));
#ifdef RME_USE_QUAZIP
    aboutContent += tr("\nThis Program is linked against QuaZip %1.\n"
                       "Since Quazip does not provide a way to detect version number, "
                       "we cannot know which version we are loading when running.")
                        .arg(QStringLiteral(RME_USE_QUAZIP));
#endif
    QMessageBox::about(this, tr("About RMEssentials"), aboutContent);
}

void MainDialog::checkForUpdate()
{
#if 1
    QFile v(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/RMESSENTIALS/versioninfo"));
#else
    QFile v(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/RMESSENTIALS/versioninfotest"));
#endif
    if (v.open(QIODevice::ReadOnly)) {
        QString version = QString::fromUtf8(v.readAll());
        v.close();

        if (QStringLiteral(RMEVERSION) < version) {
            QFile n(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/RMESSENTIALS/whatsnew"));
            QString whatsnew = tr("Failed to load whatsnew from network");
            if (n.open(QIODevice::ReadOnly)) {
                whatsnew = QString::fromUtf8(n.readAll());
                n.close();
            }

            QFile l(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/RMESSENTIALS/dlurl"));
            QString link = QStringLiteral("http://pan.baidu.com/s/1eQvwPzW");
            if (l.open(QIODevice::ReadOnly)) {
                link = QString::fromUtf8(l.readAll());
                l.close();
            }

            QFile p(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/RMESSENTIALS/dlpasswd"));
            QString passwd = QStringLiteral("at7c");
            if (p.open(QIODevice::ReadOnly)) {
                passwd = QString::fromUtf8(p.readAll());
                p.close();
            }

            setWindowTitle(windowTitle() + tr("  new version %1 available").arg(version));
            if (isVisible()) {
                QString contents = tr("New version avaliable!! Version number: %1<br />"
                                      "You can download the new version at <a href=\'%2\'>here</a>, "
                                      "the password is \"%3\"<br /><br />"
                                      "What\'s new in version %1: <br /> %4")
                                       .arg(version, link, passwd, whatsnew);
                QMessageBox::information(this, tr("RMEssentials"), contents);
            }
        }
    }
}

#ifdef Q_OS_ANDROID
#define CURRENTDIRPREFIX \
    ""                   \
    "assets:/"           \
    ""
#else
#define CURRENTDIRPREFIX \
    ""                   \
    "assets/"            \
    ""
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir::setCurrent(qApp->applicationDirPath());

    QTranslator qt_translator;
    (void)qt_translator.load(QStringLiteral(CURRENTDIRPREFIX "qt_zh_CN.qm"));
    qApp->installTranslator(&qt_translator);

    QTranslator qtbase_translator;
    (void)qtbase_translator.load(QStringLiteral(CURRENTDIRPREFIX "qtbase_zh_CN.qm"));
    qApp->installTranslator(&qtbase_translator);

    QTranslator translator;
    (void)translator.load(QStringLiteral(CURRENTDIRPREFIX "rmessentials.qm"));
    qApp->installTranslator(&translator);

    MainDialog w;
#ifdef MOBILE_DEVICES
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
