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

QByteArray RmeChartNote::toImdNote() const
{
    QByteArray arr;
    arr.resize(11);

    uint16_t *type = reinterpret_cast<uint16_t *>(arr.data());
    uint32_t *timestamp = reinterpret_cast<uint32_t *>(arr.data() + 2);
    uint8_t *datatrack = reinterpret_cast<uint8_t *>(arr.data() + 6);
    int32_t *action = reinterpret_cast<int32_t *>(arr.data() + 7);

    *datatrack = track - 3;
    *timestamp = this->timestamp;

    // attr == 3: long press start / single slide
    if (attr == 3) {
        // dur == 0: single slide / long press start with slide
        if (timeDur == 0) {
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

            *action = (int32_t)(timeDur);
        }
    } else if (attr == 4) {
        if (timeDur == 0) {
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

            *action = (int32_t)(timeDur);
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
    ob.insert(QStringLiteral("tick"), (qint64)(RmeChartNote::timestampToTick(timestamp, bpm)));
    ob.insert(QStringLiteral("key"), key);
    if (attr != 0 && timeDur != 0)
        ob.insert(QStringLiteral("dur"), (qint64)(RmeChartNote::timestampToTick(timeDur, bpm)));
    else
        ob.insert(QStringLiteral("dur"), (qint64)0);
    ob.insert(QStringLiteral("isEnd"), isEnd ? 1 : 0);
    ob.insert(QStringLiteral("toTrack"), toTrack);
    ob.insert(QStringLiteral("volume"), volume);
    ob.insert(QStringLiteral("pan"), pan);
    ob.insert(QStringLiteral("attr"), attr);

    if (version >= RmeChartVersion::v1_2_3) {
        ob.insert(QStringLiteral("time"), (qint64)timestamp);
        if (timeDur == 0 && attr != 0)
            ob.insert(QStringLiteral("time_dur"), (qint64)((qint64)toTrack - (qint64)track));
        else
            ob.insert(QStringLiteral("time_dur"), (qint64)timeDur);
        ob.insert(QStringLiteral("idx"), idx);
    }

    return ob;
}

bool RmeChartNote::operator<(const RmeChartNote &arg2) const
{
    // "one main rule": earlier note must be put before
    if (timestamp < arg2.timestamp)
        return true;

    // only here: notes appear same time need sorting
    if (timestamp == arg2.timestamp) {
        // make sure 0x21 / 0x61 appears before corresponding 0x22 / 0xA2
        // (this) 0x21 / 0x61 -> attr != 0, dur == 0, isEnd == false, toTrack != track
        // (arg2) 0x22 / 0xA2 -> attr == 4, dur != 0
        // (and) if toTrack == arg2.track and all of above then return true!
        if (attr != 0 && timeDur == 0 && !isEnd && toTrack != track && arg2.attr == 4 && arg2.timeDur != 0 && toTrack == arg2.track)
            return true;

        // strong sequence guarantee of above: reverse sequence is not allowed, so reverse arg2 and this for above judgement and return false
        if (arg2.attr != 0 && arg2.timeDur == 0 && !arg2.isEnd && arg2.toTrack != arg2.track && attr == 4 && timeDur != 0 && arg2.toTrack == track)
            return false;

        // After all, sort by track
        return track < arg2.track;
    }

    // still "one main rule": later note must be put afterwards
    return false;
}

RmeChartNote RmeChartNote::fromImdNote(const QByteArray &arr, bool *ok)
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
    note.timestamp = *timestamp;

    switch (*type) {
    case 0x0:
        break;
    case 0x1:
        note.attr = 3;
        note.timeDur = 0;
        note.isEnd = true;
        note.toTrack = note.track + *action;
        break;
    case 0x2:
        note.attr = 3;
        note.timeDur = *action;
        note.isEnd = true;
        note.toTrack = note.track;
        break;
    case 0x21:
        note.attr = 4;
        note.timeDur = 0;
        note.isEnd = false;
        note.toTrack = note.track + *action;
        break;
    case 0x22:
        note.attr = 4;
        note.timeDur = *action;
        note.isEnd = false;
        note.toTrack = note.track;
        break;
    case 0x61:
        note.attr = 3;
        note.timeDur = 0;
        note.isEnd = false;
        note.toTrack = note.track + *action;
        break;
    case 0x62:
        note.attr = 3;
        note.timeDur = *action;
        note.isEnd = false;
        note.toTrack = note.track;
        break;
    case 0xA1:
        note.attr = 4;
        note.timeDur = 0;
        note.isEnd = true;
        note.toTrack = note.track + *action;
        break;
    case 0xA2:
        note.attr = 4;
        note.timeDur = *action;
        note.isEnd = true;
        note.toTrack = note.track;
        break;
    default:
        return note;
    }

    *ok = true;
    return note;
}

RmeChartNote RmeChartNote::fromJsonNote(const QJsonObject &ob, unsigned char track, double bpm, bool *ok)
{
    // ignore 1.2.3+ idx
    // ignore 1.2.1/2 volume / pan

    RmeChartNote note {0};
    note.track = track;

    if (ok == nullptr) {
        static bool _ok;
        ok = &_ok;
    }

    *ok = false;

    // use 1.2.3+ time if possible
    if (ob.contains(QStringLiteral("time")))
        note.timestamp = ob.value(QStringLiteral("time")).toVariant().toULongLong();
    else if (ob.contains(QStringLiteral("tick")))
        note.timestamp = RmeChartNote::tickToTimestamp(ob.value(QStringLiteral("tick")).toVariant().toULongLong(), bpm);
    else
        return note;

    if (ob.contains(QStringLiteral("key")))
        note.key = ob.value(QStringLiteral("key")).toVariant().toInt();
    // silently accept data without "key". It is useless actually.

    // "dur" is the correct duration in actual chart. "time_dur" is only calculation result from IMD?
    // so if "dur" is 0, we should ignore time_dur completely!
    // the existing value in JSON converted from IMD may be completely wrong!
    // "time_dur" can only be referenced if "dur" is not zero
    if (ob.contains(QStringLiteral("dur"))) {
        qulonglong dur = ob.value(QStringLiteral("dur")).toVariant().toULongLong();
        if (dur != 0) {
            if (ob.contains(QStringLiteral("time_dur"))) {
                note.timeDur = ob.value(QStringLiteral("time_dur")).toVariant().toULongLong();
            } else if (ob.contains(QStringLiteral("tick"))) {
                qulonglong tick = ob.value(QStringLiteral("tick")).toVariant().toULongLong();
                qulonglong tickplusdur = tick + dur;
                note.timeDur = RmeChartNote::tickToTimestamp(tickplusdur, bpm) - note.timestamp;
            } else {
                return note;
            }
        } else {
            note.timeDur = 0;
        }
    } else {
        return note;
    }

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

unsigned int RmeChartNote::timestampToTick(uint32_t timestamp, double bpm)
{
    return qRound64(timestamp * bpm / 1250.);
}

uint32_t RmeChartNote::tickToTimestamp(unsigned int tick, double bpm)
{
    return tick * 1250 / bpm;
}

void RmeChart::sortNotes()
{
    std::list<RmeChartNote> temporaryList(notes.begin(), notes.end());
    std::list<RmeChartNote> sortedNotes;
    temporaryList.sort();

    for (std::list<RmeChartNote>::iterator it = temporaryList.begin(); it != temporaryList.end(); it = temporaryList.begin()) {
        if (it->attr != 0 && !it->isEnd) {
            // 0x21 / 0x22 / 0x61 / 0x62 (but 0x61 and 0x62 won't happen here, it will be spliced in following loop
            std::list<RmeChartNote> longPress;
            longPress.splice(longPress.end(), temporaryList, it);
            bool isEnd = false;
            while (!isEnd) {
                isEnd = true;
                unsigned char track = it->toTrack;
                unsigned int timestamp = it->timestamp + it->timeDur;
                for (it = temporaryList.begin(); it != temporaryList.end(); ++it) {
                    if (it->track == track && it->timestamp == timestamp && it->attr != 0) {
                        isEnd = it->isEnd;
                        longPress.splice(longPress.end(), temporaryList, it);
                        break;
                    }
                }
            }

            sortedNotes.splice(sortedNotes.end(), longPress);
        } else {
            // 0x00 / 0x01 / 0x02 / 0xA1 / 0xA2 (but 0xA1 and 0xA2 won't happen here, since it will be spliced in above if)
            sortedNotes.splice(sortedNotes.end(), temporaryList, it);
        }
    }

    notes = QList<RmeChartNote>(sortedNotes.begin(), sortedNotes.end());
}

int RmeChart::calculateTotalKeyAmount() const
{
    int totalNum = 0;
    foreach (const RmeChartNote &note, notes) {
        if (note.timeDur == 0) {
            if (!(note.attr != 0 && !note.isEnd))
                ++totalNum;
        } else {
            //            unsigned int tickBegin = RmeChartNote::timestampToTick(note.timestamp, bpm);
            //            unsigned int tickEnd = RmeChartNote::timestampToTick(note.timestamp + note.timeDur, bpm);
            //            totalNum += ((tickEnd - tickBegin) / 12 + 1);
            totalNum += (RmeChartNote::timestampToTick(note.timeDur, bpm) / 12 + 1);
        }
    }

    return totalNum;
}

QByteArray RmeChart::toImd() const
{
    uint32_t nBpm = totalTime * bpm / 60000. + 1;

    QByteArray arr;
    arr.resize(8 + nBpm * 12 + 6 + notes.length() * 11);
    *(reinterpret_cast<uint32_t *>(arr.data())) = totalTime;
    *(reinterpret_cast<uint32_t *>(arr.data() + 4)) = nBpm;
    for (int i = 0; i < nBpm; ++i) {
        *(reinterpret_cast<uint32_t *>(arr.data() + 8 + i * 12)) = (uint32_t)(60000. / bpm * i);
        *(reinterpret_cast<double *>(arr.data() + 8 + i * 12 + 4)) = bpm;
    }
    *(reinterpret_cast<uint16_t *>(arr.data() + 8 + nBpm * 12)) = (uint16_t)0x0303;
    *(reinterpret_cast<uint32_t *>(arr.data() + 8 + nBpm * 12 + 2)) = (uint32_t)notes.size();
    int i = 0;
    foreach (const RmeChartNote &note, notes) {
        QByteArray imdNote = note.toImdNote();
        memcpy(arr.data() + 8 + nBpm * 12 + 6 + (i++) * 11, imdNote.data(), 11);
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
        out.insert(QStringLiteral("duration"), (qint64)RmeChartNote::timestampToTick(totalTime, bpm));
        out.insert(QStringLiteral("durationtime"), (qint64)totalTime);
    }

    if (version >= RmeChartVersion::v1_2_3)
        out.insert(QStringLiteral("samples"), QJsonArray());

    QMap<unsigned char /* track */, QJsonArray /* notes */> trackNoteMap;
    QList<unsigned char> tracks;

    int i = 0;
    foreach (const RmeChartNote &note, notes) {
        unsigned char track = note.track;
        if (!tracks.contains(track))
            tracks << track;
        trackNoteMap[track].append(note.toJsonNote(version, bpm, i++));
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

    chart.totalTime = *(reinterpret_cast<const uint32_t *>(arr.data()));
    const uint32_t *bpmCount = reinterpret_cast<const uint32_t *>(arr.data() + 4);

    if (arr.size() < (8 + 12 * (*bpmCount) + 6 + 11))
        return chart;

    chart.bpm = *(reinterpret_cast<const double *>(arr.data() + 12));
    // only use first BPM as a whole!
    const uint16_t *separator = reinterpret_cast<const uint16_t *>(arr.data() + 8 + (*bpmCount) * 12);
    if ((*separator) != 0x0303)
        return chart;

    const uint32_t *noteCount = reinterpret_cast<const uint32_t *>(arr.data() + 8 + (*bpmCount) * 12 + 2);

    if (arr.size() != (8 + 12 * (*bpmCount) + 6 + 11 * (*noteCount)))
        return chart;

    for (int i = 0; i < (int)(*noteCount); ++i) {
        QByteArray noteArr = QByteArray::fromRawData((arr.data() + 8 + (*bpmCount) * 12 + 2 + 4 + i * 11), 11);
        bool noteOk = false;
        RmeChartNote n = RmeChartNote::fromImdNote(noteArr, &noteOk);
        if (!noteOk)
            return chart;

        // ignore incorrect track when loading
        if (n.track >= 3 && n.track <= 8)
            chart.notes << n;
    }

    chart.sortNotes();

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
    if (!ob.value(QStringLiteral("tempo")).isDouble())
        return chart;
    chart.bpm = ob.value(QStringLiteral("tempo")).toDouble();

    if (chart.version >= RmeChartVersion::v1_3_0) {
        if (!ob.contains(QStringLiteral("duration")))
            return chart;
        if (!ob.contains(QStringLiteral("durationtime")))
            return chart;
        if (!ob.value(QStringLiteral("durationtime")).isDouble())
            return chart;
        chart.totalTime = ob.value("durationtime").toVariant().toULongLong();
    }

    // ignore "samples" totally?

    // pre-1.3.0: calculate of totalTime is needed
    unsigned int maxTime = 0;

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
        if (!trackOb.value(QStringLiteral("track")).isDouble())
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
            RmeChartNote k = RmeChartNote::fromJsonNote(noteOb, track, chart.bpm, &noteOk);
            if (!noteOk)
                return chart;

            // some 1.2.1 / 1.2.2 chart records start / stop time using invalid tracks
            // so load notes from corresponding track but do not use them.
            if ((track >= 3) && (track <= 8))
                chart.notes << k;

            // pre-1.3.0: calculate of totalTick is needed
            unsigned int currentTime = k.timestamp + k.timeDur;
            if (maxTime < currentTime)
                maxTime = currentTime;
        }
    }

    // pre-1.3.0: calculate of totalTime is needed
    if (chart.version < RmeChartVersion::v1_3_0)
        chart.totalTime = maxTime;

    chart.sortNotes();

    *ok = true;
    return chart;
}
