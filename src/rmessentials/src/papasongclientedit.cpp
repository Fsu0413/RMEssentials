#include "papasongclientedit.h"

#include <RMEssentials/RmeDownloader>
#include <RMEssentials/RmeSongClientStruct>

#include <QCheckBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExpValidator>
#include <QStandardPaths>
#include <QVBoxLayout>

#ifdef MOBILE_DEVICES
#include <QScrollArea>
#endif

using namespace RmeSong;

PapaSongClientEditDialog::PapaSongClientEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentIndex(-1)
    , m_isLoaded(false)
{
    setWindowTitle(tr("Rhythm Master PapaSong Client Editor"));

    ushSongID = new QLineEdit;
    ushSongID->setReadOnly(true);
    QPushButton *prevBtn = new QPushButton(tr("prev"));
    prevBtn->setAutoDefault(false);
    prevBtn->setDefault(false);
    connect(prevBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::movePrev);
    QPushButton *nextBtn = new QPushButton(tr("next"));
    nextBtn->setAutoDefault(false);
    nextBtn->setDefault(false);
    connect(nextBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::moveNext);
    QPushButton *saveCurrentBtn = new QPushButton(tr("save current"));
    saveCurrentBtn->setAutoDefault(false);
    saveCurrentBtn->setDefault(false);
    connect(saveCurrentBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::saveCurrent);
    // functions...
    m_popup = new QMenu(this);
    QAction *openFileBtn = m_popup->addAction(tr("open an other file"));
    connect(openFileBtn, &QAction::triggered, this, &PapaSongClientEditDialog::reloadFile);
    QAction *saveFileBtn = m_popup->addAction(tr("save file"));
    connect(saveFileBtn, &QAction::triggered, this, &PapaSongClientEditDialog::saveFile);
    QPushButton *funcBtn = new QPushButton(tr("Functions..."));
    funcBtn->setAutoDefault(false);
    funcBtn->setDefault(false);
    connect(funcBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::popup);

    QVBoxLayout *leftLayout = new QVBoxLayout;

    szSongName = new QLineEdit;
    szNoteNumber = new QLineEdit;
    szRegion = new QLineEdit;
    iOrderIndex = new QLineEdit;
    QIntValidator *iOrderIndexValidator = new QIntValidator(0, 100, this);
    iOrderIndex->setValidator(iOrderIndexValidator);
    szPath = new QLineEdit;
    QRegExpValidator *szPathValidator = new QRegExpValidator(QRegExp(QStringLiteral("[0-9a-z_]+")), this);
    szPath->setValidator(szPathValidator);
    iGameTime = new QLineEdit;
    QIntValidator *iGameTimeValidator = new QIntValidator(1, 2147483647, this);
    iGameTime->setValidator(iGameTimeValidator);
    connect(iGameTime, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::calculateSongTime);
    szStyle = new QLineEdit;
    iSongType = new QLineEdit;
    iSongType->setValidator(iGameTimeValidator);
    szArtist = new QLineEdit;
    szSongTime = new QLabel;
    szBPM = new QLineEdit;
    QDoubleValidator *szBPMValidator = new QDoubleValidator(0, 10000, 3, this);
    szBPM->setValidator(szBPMValidator);
    iVersion = new QLineEdit;
    QIntValidator *iVersionValidator = new QIntValidator(1, 2147483647, this);
    iVersion->setValidator(iVersionValidator);
    cDifficulty = new QLineEdit;
    QIntValidator *cDifficultyValidator = new QIntValidator(1, 3, this);
    cDifficulty->setValidator(cDifficultyValidator);
    cLevel = new QLineEdit;
    QIntValidator *cLevelValidator = new QIntValidator(1, 10, this);
    cLevel->setValidator(cLevelValidator);
    ucIsHide = new QCheckBox(QStringLiteral("ucIsHide"));
    ucIsReward = new QCheckBox(QStringLiteral("ucIsReward"));
    ucIsLevelReward = new QCheckBox(QStringLiteral("ucIsLevelReward"));
    ucIsOpen = new QCheckBox(QStringLiteral("ucIsOpen"));
    ucIsFree = new QCheckBox(QStringLiteral("ucIsFree"));

// for QFormLayout
#define AR(l, x) l->addRow(QStringLiteral(#x), x)

    // 1st line
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    QFormLayout *flayout0 = new QFormLayout;
    AR(flayout0, ushSongID);
    hlayout1->addLayout(flayout0);
    hlayout1->addWidget(prevBtn);
    hlayout1->addWidget(nextBtn);
    hlayout1->addWidget(saveCurrentBtn);
    hlayout1->addWidget(funcBtn);

    leftLayout->addLayout(hlayout1);
#ifndef MOBILE_DEVICES
    // 2nd, 3rd, 4th lines...
    QHBoxLayout *hlayout234 = new QHBoxLayout;
    QFormLayout *flayout2 = new QFormLayout;
    AR(flayout2, szSongName);
    AR(flayout2, szNoteNumber);
    AR(flayout2, szRegion);
    AR(flayout2, iOrderIndex);
    QFormLayout *flayout3 = new QFormLayout;
    AR(flayout3, szPath);
    AR(flayout3, iGameTime);
    AR(flayout3, szStyle);
    AR(flayout3, iSongType);
    QFormLayout *flayout4 = new QFormLayout;
    AR(flayout4, szArtist);
    AR(flayout4, szSongTime);
    AR(flayout4, szBPM);
    AR(flayout4, iVersion);
    hlayout234->addLayout(flayout2);
    hlayout234->addLayout(flayout3);
    hlayout234->addLayout(flayout4);

    // 5th line...
    QHBoxLayout *hlayout5 = new QHBoxLayout;

    QFormLayout *flayout5 = new QFormLayout;
    AR(flayout5, cDifficulty);
    hlayout5->addLayout(flayout5);

    QFormLayout *flayout6 = new QFormLayout;
    AR(flayout6, cLevel);
    hlayout5->addLayout(flayout6);

    // 6th line...
    QHBoxLayout *hlayout12 = new QHBoxLayout;

    hlayout12->addWidget(ucIsOpen);
    hlayout12->addWidget(ucIsFree);
    hlayout12->addWidget(ucIsHide);
    hlayout12->addWidget(ucIsReward);
    hlayout12->addWidget(ucIsLevelReward);

    leftLayout->addLayout(hlayout234);
    leftLayout->addLayout(hlayout5);
    leftLayout->addLayout(hlayout12);
#else
    QVBoxLayout *vlayout = new QVBoxLayout;
    QFormLayout *flayout1 = new QFormLayout;
    AR(flayout1, szSongName);
    AR(flayout1, szPath);
    AR(flayout1, szArtist);
    AR(flayout1, szNoteNumber);
    AR(flayout1, iGameTime);
    AR(flayout1, szSongTime);
    AR(flayout1, szRegion);
    AR(flayout1, szStyle);
    AR(flayout1, szBPM);
    AR(flayout1, iOrderIndex);
    AR(flayout1, iSongType);
    AR(flayout1, iVersion);
    AR(flayout1, cDifficulty);
    AR(flayout1, cLevel);
    vlayout->addLayout(flayout1);

    vlayout->addWidget(ucIsOpen);
    vlayout->addWidget(ucIsFree);
    vlayout->addWidget(ucIsHide);
    vlayout->addWidget(ucIsReward);
    vlayout->addWidget(ucIsLevelReward);

    QWidget *widget = new QWidget;
    widget->setLayout(vlayout);

    QScrollArea *area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setWidget(widget);

    leftLayout->addWidget(area);
#endif

// OK, thank you
#undef AR

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(tr("Search"));
#ifndef MOBILE_DEVICES
    m_searchEdit->setMinimumWidth(80);
#else
    m_searchEdit->setMinimumWidth(200);
#endif
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PapaSongClientEditDialog::search);

    QPushButton *searchBtn = new QPushButton(tr("Search"));
    searchBtn->setAutoDefault(false);
    searchBtn->setDefault(false);
