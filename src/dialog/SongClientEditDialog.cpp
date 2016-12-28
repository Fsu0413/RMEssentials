#include "SongClientEditDialog.h"
#include "downloader.h"
#include "songstruct.h"

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

using namespace RMSong;

SongClientEditDialog::SongClientEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentIndex(-1)
    , m_isLoaded(false)
{
    setWindowTitle(tr("Rhythm Master Song Client Editor"));

    QVBoxLayout *leftLayout = new QVBoxLayout;

    ushSongID = new QLineEdit;
    ushSongID->setReadOnly(true);
    QPushButton *prevBtn = new QPushButton(tr("prev"));
    prevBtn->setAutoDefault(false);
    prevBtn->setDefault(false);
    connect(prevBtn, &QPushButton::clicked, this, &SongClientEditDialog::movePrev);
    QPushButton *nextBtn = new QPushButton(tr("next"));
    nextBtn->setAutoDefault(false);
    nextBtn->setDefault(false);
    connect(nextBtn, &QPushButton::clicked, this, &SongClientEditDialog::moveNext);
    QPushButton *saveCurrentBtn = new QPushButton(tr("save current"));
    saveCurrentBtn->setAutoDefault(false);
    saveCurrentBtn->setDefault(false);
    connect(saveCurrentBtn, &QPushButton::clicked, this, &SongClientEditDialog::saveCurrent);
    // functions...
    m_popup = new QMenu(this);
    QAction *openFileBtn = m_popup->addAction(tr("open an other file"));
    connect(openFileBtn, &QAction::triggered, this, &SongClientEditDialog::reloadFile);
    QAction *saveFileBtn = m_popup->addAction(tr("save file"));
    connect(saveFileBtn, &QAction::triggered, this, &SongClientEditDialog::saveFile);
    m_popup->addSeparator();
    QAction *castf = m_popup->addAction(tr("Convert All Songs to Free"));
    connect(castf, &QAction::triggered, this, &SongClientEditDialog::convertToFree);
    QAction *asul = m_popup->addAction(tr("All Song Unlock"));
    connect(asul, &QAction::triggered, this, &SongClientEditDialog::allSongUnlock);
    QAction *msl = m_popup->addAction(tr("Merge Song List"));
    connect(msl, &QAction::triggered, this, &SongClientEditDialog::mergeSongList);
    QAction *pfumn = m_popup->addAction(tr("Prepare for User Making Notes"));
    connect(pfumn, &QAction::triggered, this, &SongClientEditDialog::prepareForUserMakingNotes);
    QPushButton *funcBtn = new QPushButton(tr("Functions..."));
    funcBtn->setAutoDefault(false);
    funcBtn->setDefault(false);
    connect(funcBtn, &QPushButton::clicked, this, &SongClientEditDialog::popup);

    szSongName = new QLineEdit;
    szPath = new QLineEdit;
    QRegExpValidator *szPathValidator = new QRegExpValidator(QRegExp(QStringLiteral("[0-9a-z_]+")), this);
    szPath->setValidator(szPathValidator);
    szArtist = new QLineEdit;
    szComposer = new QLineEdit;
    iGameTime = new QLineEdit;
    QIntValidator *iGameTimeValidator = new QIntValidator(1, 2147483647, this);
    iGameTime->setValidator(iGameTimeValidator);
    connect(iGameTime, &QLineEdit::textEdited, this, &SongClientEditDialog::calculateSongTime);
    szSongTime = new QLabel;
    iRegion = new QLineEdit;
    QIntValidator *iRegionValidator = new QIntValidator(0, 11, this);
    iRegion->setValidator(iRegionValidator);
    iStyle = new QLineEdit;
    QIntValidator *iStyleValidator = new QIntValidator(0, 20, this);
    iStyle->setValidator(iStyleValidator);
    szBPM = new QLineEdit;
    QDoubleValidator *szBPMValidator = new QDoubleValidator(0, 10000, 3, this);
    szBPM->setValidator(szBPMValidator);

    ucIsNew = new QCheckBox(QStringLiteral("ucIsNew"));
    ucIsHot = new QCheckBox(QStringLiteral("ucIsHot"));
    ucIsRecommend = new QCheckBox(QStringLiteral("ucIsRecommend"));
    ucIsOpen = new QCheckBox(QStringLiteral("ucIsOpen"));
    ucCanBuy = new QCheckBox(QStringLiteral("ucCanBuy"));
    bIsFree = new QCheckBox(QStringLiteral("bIsFree"));
    bSongPkg = new QCheckBox(QStringLiteral("bSongPkg"));

    iOrderIndex = new QLineEdit;
    QIntValidator *iOrderIndexValidator = new QIntValidator(0, 100, this);
    iOrderIndex->setValidator(iOrderIndexValidator);
    szFreeBeginTime = new QLineEdit;
    szFreeBeginTime->setPlaceholderText(tr("Better keep empty"));
    szFreeEndTime = new QLineEdit;
    szFreeEndTime->setPlaceholderText(tr("Better keep empty"));

    QIntValidator *hardLevelValidator = new QIntValidator(1, 10, this);
    ush4KeyEasy = new QLineEdit;
    ush4KeyEasy->setValidator(hardLevelValidator);
    ush4KeyNormal = new QLineEdit;
    ush4KeyNormal->setValidator(hardLevelValidator);
    ush4KeyHard = new QLineEdit;
    ush4KeyHard->setValidator(hardLevelValidator);
    ush5KeyEasy = new QLineEdit;
    ush5KeyEasy->setValidator(hardLevelValidator);
    ush5KeyNormal = new QLineEdit;
    ush5KeyNormal->setValidator(hardLevelValidator);
    ush5KeyHard = new QLineEdit;
    ush5KeyHard->setValidator(hardLevelValidator);
    ush6KeyEasy = new QLineEdit;
    ush6KeyEasy->setValidator(hardLevelValidator);
    ush6KeyNormal = new QLineEdit;
    ush6KeyNormal->setValidator(hardLevelValidator);
    ush6KeyHard = new QLineEdit;
    ush6KeyHard->setValidator(hardLevelValidator);

    szNoteNumber = new QLineEdit;
    szNoteNumber->setPlaceholderText(QStringLiteral("4KE,4KN,4KH,5KE,5KN,5KH,6KE,6KN,6KH"));
    szNoteNumber->setInputMask(QStringLiteral("09999,09999,09999,09999,09999,09999,09999,09999,09999"));

    iPrice = new QLineEdit;
    iPrice->setPlaceholderText(tr("Number only, Better keep empty"));
    szProductID = new QLineEdit;
    szProductID->setPlaceholderText(tr("Better keep empty"));
    iVipFlag = new QCheckBox(QStringLiteral("iVipFlag"));

    bIsHide = new QCheckBox(QStringLiteral("bIsHide"));
    bIsReward = new QCheckBox(QStringLiteral("bIsReward"));
    bIsLevelReward = new QCheckBox(QStringLiteral("bIsLevelReward"));
    iVersion = new QLineEdit;
    QIntValidator *iVersionValidator = new QIntValidator(1, 2147483647, this);
    iVersion->setValidator(iVersionValidator);

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
    AR(flayout2, szComposer);
    AR(flayout2, iRegion);
    QFormLayout *flayout3 = new QFormLayout;
    AR(flayout3, szPath);
    AR(flayout3, iGameTime);
    AR(flayout3, iStyle);
    QFormLayout *flayout4 = new QFormLayout;
    AR(flayout4, szArtist);
    AR(flayout4, szSongTime);
    AR(flayout4, szBPM);
    hlayout234->addLayout(flayout2);
    hlayout234->addLayout(flayout3);
    hlayout234->addLayout(flayout4);

    // 5th line...
    QHBoxLayout *hlayout5 = new QHBoxLayout;
    hlayout5->addWidget(ucIsNew);
    hlayout5->addWidget(ucIsHot);
    hlayout5->addWidget(ucIsRecommend);
    hlayout5->addWidget(ucIsOpen);
    hlayout5->addWidget(ucCanBuy);
    hlayout5->addWidget(bIsFree);
    hlayout5->addWidget(bSongPkg);

    // 6th, 7th, 8th, 9th lines...
    QHBoxLayout *hlayout6789 = new QHBoxLayout;
    QFormLayout *flayout5 = new QFormLayout;
    AR(flayout5, iOrderIndex);
    AR(flayout5, ush4KeyEasy);
    AR(flayout5, ush5KeyEasy);
    AR(flayout5, ush6KeyEasy);
    QFormLayout *flayout6 = new QFormLayout;
    AR(flayout6, szFreeBeginTime);
    AR(flayout6, ush4KeyNormal);
    AR(flayout6, ush5KeyNormal);
    AR(flayout6, ush6KeyNormal);
    QFormLayout *flayout7 = new QFormLayout;
    AR(flayout7, szFreeEndTime);
    AR(flayout7, ush4KeyHard);
    AR(flayout7, ush5KeyHard);
    AR(flayout7, ush6KeyHard);
    hlayout6789->addLayout(flayout5);
    hlayout6789->addLayout(flayout6);
    hlayout6789->addLayout(flayout7);

    // 10th line...
    QFormLayout *hlayout10 = new QFormLayout;
    AR(hlayout10, szNoteNumber);

    // 11th line...
    QHBoxLayout *hlayout11 = new QHBoxLayout;
    QFormLayout *flayout8 = new QFormLayout;
    AR(flayout8, iPrice);
    QFormLayout *flayout9 = new QFormLayout;
    AR(flayout9, szProductID);
    hlayout11->addLayout(flayout8);
    hlayout11->addLayout(flayout9);
    hlayout11->addWidget(iVipFlag);

    // 12th line...
    QHBoxLayout *hlayout12 = new QHBoxLayout;
    QFormLayout *flayout10 = new QFormLayout;
    AR(flayout10, iVersion);
    hlayout12->addWidget(bIsHide);
    hlayout12->addWidget(bIsReward);
    hlayout12->addWidget(bIsLevelReward);
    hlayout12->addLayout(flayout10);

    leftLayout->addLayout(hlayout234);
    leftLayout->addLayout(hlayout5);
    leftLayout->addLayout(hlayout6789);
    leftLayout->addLayout(hlayout10);
    leftLayout->addLayout(hlayout11);
    leftLayout->addLayout(hlayout12);
#else
    QVBoxLayout *vlayout = new QVBoxLayout;
    QFormLayout *flayout1 = new QFormLayout;
    AR(flayout1, szSongName);
    AR(flayout1, szPath);
    AR(flayout1, szArtist);
    AR(flayout1, szComposer);
    AR(flayout1, iGameTime);
    AR(flayout1, szSongTime);
    AR(flayout1, iRegion);
    AR(flayout1, iStyle);
    AR(flayout1, szBPM);
    vlayout->addLayout(flayout1);

    vlayout->addWidget(ucIsNew);
    vlayout->addWidget(ucIsHot);
    vlayout->addWidget(ucIsRecommend);
    vlayout->addWidget(ucIsOpen);
    vlayout->addWidget(ucCanBuy);
    vlayout->addWidget(bIsFree);
    vlayout->addWidget(bSongPkg);

    QFormLayout *flayout2 = new QFormLayout;
    AR(flayout2, iOrderIndex);
    AR(flayout2, szFreeBeginTime);
    AR(flayout2, szFreeEndTime);
    AR(flayout2, ush4KeyEasy);
    AR(flayout2, ush4KeyNormal);
    AR(flayout2, ush4KeyHard);
    AR(flayout2, ush5KeyEasy);
    AR(flayout2, ush5KeyNormal);
    AR(flayout2, ush5KeyHard);
    AR(flayout2, ush6KeyEasy);
    AR(flayout2, ush6KeyNormal);
    AR(flayout2, ush6KeyHard);
    AR(flayout2, szNoteNumber);
    AR(flayout2, iPrice);
    AR(flayout2, szProductID);
    vlayout->addLayout(flayout2);

    vlayout->addWidget(iVipFlag);
    vlayout->addWidget(bIsHide);
    vlayout->addWidget(bIsReward);
    vlayout->addWidget(bIsLevelReward);

    QFormLayout *flayout3 = new QFormLayout;
    AR(flayout3, iVersion);

    vlayout->addLayout(flayout3);

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
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SongClientEditDialog::search);

    QPushButton *searchBtn = new QPushButton(tr("Search"));
    searchBtn->setAutoDefault(false);
    searchBtn->setDefault(false);
