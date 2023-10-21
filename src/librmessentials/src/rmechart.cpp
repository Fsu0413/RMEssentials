#include "rmechart.h"

#include <QJsonArray>

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
    default:
        break;
    }

    // we have no choice but...
    return QStringLiteral("1.2.1");
}

QByteArray RmeChartKey::toImdKey(double bpm) const
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

QJsonObject RmeChartKey::toJsonKey(RmeChartVersion version, double bpm, int idx) const
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

RmeChartKey RmeChartKey::fromImdKey(const QByteArray &arr, double bpm, bool *ok)
{
    RmeChartKey key {0};

    if (ok == nullptr) {
        static bool _ok;
        ok = &_ok;
    }

    *ok = false;

    if (arr.length() != 11)
        return key;

    const uint16_t *type = reinterpret_cast<const uint16_t *>(arr.data());
    const uint32_t *timestamp = reinterpret_cast<const uint32_t *>(arr.data() + 2);
    const uint8_t *track = reinterpret_cast<const uint8_t *>(arr.data() + 6);
    const int32_t *action = reinterpret_cast<const int32_t *>(arr.data() + 7);

    key.track = *track + 3;
    key.tick = timestamp2tick(*timestamp, bpm);

    switch (*type) {
    case 0x0:
        break;
    case 0x1:
        key.attr = 3;
        key.dur = 0;
        key.isEnd = true;
        key.toTrack = key.track + *action;
        break;
    case 0x2:
        key.attr = 3;
        key.dur = timestamp2tick(*timestamp + *action, bpm) - key.tick;
        key.isEnd = true;
        key.toTrack = 0;
        break;
    case 0x21:
        key.attr = 4;
        key.dur = 0;
        key.isEnd = false;
        key.toTrack = key.track + *action;
        break;
    case 0x22:
        key.attr = 4;
        key.dur = timestamp2tick(*timestamp + *action, bpm) - key.tick;
        key.isEnd = false;
        key.toTrack = 0;
        break;
    case 0x61:
        key.attr = 3;
        key.dur = 0;
        key.isEnd = true;
        key.toTrack = key.track + *action;
        break;
    case 0x62:
        key.attr = 3;
        key.dur = timestamp2tick(*timestamp + *action, bpm) - key.tick;
        key.isEnd = true;
        key.toTrack = 0;
        break;
    case 0xA1:
        key.attr = 4;
        key.dur = 0;
        key.isEnd = false;
        key.toTrack = key.track + *action;
        break;
    case 0xA2:
        key.attr = 4;
        key.dur = timestamp2tick(*timestamp + *action, bpm) - key.tick;
        key.isEnd = false;
        key.toTrack = 0;
        break;
    default:
        return key;
    }

    *ok = true;
    return key;
}

RmeChartKey RmeChartKey::fromJsonKey(const QJsonObject &ob, unsigned char track, bool *ok)
{
    // ignore 1.2.3 time / time_dur / idx
    // ignore 1.2.1 volume / pan

    RmeChartKey key {0};
    key.track = track;

    if (ok == nullptr) {
        static bool _ok;
        ok = &_ok;
    }

    *ok = false;

    if (ob.contains(QStringLiteral("tick")))
        key.tick = ob.value(QStringLiteral("tick")).toVariant().toULongLong();
    else
        return key;

    if (ob.contains(QStringLiteral("key")))
        key.key = ob.value(QStringLiteral("key")).toVariant().toULongLong();
    else
        return key;

    if (ob.contains(QStringLiteral("dur")))
        key.dur = ob.value(QStringLiteral("dur")).toVariant().toULongLong();
    else
        return key;

    if (ob.contains(QStringLiteral("isEnd")))
        key.isEnd = (ob.value(QStringLiteral("isEnd")).toVariant().toULongLong() != 0);
    else
        return key;

    if (ob.contains(QStringLiteral("toTrack")))
        key.toTrack = ob.value(QStringLiteral("toTrack")).toVariant().toULongLong();
    else
        return key;

    if (ob.contains(QStringLiteral("attr")))
        key.toTrack = ob.value(QStringLiteral("attr")).toVariant().toULongLong();
    else
        return key;

    *ok = true;
    return key;
}

QByteArray RmeChart::toImd() const
{
    uint32_t totalTime = tick2timestamp(totalTick, bpm);
    uint32_t nBpm = totalTime * bpm / 60000. + 1;

    QList<RmeChartKey> sortedKeys = keys;
    std::sort(sortedKeys.begin(), sortedKeys.end());

    QByteArray arr;
    arr.resize(8 + nBpm * 12 + 6 + sortedKeys.length() * 11);
    *(reinterpret_cast<uint32_t *>(arr.data())) = totalTime;
    *(reinterpret_cast<uint32_t *>(arr.data() + 4)) = nBpm;
    for (int i = 0; i < nBpm; ++i) {
        *(reinterpret_cast<uint32_t *>(arr.data() + 8 + i * 12)) = (uint32_t)(60000. / bpm * i);
        *(reinterpret_cast<double *>(arr.data() + 8 + i * 12 + 4)) = bpm;
    }
    *(reinterpret_cast<uint16_t *>(arr.data() + 8 + nBpm * 12)) = (uint16_t)0x0303;
    *(reinterpret_cast<uint32_t *>(arr.data() + 8 + nBpm * 12 + 2)) = (uint32_t)sortedKeys.length();
    for (int i = 0; i < sortedKeys.length(); ++i) {
        QByteArray imdKey = sortedKeys.at(i).toImdKey(bpm);
        memcpy(arr.data() + 8 + nBpm * 12 + 6 + i * 11, imdKey.data(), 11);
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

    QList<RmeChartKey> sortedKeys = keys;
    std::sort(sortedKeys.begin(), sortedKeys.end());
    for (int i = 0; i < sortedKeys.length(); ++i) {
        tracks << sortedKeys.at(i).track;
        trackNoteMap[sortedKeys.at(i).track].append(sortedKeys.at(i).toJsonKey(version, bpm, i));
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
        QByteArray noteArr = QByteArray::fromRawData((arr.data() + 8 + (*bpmCount) * 12 + 2 + i * 11), 11);
        bool keyOk = false;
        chart.keys << RmeChartKey::fromImdKey(noteArr, chart.bpm, &keyOk);
        if (!keyOk)
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
    QString chartVersion = ob.value("version").toString();
    RmeChartVersion version(chartVersion);
    if (version == RmeChartVersion::vUnknown)
        return chart;

    if (!ob.contains(QStringLiteral("tempo")))
        return chart;
    chart.bpm = ob.value(QStringLiteral("tempo")).toDouble();
    if (version >= RmeChartVersion::v1_3_0) {
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
            RmeChartKey k = RmeChartKey::fromJsonKey(noteOb, track, &noteOk);
            if (!noteOk)
                return chart;
            chart.keys << k;

            // pre-1.3.0: calculate of totalTick is needed
            unsigned int currentTick = k.tick + k.dur;
            if (maxTick < currentTick)
                maxTick = currentTick;
        }
    }

    // pre-1.3.0: calculate of totalTick is needed
    if (version < RmeChartVersion::v1_3_0)
        chart.totalTick = maxTick;

    *ok = true;
    return chart;
}
