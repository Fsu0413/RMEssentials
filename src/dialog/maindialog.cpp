#include "maindialog.h"
#include "ChangeNameDialog.h"
#include "DownloadDialog.h"
#include "SongClientEditDialog.h"
#include "PapaSongClientEditDialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include "downloader.h"

const char *const programVersion = "20150928";

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Rhythm Master Essensials  ") + QString(programVersion));

    QVBoxLayout *alllayout = new QVBoxLayout;

    QPushButton *changename = new QPushButton(tr("Filename Changer"));
    connect(changename, &QPushButton::clicked, this, &MainDialog::showChangeNameDialog);

    QPushButton *download = new QPushButton(tr("File Downloader"));
    connect(download, &QPushButton::clicked, this, &MainDialog::showDownloadDialog);

    QPushButton *songclient = new QPushButton(tr("Song Client Editor"));
    connect(songclient, &QPushButton::clicked, this, &MainDialog::showSongClientEditDialog);

    QPushButton *papasongclient = new QPushButton(tr("Papa Song Client Editor"));
    connect(papasongclient, &QPushButton::clicked, this, &MainDialog::showPapaSongClientEditDialog);

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

#ifdef QT_NO_DEBUG
    static const QString versioninfo = "http://fsu0413.github.io/RMEssentials/versioninfo";
#else
    static const QString versioninfo = "http://fsu0413.github.io/RMEssentials/versioninfotest";
#endif

    Downloader *downloader = new Downloader;
    downloader << versioninfo;

    downloader->setSavePath(QString());

    connect(downloader, &Downloader::finished, downloader, &Downloader::deleteLater);
    connect(downloader, &Downloader::one_completed, this, &MainDialog::checkForUpdate);
    connect(this, &MainDialog::finished, downloader, &Downloader::cancel);

    downloader->start();
}

void MainDialog::showChangeNameDialog()
{
    ChangeNameDialog *dl = new ChangeNameDialog;
    connect(dl, &ChangeNameDialog::finished, dl, &ChangeNameDialog::deleteLater);
#ifndef Q_OS_ANDROID
    dl->show();
#else
    dl->showMaximized();
#endif
}

void MainDialog::showDownloadDialog()
{
    DownloadDialog *dl = new DownloadDialog;
    connect(dl, &DownloadDialog::finished, dl, &DownloadDialog::deleteLater);
#ifndef Q_OS_ANDROID
    dl->show();
#else
    dl->showMaximized();
#endif
    dl->downloadList();
}

void MainDialog::showSongClientEditDialog()
{
    SongClientEditDialog *dl = new SongClientEditDialog;
    connect(dl, &SongClientEditDialog::finished, dl, &SongClientEditDialog::deleteLater);
#ifndef Q_OS_ANDROID
    dl->show();
#else
    dl->showMaximized();
#endif
    dl->loadFile();
}

void MainDialog::showPapaSongClientEditDialog()
{
    PapaSongClientEditDialog *dl = new PapaSongClientEditDialog;
    connect(dl, &PapaSongClientEditDialog::finished, dl, &PapaSongClientEditDialog::deleteLater);
#ifndef Q_OS_ANDROID
    dl->show();
#else
    dl->showMaximized();
#endif
    dl->loadFile();
}

void MainDialog::about()
{
    QMessageBox::about(this, tr("About RMEssentials"), tr(
        "RMEssentials is a small software written by Fsu0413. \n"
        "It is used to operate the files for a game by Tencent: Rhythm Master. \n"
        "It now contains 4 main features: ChangeName, Download, SongClientEdit, PapaSongClientEdit. \n\n"
        "This Program is powered by Qt %1. The version of this program is: %2"
    ).arg(QT_VERSION_STR).arg(programVersion));
}

void MainDialog::checkForUpdate()
{
#ifdef QT_NO_DEBUG
    QFile v("downloader/versioninfo");
#else
    QFile v("downloader/versioninfotest");
#endif
    if (v.open(QIODevice::ReadOnly)) {
        QString version = v.readAll();
        v.close();

        if (QString(programVersion) < version) {
            setWindowTitle(windowTitle() + tr(" new version %1 available").arg(version));
            if (isVisible()) {
                static QString link = "http://pan.baidu.com/s/1eQvwPzW";
                static QString passwd = "at7c";
                QString contents = tr(
                    "New version avaliable!! Version number: %1<br />"
                    "You can download the new version at <a href=\'%2\'>here</a>, the password is \"%3\""
                ).arg(version).arg(link).arg(passwd);
                QMessageBox::information(this, tr("RMEssentials"), contents);
            }
        }
    }
}