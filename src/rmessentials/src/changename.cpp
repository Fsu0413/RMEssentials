#include "changename.h"

#include <RmEss/RmeChartVersion>
#include <RmEss/RmeConverter>
#include <RmEss/RmeDownloader>
#include <RmEss/RmeRenamer>
#include <RmEss/RmeUtils>

#include <QComboBox>
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
#include <QTabWidget>
#include <QVBoxLayout>

#include <algorithm>

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

    QGridLayout *totalLayout = new QGridLayout;
    for (int i = 0; i < 36; ++i) {
        static constexpr const int LineInARow = 3;

        totalLayout->addWidget(new QLabel(layDecriptions[i]), i / LineInARow, (i % LineInARow) * 2, Qt::AlignRight);
        totalLayout->addWidget(labels[i], i / LineInARow, (i % LineInARow) * 2 + 1, Qt::AlignLeft);
    }
    return totalLayout;
}

ChangeNameDialog::ChangeNameDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Rhythm Master Filename Changer"));

    m_folderName = new QLineEdit;
    m_folderName->setPlaceholderText(tr("Browse the folder using the Browse button"));
    m_folderName->setReadOnly(true);
    QPushButton *browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &ChangeNameDialog::selectFolder);
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel(tr("Folder:")));
    layout1->addWidget(m_folderName);
    layout1->addWidget(browseButton);

    QTabWidget *tabWidget = new QTabWidget;
    QWidget *tabRename = new QWidget;

    m_toRename = new QLineEdit;
    m_toRename->setPlaceholderText(tr("Input the name to rename"));
    m_toRename->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[a-z0-9_]+"))));

    QPushButton *renameButton = new QPushButton(tr("Rename!"));
    connect(renameButton, &QPushButton::clicked, this, &ChangeNameDialog::rename);

    QPushButton *toEasyButton = new QPushButton(tr("Rename to Easy"));
    connect(toEasyButton, &QPushButton::clicked, this, &ChangeNameDialog::renameToEasy);

    QLabel *changeNameRestrictionsLabel = new QLabel(
        tr("1. The target path can contain only lower case letters, numbers and underscore (\"_\") for better compatibility with Rhythm Master.\n"
           "2. For IMD and IMDJson formats the renamed files are guaranteed to work as they contains no path related information. "
           "But for RMP format, since it is encrypted we have no way to know if RMP files still work after renaming until RMP format becomes publicly available someday."));
    changeNameRestrictionsLabel->setVisible(false);
    QPushButton *showChangeNameRestrictionsBtn = new QPushButton(tr("Show renaming restrictions"));
    connect(showChangeNameRestrictionsBtn, &QPushButton::clicked, showChangeNameRestrictionsBtn, [changeNameRestrictionsLabel, showChangeNameRestrictionsBtn]() {
        if (changeNameRestrictionsLabel->isVisible()) {
            changeNameRestrictionsLabel->hide();
            showChangeNameRestrictionsBtn->setText(tr("Show renaming restrictions"));
        } else {
            changeNameRestrictionsLabel->show();
            showChangeNameRestrictionsBtn->setText(tr("Hide renaming restrictions"));
        }
    });

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(renameButton);
    layout2->addWidget(toEasyButton);
    QVBoxLayout *layoutRenameWidget = new QVBoxLayout;
    layoutRenameWidget->addWidget(new QLabel(tr("Rename:")));
    layoutRenameWidget->addWidget(m_toRename);
    layoutRenameWidget->addLayout(layout2);
    layoutRenameWidget->addWidget(showChangeNameRestrictionsBtn);
    layoutRenameWidget->addWidget(changeNameRestrictionsLabel);
    layoutRenameWidget->addStretch();
    tabRename->setLayout(layoutRenameWidget);
    tabWidget->addTab(tabRename, tr("Rename"));

    // synchroize with enum RmeChartVersions::v
    m_convertToImdJsonVersion = new QComboBox;
    m_convertToImdJsonVersion->addItem(QStringLiteral("1.2.1"));
    m_convertToImdJsonVersion->addItem(QStringLiteral("1.2.2"));
    m_convertToImdJsonVersion->addItem(QStringLiteral("1.2.3"));
    m_convertToImdJsonVersion->addItem(QStringLiteral("1.3.0"));
    QPushButton *convertToImdJsonBtn = new QPushButton(tr("Convert All IMDs to IMDJson"));
    connect(convertToImdJsonBtn, &QPushButton::clicked, this, &ChangeNameDialog::convertImdToImdJson);
    QHBoxLayout *convertToImdJsonLayout = new QHBoxLayout;
    convertToImdJsonLayout->addWidget(m_convertToImdJsonVersion);
    convertToImdJsonLayout->addWidget(convertToImdJsonBtn);
    QPushButton *convertToImdBtn = new QPushButton(tr("Convert All IMDJsons to IMD"));
    connect(convertToImdBtn, &QPushButton::clicked, this, &ChangeNameDialog::convertImdJsonToImd);
    QPushButton *encryptImdJsonBtn = new QPushButton(tr("Convert All IMDJsons to RMP"));
    encryptImdJsonBtn->setEnabled(false);
    QPushButton *decryptImdJsonBtn = new QPushButton(tr("Convert All RMPs to IMDJson"));
    decryptImdJsonBtn->setEnabled(false);

    QLabel *conversionRestrictionsLabel = new QLabel(
        tr("1. There is no property for total time in IMDJson 1.2.x format. It is calculated when loading the chart. The result may be incorrect.\n"
           "2. The \"volume\" and \"pan\" properties of IMDJson notes are totally ignored. They are filled with their default value "
           "(127/64 for 1.2.1 / 1.2.2 and 0/0 for 1.2.3 onwards) when saving to IMDJson.\n"
           "3. The \"key\" property of IMDJson notes is not present in IMD format. It is ignored when saving to IMD and filled with default value (0) when reading from IMD.\n"
           "4. The \"samples\" property of IMDJson charts is totally ignored. It is filled with an empty JSON array when saving to IMDJson 1.2.3 onwards.\n"
           "5. The \"time\", \"time_dur\" and \"idx\" properties of IMDJson 1.2.3 onwards\' notes are totally ignored. "
           "They are recalculated when saving to IMDJson 1.2.3 onwards or to IMD.\n"
           "6. There are multiple BPM entries in IMD format. Only first entry of BPM is used during loading, while others are ignored. "
           "All versions of IMDJson doesn\'t support dynamic BPM, while dynamic BPM in IMD only affects number of notes in long press. The speed of chart is not affected.\n"
           "7. Other restrictions of Qt JSON processing, like member sorting of JSON objects, etc."));
    conversionRestrictionsLabel->setVisible(false);
    QPushButton *showConversionRestrictionsBtn = new QPushButton(tr("Show conversion restrictions"));
    connect(showConversionRestrictionsBtn, &QPushButton::clicked, showConversionRestrictionsBtn, [conversionRestrictionsLabel, showConversionRestrictionsBtn]() {
        if (conversionRestrictionsLabel->isVisible()) {
            conversionRestrictionsLabel->hide();
            showConversionRestrictionsBtn->setText(tr("Show conversion restrictions"));
        } else {
            conversionRestrictionsLabel->show();
            showConversionRestrictionsBtn->setText(tr("Hide conversion restrictions"));
        }
    });

    QVBoxLayout *layoutConvertWidget = new QVBoxLayout;
    layoutConvertWidget->addLayout(convertToImdJsonLayout);
    layoutConvertWidget->addWidget(convertToImdBtn);
    layoutConvertWidget->addWidget(encryptImdJsonBtn);
    layoutConvertWidget->addWidget(decryptImdJsonBtn);
    layoutConvertWidget->addWidget(showConversionRestrictionsBtn);
    layoutConvertWidget->addWidget(conversionRestrictionsLabel);
    layoutConvertWidget->addStretch();

    QWidget *tabConvert = new QWidget;
    tabConvert->setLayout(layoutConvertWidget);
    tabWidget->addTab(tabConvert, tr("Convert"));

    QFontMetrics fm(font());
    int width = fm.horizontalAdvance(tr("Missing"));
    int imdJsonChartWidth = std::max(width, fm.horizontalAdvance(QStringLiteral("1.2.3")));
    int titleWidth = fm.horizontalAdvance(QStringLiteral("_title_ipad"));

    for (int i = 0; i < 36; ++i) {
        m_filesLabels[i] = new QLabel;
        if (i == 4)
            m_filesLabels[4]->setMinimumWidth(titleWidth);
        else if (i >= 15 && i <= 23)
            m_filesLabels[i]->setMinimumWidth(imdJsonChartWidth);
        else
            m_filesLabels[i]->setMinimumWidth(width);
    }
    QLayout *filesLayout = layFiles(m_filesLabels);

    QHBoxLayout *downLayout = new QHBoxLayout;
    downLayout->addWidget(tabWidget);
    downLayout->addLayout(filesLayout);

    QVBoxLayout *totalLayout = new QVBoxLayout;
    totalLayout->addLayout(layout1);
    totalLayout->addLayout(downLayout);

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

    if (selected.isEmpty())
        return;

    QDir d(selected);
    bool bhasMp3 = hasMp3(d);
    bool bhasNote = existNotes(d);

    if (!(bhasMp3 && bhasNote)) {
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

void ChangeNameDialog::convertImdToImdJson()
{
    RmeConverter converter;
    converter.setDir(QDir(m_folderName->text()));

    RmeChartVersion v(m_convertToImdJsonVersion->currentText());
    if (v == RmeChartVersion::vUnknown) {
        QMessageBox::critical(this, tr("Error"), tr("Unknown error"));
        return;
    }

    bool succeeded = converter.convertImdToImdJson(v);

    if (!succeeded)
        QMessageBox::critical(this, tr("Error"), tr("Some error occurred when converting"));
    else
        QMessageBox::information(this, windowTitle(), tr("Rename succeeded"));

    checkFiles(m_folderName->text());
}

void ChangeNameDialog::convertImdJsonToImd()
{
    RmeConverter converter;
    converter.setDir(QDir(m_folderName->text()));

    bool succeeded = converter.convertImdJsonToImd();

    if (!succeeded)
        QMessageBox::critical(this, tr("Error"), tr("Some error occurred when converting"));
    else
        QMessageBox::information(this, windowTitle(), tr("Rename succeeded"));

    checkFiles(m_folderName->text());
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

    QString existImdJsonVersion[9];
    for (int i = 0; i < 9; ++i) {
        if (exists[i + 15]) {
            ExistNote note = static_cast<ExistNote>(1 << (i + 12));
            existImdJsonVersion[i] = RmeUtils::existImdJsonVersion(d, note);
        }
    }

    for (int i = 0; i < 36; ++i) {
        if (!exists[i])
            m_filesLabels[i]->setText(strMissing);
        else if (i == 4)
            m_filesLabels[4]->setText(smallPngSuffix);
        else if (i >= 15 && i <= 23)
            m_filesLabels[i]->setText(existImdJsonVersion[i - 15]);
        else
            m_filesLabels[i]->setText(strExists);
    }
}
