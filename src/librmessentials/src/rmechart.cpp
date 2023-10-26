#include "rmechart.h"

#include <QJsonArray>

// remaining issues for this conversion:
// imdjson 1.2.x have no total time property. It is calculated when loading the chart and saved to imdjson 1.3.0 / imd.
// imdjson note property "volume" and "pan" are totally ignored. They are filled with 127 / 64 for imdjson 1.2.1 and 1.2.2, and 0 / 0 for imdjson 1.2.3 onwards.
// imdjson note property "key" is ignored when saving to imd, and is 0 when imported from imd.
// imdjson chart property "samples" is totally ignored. An empty Json Array is filled when saving imdjson 1.2.3 onwards.
// imdjson 1.2.3 / 1.3.0 note property "time", "time_dur", "idx" are totally ignored. They are recalculated during saving to imdjson 1.2.3 onwards / imd.
// imd BPM contains multiple entries. Only first entry is used during loading. All versions of imdjson do not support dynamic BPM.

// other Qt json restrictions like no manual sorting of QJsonObject, etc.

namespace {
inline uint32_t tick2timestamp(unsigned int tick, double bpm)
{
    return tick * 1250 / bpm;
}
inline unsigned int timestamp2tick(uint32_t timestamp, double bpm)
{
    return (timestamp + 1) * bpm / 1250;
}
}

RmeChartVersion::RmeChartVersion(const QString &versionStr)
{
    if (versionStr == QStringLiteral("1.2.1"))
        m_v = v1_2_1;
    else if (versionStr == QStringLiteral("1.2.2"))
        m_v = v1_2_2;
    else if (versionStr == QStringLiteral("1.2.3"))
        m_v = v1_2_3;
    else if (versionStr == QStringLiteral("1.3.0"))
        m_v = v1_3_0;
    else
        m_v = vUnknown;
}

QString RmeChartVersion::toString() const
{
    switch (m_v) {
    case v1_2_1:
        return QStringLiteral("1.2.1");
    case v1_2_2:
        return QStringLiteral("1.2.2");
    case v1_2_3:
        return QStringLiteral("1.2.3");
    case v1_3_0:
        return QStringLiteral("1.3.0");
    case vImd:
        return QStringLiteral("1.2.2");
    default:
        break;
    }

    // we have no choice but...
    return QStringLiteral("1.2.1");
}

QByteArray RmeChartNote::toImdNote(double bpm) const
{
    QByteArray arr;
    arr.resize(11);

    uint16_t *type = reinterpret_cast<uint16_t *>(arr.data());
    uint32_t *timestamp = reinterpret_cast<uint32_t *>(arr.data() + 2);
    uint8_t *datatrack = reinterpret_cast<uint8_t *>(arr.data() + 6);
    int32_t *action = reinterpret_cast<int32_t *>(arr.data() + 7);

    *datatrack = track - 3;
    *timestamp = tick2timestamp(tick, bpm);

    // attr == 3: long press start / single slide
    if (attr == 3) {
        // dur == 0: single slide / long press start with slide
        if (dur == 0) {
            if (isEnd)
                *type = 0x1;
            else
                *type = 0x61;

            *action = (long)(toTrack) - (long)(track);
        } else {
            if (isEnd)
                *type = 0x2;
            else
                *type = 0x62;

            int endTimeStamp = tick2timestamp(tick + dur, bpm);
            *action = endTimeStamp - *timestamp;
        }
    } else if (attr == 4) {
        if (dur == 0) {
            if (isEnd)
                *type = 0xA1;
            else
                *type = 0x21;

            *action = (long)(toTrack) - (long)(track);

        } else {
            if (isEnd)
                *type = 0xA2;
            else
                *type = 0x22;

            int endTimeStamp = tick2timestamp(tick + dur, bpm);
            *action = endTimeStamp - *timestamp;
        }
    } else {
        *type = 0;
        *action = 0;
    }

    return arr;
}

