#include "PapaSongClientEditDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QLabel>

#include <QMenu>

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

using namespace RMSong;

PapaSongClientEditDialog::PapaSongClientEditDialog(QWidget *parent)
    : QDialog(parent), currentIndex(-1), isLoaded(false)
{
    setWindowTitle(tr("Rhythm Master PapaSong Client Editor"));

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
    connect(prevBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::movePrev);
    QPushButton *nextBtn = new QPushButton(tr("next"));
    connect(nextBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::moveNext);
    QPushButton *saveCurrentBtn = new QPushButton(tr("save current"));
    connect(saveCurrentBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::saveCurrent);
    // functions...
    m_popup = new QMenu(tr("Functions..."), this);
    QAction *openFileBtn = m_popup->addAction(tr("open an other file"));
    connect(openFileBtn, &QAction::triggered, this, &PapaSongClientEditDialog::reloadFile);
    QAction *saveFileBtn = m_popup->addAction(tr("save file"));
    connect(saveFileBtn, &QAction::triggered, this, &PapaSongClientEditDialog::saveFile);
    QPushButton *funcBtn = new QPushButton(tr("Functions..."));
    connect(funcBtn, &QPushButton::clicked, this, &PapaSongClientEditDialog::popup);
    hlayout1->addLayout(flayout1);
    hlayout1->addWidget(prevBtn);
    hlayout1->addWidget(nextBtn);
    hlayout1->addWidget(saveCurrentBtn);
    hlayout1->addWidget(funcBtn);


    // 2nd, 3rd, 4th lines...
    QHBoxLayout *hlayout234 = new QHBoxLayout;
    QFormLayout *flayout2 = new QFormLayout;
    szSongName = new QLineEdit;
    szNoteNumber = new QLineEdit;
    szRegion = new QLineEdit;
    iOrderIndex = new QLineEdit;
    QIntValidator *iOrderIndexValidator = new QIntValidator(0, 100, this);
    iOrderIndex->setValidator(iOrderIndexValidator);
    AR(flayout2, szSongName);
    AR(flayout2, szNoteNumber);
    AR(flayout2, szRegion);
    AR(flayout2, iOrderIndex);
    QFormLayout *flayout3 = new QFormLayout;
    szPath = new QLineEdit;
    QRegExpValidator *szPathValidator = new QRegExpValidator(QRegExp("[0-9a-z_]+"), this);
    szPath->setValidator(szPathValidator);
    iGameTime = new QLineEdit;
    QIntValidator *iGameTimeValidator = new QIntValidator(1, 2147483647, this);
    iGameTime->setValidator(iGameTimeValidator);
    connect(iGameTime, &QLineEdit::textEdited, this, &PapaSongClientEditDialog::calculateSongTime);
    szStyle = new QLineEdit;
    iSongType = new QLineEdit;
    iSongType->setValidator(iGameTimeValidator);
    AR(flayout3, szPath);
    AR(flayout3, iGameTime);
    AR(flayout3, szStyle);
    AR(flayout3, iSongType);
    QFormLayout *flayout4 = new QFormLayout;
    szArtist = new QLineEdit;
    szSongTime = new QLabel;
    szBPM = new QLineEdit;
    QIntValidator *szBPMValidator = new QIntValidator(1, 1000, this);
    szBPM->setValidator(szBPMValidator);
    iVersion = new QLineEdit;
    QIntValidator *iVersionValidator = new QIntValidator(1, 2147483647, this);
    iVersion->setValidator(iVersionValidator);
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
    cDifficulty = new QLineEdit;
    QIntValidator *cDifficultyValidator = new QIntValidator(1, 3, this);
    cDifficulty->setValidator(cDifficultyValidator);
    AR(flayout5, cDifficulty);
    hlayout5->addLayout(flayout5);

    QFormLayout *flayout6 = new QFormLayout;
    cLevel = new QLineEdit;
    QIntValidator *cLevelValidator = new QIntValidator(1, 10, this);
    cLevel->setValidator(cLevelValidator);
    AR(flayout6, cLevel);
    hlayout5->addLayout(flayout6);

    // 6th line...
    QHBoxLayout *hlayout12 = new QHBoxLayout;
    ucIsHide = new QCheckBox("ucIsHide");
    ucIsReward = new QCheckBox("ucIsReward");
    ucIsLevelReward = new QCheckBox("ucIsLevelReward");
    ucIsOpen = new QCheckBox("ucIsOpen");
    ucIsFree = new QCheckBox("ucIsFree");

    hlayout12->addWidget(ucIsOpen);
    hlayout12->addWidget(ucIsFree);
    hlayout12->addWidget(ucIsHide);
    hlayout12->addWidget(ucIsReward);
    hlayout12->addWidget(ucIsLevelReward);

    // OK, thank you
#undef AR

    alllayout->addLayout(hlayout1);
    alllayout->addLayout(hlayout234);
    alllayout->addLayout(hlayout5);
    alllayout->addLayout(hlayout12);

    setLayout(alllayout);
}

