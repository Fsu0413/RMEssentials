#ifndef PAPASONGCLIENTEDIT_H__INCLUDED
#define PAPASONGCLIENTEDIT_H__INCLUDED

#include <RMEss/RmeSongClientFile>

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QLabel;
class QMenu;
class QListWidget;
class QListWidgetItem;

struct PapaSongClientEditDialogControls;

class PapaSongClientEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PapaSongClientEditDialog(QWidget *parent = nullptr);
    ~PapaSongClientEditDialog() override;
    bool loadFile();

private:
    RmeSong::RmePapaSongClientFile m_file;

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
    PapaSongClientEditDialogControls *m_controls;
    QMenu *m_popup;
    QLineEdit *m_searchEdit;
    QListWidget *m_searchList;
};

#endif
