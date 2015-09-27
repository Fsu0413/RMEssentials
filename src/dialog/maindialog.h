#ifndef __MAINDIALOG_H__
#define __MAINDIALOG_H__

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = NULL);

private slots:
    void showDownloadDialog();
    void showChangeNameDialog();
    void showSongClientEditDialog();
    void showPapaSongClientEditDialog();
    void about();

    void checkForUpdate();
};

#endif
