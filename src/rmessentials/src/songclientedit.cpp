#include "songclientedit.h"

#include <RmEss/RmeDownloader>
#include <RmEss/RmeSongClientStruct>
#include <RmEss/RmeSongFile>
#include <RmEss/RmeUtils>

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
#include <QRegularExpressionValidator>
#include <QStandardPaths>
#include <QVBoxLayout>

#ifdef MOBILE_DEVICES
#include <QScrollArea>
#endif

namespace {
RmeFileFormat getOpenFileName(QWidget *parent, QString &filepath)
{
    QString selectedFilter;
    filepath = QFileDialog::getOpenFileName(parent, SongClientEditDialog::tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                            SongClientEditDialog::tr("Json files") + QStringLiteral(" (*.json)") + QStringLiteral(";;") + SongClientEditDialog::tr("bin files")
                                                + QStringLiteral(" (*.bin)") + QStringLiteral(";;") + SongClientEditDialog::tr("xml files") + QStringLiteral(" (*.xml)"),
                                            &selectedFilter);
    if (filepath.isNull())
        return UnknownFormat;

    if (selectedFilter.contains(QStringLiteral(".json")))
        return JsonFormat;
    if (selectedFilter.contains(QStringLiteral(".bin")))
        return BinFormat;
    if (selectedFilter.contains(QStringLiteral(".xml")))
        return XmlFormat;

    return UnknownFormat;
}

RmeFileFormat getSaveFileName(QWidget *parent, QString &filepath)
{
    QString selectedFilter;
    filepath = QFileDialog::getSaveFileName(parent, SongClientEditDialog::tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
                                            SongClientEditDialog::tr("Json files") + QStringLiteral(" (*.json)") + QStringLiteral(";;") + SongClientEditDialog::tr("bin files")
                                                + QStringLiteral(" (*.bin)") + QStringLiteral(";;") + SongClientEditDialog::tr("xml files") + QStringLiteral(" (*.xml)"),
                                            &selectedFilter);
    if (filepath.isNull())
        return UnknownFormat;

    if (selectedFilter.contains(QStringLiteral(".json")))
        return JsonFormat;
    if (selectedFilter.contains(QStringLiteral(".bin")))
        return BinFormat;
    if (selectedFilter.contains(QStringLiteral(".xml")))
        return XmlFormat;

    return UnknownFormat;
}
}

struct SongClientEditDialogControls
{
    QLineEdit *ushSongID; // Readonly, User Making note better > 800
    QLineEdit *iVersion; // what's this?
    QLineEdit *szSongName;
    QLineEdit *szPath;
    QLineEdit *szArtist;
    QLineEdit *szComposer;
    QCheckBox *szSongTime; // Auto Generate
    QLineEdit *iGameTime; // Number only
    QLineEdit *iRegion; // QComboBox?
    QLineEdit *iStyle; // QComboBox?
    QCheckBox *ucIsNew;
    QCheckBox *ucIsHot;
    QCheckBox *ucIsRecommend;
    QLineEdit *szBPM; // Number only
    QCheckBox *ucIsOpen;
    QCheckBox *ucCanBuy;
    QLineEdit *iOrderIndex; // Number only
    QCheckBox *bIsFree;
    QCheckBox *bSongPkg;
    QLineEdit *szFreeBeginTime; // better keep empty
    QLineEdit *szFreeEndTime; // better keep empty
    QLineEdit *ush4KeyEasy; // Number only
    QLineEdit *ush4KeyNormal; // Number only
    QLineEdit *ush4KeyHard; // Number only
    QLineEdit *ush5KeyEasy; // Number only
    QLineEdit *ush5KeyNormal; // Number only
    QLineEdit *ush5KeyHard; // Number only
    QLineEdit *ush6KeyEasy; // Number only
    QLineEdit *ush6KeyNormal; // Number only
    QLineEdit *ush6KeyHard; // Number only
    QLineEdit *iPrice; // Number only
    QLineEdit *szNoteNumber; // Number only
    QLineEdit *szProductID; // better keep empty
    QCheckBox *iVipFlag;
    QCheckBox *bIsHide;
    QCheckBox *bIsReward;
    QCheckBox *bIsLevelReward;
};

