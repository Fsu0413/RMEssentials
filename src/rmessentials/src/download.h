#ifndef DOWNLOAD_H__INCLUDED
#define DOWNLOAD_H__INCLUDED

#include <RMEssentials/RmeGlobal>

#include <QDialog>
#include <QDir>
#include <QMap>

class QListWidget;
class QComboBox;
class QPushButton;
class QProgressBar;
#ifdef Q_OS_WIN
class QWinTaskbarButton;
#endif

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    enum DownloadMode
    {
        One,
        All,
        Mis
    };

    explicit DownloadDialog(QWidget *parent = nullptr);

    void downloadList();

private:
    void appendLog(const QString &log);

    QWidget *createDownloadSongTab();
    QWidget *createDownloadRoleTab();

signals:
    void busy(bool);
    void cancelDownload();

private slots:
    void downloadAndroidList();

    void downloadSongClicked();
    void downloadRoleClicked();

    void startDownloadSong(DownloadMode mode = One);
    void startDownloadRole();
    void startDownloadNoteImage();
    void oneCompleted(const QString &url);
    void oneUncompressed(const QString &filename);
    void oneFailed(const QString &url);
    void allCompleted();
    void canceled();
    void startUncompress();
    void loadPaths();
    void setBusy(bool b);

    void startDownloadAllSong();
    void startDownloadNext();
    void startDownloadAllMissingSong();
    void startDownloadNextMissing();
    void downloadProgress(quint64 downloaded, quint64 total);

protected:
    void closeEvent(QCloseEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    QListWidget *m_list;
    QProgressBar *m_progressBar;

    QComboBox *m_songNameCombo;
    QPushButton *m_downloadSongBtn;

    QComboBox *m_roleNameCombo;
    QPushButton *m_downloadRoleBtn;

    QMap<int, QStringList> m_rolePadUiMap;

    bool m_busy;
    bool m_exitRequested;

#ifdef Q_OS_WIN
    QWinTaskbarButton *m_taskbarBtn;
#endif
};

#endif
