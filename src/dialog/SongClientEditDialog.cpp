#include "SongClientEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QDoubleValidator>
#include <QLabel>
#include <QListWidget>

#include <QMenu>

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

using namespace RMSong;

SongClientEditDialog::SongClientEditDialog(QWidget *parent)
    : QDialog(parent), m_currentIndex(-1), m_isLoaded(false)
{
    setWindowTitle(tr("Rhythm Master Song Client Editor"));

    QVBoxLayout *leftLayout = new QVBoxLayout;

    // for QFormLayout
#define AR(l, x) l->addRow(#x, x)

    // 1st line
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    QFormLayout *flayout1 = new QFormLayout;
    ushSongID = new QLineEdit;
    ushSongID->setReadOnly(true);
    AR(flayout1, ushSongID);

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
    m_popup = new QMenu(tr("Functions..."), this);
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
    QPushButton *funcBtn = new QPushButton(tr("Functions..."));
    funcBtn->setAutoDefault(false);
    funcBtn->setDefault(false);
    connect(funcBtn, &QPushButton::clicked, this, &SongClientEditDialog::popup);
    hlayout1->addLayout(flayout1);
    hlayout1->addWidget(prevBtn);
    hlayout1->addWidget(nextBtn);
    hlayout1->addWidget(saveCurrentBtn);
    hlayout1->addWidget(funcBtn);


    // 2nd, 3rd, 4th lines...
    QHBoxLayout *hlayout234 = new QHBoxLayout;
    QFormLayout *flayout2 = new QFormLayout;
    szSongName = new QLineEdit;
    szComposer = new QLineEdit;
    iRegion = new QLineEdit;
    QIntValidator *iRegionValidator = new QIntValidator(0, 11, this);
    iRegion->setValidator(iRegionValidator);
    AR(flayout2, szSongName);
    AR(flayout2, szComposer);
    AR(flayout2, iRegion);
    QFormLayout *flayout3 = new QFormLayout;
    szPath = new QLineEdit;
    QRegExpValidator *szPathValidator = new QRegExpValidator(QRegExp("[0-9a-z_]+"), this);
    szPath->setValidator(szPathValidator);
    iGameTime = new QLineEdit;
    QIntValidator *iGameTimeValidator = new QIntValidator(1, 2147483647, this);
    iGameTime->setValidator(iGameTimeValidator);
    connect(iGameTime, &QLineEdit::textEdited, this, &SongClientEditDialog::calculateSongTime);
    iStyle = new QLineEdit;
    QIntValidator *iStyleValidator = new QIntValidator(0, 20, this);
    iStyle->setValidator(iStyleValidator);
    AR(flayout3, szPath);
    AR(flayout3, iGameTime);
    AR(flayout3, iStyle);
    QFormLayout *flayout4 = new QFormLayout;
    szArtist = new QLineEdit;
    szSongTime = new QLabel;
    szBPM = new QLineEdit;
    QDoubleValidator *szBPMValidator = new QDoubleValidator(0, 10000, 3, this);
    szBPM->setValidator(szBPMValidator);
    AR(flayout4, szArtist);
    AR(flayout4, szSongTime);
    AR(flayout4, szBPM);
    hlayout234->addLayout(flayout2);
    hlayout234->addLayout(flayout3);
    hlayout234->addLayout(flayout4);

    // 5th line...
    QHBoxLayout *hlayout5 = new QHBoxLayout;
    ucIsNew = new QCheckBox("ucIsNew");
    ucIsHot = new QCheckBox("ucIsHot");
    ucIsRecommend = new QCheckBox("ucIsRecommend");
    ucIsOpen = new QCheckBox("ucIsOpen");
    ucCanBuy = new QCheckBox("ucCanBuy");
    bIsFree = new QCheckBox("bIsFree");
    bSongPkg = new QCheckBox("bSongPkg");
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
    iOrderIndex = new QLineEdit;
    QIntValidator *iOrderIndexValidator = new QIntValidator(0, 100, this);
    iOrderIndex->setValidator(iOrderIndexValidator);
    QIntValidator *hardLevelValidater = new QIntValidator(1, 10, this);
    ush4KeyEasy = new QLineEdit;
    ush4KeyEasy->setValidator(hardLevelValidater);
    ush5KeyEasy = new QLineEdit;
    ush5KeyEasy->setValidator(hardLevelValidater);
    ush6KeyEasy = new QLineEdit;
    ush6KeyEasy->setValidator(hardLevelValidater);
    AR(flayout5, iOrderIndex);
    AR(flayout5, ush4KeyEasy);
    AR(flayout5, ush5KeyEasy);
    AR(flayout5, ush6KeyEasy);
    QFormLayout *flayout6 = new QFormLayout;
    szFreeBeginTime = new QLineEdit;
    szFreeBeginTime->setPlaceholderText(tr("Better keep empty"));
    ush4KeyNormal = new QLineEdit;
    ush4KeyNormal->setValidator(hardLevelValidater);
    ush5KeyNormal = new QLineEdit;
    ush5KeyNormal->setValidator(hardLevelValidater);
    ush6KeyNormal = new QLineEdit;
    ush6KeyNormal->setValidator(hardLevelValidater);
    AR(flayout6, szFreeBeginTime);
    AR(flayout6, ush4KeyNormal);
    AR(flayout6, ush5KeyNormal);
    AR(flayout6, ush6KeyNormal);
    QFormLayout *flayout7 = new QFormLayout;
    szFreeEndTime = new QLineEdit;
    szFreeEndTime->setPlaceholderText(tr("Better keep empty"));
    ush4KeyHard = new QLineEdit;
    ush4KeyHard->setValidator(hardLevelValidater);
    ush5KeyHard = new QLineEdit;
    ush5KeyHard->setValidator(hardLevelValidater);
    ush6KeyHard = new QLineEdit;
    ush6KeyHard->setValidator(hardLevelValidater);
    AR(flayout7, szFreeEndTime);
    AR(flayout7, ush4KeyHard);
    AR(flayout7, ush5KeyHard);
    AR(flayout7, ush6KeyHard);
    hlayout6789->addLayout(flayout5);
    hlayout6789->addLayout(flayout6);
    hlayout6789->addLayout(flayout7);

    // 10th line...
    QFormLayout *hlayout10 = new QFormLayout;
    szNoteNumber = new QLineEdit;
    szNoteNumber->setPlaceholderText("4KE,4KN,4KH,5KE,5KN,5KH,6KE,6KN,6KH");
    szNoteNumber->setInputMask("09999,09999,09999,09999,09999,09999,09999,09999,09999");
    AR(hlayout10, szNoteNumber);

    // 11th line...
    QHBoxLayout *hlayout11 = new QHBoxLayout;
    QFormLayout *flayout8 = new QFormLayout;
    iPrice = new QLineEdit;
    iPrice->setPlaceholderText(tr("Number only, Better keep empty"));
    AR(flayout8, iPrice);
    QFormLayout *flayout9 = new QFormLayout;
    szProductID = new QLineEdit;
    szProductID->setPlaceholderText(tr("Better keep empty"));
    AR(flayout9, szProductID);
    iVipFlag = new QCheckBox("iVipFlag");
    hlayout11->addLayout(flayout8);
    hlayout11->addLayout(flayout9);
    hlayout11->addWidget(iVipFlag);

    // 12th line...
    QHBoxLayout *hlayout12 = new QHBoxLayout;
    bIsHide = new QCheckBox("bIsHide");
    bIsReward = new QCheckBox("bIsReward");
    bIsLevelReward = new QCheckBox("bIsLevelReward");
    QFormLayout *flayout10 = new QFormLayout;
    iVersion = new QLineEdit;
    QIntValidator *iVersionValidator = new QIntValidator(1, 2147483647, this);
    iVersion->setValidator(iVersionValidator);
    AR(flayout10, iVersion);
    hlayout12->addWidget(bIsHide);
    hlayout12->addWidget(bIsReward);
    hlayout12->addWidget(bIsLevelReward);
    hlayout12->addLayout(flayout10);

    // OK, thank you
#undef AR

    leftLayout->addLayout(hlayout1);
    leftLayout->addLayout(hlayout234);
    leftLayout->addLayout(hlayout5);
    leftLayout->addLayout(hlayout6789);
    leftLayout->addLayout(hlayout10);
    leftLayout->addLayout(hlayout11);
    leftLayout->addLayout(hlayout12);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->setMinimumWidth(80);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SongClientEditDialog::search);

    QPushButton *searchBtn = new QPushButton(tr("Search"));
    searchBtn->setAutoDefault(false);
    searchBtn->setDefault(false);
    searchBtn->setMaximumWidth(60);
    connect(searchBtn, &QPushButton::clicked, this, &SongClientEditDialog::search);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(searchBtn);

    m_searchList = new QListWidget;
    m_searchList->setSortingEnabled(false);
    connect(m_searchList, &QListWidget::itemDoubleClicked, this, &SongClientEditDialog::searchResultDblClicked);
    
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addLayout(searchLayout);
    rightLayout->addWidget(m_searchList);

    QHBoxLayout *alllayout = new QHBoxLayout;
    alllayout->addLayout(leftLayout);
    alllayout->addLayout(rightLayout);

    setLayout(alllayout);
}

SongClientEditDialog::~SongClientEditDialog()
{
}

bool SongClientEditDialog::reloadFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");

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
#ifndef Q_OS_ANDROID
    QDir d("downloader");
#else
    QDir d("/sdcard/RM/res");
#endif
    QString filepath;
    if (d.exists() && d.exists("mrock_song_client_android.bin"))
        filepath = d.absoluteFilePath("mrock_song_client_android.bin");
    else {
        QMessageBox::information(this, tr("RMEssentials"), tr("mrock_song_client_android.bin doesn't exist, please select the file to open."));
        filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");
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

    QString filepath = QFileDialog::getSaveFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");
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

#define RP_NM(p) p->setText(QString::number(c.m_ ## p))
#define RP_ST(p) p->setText(c.m_ ## p)
#define RP_BL(p) p->setChecked(c.m_ ## p)

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

#define SP_NS(p) c.m_ ## p = p->text().toShort()
#define SP_NI(p) c.m_ ## p = p->text().toInt()
#define SP_ST(p) c.m_ ## p = p->text()
#define SP_BN(p) c.m_ ## p = (p->isChecked() ? 1 : 0)
#define SP_BL(p) c.m_ ## p = p->isChecked()

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

    QString filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");

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