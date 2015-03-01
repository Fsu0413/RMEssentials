#include "SongClientEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QLabel>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

using namespace RMSong;

namespace {
    const char fileheader[0x88] = {
        0x4D, 0x53, 0x45, 0x53, 0x06, 0x00, 0x00, 0x00, 0x3E, 0x03, 0x00, 0x00, 0x73, 0x01, 0x00, 0x00,
        0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
        0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x55, 0x54, 0x46, 0x2D, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x32, 0x35, 0x64, 0x35, 0x64, 0x39, 0x66, 0x31, 0x37, 0x37, 0x35, 0x33, 0x65, 0x33, 0x37, 0x32,
        0x34, 0x37, 0x34, 0x63, 0x63, 0x38, 0x61, 0x61, 0x37, 0x62, 0x33, 0x37, 0x30, 0x65, 0x38, 0x39,
        0x00, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00
    };
}

SongClientEditDialog::SongClientEditDialog(QWidget *parent)
    : QDialog(parent), currentIndex(-1), isLoaded(false)
{
    setWindowTitle(tr("Rhythm Master Song Client Editor"));

    QVBoxLayout *alllayout = new QVBoxLayout;

// for QFormLayout
#define AR(l, x) l->addRow(#x, x)

    // 1st line
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    QFormLayout *flayout1 = new QFormLayout;
    ushSongID = new QLineEdit;
    ushSongID->setReadOnly(true);
    AR(flayout1, ushSongID);

    QPushButton *prevBtn = new QPushButton(tr("prev"));
    connect(prevBtn, &QPushButton::clicked, this, &SongClientEditDialog::movePrev);
    QPushButton *nextBtn = new QPushButton(tr("next"));
    connect(nextBtn, &QPushButton::clicked, this, &SongClientEditDialog::moveNext);
    QPushButton *saveCurrentBtn = new QPushButton(tr("save current"));
    connect(saveCurrentBtn, &QPushButton::clicked, this, &SongClientEditDialog::saveCurrent);
    QPushButton *saveAllBtn = new QPushButton(tr("save file"));
    connect(saveAllBtn, &QPushButton::clicked, this, &SongClientEditDialog::saveFile);
    hlayout1->addLayout(flayout1);
    hlayout1->addWidget(prevBtn);
    hlayout1->addWidget(nextBtn);
    hlayout1->addWidget(saveCurrentBtn);
    hlayout1->addWidget(saveAllBtn);

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
    QIntValidator *szBPMValidator = new QIntValidator(1, 1000, this);
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
    //szNoteNumber->setInputMask("9000,9000,9000,9000,9000,9000,9000,9000,9000");
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

    alllayout->addLayout(hlayout1);
    alllayout->addLayout(hlayout234);
    alllayout->addLayout(hlayout5);
    alllayout->addLayout(hlayout6789);
    alllayout->addLayout(hlayout10);
    alllayout->addLayout(hlayout11);
    alllayout->addLayout(hlayout12);

    setLayout(alllayout);
}

SongClientEditDialog::~SongClientEditDialog() {
    if (!songs.isEmpty()) {
        foreach (SongStruct *const &s, songs)
            delete s;
    }
}

bool SongClientEditDialog::loadFile() {
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
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray ba = f.readAll();
        if (ba.size() % 0x33e == 0x88) {
            for (int i = 0x88; i < ba.size(); i += 0x33e) {
                QByteArray sp = ba.mid(i, 0x33e);
                SongStruct *ss = new SongStruct;
                Array2Song(sp, *ss);
                songs << ss;
            }
            if (!songs.isEmpty()) {
                isLoaded = true;
                currentIndex = 0;
                readCurrent();
                return true;
            }
        }
        f.close();
    }
    return false;
}

void SongClientEditDialog::saveFile() {
    QString filepath = QFileDialog::getSaveFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");
    QFile f(filepath);
    if (f.exists() && QMessageBox::question(this, tr("RMEssentials"), tr("File is already exists, do you want to overwrite?")) == QMessageBox::No)
        return;

    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(fileheader, 0x88);
        foreach (SongStruct *const &s, songs) {
            QByteArray arr;
            Song2Array(*s, arr);
            f.write(arr.constData(), 0x33e);
        }
        f.close();
    }
}

void SongClientEditDialog::moveNext() {
    if (currentIndex + 1 == songs.length())
        return;

    ++currentIndex;
    readCurrent();
}

void SongClientEditDialog::movePrev() {
    if (currentIndex <= 0)
        return;
    
    --currentIndex;
    readCurrent();
}

void SongClientEditDialog::readCurrent() {
    const SongStruct &c = *(songs.at(currentIndex));

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

void SongClientEditDialog::calculateSongTime() {
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

void SongClientEditDialog::saveCurrent() {
    SongStruct &c = *(songs[currentIndex]);

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
