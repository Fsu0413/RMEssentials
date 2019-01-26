#include "papasongclientedit.h"
#include "pastelineedit.h"

#include <RMEss/RmeDownloader>
#include <RMEss/RmeSongClientStruct>
#include <RMEss/RmeUtils>

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

struct PapaSongClientEditDialogControls
{
    QLineEdit *ushSongID; // Readonly, User Making note better > 10800
    QLineEdit *iVersion; // what's this?
    PasteLineEdit *szSongName;
    PasteLineEdit *szArtist;
    QLineEdit *cDifficulty;
    QLineEdit *cLevel;
    PasteLineEdit *szPath;
    QLabel *szSongTime; // Auto Generate
    QLineEdit *iGameTime; // Number only
    QLineEdit *szRegion;
    QLineEdit *szStyle;
    QLineEdit *szBPM; // Number only
    QLineEdit *szNoteNumber; // Number only
    QLineEdit *iOrderIndex; // Number only
    QCheckBox *ucIsOpen;
    QCheckBox *ucIsFree;
    QCheckBox *ucIsHide;
    QCheckBox *ucIsReward;
    QCheckBox *ucIsLevelReward;
    QLineEdit *iSongType; // better keep empty
};

PapaSongClientEditDialog::PapaSongClientEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentIndex(-1)
    , m_isLoaded(false)
    , m_isContentEdited(false)
    , m_controls(new PapaSongClientEditDialogControls)
{
    setWindowTitle(tr("Rhythm Master PapaSong Client Editor"));

    m_controls->ushSongID = new QLineEdit;
    m_controls->ushSongID->setReadOnly(true);
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
    m_popup->addSeparator();
    QAction *cp = m_popup->addAction(tr("Create Patch from another file base"));
    connect(cp, &QAction::triggered, this, &PapaSongClientEditDialog::createPatch);
    QAction *ap = m_popup->addAction(tr("Apply Patch File"));
    connect(ap, &QAction::triggered, this, &PapaSongClientEditDialog::applyPatch);
    QPushButton *funcBtn = new QPushButton(tr("Functions..."));
    funcBtn->setAutoDefault(false);
    funcBtn->setDefault(false);
    funcBtn->setMenu(m_popup);

    QVBoxLayout *leftLayout = new QVBoxLayout;

    m_controls->szSongName = new PasteLineEdit;
    connect(m_controls->szSongName, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->szNoteNumber = new QLineEdit;
    connect(m_controls->szNoteNumber, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->szRegion = new QLineEdit;
    connect(m_controls->szRegion, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->iOrderIndex = new QLineEdit;
    connect(m_controls->iOrderIndex, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    QIntValidator *iOrderIndexValidator = new QIntValidator(0, 100, this);
    m_controls->iOrderIndex->setValidator(iOrderIndexValidator);
    m_controls->szPath = new PasteLineEdit;
    connect(m_controls->szPath, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    QRegExpValidator *szPathValidator = new QRegExpValidator(QRegExp(QStringLiteral("[0-9a-z_]+")), this);
    m_controls->szPath->setValidator(szPathValidator);
    m_controls->iGameTime = new QLineEdit;
    connect(m_controls->iGameTime, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    QIntValidator *iGameTimeValidator = new QIntValidator(1, 2147483647, this);
    m_controls->iGameTime->setValidator(iGameTimeValidator);
    connect(m_controls->iGameTime, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::calculateSongTime);
    m_controls->szStyle = new QLineEdit;
    connect(m_controls->szStyle, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->iSongType = new QLineEdit;
    connect(m_controls->iSongType, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->iSongType->setValidator(iGameTimeValidator);
    m_controls->szArtist = new PasteLineEdit;
    connect(m_controls->szArtist, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->szSongTime = new QLabel;
    m_controls->szBPM = new QLineEdit;
    connect(m_controls->szBPM, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    QDoubleValidator *szBPMValidator = new QDoubleValidator(0, 10000, 3, this);
    m_controls->szBPM->setValidator(szBPMValidator);
    m_controls->iVersion = new QLineEdit;
    QIntValidator *iVersionValidator = new QIntValidator(1, 2147483647, this);
    m_controls->iVersion->setValidator(iVersionValidator);
    m_controls->cDifficulty = new QLineEdit;
    connect(m_controls->cDifficulty, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    QIntValidator *cDifficultyValidator = new QIntValidator(1, 3, this);
    m_controls->cDifficulty->setValidator(cDifficultyValidator);
    m_controls->cLevel = new QLineEdit;
    connect(m_controls->cLevel, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::contentEdited);
    QIntValidator *cLevelValidator = new QIntValidator(1, 10, this);
    m_controls->cLevel->setValidator(cLevelValidator);

    m_controls->ucIsHide = new QCheckBox(QStringLiteral("ucIsHide"));
    connect(m_controls->ucIsHide, &QCheckBox::stateChanged, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->ucIsReward = new QCheckBox(QStringLiteral("ucIsReward"));
    connect(m_controls->ucIsReward, &QCheckBox::stateChanged, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->ucIsLevelReward = new QCheckBox(QStringLiteral("ucIsLevelReward"));
    connect(m_controls->ucIsLevelReward, &QCheckBox::stateChanged, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->ucIsOpen = new QCheckBox(QStringLiteral("ucIsOpen"));
    connect(m_controls->ucIsOpen, &QCheckBox::stateChanged, this, &PapaSongClientEditDialog::contentEdited);
    m_controls->ucIsFree = new QCheckBox(QStringLiteral("ucIsFree"));
    connect(m_controls->ucIsFree, &QCheckBox::stateChanged, this, &PapaSongClientEditDialog::contentEdited);

// for QFormLayout
#define AR(l, x) l->addRow(QStringLiteral(#x), m_controls->x)

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

    hlayout12->addWidget(m_controls->ucIsOpen);
    hlayout12->addWidget(m_controls->ucIsFree);
    hlayout12->addWidget(m_controls->ucIsHide);
    hlayout12->addWidget(m_controls->ucIsReward);
    hlayout12->addWidget(m_controls->ucIsLevelReward);

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

    vlayout->addWidget(m_controls->ucIsOpen);
    vlayout->addWidget(m_controls->ucIsFree);
    vlayout->addWidget(m_controls->ucIsHide);
    vlayout->addWidget(m_controls->ucIsReward);
    vlayout->addWidget(m_controls->ucIsLevelReward);

    QWidget *widget = new QWidget;
    widget->setLayout(vlayout);

    QScrollArea *area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setWidget(widget);

    leftLayout->addWidget(area);
#endif

// OK, thank you
#undef AR

    QString searchText = tr("Search");

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(searchText);
    m_searchEdit->setMinimumWidth(m_searchEdit->fontMetrics().width(searchText) * 2);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PapaSongClientEditDialog::search);

    QPushButton *searchBtn = new QPushButton(searchText);
    searchBtn->setAutoDefault(false);
    searchBtn->setDefault(false);
    searchBtn->setFixedWidth(searchBtn->fontMetrics().width(searchText) * 1.7);
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

PapaSongClientEditDialog::~PapaSongClientEditDialog()
{
    delete m_controls;
}

bool PapaSongClientEditDialog::reloadFile()
{
    QString filepath
        = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));

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
    QDir d(RmeDownloader::binDownloadPath());

    QString filepath;
    if (d.exists() && d.exists(QStringLiteral("mrock_papasong_client.bin")))
        filepath = d.absoluteFilePath(QStringLiteral("mrock_papasong_client.bin"));
    else {
        QMessageBox::information(this, tr("RMEssentials"), tr("mrock_papasong_client.bin doesn't exist, please select the file to open."));
        filepath
            = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));
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

bool PapaSongClientEditDialog::askForSaveModified()
{
    if (!m_isContentEdited)
        return true;

    int r = QMessageBox::question(this, tr("RMEssentials"), tr("Content of this page is modified."), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (r == QMessageBox::Save) {
        saveCurrent();
        return true;
    } else if (r == QMessageBox::Discard)
        return true;

    return false;
}

void PapaSongClientEditDialog::saveFile()
{
    if (!m_isLoaded)
        return;

    if (!askForSaveModified())
        return;

    QString filepath
        = QFileDialog::getSaveFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));
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

    if (!askForSaveModified())
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

    if (!askForSaveModified())
        return;

    if (m_currentIndex <= 0)
        return;

    --m_currentIndex;
    readCurrent();
}

void PapaSongClientEditDialog::readCurrent()
{
    const RmePapaSongClientItemStruct &c = *(m_file.song(m_currentIndex));

#define RP_NM(p) m_controls->p->setText(QString::number(c.m_##p))
#define RP_ST(p) m_controls->p->setText(c.m_##p)
#define RP_BL(p) m_controls->p->setChecked(c.m_##p)

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

void PapaSongClientEditDialog::calculateSongTime()
{
    int gameTime = m_controls->iGameTime->text().toInt();
    m_controls->szSongTime->setText(RmeUtils::calculateSongTime(gameTime));
}

void PapaSongClientEditDialog::saveCurrent()
{
    RmePapaSongClientItemStruct &c = *(m_file.song(m_currentIndex));

#define SP_NS(p) c.m_##p = m_controls->p->text().toShort()
#define SP_NI(p) c.m_##p = m_controls->p->text().toInt()
#define SP_ST(p) c.m_##p = m_controls->p->text()
#define SP_BN(p) c.m_##p = (m_controls->p->isChecked() ? 1 : 0)
#define SP_BL(p) c.m_##p = m_controls->p->isChecked()

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

    foreach (int n, searchResult) {
        RmePapaSongClientItemStruct *song = m_file.song(n);
        QListWidgetItem *i = new QListWidgetItem(song->m_szSongName);
        i->setData(Qt::UserRole + 1, n);
        m_searchList->addItem(i);
    }

    if (searchResult.length() == 1) {
        if (!askForSaveModified())
            return;

        m_currentIndex = searchResult.first();
        readCurrent();
    }
}

void PapaSongClientEditDialog::searchResultDblClicked(QListWidgetItem *index)
{
    if (!m_isLoaded)
        return;

    if (index == nullptr)
        return;

    if (!askForSaveModified())
        return;

    bool ok = false;
    int i = index->data(Qt::UserRole + 1).toInt(&ok);

    if (!ok)
        return;

    m_currentIndex = i;
    readCurrent();
}

void PapaSongClientEditDialog::createPatch()
{
    if (!m_isLoaded)
        return;

    if (!askForSaveModified())
        return;

    QString filepath
        = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + QStringLiteral(" (*.bin)"));

    QFile f(filepath);
    if (!f.exists())
        return;

    RmePapaSongClientFile file2;
    if (!file2.readInfoFromDevice(&f, BinFormat)) {
        QMessageBox::warning(this, tr("RMEssentials"), tr("Load info from file failed."));
        return;
    }

    QString filepathToSave
        = QFileDialog::getSaveFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Json files") + QStringLiteral(" (*.json)"));

    QFile f2(filepathToSave);
    if (!m_file.savePatchToDevice(&f2, file2)) {
        QMessageBox::warning(this, tr("RMEssentials"), tr("Save file failed"));
        return;
    }
}

void PapaSongClientEditDialog::applyPatch()
{
    if (!m_isLoaded)
        return;

    if (!askForSaveModified())
        return;

    QString filepath
        = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("Json files") + QStringLiteral(" (*.json)"));
    QFile f(filepath);

    if (!f.exists())
        return;

    if (!m_file.applyPatchFromDevice(&f)) {
        QMessageBox::warning(this, tr("RMEssentials"), tr("Apply patch from device failed"));
        return;
    }
    readCurrent();
}

void PapaSongClientEditDialog::contentEdited()
{
    m_isContentEdited = true;
}

void PapaSongClientEditDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

    loadFile();
}
