#ifndef SONGCLIENTEDIT_H__INCLUDED
#define SONGCLIENTEDIT_H__INCLUDED

#include <RMEssentials/RmeSongClientFile>

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QLabel;
class QMenu;
class QListWidget;
class QListWidgetItem;

struct SongClientEditDialogControls;

class SongClientEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SongClientEditDialog(QWidget *parent = nullptr);
    ~SongClientEditDialog() override;
    bool loadFile();

private:
    RmeSong::RmeSongClientFile m_file;

    int m_currentIndex;
    bool m_isLoaded;

private slots:
    void movePrev();
    void moveNext();
    void readCurrent();
    void calculateSongTime();
    void saveCurrent();
    void saveFile();
    void convertToFree();
    void allSongUnlock();
    bool reloadFile();
    void search();
    void searchResultDblClicked(QListWidgetItem *index);
    void mergeSongList();
    void prepareForUserMakingNotes();
    void createPatch();
    void applyPatch();

private:
    SongClientEditDialogControls *m_controls;
    QMenu *m_popup;
    QLineEdit *m_searchEdit;
    QListWidget *m_searchList;
};

#endif
