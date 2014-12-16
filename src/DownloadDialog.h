#ifndef __DOWNLOADDIALOG_H__
#define __DOWNLOADDIALOG_H__

#include <QDialog>

class QListWidget;
class QComboBox;
class QPushButton;

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    DownloadDialog(QWidget *parent = NULL);

    void downloadList();

private:
    void appendLog(const QString &log);

signals:
    void busy(bool);
    void cancel_download();

private slots:
    void downloadClicked();
    void startDownload();
    void oneCompleted(const QString &url);
    void oneUncompressed(const QString &filename);
    void oneFailed(const QString &url);
    void errorOccurred();
    void allCompleted();
    void canceled();
    void startUncompress();
    void loadPaths();
    void setBusy(bool b);

protected:
    virtual void closeEvent(QCloseEvent *e);

private:
    QListWidget *m_list;
    QComboBox *m_nameCombo;
    QPushButton *m_downloadBtn;

    bool m_busy;
};

#endif