QJsonObject RmeChartNote::toJsonNote(RmeChartVersion version, double bpm, int idx) const
{
    // 1.2.1 is same as 1.2.2, while 1.2.3 is same as 1.3.0

    // 1.2.3 / 1.3.0: volume = 0, pan = 0
    int volume = 0;
    int pan = 0;
    // 1.2.1 / 1.2.2: volume = 127, pan = 64
    if (version <= RmeChartVersion::v1_2_2) {
        volume = 127;
        pan = 64;
    }

    QJsonObject ob;
    ob.insert(QStringLiteral("tick"), (qint64)tick);
    ob.insert(QStringLiteral("key"), key);
    ob.insert(QStringLiteral("dur"), (qint64)dur);
    ob.insert(QStringLiteral("isEnd"), isEnd ? 1 : 0);
    ob.insert(QStringLiteral("toTrack"), toTrack);
    ob.insert(QStringLiteral("volume"), volume);
    ob.insert(QStringLiteral("pan"), pan);
    ob.insert(QStringLiteral("attr"), attr);

    if (version >= RmeChartVersion::v1_2_3) {
        uint32_t time = tick2timestamp(tick, bpm);
        uint32_t timeEnd = tick2timestamp(tick + dur, bpm);
        uint32_t timeDur = timeEnd - time;
        if (toTrack != 0 && timeDur == 0)
            timeDur = 1;

        ob.insert(QStringLiteral("time"), (qint64)time);
        ob.insert(QStringLiteral("time_dur"), (qint64)timeDur);

        ob.insert(QStringLiteral("idx"), idx);
    }

    return ob;
}

RmeChartNote RmeChartNote::fromImdNote(const QByteArray &arr, double bpm, bool *ok)
{
    RmeChartNote note {0};

    if (ok == nullptr) {
        static bool _ok;
        ok = &_ok;
    }

    *ok = false;

    if (arr.length() != 11)
        return note;

    const uint16_t *type = reinterpret_cast<const uint16_t *>(arr.data());
    const uint32_t *timestamp = reinterpret_cast<const uint32_t *>(arr.data() + 2);
    const uint8_t *track = reinterpret_cast<const uint8_t *>(arr.data() + 6);
    const int32_t *action = reinterpret_cast<const int32_t *>(arr.data() + 7);

    note.track = *track + 3;
    note.tick = timestamp2tick(*timestamp, bpm);

    switch (*type) {
    case 0x0:
        break;
    case 0x1:
        note.attr = 3;
        note.dur = 0;
        note.isEnd = true;
        note.toTrack = note.track + *action;
        break;
    case 0x2:
        note.attr = 3;
        note.dur = timestamp2tick(*timestamp + *action, bpm) - note.tick;
        note.isEnd = true;
        note.toTrack = note.track;
        break;
    case 0x21:
        note.attr = 4;
        note.dur = 0;
        note.isEnd = false;
        note.toTrack = note.track + *action;
        break;
    case 0x22:
        note.attr = 4;
        note.dur = timestamp2tick(*timestamp + *action, bpm) - note.tick;
        note.isEnd = false;
        note.toTrack = note.track;
        break;
    case 0x61:
        note.attr = 3;
        note.dur = 0;
        note.isEnd = true;
        note.toTrack = note.track + *action;
        break;
    case 0x62:
        note.attr = 3;
        note.dur = timestamp2tick(*timestamp + *action, bpm) - note.tick;
        note.isEnd = true;
        note.toTrack = note.track;
        break;
    case 0xA1:
        note.attr = 4;
        note.dur = 0;
        note.isEnd = false;
        note.toTrack = note.track + *action;
        break;
    case 0xA2:
        note.attr = 4;
        note.dur = timestamp2tick(*timestamp + *action, bpm) - note.tick;
        note.isEnd = false;
        note.toTrack = note.track;
        break;
    default:
        return note;
    }

    *ok = true;
    return note;
}