#ifndef MOBILE_DEVICES
    searchBtn->setMaximumWidth(60);
#else
    searchBtn->setMinimumWidth(120);
    searchBtn->setMaximumWidth(120);
#endif
    connect(searchBtn, &QPushButton::clicked, this, &SongClientEditDialog::search);

    m_searchList = new QListWidget;
    m_searchList->setSortingEnabled(false);
    connect(m_searchList, &QListWidget::itemDoubleClicked, this, &SongClientEditDialog::searchResultDblClicked);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(searchBtn);
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addLayout(searchLayout);
    rightLayout->addWidget(m_searchList);

    QHBoxLayout *alllayout = new QHBoxLayout;
    alllayout->addLayout(leftLayout);
    alllayout->addLayout(rightLayout);

    setLayout(alllayout);
}

bool SongClientEditDialog::reloadFile()
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

bool SongClientEditDialog::loadFile()
{
    QDir d(Downloader::downloadPath());

    QString filepath;
    if (d.exists() && d.exists(QStringLiteral("mrock_song_client_android.bin")))
        filepath = d.absoluteFilePath(QStringLiteral("mrock_song_client_android.bin"));
    else {
        QMessageBox::information(this, tr("RMEssentials"), tr("mrock_song_client_android.bin doesn't exist, please select the file to open."));
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

void SongClientEditDialog::saveFile()
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

void SongClientEditDialog::moveNext()
{
    if (!m_isLoaded)
        return;

    if (m_currentIndex + 1 == m_file.songCount())
        return;

    ++m_currentIndex;
    readCurrent();
}

void SongClientEditDialog::movePrev()
{
    if (!m_isLoaded)
        return;

    if (m_currentIndex <= 0)
        return;

    --m_currentIndex;
    readCurrent();
}

void SongClientEditDialog::readCurrent()
{
    const SongClientItemStruct &c = *(m_file.song(m_currentIndex));

#define RP_NM(p) p->setText(QString::number(c.m_##p))
#define RP_ST(p) p->setText(c.m_##p)
#define RP_BL(p) p->setChecked(c.m_##p)

    RP_NM(ushSongID);
    RP_NM(iVersion);
    RP_ST(szSongName);
    RP_ST(szPath);
    RP_ST(szArtist);
    RP_ST(szComposer);
    RP_ST(szSongTime);
    RP_NM(iGameTime);
    RP_NM(iRegion);
    RP_NM(iStyle);
    RP_ST(szBPM);
    RP_BL(ucIsNew);
    RP_BL(ucIsHot);
    RP_BL(ucIsRecommend);
    RP_BL(ucIsOpen);
    RP_BL(ucCanBuy);
    RP_BL(bIsFree);
    RP_BL(bSongPkg);
    RP_NM(iOrderIndex);
    RP_ST(szFreeBeginTime);
    RP_ST(szFreeEndTime);
    RP_NM(ush4KeyEasy);
    RP_NM(ush4KeyNormal);
    RP_NM(ush4KeyHard);
    RP_NM(ush5KeyEasy);
    RP_NM(ush5KeyNormal);
    RP_NM(ush5KeyHard);
    RP_NM(ush6KeyEasy);
    RP_NM(ush6KeyNormal);
    RP_NM(ush6KeyHard);
    RP_ST(szNoteNumber);
    RP_NM(iPrice);
    RP_ST(szProductID);
    RP_BL(iVipFlag);
    RP_BL(bIsHide);
    RP_BL(bIsReward);
    RP_BL(bIsLevelReward);

#undef RP_BL
#undef RP_ST
#undef RP_NM
}

void SongClientEditDialog::convertToFree()
{
    if (!m_isLoaded)
        return;

    for (int i = 0; i < m_file.songCount(); ++i) {
        SongClientItemStruct *c = m_file.song(i);
        if (!IsLevel(*c)) {
            c->m_ucIsOpen = true;
            c->m_bIsHide = false;
            c->m_bIsLevelReward = false;
            c->m_bIsReward = false;
            c->m_ucCanBuy = false;
            c->m_bIsFree = false;
        }
    }

    readCurrent();
}

void SongClientEditDialog::allSongUnlock()
{
    if (!m_isLoaded)
        return;

    for (int i = 0; i < m_file.songCount(); ++i) {
        SongClientItemStruct *c = m_file.song(i);
        if (!IsLevel(*c))
            c->m_bIsFree = false;
    }

    readCurrent();
}

void SongClientEditDialog::popup()
{
    m_popup->popup(QCursor::pos());
}

void SongClientEditDialog::calculateSongTime()
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

void SongClientEditDialog::saveCurrent()
{
    SongClientItemStruct &c = *(m_file.song(m_currentIndex));

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
    SP_ST(szComposer);
    SP_ST(szSongTime);
    SP_NI(iGameTime);
    SP_NI(iRegion);
    SP_NI(iStyle);
    SP_ST(szBPM);
    SP_BN(ucIsNew);
    SP_BN(ucIsHot);
    SP_BN(ucIsRecommend);
    SP_BN(ucIsOpen);
    SP_BL(ucCanBuy);
    SP_BL(bIsFree);
    SP_BL(bSongPkg);
    SP_NI(iOrderIndex);
    SP_ST(szFreeBeginTime);
    SP_ST(szFreeEndTime);
    SP_NS(ush4KeyEasy);
    SP_NS(ush4KeyNormal);
    SP_NS(ush4KeyHard);
    SP_NS(ush5KeyEasy);
    SP_NS(ush5KeyNormal);
    SP_NS(ush5KeyHard);
    SP_NS(ush6KeyEasy);
    SP_NS(ush6KeyNormal);
    SP_NS(ush6KeyHard);
    SP_ST(szNoteNumber);
    SP_NI(iPrice);
    SP_ST(szProductID);
    SP_BN(iVipFlag);
    SP_BL(bIsHide);
    SP_BL(bIsReward);
    SP_BL(bIsLevelReward);

#undef SP_BL
#undef SP_BN
#undef SP_ST
#undef SP_NI
#undef SP_NS
}

void SongClientEditDialog::search()
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
        SongClientItemStruct *song = m_file.song(n);
        QListWidgetItem *i = new QListWidgetItem(song->m_szSongName);
        i->setData(Qt::UserRole + 1, n);
        m_searchList->addItem(i);
    }
}

