#ifndef CHANGENAME_H
#define CHANGENAME_H

#include <RMEssentials/RmeGlobal>

#include <QDialog>

class QLineEdit;
class QLabel;
class QLayout;

class ChangeNameDialog : public QDialog
{
    Q_OBJECT

public:
    ChangeNameDialog(QWidget *parent = nullptr);

private:
    QLineEdit *m_folderName;
    QLineEdit *m_toRename;

    QLabel *m_filesLabels[18];

private slots:
    void selectFolder();
    void rename();
    void checkFiles(const QString &folder);
    void renameToEasy();

signals:
    void folderSelected(const QString &folder);

private:
    static QLayout *layFiles(QLabel *labels[18]);
};

#endif