PapaSongClientEditDialog::~PapaSongClientEditDialog()
{
    if (!songs.isEmpty()) {
        foreach(PapaSongStruct *const &s, songs)
            delete s;
    }
}

bool PapaSongClientEditDialog::reloadFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");

    QFile f(filepath);
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray ba = f.readAll();

        if (ba.size() % 0x169 == 0x88) {

            if (!songs.isEmpty()) {
                foreach(PapaSongStruct *const &s, songs)
                    delete s;
                songs.clear();
                isLoaded = false;
                currentIndex = -1;
            }

            fileHeader = ba.mid(0, 0x88);
            for (int i = 0x88; i < ba.size(); i += 0x169) {
                QByteArray sp = ba.mid(i, 0x169);
                PapaSongStruct *ss = new PapaSongStruct;
                Array2Song(sp, *ss);
                songs << ss;
            }
            if (!songs.isEmpty()) {
                isLoaded = true;
                currentIndex = 0;
                readCurrent();
                f.close();
                return true;
            }
        }
        f.close();
    }

    return false;
}

bool PapaSongClientEditDialog::loadFile()
{
#ifndef Q_OS_ANDROID
    QDir d("downloader");
#else
    QDir d("/sdcard/RM/res");
#endif
    QString filepath;
    if (d.exists() && d.exists("mrock_papasong_client.bin"))
        filepath = d.absoluteFilePath("mrock_papasong_client.bin");
    else {
        QMessageBox::information(this, tr("RMEssentials"), tr("mrock_papasong_client.bin doesn't exist, please select the file to open."));
        filepath = QFileDialog::getOpenFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");
    }

    QFile f(filepath);
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray ba = f.readAll();
        if (ba.size() % 0x169 == 0x88) {
            fileHeader = ba.mid(0, 0x88);
            for (int i = 0x88; i < ba.size(); i += 0x169) {
                QByteArray sp = ba.mid(i, 0x169);
                PapaSongStruct *ss = new PapaSongStruct;
                Array2Song(sp, *ss);
                songs << ss;
            }
            if (!songs.isEmpty()) {
                isLoaded = true;
                currentIndex = 0;
                readCurrent();
                f.close();
                return true;
            }
        }
        f.close();
    }
    return false;
}

void PapaSongClientEditDialog::saveFile()
{
    QString filepath = QFileDialog::getSaveFileName(this, tr("RMEssentials"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), tr("bin files") + " (*.bin)");
    QFile f(filepath);
    if (f.exists() && QMessageBox::question(this, tr("RMEssentials"), tr("File is already exists, do you want to overwrite?")) == QMessageBox::No)
        return;

    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(fileHeader, 0x88);
        foreach (PapaSongStruct *const &s, songs) {
            QByteArray arr;
            Song2Array(*s, arr);
            f.write(arr.constData(), 0x169);
        }
        f.close();
    }
}

void PapaSongClientEditDialog::moveNext()
{
    if (currentIndex + 1 == songs.length())
        return;

    ++currentIndex;
    readCurrent();
}

void PapaSongClientEditDialog::movePrev()
{
    if (currentIndex <= 0)
        return;

    --currentIndex;
    readCurrent();
}

void PapaSongClientEditDialog::readCurrent()
{
    const PapaSongStruct &c = *(songs.at(currentIndex));

#define RP_NM(p) p->setText(QString::number(c.m_ ## p))
#define RP_ST(p) p->setText(c.m_ ## p)
#define RP_BL(p) p->setChecked(c.m_ ## p)

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
    PapaSongStruct &c = *(songs[currentIndex]);

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
