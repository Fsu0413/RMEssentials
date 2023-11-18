#ifndef RMECHART_H__INCLUDED
#define RMECHART_H__INCLUDED

#include "rmeglobal.h"

#include <QByteArray>
#include <QJsonObject>

#include <cmath>

struct LIBRMESSENTIALS_EXPORT RmeChartVersion
{
    enum v
    {
        vUnknown,
        vImd = 0xffffffff,

        v1_2_1 = 0x121,
        v1_2_2 = 0x122,
        v1_2_3 = 0x123,
        v1_3_0 = 0x130,
    };

    constexpr RmeChartVersion(v version = vUnknown)
        : m_v(version)
    {
    }

    constexpr RmeChartVersion &operator=(v version)
    {
        m_v = version;
        return *this;
    }
    // constexpr RmeChartVersion(const RmeChartVersion &) = default;
    // constexpr RmeChartVersion& operator=(const RmeChartVersion &) = default;
    // ~RmeChartVersion(const RmeChartVersion &) = default;

    constexpr operator v() const
    {
        return m_v;
    }

    explicit RmeChartVersion(const QString &versionStr);
    QString toString() const;

private:
    v m_v;
};

// based on imdjson 1.2.1 format
struct LIBRMESSENTIALS_EXPORT RmeChartNote
{
    unsigned char track; // JSON track -> 3 / 4 / 5 / 6 / 7 / 8
    unsigned int timestamp; // IMD / IMDJSON 1.2.3+ time
    int key;
    bool isEnd;
    unsigned char toTrack;
    unsigned int timeDur; // IMD / IMDJSON 1.2.3+ time duration
    unsigned char attr;

    RmeChartNote() = default;
    RmeChartNote(const RmeChartNote &) = default;
    RmeChartNote &operator=(const RmeChartNote &) = default;
    RmeChartNote(RmeChartNote &&) = default;
    RmeChartNote &operator=(RmeChartNote &&) = default;

    QByteArray toImdNote() const;
    QJsonObject toJsonNote(RmeChartVersion version, double bpm, int idx = 0) const;

    bool operator<(const RmeChartNote &arg2) const;
    constexpr inline bool operator==(const RmeChartNote &arg2) const
    {
        return track == arg2.track && timestamp == arg2.timestamp && isEnd == arg2.isEnd && toTrack == arg2.toTrack && timeDur == arg2.timeDur && attr == arg2.attr;
    }

    static RmeChartNote fromImdNote(const QByteArray &arr, bool *ok = nullptr);
    static RmeChartNote fromJsonNote(const QJsonObject &ob, unsigned char track, double bpm, bool *ok = nullptr);
};

struct LIBRMESSENTIALS_EXPORT RmeChart
{
    unsigned int totalTime;
    double bpm; // OR QList<RmeBpm> bpms;
    std::list<RmeChartNote> notes; // not keys by track!
    RmeChartVersion version;

    void sortNotes();

    QByteArray toImd() const;
    QJsonObject toJson(RmeChartVersion version = RmeChartVersion::v1_2_1) const;

    static RmeChart fromImd(const QByteArray &arr, bool *ok = nullptr);
    static RmeChart fromJson(const QJsonObject &ob, bool *ok = nullptr);
};

#endif
