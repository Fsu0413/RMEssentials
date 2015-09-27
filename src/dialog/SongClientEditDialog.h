#ifndef __SONGCLIENTEDITDIALOG_H__
#define __SONGCLIENTEDITDIALOG_H__

#include "songstruct.h"
#include "songfile.h"

class QLineEdit;
class QCheckBox;
class QLabel;
class QMenu;


class SongClientEditDialog : public QDialog
{
    Q_OBJECT

public:
    SongClientEditDialog(QWidget *parent = NULL);
    ~SongClientEditDialog();

    bool loadFile();

private:
    RMSong::SongClientFile m_file;

    int m_currentIndex;
    bool m_isLoaded;

private slots:
    void movePrev();
    void moveNext();
    void readCurrent();
    //void addNew();
    void calculateSongTime();
    void saveCurrent();
    void saveFile();
    void convertToFree();
    bool reloadFile();

    void popup();

private:
    // controls
    QLineEdit *ushSongID; // Readonly, User Making note better > 800
    QLineEdit *iVersion; // what's this?
    QLineEdit *szSongName;
    QLineEdit *szPath;
    QLineEdit *szArtist;
    QLineEdit *szComposer;
    QLabel *szSongTime; // Auto Generate
    QLineEdit *iGameTime; // Number only
    QLineEdit *iRegion; // QComboBox?
    QLineEdit *iStyle; // QComboBox?
    QCheckBox *ucIsNew; // convert to bool? using QCheckBox?
    QCheckBox *ucIsHot; // convert to bool? using QCheckBox?
    QCheckBox *ucIsRecommend; // convert to bool? using QCheckBox?
    QLineEdit *szBPM; // Number only
    QCheckBox *ucIsOpen; // convert to bool? using QCheckBox?
    QCheckBox *ucCanBuy;
    QLineEdit *iOrderIndex; // Number only
    QCheckBox *bIsFree;
    QCheckBox *bSongPkg;
    QLineEdit *szFreeBeginTime; // better keep empty
    QLineEdit *szFreeEndTime; // better keep empty
    QLineEdit *ush4KeyEasy;  // Number only
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
    QCheckBox *iVipFlag; // convert to bool? using QCheckBox?
    QCheckBox *bIsHide;
    QCheckBox *bIsReward;
    QCheckBox *bIsLevelReward; // OBSOLETE???

    QMenu *m_popup;

};

#endif