RmeChartNote RmeChartNote::fromJsonNote(const QJsonObject &ob, unsigned char track, bool *ok)
{
    // ignore 1.2.3 time / time_dur / idx
    // ignore 1.2.1 volume / pan

    RmeChartNote note {0};
    note.track = track;

    if (ok == nullptr) {
        static bool _ok;
        ok = &_ok;
    }

    *ok = false;

    if (ob.contains(QStringLiteral("tick")))
        note.tick = ob.value(QStringLiteral("tick")).toVariant().toULongLong();
    else
        return note;

    if (ob.contains(QStringLiteral("key")))
        note.key = ob.value(QStringLiteral("key")).toVariant().toInt();
    else
        return note;

    if (ob.contains(QStringLiteral("dur")))
        note.dur = ob.value(QStringLiteral("dur")).toVariant().toULongLong();
    else
        return note;

    if (ob.contains(QStringLiteral("isEnd")))
        note.isEnd = (ob.value(QStringLiteral("isEnd")).toVariant().toULongLong() != 0);
    else
        return note;

    if (ob.contains(QStringLiteral("toTrack")))
        note.toTrack = ob.value(QStringLiteral("toTrack")).toVariant().toULongLong();
    else
        return note;

    if (ob.contains(QStringLiteral("attr")))
        note.attr = ob.value(QStringLiteral("attr")).toVariant().toULongLong();
    else
        return note;

    *ok = true;
    return note;
}

QByteArray RmeChart::toImd() const
{
    uint32_t totalTime = tick2timestamp(totalTick, bpm);
    uint32_t nBpm = totalTime * bpm / 60000. + 1;

    QList<RmeChartNote> sortedNotes = notes;
    std::sort(sortedNotes.begin(), sortedNotes.end());

    QByteArray arr;
    arr.resize(8 + nBpm * 12 + 6 + sortedNotes.length() * 11);
    *(reinterpret_cast<uint32_t *>(arr.data())) = totalTime;
    *(reinterpret_cast<uint32_t *>(arr.data() + 4)) = nBpm;
    for (int i = 0; i < nBpm; ++i) {
        *(reinterpret_cast<uint32_t *>(arr.data() + 8 + i * 12)) = (uint32_t)(60000. / bpm * i);
        *(reinterpret_cast<double *>(arr.data() + 8 + i * 12 + 4)) = bpm;
    }
    *(reinterpret_cast<uint16_t *>(arr.data() + 8 + nBpm * 12)) = (uint16_t)0x0303;
    *(reinterpret_cast<uint32_t *>(arr.data() + 8 + nBpm * 12 + 2)) = (uint32_t)sortedNotes.length();
    for (int i = 0; i < sortedNotes.length(); ++i) {
        QByteArray imdNote = sortedNotes.at(i).toImdNote(bpm);
        memcpy(arr.data() + 8 + nBpm * 12 + 6 + i * 11, imdNote.data(), 11);
    }

    return arr;
}

QJsonObject RmeChart::toJson(RmeChartVersion version) const
{
    QJsonObject out;
    out.insert(QStringLiteral("signature"), QStringLiteral("BNDQ"));
    out.insert(QStringLiteral("version"), version.toString());
    out.insert(QStringLiteral("tempo"), bpm);

    if (version >= RmeChartVersion::v1_3_0) {
        out.insert(QStringLiteral("duration"), (qint64)totalTick);
        out.insert(QStringLiteral("durationtime"), (qint64)tick2timestamp(totalTick, bpm));
    }

    if (version >= RmeChartVersion::v1_2_3)
        out.insert(QStringLiteral("samples"), QJsonArray());

    QMap<unsigned char /* track */, QJsonArray /* notes */> trackNoteMap;
    QList<unsigned char> tracks;

    QList<RmeChartNote> sortedNotes = notes;
    std::sort(sortedNotes.begin(), sortedNotes.end());
    for (int i = 0; i < sortedNotes.length(); ++i) {
        unsigned char track = sortedNotes.at(i).track;
        if (!tracks.contains(track))
            tracks << track;
        trackNoteMap[track].append(sortedNotes.at(i).toJsonNote(version, bpm, i));
    }
    std::sort(tracks.begin(), tracks.end());
    QJsonArray trackJson;
    foreach (unsigned char track, tracks) {
        QJsonObject oneTrackJson;
        oneTrackJson.insert(QStringLiteral("track"), (qint64)track);
        oneTrackJson.insert(QStringLiteral("note"), trackNoteMap.value(track));
        trackJson.append(oneTrackJson);
    }
    out.insert(QStringLiteral("tracks"), trackJson);

    return out;
}

