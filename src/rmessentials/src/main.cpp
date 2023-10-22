#include "main.h"

#include "changename.h"
#include "download.h"
#include "papasongclientedit.h"
#include "songclientedit.h"

#include <RmEss/RmeChart>
#include <RmEss/RmeDownloader>

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QStringList>
#include <QStyleFactory>
#include <QTextStream>
#include <QTranslator>
#include <QVBoxLayout>

#ifdef Q_OS_ANDROID
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAndroidJniObject>
#include <QtAndroidExtras>
#elif QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#include <QJniObject>
#include <QtCore/private/qandroidextras_p.h>
#endif
#endif

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
#ifdef Q_OS_ANDROID
    , m_receiver(nullptr)
#endif
    , m_isPermissionOk(false)
{
    setWindowTitle(tr("Rhythm Master Essensials  ") + QStringLiteral(RMEVERSION));

    QVBoxLayout *alllayout = new QVBoxLayout;
    m_changeNameBtn = new QPushButton(tr("Filename Changer"));
    connect(m_changeNameBtn, &QPushButton::clicked, this, &MainDialog::showChangeNameDialog);
    m_changeNameBtn->setEnabled(false);

    m_downloadBtn = new QPushButton(tr("File Downloader"));
    connect(m_downloadBtn, &QPushButton::clicked, this, &MainDialog::showDownloadDialog);
    m_downloadBtn->setEnabled(false);

    m_songEditorBtn = new QPushButton(tr("Song Client Editor"));
    connect(m_songEditorBtn, &QPushButton::clicked, this, &MainDialog::showSongClientEditDialog);
    m_songEditorBtn->setEnabled(false);

    m_papaSongEditorBtn = new QPushButton(tr("Papa Song Client Editor"));
    connect(m_papaSongEditorBtn, &QPushButton::clicked, this, &MainDialog::showPapaSongClientEditDialog);
    m_papaSongEditorBtn->setEnabled(false);
    m_papaSongEditorBtn->setVisible(false);

    QMenu *popup = new QMenu(this);
    QAction *aboutThis = popup->addAction(tr("About RMEssentials..."));
    connect(aboutThis, &QAction::triggered, this, &MainDialog::about);
    popup->addSeparator();
    QAction *aboutQt = popup->addAction(tr("About Qt..."));
    connect(aboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
#ifdef RME_USE_QUAZIP
    QAction *aboutQuaZip = popup->addAction(tr("About QuaZip..."));
    connect(aboutQuaZip, &QAction::triggered, this, &MainDialog::aboutQuaZip);
#endif

    QPushButton *aboutBtn = new QPushButton(tr("About"));
    aboutBtn->setAutoDefault(false);
    aboutBtn->setDefault(false);
    aboutBtn->setMenu(popup);

    alllayout->addWidget(m_changeNameBtn);
    alllayout->addWidget(m_downloadBtn);
    alllayout->addWidget(m_songEditorBtn);
    alllayout->addWidget(m_papaSongEditorBtn);
    alllayout->addWidget(aboutBtn);

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

    connect(downloader, &RmeDownloader::allCompleted, this, &MainDialog::metainfoDownloaded);
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
    aboutContent += tr("\n\nThis Program is linked against QuaZip %1.").arg(QStringLiteral(RME_USE_QUAZIP));
#endif
    QMessageBox::about(this, tr("About RMEssentials"), aboutContent);
}

#ifdef RME_USE_QUAZIP
void MainDialog::aboutQuaZip()
{
    QString aboutContent = tr("This program is linked against QuaZip %1, which is licensed under LGPL 2.1.<br />"
                              "QuaZip GitHub page: <a href=\"%2\">%2</a>")
                               .arg(QStringLiteral(RME_USE_QUAZIP), QStringLiteral("https://github.com/stachenov/quazip"));

    QMessageBox::about(this, tr("About QuaZip"), aboutContent);
}
#endif

void MainDialog::metainfoDownloaded()
{
    // checkForUpdate
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

void MainDialog::permissionCheckOk()
{
    m_isPermissionOk = true;
    enableButtons();
}

void MainDialog::enableButtons()
{
    // ChangeName Dialog and Papa song editor Dialog are currently temporarily disabled due to technical restriction of Rhythm Master Remastered

    m_changeNameBtn->setEnabled(m_isPermissionOk);
    m_downloadBtn->setEnabled(m_isPermissionOk);
    m_songEditorBtn->setEnabled(m_isPermissionOk);
    m_papaSongEditorBtn->setEnabled(false);
}

bool MainDialog::checkPermission()
{
    QString s = RmeDownloader::binDownloadPath();
    bool flag = false;
    if (!s.isEmpty()) {
        QDir d(s);
        if (d.exists()) {
            QFile f(d.absoluteFilePath(QStringLiteral("test.rmessentials")));
            if (f.open(QFile::WriteOnly | QFile::Truncate)) {
                f.write(QByteArray("hello RMEssentials"));
                f.close();

                if (f.open(QFile::ReadOnly)) {
                    QByteArray arr = f.readAll();
                    f.close();
                    if (arr == QByteArray("hello RMEssentials")) {
                        d.remove(QStringLiteral("test.rmessentials"));
                        flag = true;
                    }
                }
            }
        }
    }

    return flag;
}

#ifdef Q_OS_ANDROID

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace QtAndroidPrivate {
using namespace QtAndroid;
}
using QJniObject = QAndroidJniObject;
#endif

class MainDialog::AndroidResultReceiver : public QAndroidActivityResultReceiver
{
public:
    AndroidResultReceiver(MainDialog *dialog)
        : m_dialog(dialog)
    {
    }
    ~AndroidResultReceiver() override = default;

public:
    void handleActivityResult(int, int, const QJniObject &) override
    {
        m_dialog->permissionRequestCallback();
    }

private:
    MainDialog *m_dialog;
};

void MainDialog::requestForLegacyPermission()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QtAndroid::PermissionResult result = QtAndroid::checkPermission(QStringLiteral("android.permission.READ_EXTERNAL_STORAGE"));
    if (result != QtAndroid::PermissionResult::Granted) {
        QtAndroid::PermissionResultMap m
            = QtAndroid::requestPermissionsSync({QStringLiteral("android.permission.READ_EXTERNAL_STORAGE"), QStringLiteral("android.permission.WRITE_EXTERNAL_STORAGE")});
        foreach (QtAndroid::PermissionResult r, m) {
            if (r != QtAndroid::PermissionResult::Granted)
                break;
        }
    }
#else
    QFuture<QtAndroidPrivate::PermissionResult> future = QtAndroidPrivate::checkPermission(QtAndroidPrivate::Storage);
    future.waitForFinished();

    if (future.result() != QtAndroidPrivate::Authorized) {
        QFuture<QtAndroidPrivate::PermissionResult> future2 = QtAndroidPrivate::requestPermission(QtAndroidPrivate::Storage);
        future2.waitForFinished();
    }
#endif

    legacyPermissionRequestCallback();
}

void MainDialog::legacyPermissionRequestCallback()
{
    if (checkPermission())
        permissionCheckOk();
    else if (QtAndroidPrivate::androidSdkVersion() >= 30)
        requestForPermission();
    else
        QMessageBox::warning(this, tr("RMEssentials"),
                             tr("External storage permission not granted. "
                                "RMEssentials won't work without this permission. "
                                "Please check your permission setting."),
                             QMessageBox::Ok);
}

void MainDialog::requestForPermission()
{
    if (m_receiver == nullptr)
        m_receiver = new AndroidResultReceiver(this);

    QMessageBox::information(this, tr("RMEssentials"),
                             tr("Please find RMEssentials in following UI and grant external storage permission. "
                                "RMEssentials won't work without it."),
                             QMessageBox::Ok);

    QAndroidIntent indent(QStringLiteral("android.settings.MANAGE_ALL_FILES_ACCESS_PERMISSION"));
    QtAndroidPrivate::startActivity(indent, 0, m_receiver);
}

void MainDialog::permissionRequestCallback()
{
    if (checkPermission())
        permissionCheckOk();
    else
        QMessageBox::warning(this, tr("RMEssentials"),
                             tr("External storage permission not granted. "
                                "RMEssentials won't work without this permission. "
                                "Please check your permission setting."),
                             QMessageBox::Ok);
}
#endif

MainDialog::~MainDialog()
{
#ifdef Q_OS_ANDROID
    delete m_receiver;
#endif
}

void MainDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (checkPermission())
        permissionCheckOk();
#ifdef Q_OS_ANDROID
    else
        requestForLegacyPermission();
#endif
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
#ifdef Q_OS_MACOS
    // macOS style sucks. It doesn't expand QLineEdit by default.
    // I tried to set QSizePolicy but it doesn't work.
    // so let's use Fusion style on macOS.

    if (QStyleFactory::keys().contains(QStringLiteral("Fusion"), Qt::CaseInsensitive))
        QApplication::setStyle(QStringLiteral("Fusion"));
#endif

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

QString currentNum()
{
    return QStringLiteral("554");
}
