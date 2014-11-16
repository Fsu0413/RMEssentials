#ifndef __RENAMER_H__
#define __RENAMER_H__

#include <QThread>
#include <QDir>

class Renamer : public QThread
{
    Q_OBJECT
    Q_PROPERTY(QDir dir READ dir WRITE setDir)
    Q_PROPERTY(QString toRename READ toRename WRITE setToRename)

public:
    virtual void run();

private:
    bool renameMp3();
    bool renameBigPng();
    bool renameSmallPng();
    bool renameImds();
    bool renameSelf();
    bool deleteExtra();

signals:
    void rename_finished(bool success);

public:
    inline void setDir(const QDir &d) { m_d = d; }
    inline const QDir &dir() const { return m_d; }
    inline void setToRename(const QString &to) { m_toRename = to; }
    inline const QString &toRename() const { return m_toRename; }

private:
    QDir m_d;
    QString m_toRename;
};

#endif // __RENAMER_H__
