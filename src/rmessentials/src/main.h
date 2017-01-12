#ifndef MAIN_H
#define MAIN_H

#include <RMEssentials/RmeGlobal>

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
