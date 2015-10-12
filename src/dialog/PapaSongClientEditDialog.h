#ifndef __PAPASONGCLIENTEDITDIALOG_H__
#define __PAPASONGCLIENTEDITDIALOG_H__

#include "songstruct.h"
#include "songfile.h"

class QLineEdit;
class QCheckBox;
class QLabel;
class QMenu;
class QListWidget;
class QListWidgetItem;

#include <QDialog>

class PapaSongClientEditDialog : public QDialog
{
    Q_OBJECT

public:
    PapaSongClientEditDialog(QWidget *parent = NULL);
    ~PapaSongClientEditDialog();

    bool loadFile();

private:
    RMSong::PapaSongClientFile m_file;

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
    //void convertToFree();
    bool reloadFile();

    void search();
    void searchResultDblClicked(QListWidgetItem *index);

    void popup();

private:
    // controls
    QLineEdit *ushSongID; // Readonly, User Making note better > 800
    QLineEdit *iVersion; // what's this?
    QLineEdit *szSongName;
    QLineEdit *szArtist;
    QLineEdit *cDifficulty;
    QLineEdit *cLevel;
    QLineEdit *szPath;
    QLabel *szSongTime; // Auto Generate
    QLineEdit *iGameTime; // Number only
    QLineEdit *szRegion;
    QLineEdit *szStyle;
    QLineEdit *szBPM; // Number only
    QLineEdit *szNoteNumber; // Number only
    QLineEdit *iOrderIndex; // Number only
    QCheckBox *ucIsOpen; // convert to bool? using QCheckBox?
    QCheckBox *ucIsFree;
    QCheckBox *ucIsHide;
    QCheckBox *ucIsReward;
    QCheckBox *ucIsLevelReward; // OBSOLETE???
    QLineEdit *iSongType; // better keep empty

    QMenu *m_popup;
    QLineEdit *m_searchEdit;
    QListWidget *m_searchList;

};


#endif
