#include "ChangeNameDialog.h"
#include "utils.h"
#include "renamer.h"
#include "downloader.h"

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

QLayout *ChangeNameDialog::layFiles(QLabel *labels[18])
{
    static QStringList layDecriptions;
    if (layDecriptions.isEmpty())
        layDecriptions << tr("MP3:") << tr("Big PNG:") << tr("Small PNG:")
                       << tr("Papa Big PNG:") << tr("Para Small PNG:") << QString()
                       << tr("4K Easy IMD:") << tr("4K Normal IMD:") << tr("4K Hard IMD:")
                       << tr("5K Easy IMD:") << tr("5K Normal IMD:") << tr("5K Hard IMD:")
                       << tr("6K Easy IMD:") << tr("6K Normal IMD:") << tr("6K Hard IMD:")
                       << tr("Easy MDE:") << tr("Normal MDE:") << tr("Hard MDE:");


    QHBoxLayout *totalLayout = new QHBoxLayout;
    for (int i = 0; i < 3; ++i) {
        QFormLayout *rowLayout = new QFormLayout;
        for (int j = i; j < 18; j += 3)
            rowLayout->addRow(layDecriptions[j], labels[j]);

        totalLayout->addLayout(rowLayout);
    }
    return totalLayout;
}

ChangeNameDialog::ChangeNameDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Rhythm Master Filename Changer"));

    QVBoxLayout *totalLayout = new QVBoxLayout;

    QFormLayout *flayout = new QFormLayout;

    m_folderName = new QLineEdit;
    m_folderName->setPlaceholderText(tr("Browse the folder using the Browse button"));
    m_folderName->setReadOnly(true);
    QPushButton *browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &ChangeNameDialog::selectFolder);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(m_folderName);
    layout1->addWidget(browseButton);
    flayout->addRow(tr("Folder:"), layout1);

    m_toRename = new QLineEdit;
    m_toRename->setPlaceholderText(tr("Input the name to rename"));
    flayout->addRow(tr("Rename:"), m_toRename);

    totalLayout->addLayout(flayout);

    QFontMetrics fm(font());
    int width = fm.width(tr("Missing"));
    int titleWidth = fm.width(QStringLiteral("_title_ipad"));

    for (int i = 0; i < 18; ++i) {
        m_filesLabels[i] = new QLabel;
        if (i != 2)
            m_filesLabels[i]->setMinimumWidth(width);
        else
            m_filesLabels[2]->setMinimumWidth(titleWidth);
    }
    QLayout *filesLayout = layFiles(m_filesLabels);
    totalLayout->addLayout(filesLayout);

    QPushButton *renameButton = new QPushButton(tr("Rename!"));
    connect(renameButton, &QPushButton::clicked, this, &ChangeNameDialog::rename);

    QPushButton *toEasyButton = new QPushButton(tr("Rename to Easy"));
    connect(toEasyButton, &QPushButton::clicked, this, &ChangeNameDialog::renameToEasy);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(renameButton);
    layout2->addWidget(toEasyButton);

    totalLayout->addLayout(layout2);

    setLayout(totalLayout);

    connect(this, &ChangeNameDialog::folder_selected, this, &ChangeNameDialog::checkFiles);
}

void ChangeNameDialog::selectFolder()
{

    QString dir;
    QDir d_detect(Downloader::downloadPath());
    if (d_detect.exists())
        dir = d_detect.absolutePath();
    else
        dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

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

void ChangeNameDialog::rename()
{
    if (m_toRename->text().isEmpty()) {
        QMessageBox::information(this, windowTitle(), tr("Please enter the new name"));
        return;
    }

    QString originPath = QDir(m_folderName->text()).dirName().toLower();

    if (m_toRename->text().toLower() == originPath) {
        QMessageBox::information(this, windowTitle(), tr("The new name is same with the old name, please change the new name"));
        return;
    }

    if (QMessageBox::question(this, windowTitle(), tr("You are now renaming %1 to %2.\nAre you sure?").arg(originPath).arg(m_toRename->text())) == QMessageBox::Yes) {
        Renamer renamer;
        renamer.setDir(QDir(m_folderName->text()));
        renamer.setToRename(m_toRename->text());

        bool succeeded = renamer.run();
        if (!succeeded)
            QMessageBox::critical(this, tr("Error"), tr("unknown error"));
        else {
            QMessageBox::information(this, windowTitle(), tr("Rename succeeded"));
            m_folderName->setText(renamer.dir().absolutePath());
            checkFiles(m_folderName->text());
        }
    }
}

void ChangeNameDialog::renameToEasy()
{
    QString originPath = QDir(m_folderName->text()).dirName().toLower();

    if (QMessageBox::question(this, windowTitle(), tr("You are now renaming %1 to easy.\nAre you sure?").arg(originPath)) == QMessageBox::Yes) {
        Renamer renamer;
        renamer.setDir(QDir(m_folderName->text()));

        bool succeeded = renamer.runToEasy();
        if (!succeeded)
            QMessageBox::critical(this, tr("Error"), tr("unknown error"));
        else {
            QMessageBox::information(this, windowTitle(), tr("Rename succeeded"));
            checkFiles(m_folderName->text());
        }
    }
}

void ChangeNameDialog::checkFiles(const QString &folder)
{
    static const QString strExists = tr("Exists");
    static const QString strMissing = tr("Missing");

    QDir d(folder);
    bool exists[18] = {false};
    exists[0] = hasMp3(d);
    exists[1] = hasBigPng(d);
    QString smallPngSuffix;
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