RmeChart RmeChart::fromImd(const QByteArray &arr, bool *ok)
{
    RmeChart chart;

    if (ok == nullptr) {
        static bool _ok;
        ok = &_ok;
    }

    *ok = false;

    if (arr.size() < (8 + 12 + 6 + 11))
        return chart;

    chart.version = RmeChartVersion::vImd;

    const uint32_t *totalTime = reinterpret_cast<const uint32_t *>(arr.data());
    const uint32_t *bpmCount = reinterpret_cast<const uint32_t *>(arr.data() + 4);

    if (arr.size() < (8 + 12 * (*bpmCount) + 6 + 11))
        return chart;

    chart.bpm = *(reinterpret_cast<const double *>(arr.data() + 12));
    // only use first BPM as a whole!
    chart.totalTick = timestamp2tick(*totalTime, chart.bpm);

    const uint16_t *separator = reinterpret_cast<const uint16_t *>(arr.data() + 8 + (*bpmCount) * 12);
    if ((*separator) != 0x0303)
        return chart;

    const uint32_t *noteCount = reinterpret_cast<const uint32_t *>(arr.data() + 8 + (*bpmCount) * 12 + 2);

    if (arr.size() != (8 + 12 * (*bpmCount) + 6 + 11 * (*noteCount)))
        return chart;

    for (int i = 0; i < (int)(*noteCount); ++i) {
        QByteArray noteArr = QByteArray::fromRawData((arr.data() + 8 + (*bpmCount) * 12 + 2 + 4 + i * 11), 11);
        bool noteOk = false;
        chart.notes << RmeChartNote::fromImdNote(noteArr, chart.bpm, &noteOk);
        if (!noteOk)
            return chart;
    }
    *ok = true;
    return chart;
}

RmeChart RmeChart::fromJson(const QJsonObject &ob, bool *ok)
{
    RmeChart chart;

    if (ok == nullptr) {
        static bool _ok;
        ok = &_ok;
    }

    *ok = false;
    if (!ob.contains(QStringLiteral("signature")))
        return chart;
    if (ob.value(QStringLiteral("signature")) != QStringLiteral("BNDQ"))
        return chart;

    if (!ob.contains(QStringLiteral("version")))
        return chart;
    QString chartVersion = ob.value(QStringLiteral("version")).toString();
    chart.version = RmeChartVersion(chartVersion);
    if (chart.version == RmeChartVersion::vUnknown)
        return chart;

    if (!ob.contains(QStringLiteral("tempo")))
        return chart;
    chart.bpm = ob.value(QStringLiteral("tempo")).toDouble();
    if (chart.version >= RmeChartVersion::v1_3_0) {
        if (!ob.contains(QStringLiteral("duration")))
            return chart;
        if (!ob.contains(QStringLiteral("durationtime")))
            return chart;
        chart.totalTick = ob.value("duration").toVariant().toULongLong();
    }

    // ignore "samples" totally?

    // pre-1.3.0: calculate of totalTick is needed
    unsigned int maxTick = 0;

    if (!ob.contains(QStringLiteral("tracks")))
        return chart;
    QJsonValue tracksV = ob.value("tracks");
    if (!tracksV.isArray())
        return chart;
    QJsonArray tracksArr = tracksV.toArray();
    for (const QJsonValue &trackV : tracksArr) {
        if (!trackV.isObject())
            return chart;

        QJsonObject trackOb = trackV.toObject();
        if (!trackOb.contains(QStringLiteral("track")))
            return chart;
        unsigned char track = trackOb.value(QStringLiteral("track")).toVariant().toULongLong();
        if (!trackOb.contains(QStringLiteral("note")))
            return chart;
        QJsonValue notesV = trackOb.value(QStringLiteral("note"));
        if (!notesV.isArray())
            return chart;
        QJsonArray notesArr = notesV.toArray();
        for (const QJsonValue &noteV : notesArr) {
            if (!noteV.isObject())
                return chart;
            QJsonObject noteOb = noteV.toObject();
            bool noteOk = false;
            RmeChartNote k = RmeChartNote::fromJsonNote(noteOb, track, &noteOk);
            if (!noteOk)
                return chart;
            chart.notes << k;

            // pre-1.3.0: calculate of totalTick is needed
            unsigned int currentTick = k.tick + k.dur;
            if (maxTick < currentTick)
                maxTick = currentTick;
        }
    }

    // pre-1.3.0: calculate of totalTick is needed
    if (chart.version < RmeChartVersion::v1_3_0)
        chart.totalTick = maxTick;

    *ok = true;
    return chart;
}