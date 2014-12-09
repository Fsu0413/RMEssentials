#ifndef __DOWNLOADDIALOG_H__
#define __DOWNLOADDIALOG_H__

#include <QDialog>

class QListWidget;
//class QLineEdit;
class QComboBox;

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    DownloadDialog(QWidget *parent = NULL);

    void downloadList();

private:
    void appendLog(const QString &log);

signals:
    void enable_controls(bool);

private slots:
    void startDownload();
    void oneCompleted(const QString &url);
    void oneFailed(const QString &url);
    void errorOccurred();
    void allCompleted();
    void startUncompress();
    void loadPaths();

private:
    QListWidget *m_list;
    //QLineEdit *m_nameEdit;
    QComboBox *m_nameCombo;
};

#endif
