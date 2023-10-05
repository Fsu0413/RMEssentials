#ifndef DOWNLOAD_H__INCLUDED
#define DOWNLOAD_H__INCLUDED

#include <RMEss/RmeGlobal>

#include <QDialog>
#include <QDir>
#include <QMap>
#include <QPushButton>

class QListWidget;
class QComboBox;
class QProgressBar;
class QCheckBox;
#ifdef QT_WINEXTRAS_LIB
class QWinTaskbarButton;
#endif

class DownloadButton : public QPushButton
{
    Q_OBJECT

public:
    explicit DownloadButton(QWidget *parent = nullptr);
    ~DownloadButton() = default;

    static QString startDownloadTitle;
    static QString cancelDownloadTitle;

public slots:
    void setBusy(bool b);

private:
    Q_DISABLE_COPY(DownloadButton)
};

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    enum DownloadMode
    {
        One,
        All,
        Mis,
        OneLegacy,
        AllLegacy,
        MisLegacy,
    };

    explicit DownloadDialog(QWidget *parent = nullptr);

    void downloadList();

private:
    void appendLog(const QString &log);

    QWidget *createDownloadSongTab();
    QWidget *createDownloadLegacySongTab();

    QSet<QString> loadSongClientJsonImpl(const QByteArray &arr);
    QSet<QString> loadSongClientJson(const QString &fileName);
    QSet<QString> loadMd5ListJsonImpl(const QByteArray &arr);
    QSet<QString> loadMd5ListJson(const QString &fileName);
    QSet<QString> loadMd5ListXml(const QString &fileName);

signals:
    void busy(bool);
    void cancelDownload();

private slots:
    void downloadSongClicked();
    void downloadLegacySongClicked();

    void warnEncryptedChart();

    void startDownloadSong(DownloadDialog::DownloadMode mode);
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

    void startDownloadAllLegacySong();
    void startDownloadNextLegacy();
    void startDownloadAllMissingLegacySong();
    void startDownloadNextMissingLegacy();

    void downloadProgress(quint64 downloaded, quint64 total);

protected:
    void closeEvent(QCloseEvent *e) override;
    void showEvent(QShowEvent *e) override;

private:
    QListWidget *m_list;
    QProgressBar *m_progressBar;

    QComboBox *m_songNameCombo;
    QComboBox *m_legacySongNameCombo;

    bool m_busy;
    bool m_exitRequested;

    bool m_encryptedChartWarned;

#ifdef QT_WINEXTRAS_LIB
    QWinTaskbarButton *m_taskbarBtn;
#endif
};

#endif
