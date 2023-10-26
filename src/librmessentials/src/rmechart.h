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
    unsigned int tick; // JSON tick -> affected by BPM
    int key;
    bool isEnd;
    unsigned char toTrack;
    unsigned int dur;
    unsigned char attr;

    RmeChartNote(const RmeChartNote &) = default;
    RmeChartNote &operator=(const RmeChartNote &) = default;
    RmeChartNote(RmeChartNote &&) = default;
    RmeChartNote &operator=(RmeChartNote &&) = default;

    QByteArray toImdNote(double bpm) const;
    QJsonObject toJsonNote(RmeChartVersion version = RmeChartVersion::v1_2_1, double bpm = nan(nullptr), int idx = 0) const;

    constexpr inline bool operator<(const RmeChartNote &arg2) const
    {
        // if statement can't be in constexpr so use logical operators
        return (tick < arg2.tick)
            || (tick == arg2.tick
                && ((attr != 0 && arg2.attr == 0) || (attr != 0 && arg2.attr != 0 && dur == 0 && arg2.dur != 0)
                    || ((((attr == 0) && (arg2.attr == 0)) || ((attr != 0) && (arg2.attr != 0) && ((dur == 0 && arg2.dur == 0) || (dur != 0 && arg2.dur != 0))))
                        && (track < arg2.track))));
    }
    constexpr inline bool operator==(const RmeChartNote &arg2) const
    {
        return track == arg2.track && tick == arg2.tick && key == arg2.key && isEnd == arg2.isEnd && toTrack == arg2.toTrack && dur == arg2.dur && attr == arg2.attr;
    }

    static RmeChartNote fromImdNote(const QByteArray &arr, double bpm, bool *ok = nullptr);
    static RmeChartNote fromJsonNote(const QJsonObject &ob, unsigned char track, bool *ok = nullptr);
};

struct LIBRMESSENTIALS_EXPORT RmeChart
{
    unsigned int totalTick;
    double bpm; // OR QList<RmeBpm> bpms;
    QList<RmeChartNote> notes; // not keys by track!
    RmeChartVersion version;

    QByteArray toImd() const;
    QJsonObject toJson(RmeChartVersion version = RmeChartVersion::v1_2_1) const;

    static RmeChart fromImd(const QByteArray &arr, bool *ok = nullptr);
    static RmeChart fromJson(const QJsonObject &ob, bool *ok = nullptr);
};

#endif
