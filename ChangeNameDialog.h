#ifndef __CHANGENAMEDIALOG_H__
#define __CHANGENAMEDIALOG_H__

#include <QDialog>

class QLineEdit;
class QLabel;
class QString;
class QLayout;

class ChangeNameDialog : public QDialog
{
    Q_OBJECT

public:

    ChangeNameDialog(QWidget *parent = 0);
    ~ChangeNameDialog();

private:
    QLineEdit *m_folderName;
    QLineEdit *m_toRename;

    QLabel *m_filesLabels[12];
    // 0 :mp3    1 :Bigpng 2 :SmallPng
    // 3 :4KEZ   4 :4KNM   5 :4KHD
    // 6 :5KEZ   7 :5KNM   8 :5KHD
    // 9 :6KEZ   10:6KNM   11:6KHD

private slots:
    void selectFolder();
    void renameAsk();
    void checkFiles(const QString &folder);
    void renameFinished(bool succeeded);

signals:
    void folder_selected(const QString &folder);
    void enable_widgets(bool enable);

private:
    // 0 :mp3    1 :Bigpng 2 :SmallPng
    // 3 :4KEZ   4 :4KNM   5 :4KHD
    // 6 :5KEZ   7 :5KNM   8 :5KHD
    // 9 :6KEZ   10:6KNM   11:6KHD
    static QLayout *layFiles(QLabel *labels[12]);

};

#endif // __MAINDIALOG_H__