#ifndef MOBILE_DEVICES
    searchBtn->setMaximumWidth(60);
#else
    searchBtn->setMinimumWidth(120);
    searchBtn->setMaximumWidth(120);
#endif
    connect(searchBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::search);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(searchBtn);

    m_searchList = new QListWidget;
    m_searchList->setSortingEnabled(false);
    connect(m_searchList, &QListWidget::itemDoubleClicked, this, &PapaSongClientEditDialog::searchResultDblClicked);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addLayout(searchLayout);
    rightLayout->addWidget(m_searchList);

    QHBoxLayout *alllayout = new QHBoxLayout;
    alllayout->addLayout(leftLayout);
    alllayout->addLayout(rightLayout);

    setLayout(alllayout);
}

bool PapaSongClientEditDialog::reloadFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));

    QFile f(filepath);
    if (!f.exists())
        return false;

    if (m_file.readInfoFromDevice(&f, BinFormat)) {
        m_isLoaded = true;
        m_currentIndex = 0;
        readCurrent();
        return true;
    } else
        QMessageBox::critical(this, tr("RMEssentials"), tr("Read file failed"));

    return false;
}

bool PapaSongClientEditDialog::loadFile()
{
    QDir d(RmeDownloader::downloadPath());

    QString filepath;
    if (d.exists() && d.exists(QStringLiteral("mrock_papasong_client.bin")))
        filepath = d.absoluteFilePath(QStringLiteral("mrock_papasong_client.bin"));
    else {
        QMessageBox::information(this, tr("RMEssentials"), tr("mrock_papasong_client.bin doesn't exist, please select the file to open."));
        filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));
    }

    QFile f(filepath);
    if (!f.exists())
        return false;

    if (m_file.readInfoFromDevice(&f, BinFormat)) {
        m_isLoaded = true;
        m_currentIndex = 0;
        readCurrent();
        return true;
    } else
        QMessageBox::critical(this, tr("RMEssentials"), tr("Read file failed"));

    return false;
}