SongClientEditDialog::SongClientEditDialog(QWidget *parent)
    : QDialog(parent)
    , m_currentIndex(-1)
    , m_isLoaded(false)
    , m_isContentEdited(false)
    , m_controls(new SongClientEditDialogControls)
{
    setWindowTitle(tr("Rhythm Master Song Client Editor"));

    QVBoxLayout *leftLayout = new QVBoxLayout;

    m_controls->ushSongID = new QLineEdit;
    m_controls->ushSongID->setReadOnly(true);
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
    QMenu *popup = new QMenu(this);
    QAction *openFileBtn = popup->addAction(tr("open an other file"));
    connect(openFileBtn, &QAction::triggered, this, &SongClientEditDialog::reloadFile);
    QAction *saveFileBtn = popup->addAction(tr("save file"));
    connect(saveFileBtn, &QAction::triggered, this, &SongClientEditDialog::saveFile);
    popup->addSeparator();
    QAction *cp = popup->addAction(tr("Create Patch from another file base"));
    connect(cp, &QAction::triggered, this, &SongClientEditDialog::createPatch);
    QAction *ap = popup->addAction(tr("Apply Patch File"));
    connect(ap, &QAction::triggered, this, &SongClientEditDialog::applyPatch);
    popup->addSeparator();
    QAction *createWiki = popup->addAction(tr("Create WikiTable"));
    connect(createWiki, &QAction::triggered, this, &SongClientEditDialog::saveWikiTable);
    QPushButton *funcBtn = new QPushButton(tr("Functions..."));
    funcBtn->setAutoDefault(false);
    funcBtn->setDefault(false);
    funcBtn->setMenu(popup);

    m_controls->szSongName = new QLineEdit;
    connect(m_controls->szSongName, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->szPath = new QLineEdit;
    connect(m_controls->szPath, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    QRegularExpressionValidator *szPathValidator = new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[0-9a-z_]+")), this);
    m_controls->szPath->setValidator(szPathValidator);
    m_controls->szArtist = new QLineEdit;
    connect(m_controls->szArtist, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->szComposer = new QLineEdit;
    connect(m_controls->szComposer, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->iGameTime = new QLineEdit;
    connect(m_controls->iGameTime, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    QIntValidator *iGameTimeValidator = new QIntValidator(1, 2147483647, this);
    m_controls->iGameTime->setValidator(iGameTimeValidator);
    connect(m_controls->iGameTime, &QLineEdit::textEdited, this, &SongClientEditDialog::calculateSongTime);
    m_controls->szSongTime = new QCheckBox;
    connect(m_controls->szSongTime, &QCheckBox::toggled, this, &SongClientEditDialog::calculateSongTime);
    connect(m_controls->szSongTime, &QCheckBox::toggled, this, &SongClientEditDialog::contentEdited);
    m_controls->iRegion = new QLineEdit;
    connect(m_controls->iRegion, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    QIntValidator *iRegionValidator = new QIntValidator(0, 2147483647, this);
    m_controls->iRegion->setValidator(iRegionValidator);
    m_controls->iStyle = new QLineEdit;
    connect(m_controls->iStyle, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    QIntValidator *iStyleValidator = new QIntValidator(0, 2147483647, this);
    m_controls->iStyle->setValidator(iStyleValidator);
    m_controls->szBPM = new QLineEdit;
    connect(m_controls->szBPM, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    QDoubleValidator *szBPMValidator = new QDoubleValidator(1., 625., 1, this);
    m_controls->szBPM->setValidator(szBPMValidator);

    m_controls->ucIsNew = new QCheckBox(QStringLiteral("ucIsNew"));
    connect(m_controls->ucIsNew, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->ucIsHot = new QCheckBox(QStringLiteral("ucIsHot"));
    connect(m_controls->ucIsHot, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->ucIsRecommend = new QCheckBox(QStringLiteral("ucIsRecommend"));
    connect(m_controls->ucIsRecommend, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->ucIsOpen = new QCheckBox(QStringLiteral("ucIsOpen"));
    connect(m_controls->ucIsOpen, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->ucCanBuy = new QCheckBox(QStringLiteral("ucCanBuy"));
    connect(m_controls->ucCanBuy, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->bIsFree = new QCheckBox(QStringLiteral("bIsFree"));
    connect(m_controls->bIsFree, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->bSongPkg = new QCheckBox(QStringLiteral("bSongPkg"));
    connect(m_controls->bSongPkg, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);

    m_controls->iOrderIndex = new QLineEdit;
    connect(m_controls->iOrderIndex, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    QIntValidator *iOrderIndexValidator = new QIntValidator(0, 100, this);
    m_controls->iOrderIndex->setValidator(iOrderIndexValidator);
    m_controls->szFreeBeginTime = new QLineEdit;
    m_controls->szFreeBeginTime->setPlaceholderText(tr("Better keep empty"));
    m_controls->szFreeEndTime = new QLineEdit;
    m_controls->szFreeEndTime->setPlaceholderText(tr("Better keep empty"));

    QIntValidator *hardLevelValidator = new QIntValidator(1, 10, this);
    m_controls->ush4KeyEasy = new QLineEdit;
    connect(m_controls->ush4KeyEasy, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush4KeyEasy->setValidator(hardLevelValidator);
    m_controls->ush4KeyNormal = new QLineEdit;
    connect(m_controls->ush4KeyNormal, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush4KeyNormal->setValidator(hardLevelValidator);
    m_controls->ush4KeyHard = new QLineEdit;
    connect(m_controls->ush4KeyHard, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush4KeyHard->setValidator(hardLevelValidator);
    m_controls->ush5KeyEasy = new QLineEdit;
    connect(m_controls->ush5KeyEasy, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush5KeyEasy->setValidator(hardLevelValidator);
    m_controls->ush5KeyNormal = new QLineEdit;
    connect(m_controls->ush5KeyNormal, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush5KeyNormal->setValidator(hardLevelValidator);
    m_controls->ush5KeyHard = new QLineEdit;
    connect(m_controls->ush5KeyHard, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush5KeyHard->setValidator(hardLevelValidator);
    m_controls->ush6KeyEasy = new QLineEdit;
    connect(m_controls->ush6KeyEasy, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush6KeyEasy->setValidator(hardLevelValidator);
    m_controls->ush6KeyNormal = new QLineEdit;
    connect(m_controls->ush6KeyNormal, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush6KeyNormal->setValidator(hardLevelValidator);
    m_controls->ush6KeyHard = new QLineEdit;
    connect(m_controls->ush6KeyHard, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->ush6KeyHard->setValidator(hardLevelValidator);

    m_controls->szNoteNumber = new QLineEdit;
    connect(m_controls->szNoteNumber, &QLineEdit::textEdited, this, &SongClientEditDialog::contentEdited);
    m_controls->szNoteNumber->setPlaceholderText(QStringLiteral("4KE,4KN,4KH,5KE,5KN,5KH,6KE,6KN,6KH"));
    m_controls->szNoteNumber->setInputMask(QStringLiteral("00009,00009,00009,00009,00009,00009,00009,00009,00009;_"));

    m_controls->iPrice = new QLineEdit;
    m_controls->iPrice->setPlaceholderText(tr("Number only, Better keep empty"));
    m_controls->szProductID = new QLineEdit;
    m_controls->szProductID->setPlaceholderText(tr("Better keep empty"));
    m_controls->iVipFlag = new QCheckBox(QStringLiteral("iVipFlag"));

    m_controls->bIsHide = new QCheckBox(QStringLiteral("bIsHide"));
    connect(m_controls->bIsHide, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->bIsReward = new QCheckBox(QStringLiteral("bIsReward"));
    connect(m_controls->bIsReward, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->bIsLevelReward = new QCheckBox(QStringLiteral("bIsLevelReward"));
    connect(m_controls->bIsLevelReward, &QCheckBox::stateChanged, this, &SongClientEditDialog::contentEdited);
    m_controls->iVersion = new QLineEdit;
    QIntValidator *iVersionValidator = new QIntValidator(1, 2147483647, this);
    m_controls->iVersion->setValidator(iVersionValidator);

    // 1st line
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addWidget(new QLabel(QStringLiteral("ushSongID")));
    hlayout1->addWidget(m_controls->ushSongID);
    hlayout1->addWidget(prevBtn);
    hlayout1->addWidget(nextBtn);
    hlayout1->addWidget(saveCurrentBtn);
    hlayout1->addWidget(funcBtn);

    leftLayout->addLayout(hlayout1);

#ifndef MOBILE_DEVICES

    QGridLayout *glayout = new QGridLayout;

    // 2nd line...
    glayout->addWidget(new QLabel(QStringLiteral("szSongName")), 0, 0, 1, 7);
    glayout->addWidget(m_controls->szSongName, 0, 7, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("szPath")), 0, 35, 1, 7);
    glayout->addWidget(m_controls->szPath, 0, 42, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("szArtist")), 0, 70, 1, 7);
    glayout->addWidget(m_controls->szArtist, 0, 77, 1, 28);

    // 3rd line...
    glayout->addWidget(new QLabel(QStringLiteral("szComposer")), 1, 0, 1, 7);
    glayout->addWidget(m_controls->szComposer, 1, 7, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("iGameTime")), 1, 35, 1, 7);
    glayout->addWidget(m_controls->iGameTime, 1, 42, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("szSongTime")), 1, 70, 1, 7);
    glayout->addWidget(m_controls->szSongTime, 1, 77, 1, 28);

    // 4th line...
    glayout->addWidget(new QLabel(QStringLiteral("iRegion")), 2, 0, 1, 7);
    glayout->addWidget(m_controls->iRegion, 2, 7, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("iStyle")), 2, 35, 1, 7);
    glayout->addWidget(m_controls->iStyle, 2, 42, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("szBPM")), 2, 70, 1, 7);
    glayout->addWidget(m_controls->szBPM, 2, 77, 1, 28);

    // 5th line...
    glayout->addWidget(m_controls->ucIsNew, 3, 0, 1, 15);
    glayout->addWidget(m_controls->ucIsHot, 3, 15, 1, 15);
    glayout->addWidget(m_controls->ucIsRecommend, 3, 30, 1, 15);
    glayout->addWidget(m_controls->ucIsOpen, 3, 45, 1, 15);
    glayout->addWidget(m_controls->ucCanBuy, 3, 60, 1, 15);
    glayout->addWidget(m_controls->bIsFree, 3, 75, 1, 15);
    glayout->addWidget(m_controls->bSongPkg, 3, 90, 1, 15);

    // 6th line...
    glayout->addWidget(new QLabel(QStringLiteral("iOrderIndex")), 4, 0, 1, 7);
    glayout->addWidget(m_controls->iOrderIndex, 4, 7, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("szFreeBeginTime")), 4, 35, 1, 7);
    glayout->addWidget(m_controls->szFreeBeginTime, 4, 42, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("szFreeEndTime")), 4, 70, 1, 7);
    glayout->addWidget(m_controls->szFreeEndTime, 4, 77, 1, 28);

    // 7th line...
    glayout->addWidget(new QLabel(QStringLiteral("ush4KeyEasy")), 5, 0, 1, 7);
    glayout->addWidget(m_controls->ush4KeyEasy, 5, 7, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("ush4KeyNormal")), 5, 35, 1, 7);
    glayout->addWidget(m_controls->ush4KeyNormal, 5, 42, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("ush4KeyHard")), 5, 70, 1, 7);
    glayout->addWidget(m_controls->ush4KeyHard, 5, 77, 1, 28);

    // 8th line...
    glayout->addWidget(new QLabel(QStringLiteral("ush5KeyEasy")), 6, 0, 1, 7);
    glayout->addWidget(m_controls->ush5KeyEasy, 6, 7, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("ush5KeyNormal")), 6, 35, 1, 7);
    glayout->addWidget(m_controls->ush5KeyNormal, 6, 42, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("ush5KeyHard")), 6, 70, 1, 7);
    glayout->addWidget(m_controls->ush5KeyHard, 6, 77, 1, 28);

    // 9th line...
    glayout->addWidget(new QLabel(QStringLiteral("ush6KeyEasy")), 7, 0, 1, 7);
    glayout->addWidget(m_controls->ush6KeyEasy, 7, 7, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("ush6KeyNormal")), 7, 35, 1, 7);
    glayout->addWidget(m_controls->ush6KeyNormal, 7, 42, 1, 28);
    glayout->addWidget(new QLabel(QStringLiteral("ush6KeyHard")), 7, 70, 1, 7);
    glayout->addWidget(m_controls->ush6KeyHard, 7, 77, 1, 28);

    // 10th line...
    glayout->addWidget(new QLabel(QStringLiteral("szNoteNumber")), 8, 0, 1, 7);
    glayout->addWidget(m_controls->szNoteNumber, 8, 7, 1, 98);

    // 11th line...
    glayout->addWidget(new QLabel(QStringLiteral("iPrice")), 9, 0, 1, 7);
    glayout->addWidget(m_controls->iPrice, 9, 7, 1, 38);
    glayout->addWidget(new QLabel(QStringLiteral("szProductID")), 9, 45, 1, 7);
    glayout->addWidget(m_controls->szProductID, 9, 52, 1, 38);
    glayout->addWidget(m_controls->iVipFlag, 9, 90, 1, 15);

    // 12th line...
    glayout->addWidget(m_controls->bIsHide, 10, 0, 1, 15);
    glayout->addWidget(m_controls->bIsReward, 10, 15, 1, 15);
    glayout->addWidget(m_controls->bIsLevelReward, 10, 30, 1, 15);
    glayout->addWidget(new QLabel(QStringLiteral("iVersion")), 10, 45, 1, 7);
    glayout->addWidget(m_controls->iVersion, 10, 52, 1, 53);

    leftLayout->addLayout(glayout);

#else

    // for QFormLayout
#define AR(l, x) l->addRow(QStringLiteral(#x), m_controls->x)

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

    vlayout->addWidget(m_controls->ucIsNew);
    vlayout->addWidget(m_controls->ucIsHot);
    vlayout->addWidget(m_controls->ucIsRecommend);
    vlayout->addWidget(m_controls->ucIsOpen);
    vlayout->addWidget(m_controls->ucCanBuy);
    vlayout->addWidget(m_controls->bIsFree);
    vlayout->addWidget(m_controls->bSongPkg);

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

    vlayout->addWidget(m_controls->iVipFlag);
    vlayout->addWidget(m_controls->bIsHide);
    vlayout->addWidget(m_controls->bIsReward);
    vlayout->addWidget(m_controls->bIsLevelReward);

    QFormLayout *flayout3 = new QFormLayout;
    AR(flayout3, iVersion);

    vlayout->addLayout(flayout3);

    QWidget *widget = new QWidget;
    widget->setLayout(vlayout);

    QScrollArea *area = new QScrollArea;
    area->setWidgetResizable(true);
    area->setWidget(widget);

    leftLayout->addWidget(area);

    // OK, thank you
#undef AR

#endif

    QString searchText = tr("Search");

    m_searchEdit = new QLineEdit;
    m_searchEdit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_searchEdit->setPlaceholderText(searchText);
    m_searchEdit->setMinimumWidth(m_searchEdit->fontMetrics().horizontalAdvance(searchText) * 2);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SongClientEditDialog::search);

    QPushButton *searchBtn = new QPushButton(searchText);
    searchBtn->setAutoDefault(false);
    searchBtn->setDefault(false);
    searchBtn->setFixedWidth(searchBtn->fontMetrics().horizontalAdvance(searchText) * 1.7);
    connect(searchBtn, &QPushButton::clicked, this, &SongClientEditDialog::search);

    m_searchList = new QListWidget;
    m_searchList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
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

SongClientEditDialog::~SongClientEditDialog()
{
    delete m_controls;
}

bool SongClientEditDialog::reloadFile()
{
    QString filepath;
    auto format = getOpenFileName(this, filepath);
    if (filepath.isNull())
        return false;

    QFile f(filepath);
    if (!f.exists())
        return false;

    if (m_file.readInfoFromDevice(&f, format)) {
        setWindowTitle(tr("Rhythm Master Song Client Editor"));
        m_isLoaded = true;
        m_currentIndex = 0;

        m_searchList->clear();
        for (int i = 0; i < m_file.songCount(); ++i) {
            const RmeSongClientItemStruct *song = m_file.song(i);
            QListWidgetItem *item = new QListWidgetItem(song->m_szSongName);
            item->setData(Qt::UserRole + 1, i);
            m_searchList->addItem(item);
        }

        readCurrent();
        return true;
    } else
        QMessageBox::critical(this, tr("RMEssentials"), tr("Read file failed"));

    return false;
}

bool SongClientEditDialog::loadFile()
{
    QDir d(RmeDownloader::binDownloadPath());

    QString filepath;
    RmeFileFormat format = JsonFormat;
    if (d.exists() && d.exists(QStringLiteral("mrock_song_client_android.json.decrypted")))
        filepath = d.absoluteFilePath(QStringLiteral("mrock_song_client_android.json.decrypted"));
    else {
        QMessageBox::information(this, tr("RMEssentials"), tr("mrock_song_client_android.json.decrypted doesn't exist, please select the file to open."));
        format = getOpenFileName(this, filepath);
    }
    if (filepath.isNull())
        return false;

    QFile f(filepath);
    if (!f.exists())
        return false;

    if (m_file.readInfoFromDevice(&f, format)) {
        setWindowTitle(tr("Rhythm Master Song Client Editor"));
        m_isLoaded = true;
        m_currentIndex = 0;

        m_searchList->clear();
        for (int i = 0; i < m_file.songCount(); ++i) {
            const RmeSongClientItemStruct *song = m_file.song(i);
            QListWidgetItem *item = new QListWidgetItem(song->m_szSongName);
            item->setData(Qt::UserRole + 1, i);
            m_searchList->addItem(item);
        }

        readCurrent();
        return true;
    } else
        QMessageBox::critical(this, tr("RMEssentials"), tr("Read file failed"));

    return false;
}

bool SongClientEditDialog::askForSaveModified()
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

void SongClientEditDialog::saveFile()
{
    if (!m_isLoaded)
        return;

    if (!askForSaveModified())
        return;

    QString filepath;
    auto format = getSaveFileName(this, filepath);
    if (filepath.isNull())
        return;

    QFile f(filepath);

    if (!m_file.saveInfoToDevice(&f, format))
        QMessageBox::critical(this, tr("RMEssentials"), tr("Save file failed"));
}

void SongClientEditDialog::moveNext()
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

void SongClientEditDialog::movePrev()
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

void SongClientEditDialog::readCurrent()
{
    const RmeSongClientItemStruct &c = *(m_file.song(m_currentIndex));

#define RP_NM(p) m_controls->p->setText(QString::number(c.m_##p))
#define RP_ST(p) m_controls->p->setText(c.m_##p)
#define RP_BL(p) m_controls->p->setChecked(c.m_##p)

    RP_NM(ushSongID);
    RP_NM(iVersion);
    RP_ST(szSongName);
    RP_ST(szPath);
    RP_ST(szArtist);
    RP_ST(szComposer);
    RP_ST(szSongTime);
    RP_NM(iGameTime);
    m_controls->szSongTime->setChecked(RmeUtils::calculateSongTime(c.m_iGameTime, true) == c.m_szSongTime);
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

    m_isContentEdited = false;
}

void SongClientEditDialog::calculateSongTime()
{
    int gameTime = m_controls->iGameTime->text().toInt();
    m_controls->szSongTime->setText(RmeUtils::calculateSongTime(gameTime, m_controls->szSongTime->isChecked()));
}

void SongClientEditDialog::saveCurrent()
{
    RmeSongClientItemStruct &c = *(m_file.song(m_currentIndex));

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

    m_isContentEdited = false;
}

void SongClientEditDialog::search()
{
    m_searchList->clear();

    QList<int> searchResult = m_file.search(m_searchEdit->text());

    if (searchResult.isEmpty())
        return;

    foreach (int n, searchResult) {
        RmeSongClientItemStruct *song = m_file.song(n);
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

void SongClientEditDialog::searchResultDblClicked(QListWidgetItem *index)
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

void SongClientEditDialog::createPatch()
{
    if (!m_isLoaded)
        return;

    if (!askForSaveModified())
        return;

    QString filepath;

    auto format = getOpenFileName(this, filepath);

    if (filepath.isNull())
        return;

    QFile f(filepath);
    if (!f.exists())
        return;

    RmeSongClientFile file2;
    if (!file2.readInfoFromDevice(&f, format)) {
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

void SongClientEditDialog::applyPatch()
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

void SongClientEditDialog::saveWikiTable()
{
    if (!m_isLoaded)
        return;

    if (!askForSaveModified())
        return;

    QString filepathToSave
        = QFileDialog::getSaveFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("TXT files") + QStringLiteral(" (*.txt)"));

    QFile f2(filepathToSave);
    if (!m_file.saveWikiTable(&f2)) {
        QMessageBox::warning(this, tr("RMEssentials"), tr("Save file failed"));
        return;
    }
}

void SongClientEditDialog::contentEdited()
{
    m_isContentEdited = true;
}

void SongClientEditDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

#ifndef MOBILE_DEVICES
    resize(minimumSize());
#endif

    loadFile();
}
