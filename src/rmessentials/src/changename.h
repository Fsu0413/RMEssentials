#ifndef CHANGENAME_H__INCLUDED
#define CHANGENAME_H__INCLUDED

#include <RmEss/RmeGlobal>

#include <QDialog>

class QComboBox;
class QLineEdit;
class QLabel;
class QLayout;

class ChangeNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeNameDialog(QWidget *parent = nullptr);

private:
    QLineEdit *m_folderName;
    QLineEdit *m_toRename;

    QLabel *m_filesLabels[36];

    QComboBox *m_convertToImdJsonVersion;

private slots:
    void selectFolder();
    void rename();
    void checkFiles(const QString &folder);
    void renameToEasy();

signals:
    void folderSelected(const QString &folder);

private:
    static QLayout *layFiles(QLabel *labels[36]);
};

#endif
