#include "maindialog.h"
#include "utils.h"
#include "renamer.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>



QLayout *MainDialog::layFiles(QLabel *labels[12]) {
    static QStringList layDecriptions;
    if (layDecriptions.isEmpty())
        layDecriptions << tr("MP3:")          << tr("Big PNG:")        << tr("Small PNG:")
                       << tr("Papa Big PNG:") << tr("Para Small PNG:") << QString()
                       << tr("4K Easy IMD:")  << tr("4K Normal IMD:")  << tr("4K Hard IMD:")
                       << tr("5K Easy IMD:")  << tr("5K Normal IMD:")  << tr("5K Hard IMD:")
                       << tr("6K Easy IMD:")  << tr("6K Normal IMD:")  << tr("6K Hard IMD:")
                       << tr("Easy MDE:")     << tr("Normal MDE:")     << tr("Hard MDE:");


    QHBoxLayout *totalLayout = new QHBoxLayout;
    for (int i = 0; i < 3; ++i) {
        QFormLayout *rowLayout = new QFormLayout;
        for (int j = i; j < 18; j += 3)
            rowLayout->addRow(layDecriptions[j], labels[j]);

        totalLayout->addLayout(rowLayout);
    }
    return totalLayout;
}

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("RMNameChanger"));

    QVBoxLayout *totalLayout = new QVBoxLayout;

    QFormLayout *flayout = new QFormLayout;

    m_folderName = new QLineEdit;
    m_folderName->setPlaceholderText(tr("Browse the folder using the Browse button"));
    m_folderName->setReadOnly(true);
    connect(this, &MainDialog::enable_widgets, m_folderName, &QLineEdit::setEnabled);
    QPushButton *browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &MainDialog::selectFolder);
    connect(this, &MainDialog::enable_widgets, browseButton, &QPushButton::setEnabled);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_folderName);
    layout1->addWidget(browseButton);
    flayout->addRow(tr("Folder:"), layout1);

    m_toRename = new QLineEdit;
    m_toRename->setPlaceholderText(tr("Input the name to rename"));
    flayout->addRow(tr("Rename:"), m_toRename);
    connect(this, &MainDialog::enable_widgets, m_toRename, &QLineEdit::setEnabled);

    totalLayout->addLayout(flayout);

    QFontMetrics fm(font());
    int width = fm.width(tr("Missing"));

    for (int i = 0; i < 18; ++i) {
        m_filesLabels[i] = new QLabel;
        m_filesLabels[i]->setMinimumWidth(width);
    }
    QLayout *filesLayout = layFiles(m_filesLabels);
    totalLayout->addLayout(filesLayout);

    QPushButton *renameButton = new QPushButton(tr("Rename!"));
    connect(this, &MainDialog::enable_widgets, renameButton, &QPushButton::setEnabled);
    connect(renameButton, &QPushButton::clicked, this, &MainDialog::renameAsk);
    totalLayout->addWidget(renameButton);

    setLayout(totalLayout);

    connect(this, &MainDialog::folder_selected, this, &MainDialog::checkFiles);
}

MainDialog::~MainDialog()
{

}

void MainDialog::selectFolder()
{
#ifdef Q_OS_ANDROID
    QString dir;
    QDir d_detect("/sdcard/RM/res/song");
    if (d_detect.exists())
        dir = d_detect.absolutePath();
    else
        dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#else
    QString dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif
    QString selected = QFileDialog::getExistingDirectory(this, QString(), dir/*, QFileDialog::DontUseNativeDialog*/);

    QDir d(selected);
    bool bhasMp3 = hasMp3(d);
    bool bhasBigPng = hasBigPng(d);
    bool bhasNote = existNotes(d);

    if (!(bhasMp3 && bhasBigPng && bhasNote)) {
        QMessageBox::critical(this, tr("Error"), tr("The folder you selected is not usable in Rhythm Master, please select again."));
        return;
    }

    m_folderName->setText(d.absolutePath());
    emit folder_selected(d.absolutePath());
}

void MainDialog::renameAsk()
{
    if (m_toRename->text().isEmpty()) {
        QMessageBox::information(this, windowTitle(), tr("Please enter the new name"));
        return;
    }

    QString originPath = QDir(m_folderName->text()).dirName().toLower();

    if (QMessageBox::question(this, windowTitle(), tr("You are now renaming %1 to %2.\nAre you sure?").arg(originPath).arg(m_toRename->text())) == QMessageBox::Yes) {
        emit enable_widgets(false);
        Renamer *renamer = new Renamer;
        renamer->setDir(QDir(m_folderName->text()));
        renamer->setToRename(m_toRename->text());
        connect(renamer, &Renamer::rename_finished, this, &MainDialog::renameFinished);
        connect(renamer, &Renamer::finished, renamer, &Renamer::deleteLater);
        renamer->run();
    }
}

void MainDialog::checkFiles(const QString &folder)
{
    static const QString strExists = tr("Exists");
    static const QString strMissing = tr("Missing");

    QDir d(folder);
    bool exists[18] = {false};
    exists[0] = hasMp3(d);
    exists[1] = hasBigPng(d);
    const char *smallPngSuffix = NULL;
    exists[2] = hasSmallPng(d, smallPngSuffix);
    exists[3] = hasPapaBigPng(d);
    exists[4] = hasPapaSmallPng(d);

    ExistNotes imds = existNotes(d);
    exists[6] = imds & IMD_4K_EZ;
    exists[7] = imds & IMD_4K_NM;
    exists[8] = imds & IMD_4K_HD;
    exists[9] = imds & IMD_5K_EZ;
    exists[10] = imds & IMD_5K_NM;
    exists[11] = imds & IMD_5K_HD;
    exists[12] = imds & IMD_6K_EZ;
    exists[13] = imds & IMD_6K_NM;
    exists[14] = imds & IMD_6K_HD;
    exists[15] = imds & MDE_EZ;
    exists[16] = imds & MDE_NM;
    exists[17] = imds & MDE_HD;

    for (int i = 0; i < 18; ++i) {
        if (i == 5)
            continue;
        else if (!exists[i])
            m_filesLabels[i]->setText(strMissing);
        else if (i != 2)
            m_filesLabels[i]->setText(strExists);
        else
            m_filesLabels[2]->setText(smallPngSuffix);
    }
}

void MainDialog::renameFinished(bool succeeded)
{
    if (!succeeded)
        QMessageBox::critical(this, tr("Error"), tr("unknown error"));
    else {
        QMessageBox::information(this, windowTitle(), tr("Rename succeeded"));
        Renamer *renamer = qobject_cast<Renamer *>(sender());
        if (renamer)
            m_folderName->setText(renamer->dir().absolutePath());
    }

    emit enable_widgets(true);
}
