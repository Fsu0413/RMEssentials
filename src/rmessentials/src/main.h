#ifndef MAIN_H__INCLUDED
#define MAIN_H__INCLUDED

#include <RmEss/RmeGlobal>

#include <QDialog>

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainDialog(QWidget *parent = nullptr);
    ~MainDialog() override;

private slots:
    void showDownloadDialog();
    void showChangeNameDialog();
    void showSongClientEditDialog();
    void showPapaSongClientEditDialog();
    void showChartViewer();
    void about();
#ifdef RME_USE_QUAZIP
    void aboutQuaZip();
#endif
    void aboutXxtea();

    void metainfoDownloaded();

public slots:
    void permissionCheckOk();
    void enableButtons();

private slots:
    void oneMetainfoFileDownloaded(const QString &url);
    bool checkPermission();

#ifdef Q_OS_ANDROID

private:
    class AndroidResultReceiver;
    friend class MainDialog::AndroidResultReceiver;

    AndroidResultReceiver *m_receiver;

private slots:
    void requestForLegacyPermission();
    void legacyPermissionRequestCallback();

    void requestForPermission();
    void permissionRequestCallback();
#endif
private:
    bool m_isPermissionOk;
    bool m_isNumOk;

private:
    QPushButton *m_changeNameBtn;
    QPushButton *m_downloadBtn;
    QPushButton *m_songEditorBtn;
    QPushButton *m_papaSongEditorBtn;
    QPushButton *m_chartViewBtn;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

extern QString currentNum();
extern QByteArray currentHash();

#endif
