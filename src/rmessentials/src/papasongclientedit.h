#ifndef PAPASONGCLIENTEDIT_H
#define PAPASONGCLIENTEDIT_H

#include <RMEssentials/RmeSongClientFile>

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QLabel;
class QMenu;
class QListWidget;
class QListWidgetItem;

class PapaSongClientEditDialog : public QDialog
{
    Q_OBJECT

public:
    PapaSongClientEditDialog(QWidget *parent = nullptr);

    bool loadFile();

private:
    RmeSong::RmePapaSongClientFile m_file;

    int m_currentIndex;
    bool m_isLoaded;

private slots:
    void movePrev();
    void moveNext();
    void readCurrent();
    void calculateSongTime();
    void saveCurrent();
    void saveFile();
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