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

private:
    PapaSongClientEditDialogControls *m_controls;
    QMenu *m_popup;
    QLineEdit *m_searchEdit;
    QListWidget *m_searchList;
};

#endif
