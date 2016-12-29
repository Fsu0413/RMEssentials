#ifndef __DOWNLOADDIALOG_H__
#define __DOWNLOADDIALOG_H__

#include <QDialog>
#include <QDir>

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

    DownloadDialog(QWidget *parent = nullptr);

    void downloadList();

private:
    void appendLog(const QString &log);

signals:
    void busy(bool);
    void cancel_download();

private slots:
    void downloadAndroidList();

    void downloadClicked();
    void startDownload(DownloadMode mode = One);
    void oneCompleted(const QString &url);
    void oneUncompressed(const QString &filename);
    void oneFailed(const QString &url);
    void errorOccurred();
    void allCompleted();
    void canceled();
    void startUncompress();
    void loadPaths();
    void setBusy(bool b);

    void startDownloadAll();
    void startDownloadNext();
    void startDownloadAllMissing();
    void startDownloadNextMissing();
    void downloadProgress(quint64 downloaded, quint64 total);

protected:
    void closeEvent(QCloseEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    QListWidget *m_list;
    QComboBox *m_nameCombo;
    QPushButton *m_downloadBtn;
    QProgressBar *m_progressBar;

    bool m_busy;
    bool m_exitRequested;

#ifdef Q_OS_WIN
    QWinTaskbarButton *m_taskbarBtn;
#endif
};

#endif
