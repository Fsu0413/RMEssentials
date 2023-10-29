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

    RmeChartNote() = default;
    RmeChartNote(const RmeChartNote &) = default;
    RmeChartNote &operator=(const RmeChartNote &) = default;
    RmeChartNote(RmeChartNote &&) = default;
    RmeChartNote &operator=(RmeChartNote &&) = default;

    QByteArray toImdNote(double bpm) const;
    QJsonObject toJsonNote(RmeChartVersion version = RmeChartVersion::v1_2_1, double bpm = nan(nullptr), int idx = 0) const;

    constexpr inline bool operator<(const RmeChartNote &arg2) const
    {
        // "one main rule": earlier note must be put before
        if (tick < arg2.tick)
            return true;

        // only here: notes appear same time need sorting
        if (tick == arg2.tick) {
            // make sure 0x21 / 0x61 appears before corresponding 0x22 / 0xA2
            // (this) 0x21 / 0x61 -> attr != 0, dur == 0, isEnd == false, toTrack != track
            // (arg2) 0x22 / 0xA2 -> attr == 4, dur != 0
            // (and) if toTrack == arg2.track and all of above then return true!
            if (attr != 0 && dur == 0 && !isEnd && toTrack != track && arg2.attr == 4 && arg2.dur != 0 && toTrack == arg2.track)
                return true;

            // strong sequence guarantee of above: reverse sequence is not allowed, so reverse arg2 and this for above judgement and return false
            if (arg2.attr != 0 && arg2.dur == 0 && !arg2.isEnd && arg2.toTrack != arg2.track && attr == 4 && dur != 0 && arg2.toTrack == track)
                return false;

            // slide / long-press is put before single-click
            if (attr != 0 && arg2.attr == 0)
                return true;
            // strong sequence guarantee of above
            if (arg2.attr != 0 && attr == 0)
                return false;

            // After all, sort by track
            return track < arg2.track;
        }

        // still "one main rule": later note must be put afterwards
        return false;
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
