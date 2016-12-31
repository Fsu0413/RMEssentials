// GEN: Renamer

#ifndef __RENAMER_H__
#define __RENAMER_H__

#include "rmeglobal.h"

class QDir;
class QString;
class RmeRenamerPrivate;

class LIBRMESSENTIALS_EXPORT RmeRenamer
{
public:
    RmeRenamer();
    ~RmeRenamer();

    bool run();
    bool runToEasy();

    void setDir(const QDir &d);
    const QDir &dir() const;
    void setToRename(const QString &to);
    const QString &toRename() const;

private:
    Q_DISABLE_COPY(RmeRenamer)
    Q_DECLARE_PRIVATE(RmeRenamer)
    RmeRenamerPrivate *d_ptr;
};

#endif // __RENAMER_H__
