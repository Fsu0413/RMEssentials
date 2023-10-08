#include "changename.h"

#include <RMEss/RmeDownloader>
#include <RMEss/RmeRenamer>
#include <RMEss/RmeUtils>

#include <QFileDialog>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QStandardPaths>
#include <QVBoxLayout>

using namespace RmeUtils;

QLayout *ChangeNameDialog::layFiles(QLabel *labels[36])
{
    // clang-format off
    static QStringList layDecriptions = {tr("MP3:"),              tr("Big / Papa PNG:"),     tr("Small PNG:"),
                                         tr("Old Big PNG:"),      tr("Old Small PNG:"),      tr("Papa Small PNG:"),
                                         tr("4K Easy IMD:"),      tr("4K Normal IMD:"),      tr("4K Hard IMD:"),
                                         tr("5K Easy IMD:"),      tr("5K Normal IMD:"),      tr("5K Hard IMD:"),
                                         tr("6K Easy IMD:"),      tr("6K Normal IMD:"),      tr("6K Hard IMD:"),
                                         tr("4K Easy IMDJson:"),  tr("4K Normal IMDJson:"),  tr("4K Hard IMDJson:"),
                                         tr("5K Easy IMDJson:"),  tr("5K Normal IMDJson:"),  tr("5K Hard IMDJson:"),
                                         tr("6K Easy IMDJson:"),  tr("6K Normal IMDJson:"),  tr("6K Hard IMDJson:"),
                                         tr("4K Easy RMP:"),      tr("4K Normal RMP:"),      tr("4K Hard RMP:"),
                                         tr("5K Easy RMP:"),      tr("5K Normal RMP:"),      tr("5K Hard RMP:"),
                                         tr("6K Easy RMP:"),      tr("6K Normal RMP:"),      tr("6K Hard RMP:"),
                                         tr("Easy MDE:"),         tr("Normal MDE:"),         tr("Hard MDE:")};
    // clang-format on

    QHBoxLayout *totalLayout = new QHBoxLayout;
    for (int i = 0; i < 3; ++i) {
        QFormLayout *rowLayout = new QFormLayout;
        for (int j = i; j < 36; j += 3)
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
    m_toRename->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[a-z0-9_]+"))));

    totalLayout->addLayout(flayout);

    QFontMetrics fm(font());
    int width = fm.horizontalAdvance(tr("Missing"));
    int titleWidth = fm.horizontalAdvance(QStringLiteral("_title_ipad"));

    for (int i = 0; i < 36; ++i) {
        m_filesLabels[i] = new QLabel;
        if (i != 4)
            m_filesLabels[i]->setMinimumWidth(width);
        else
            m_filesLabels[4]->setMinimumWidth(titleWidth);
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

    connect(this, &ChangeNameDialog::folderSelected, this, &ChangeNameDialog::checkFiles);
}

void ChangeNameDialog::selectFolder()
{
    QString dir;
    QDir d_detect(RmeDownloader::songDownloadPath());
    if (d_detect.exists())
        dir = d_detect.absolutePath();
    else
        dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString selected = QFileDialog::getExistingDirectory(this, QString(), dir);

    QDir d(selected);
    bool bhasMp3 = hasMp3(d);
    bool bhasNewBigPng = hasNewBigPng(d);
    bool bhasBigPng = hasBigPng(d);
    bool bhasNote = existNotes(d);

    if (!(bhasMp3 && (bhasNewBigPng || bhasBigPng) && bhasNote)) {
        QMessageBox::critical(this, tr("Error"), tr("The folder you selected is not usable in Rhythm Master, please select again."));
        return;
    }

    m_folderName->setText(d.absolutePath());
    emit folderSelected(d.absolutePath());
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

    if (QMessageBox::question(this, windowTitle(),
                              tr("You are now renaming %1 to %2.\n"
                                 "Caution!! All unrelated files will be deleted.\n"
                                 "Are you sure?")
                                  .arg(originPath, m_toRename->text()))
        == QMessageBox::Yes) {
        RmeRenamer renamer;
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
        RmeRenamer renamer;
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
    bool exists[36] = {false};
    exists[0] = hasMp3(d);
    exists[1] = hasNewBigPng(d);
    exists[2] = hasNewSmallPng(d);
    exists[3] = hasBigPng(d);
    QString smallPngSuffix;
    exists[4] = hasSmallPng(d, smallPngSuffix);
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
    exists[15] = imds & IMDJSON_4K_EZ;
    exists[16] = imds & IMDJSON_4K_NM;
    exists[17] = imds & IMDJSON_4K_HD;
    exists[18] = imds & IMDJSON_5K_EZ;
    exists[19] = imds & IMDJSON_5K_NM;
    exists[20] = imds & IMDJSON_5K_HD;
    exists[21] = imds & IMDJSON_6K_EZ;
    exists[22] = imds & IMDJSON_6K_NM;
    exists[23] = imds & IMDJSON_6K_HD;
    exists[24] = imds & RMP_4K_EZ;
    exists[25] = imds & RMP_4K_NM;
    exists[26] = imds & RMP_4K_HD;
    exists[27] = imds & RMP_5K_EZ;
    exists[28] = imds & RMP_5K_NM;
    exists[29] = imds & RMP_5K_HD;
    exists[30] = imds & RMP_6K_EZ;
    exists[31] = imds & RMP_6K_NM;
    exists[32] = imds & RMP_6K_HD;
    exists[33] = imds & MDE_EZ;
    exists[34] = imds & MDE_NM;
    exists[35] = imds & MDE_HD;

    for (int i = 0; i < 36; ++i) {
        if (!exists[i])
            m_filesLabels[i]->setText(strMissing);
        else if (i != 4)
            m_filesLabels[i]->setText(strExists);
        else
            m_filesLabels[4]->setText(smallPngSuffix);
    }
}
