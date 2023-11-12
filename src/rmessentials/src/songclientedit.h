#ifndef SONGCLIENTEDIT_H__INCLUDED
#define SONGCLIENTEDIT_H__INCLUDED

#include <RmEss/RmeSongClientFile>

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
    RmeSongClientFile m_file;

    int m_currentIndex;
    bool m_isLoaded;

    bool m_isContentEdited;

private:
    bool askForSaveModified();

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
    void createPatch();
    void applyPatch();
    void saveWikiTable();

    void contentEdited();

protected:
    void showEvent(QShowEvent *e) override;

private:
    SongClientEditDialogControls *m_controls;
    QLineEdit *m_searchEdit;
    QListWidget *m_searchList;
};

#endif