void PapaSongClientEditDialog::saveFile()
{
    if (!m_isLoaded)
        return;

    QString filepath = QFileDialog::getSaveFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));
    QFile f(filepath);
    if (f.exists() && QMessageBox::question(this, tr("RMEssentials"), tr("File is already exists, do you want to overwrite?")) == QMessageBox::No)
        return;

    if (!m_file.saveInfoToDevice(&f, BinFormat))
        QMessageBox::critical(this, tr("RMEssentials"), tr("Save file failed"));
}

void PapaSongClientEditDialog::moveNext()
{
    if (!m_isLoaded)
        return;

    if (m_currentIndex + 1 == m_file.songCount())
        return;

    ++m_currentIndex;
    readCurrent();
}

void PapaSongClientEditDialog::movePrev()
{
    if (!m_isLoaded)
        return;

    if (m_currentIndex <= 0)
        return;

    --m_currentIndex;
    readCurrent();
}

void PapaSongClientEditDialog::readCurrent()
{
    const RmePapaSongClientItemStruct &c = *(m_file.song(m_currentIndex));

#define RP_NM(p) p->setText(QString::number(c.m_##p))
#define RP_ST(p) p->setText(c.m_##p)
#define RP_BL(p) p->setChecked(c.m_##p)

    RP_NM(ushSongID);
    RP_NM(iVersion);
    RP_ST(szSongName);
    RP_ST(szPath);
    RP_ST(szArtist);
    RP_ST(szSongTime);
    RP_NM(iGameTime);
    RP_ST(szRegion);
    RP_ST(szStyle);
    RP_ST(szBPM);
    RP_BL(ucIsOpen);
    RP_BL(ucIsFree);
    RP_NM(iOrderIndex);
    RP_ST(szNoteNumber);
    RP_BL(ucIsHide);
    RP_BL(ucIsReward);
    RP_BL(ucIsLevelReward);
    RP_NM(iSongType);
    RP_NM(cLevel);
    RP_NM(cDifficulty);

#undef RP_BL
#undef RP_ST
#undef RP_NM
}

void PapaSongClientEditDialog::popup()
{
    m_popup->popup(QCursor::pos());
}

void PapaSongClientEditDialog::calculateSongTime()
{
    int gameTime = iGameTime->text().toInt();
    float songTime = gameTime / 1440.f;
    QString r = QString::number(songTime);
    if (r.length() > 8) {
        int r9 = r.at(8).toLatin1() - 48;
        if (r9 >= 5) {
            songTime += 0.000001f;
            r = QString::number(songTime);
        }
    }
    r = r.left(8);
    szSongTime->setText(r);
}

void PapaSongClientEditDialog::saveCurrent()
{
    RmePapaSongClientItemStruct &c = *(m_file.song(m_currentIndex));

#define SP_NS(p) c.m_##p = p->text().toShort()
#define SP_NI(p) c.m_##p = p->text().toInt()
#define SP_ST(p) c.m_##p = p->text()
#define SP_BN(p) c.m_##p = (p->isChecked() ? 1 : 0)
#define SP_BL(p) c.m_##p = p->isChecked()

    SP_NS(ushSongID);
    SP_NI(iVersion);
    SP_ST(szSongName);
    SP_ST(szPath);
    SP_ST(szArtist);
    SP_ST(szSongTime);
    SP_NI(iGameTime);
    SP_ST(szRegion);
    SP_ST(szStyle);
    SP_ST(szBPM);
    SP_BN(ucIsOpen);
    SP_BN(ucIsFree);
    SP_NI(iOrderIndex);
    SP_ST(szNoteNumber);
    SP_BN(ucIsHide);
    SP_BN(ucIsReward);
    SP_BN(ucIsLevelReward);
    SP_NI(iSongType);
    SP_NS(cLevel);
    SP_NS(cDifficulty);

#undef SP_BL
#undef SP_BN
#undef SP_ST
#undef SP_NI
#undef SP_NS
}

void PapaSongClientEditDialog::search()
{
    m_searchList->clear();

    QList<int> searchResult = m_file.search(m_searchEdit->text());

    if (searchResult.isEmpty())
        return;

    if (searchResult.length() == 1) {
        m_currentIndex = searchResult.first();
        readCurrent();
    }

    foreach (int n, searchResult) {
        RmePapaSongClientItemStruct *song = m_file.song(n);
        QListWidgetItem *i = new QListWidgetItem(song->m_szSongName);
        i->setData(Qt::UserRole + 1, n);
        m_searchList->addItem(i);
    }
}

void PapaSongClientEditDialog::searchResultDblClicked(QListWidgetItem *index)
{
    if (index == nullptr)
        return;

    bool ok = false;
    int i = index->data(Qt::UserRole + 1).toInt(&ok);

    if (!ok)
        return;

    m_currentIndex = i;
    readCurrent();
}
