#ifndef RMERENAMER_H__INCLUDED
#define RMERENAMER_H__INCLUDED

#include "rmeglobal.h"

class QDir;
class QString;
class RmeRenamerPrivate;
struct RmeChartVersion;

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

class RmeConverterPrivate;

class LIBRMESSENTIALS_EXPORT RmeConverter
{
public:
    RmeConverter();
    ~RmeConverter();

    bool convertImdToImdJson(const RmeChartVersion &version);
    bool convertImdJsonToImd();
    bool convertRmpToImdJson();
    bool convertImdJsonToRmp();

    void setDir(const QDir &d);
    const QDir &dir() const;

private:
    Q_DISABLE_COPY(RmeConverter)
    Q_DECLARE_PRIVATE(RmeConverter)
    RmeConverterPrivate *d_ptr;
};

#endif
