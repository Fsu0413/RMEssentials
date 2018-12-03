#ifndef MAIN_H__INCLUDED
#define MAIN_H__INCLUDED

#include <RMEss/RmeGlobal>

#include <QDialog>

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = nullptr);

private slots:
    void showDownloadDialog();
    void showChangeNameDialog();
    void showSongClientEditDialog();
    void showPapaSongClientEditDialog();
    void about();

    void checkForUpdate();
};

#endif
