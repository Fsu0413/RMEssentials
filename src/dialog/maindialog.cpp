#include "maindialog.h"
#include "ChangeNameDialog.h"
#include "DownloadDialog.h"
#include "SongClientEditDialog.h"

#include <QVBoxLayout>
#include <QPushButton>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Rhythm Master Essensials"));

    QVBoxLayout *alllayout = new QVBoxLayout;

    QPushButton *changename = new QPushButton(tr("Filename Changer"));
    connect(changename, &QPushButton::clicked, this, &MainDialog::showChangeNameDialog);

    QPushButton *download = new QPushButton(tr("File Downloader"));
    connect(download, &QPushButton::clicked, this, &MainDialog::showDownloadDialog);

    QPushButton *songclient = new QPushButton(tr("Song Client Editor"));
    connect(songclient, &QPushButton::clicked, this, &MainDialog::showSongClientEditDialog);

    alllayout->addWidget(changename);
    alllayout->addWidget(download);
    alllayout->addWidget(songclient);

    setLayout(alllayout);

    setMinimumWidth(150);
}

void MainDialog::showChangeNameDialog() {
    ChangeNameDialog *dl = new ChangeNameDialog;
    connect(dl, &ChangeNameDialog::finished, dl, &ChangeNameDialog::deleteLater);
#ifndef Q_OS_ANDROID
    dl->show();
#else
    dl->showMaximized();
#endif
}

void MainDialog::showDownloadDialog() {
    DownloadDialog *dl = new DownloadDialog;
    connect(dl, &DownloadDialog::finished, dl, &DownloadDialog::deleteLater);
#ifndef Q_OS_ANDROID
    dl->show();
#else
    dl->showMaximized();
#endif
    dl->downloadList();
}

void MainDialog::showSongClientEditDialog() {
    SongClientEditDialog *dl = new SongClientEditDialog;
    connect(dl, &SongClientEditDialog::finished, dl, &SongClientEditDialog::deleteLater);
#ifndef Q_OS_ANDROID
    dl->show();
#else
    dl->showMaximized();
#endif
    dl->loadFile();
}