void SongClientEditDialog::searchResultDblClicked(QListWidgetItem *index)
{
    if (index == NULL)
        return;

    bool ok = false;
    int i = index->data(Qt::UserRole + 1).toInt(&ok);

    if (!ok)
        return;

    m_currentIndex = i;
    readCurrent();
}

void SongClientEditDialog::mergeSongList()
{
    if (!m_isLoaded)
        return;

    QString filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));

    QFile f(filepath);
    if (!f.exists())
        return;

    SongClientFile *file2 = new SongClientFile;
    if (!file2->readInfoFromDevice(&f, BinFormat)) {
        delete file2;
        return;
    }

    if (QMessageBox::question(this, tr("RMEssentials"), tr("Are you sure to merge the current loaded file to the select file?")) == QMessageBox::No)
        return;

    m_file.mergeSongList(file2);
    m_currentIndex = 0;
    readCurrent();
}

void SongClientEditDialog::prepareForUserMakingNotes()
{
    if (!m_isLoaded)
        return;

    if (QMessageBox::question(this, tr("RMEssentials"), tr("Please be sure that the current open file is the offical one from the server of RM!!!!<br />Are you sure to proceed?")) == QMessageBox::No)
        return;

    for (int i = 0; i < m_file.songCount(); ++i) {
        SongClientItemStruct *c = m_file.song(i);
        if (c->m_ucIsOpen && !c->m_bIsReward && !c->m_bIsHide && !c->m_bIsLevelReward && !c->m_ucCanBuy && !c->m_szSongName.startsWith(QStringLiteral("【限时】"))) {
            // I have also been drunk.... We must use Chinese here, so I add UTF-8 BOM to this file otherwise it will cause a messed encoding in MSVC.
            c->m_szComposer = QStringLiteral("Offical Free Song");
            c->m_iOrderIndex = 1;
        } else {
            c->m_szComposer = QStringLiteral("Offical Non-free Song");
            c->m_iOrderIndex = 0;
        }
    }

    readCurrent();
}
