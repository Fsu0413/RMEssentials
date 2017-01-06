#include "rmesongstruct.h"

const QString RmeSong::RmeSongClientHeaderStruct::CreateTime = QStringLiteral("   0-00-00 00:00:00");

RmeSong::RmeSongClientHeaderStruct::RmeSongClientHeaderStruct()
    : Magic(0)
    , Version(0)
    , Unit(0)
    , Count(0)
    , ResVersion(0)
    , DataOffset(0)
{
}

void RmeSong::RmeSongClientHeaderStruct::parseByteArray(const QByteArray &arr)
{
    Q_ASSERT(arr.length() == 0x88);
    const char *data = arr.constData();

#define GETX(type, name, offset) \
    this->name = *(reinterpret_cast<const type *>(data + offset))
#define GETSTR(name, offset) \
    this->name = QString::fromUtf8(data + offset)

    GETX(int32_t, Magic, 0x0);
    GETX(int32_t, Version, 0x4);
    GETX(int32_t, Unit, 0x8);
    GETX(int32_t, Count, 0xc);
    GETSTR(MetalibHash, 0x10);
    GETX(int32_t, ResVersion, 0x34);
    GETSTR(ResEncoding, 0x40);
    GETSTR(ContentHash, 0x60);
    GETX(int32_t, DataOffset, 0x84);

#undef GETSTR
#undef GETX
}

QByteArray RmeSong::RmeSongClientHeaderStruct::toByteArray() const
{
    QByteArray arr;
    arr.resize(0x88);
    memset(arr.data(), 0, 0x88);

#define SETX(name, offset)                                                \
    do {                                                                  \
        size_t length = sizeof(this->name);                               \
        const char *data = reinterpret_cast<const char *>(&(this->name)); \
        for (size_t i = 0; i < length; ++i)                               \
            arr[offset + i] = data[i];                                    \
    } while (false)

#define SETSTR(name, offset, len)                 \
    do {                                          \
        QByteArray dataArr = this->name.toUtf8(); \
        const char *data = dataArr.constData();   \
        for (int i = 0; i < len; ++i) {           \
            if (i < dataArr.length())             \
                arr[offset + i] = data[i];        \
            else                                  \
                arr[offset + i] = '\0';           \
        }                                         \
    } while (false)

    SETX(Magic, 0x0);
    SETX(Version, 0x4);
    SETX(Unit, 0x8);
    SETX(Count, 0xc);
    SETSTR(MetalibHash, 0x10, 0x24);
    SETX(ResVersion, 0x34);
    SETSTR(ResEncoding, 0x40, 0x20);
    SETSTR(ContentHash, 0x60, 0x24);
    SETX(DataOffset, 0x84);

#undef SETSTR
#undef SETX

    return arr;
}

void RmeSong::RmeSongClientHeaderStruct::parseMap(const QVariantMap &map)
{
#define GETSTR(name) \
    this->name = map[QStringLiteral(#name)].toString()
#define GETINT(name) \
    this->name = map[QStringLiteral(#name)].toString().trimmed().toInt()

    GETINT(Magic);
    GETINT(Version);
    GETINT(Unit);
    GETINT(Count);
    GETSTR(MetalibHash);
    GETINT(ResVersion);
    GETSTR(ResEncoding);
    GETSTR(ContentHash);
    GETINT(DataOffset);

#undef GETSTR
#undef GETINT
}

QVariantMap RmeSong::RmeSongClientHeaderStruct::toMap() const
{
    QVariantMap map;

#define SETSTR(name) \
    map[QStringLiteral(#name)] = this->name
#define SETINT(name) \
    map[QStringLiteral(#name)] = QString::number(static_cast<int>(this->name)) + QStringLiteral(" ")

    SETINT(Magic);
    SETINT(Version);
    SETINT(Unit);
    SETINT(Count);
    SETSTR(MetalibHash);
    SETINT(ResVersion);
    SETSTR(CreateTime); // important!!!! It don't need any convertion and can be used here
    SETSTR(ResEncoding);
    SETSTR(ContentHash);
    SETINT(DataOffset);

#undef SETSTR
#undef SETINT

    return map;
}

RmeSong::RmeSongClientItemStruct::RmeSongClientItemStruct()
    : m_ushSongID(0)
    , m_iVersion(0)
    , m_iGameTime(0)
    , m_iRegion(0)
    , m_iStyle(0)
    , m_ucIsNew(0)
    , m_ucIsHot(0)
    , m_ucIsRecommend(0)
    , m_ucIsOpen(0)
    , m_ucCanBuy(false)
    , m_iOrderIndex(0)
    , m_bIsFree(false)
    , m_bSongPkg(false)
    , m_ush4KeyEasy(0)
    , m_ush4KeyNormal(0)
    , m_ush4KeyHard(0)
    , m_ush5KeyEasy(0)
    , m_ush5KeyNormal(0)
    , m_ush5KeyHard(0)
    , m_ush6KeyEasy(0)
    , m_ush6KeyNormal(0)
    , m_ush6KeyHard(0)
    , m_iPrice(0)
    , m_iVipFlag(0)
    , m_bIsHide(false)
    , m_bIsReward(false)
    , m_bIsLevelReward(false)

{
}

void RmeSong::RmeSongClientItemStruct::parseByteArray(const QByteArray &arr)
{
    Q_ASSERT(arr.length() == 0x33e);
    const char *data = arr.constData();

#define GETX(type, name, offset) \
    this->m_##name = *(reinterpret_cast<const type *>(data + offset))
#define GETSTR(name, offset) \
    this->m_##name = QString::fromUtf8(data + offset)

    GETX(int16_t, ushSongID, 0x0);
    GETX(int32_t, iVersion, 0x2);
    GETSTR(szSongName, 0x6);
    GETSTR(szPath, 0x46);
    GETSTR(szArtist, 0x86);
    GETSTR(szComposer, 0xc6);
    GETSTR(szSongTime, 0x106);
    GETX(int32_t, iGameTime, 0x146);
    GETX(int32_t, iRegion, 0x14a);
    GETX(int32_t, iStyle, 0x14e);
    GETX(int16_t, ucIsNew, 0x152);
    GETX(int16_t, ucIsHot, 0x154);
    GETX(int16_t, ucIsRecommend, 0x156);
    GETSTR(szBPM, 0x158);
    GETX(int16_t, ucIsOpen, 0x198);
    GETX(bool, ucCanBuy, 0x19a);
    GETX(int32_t, iOrderIndex, 0x19b);
    GETX(bool, bIsFree, 0x19f);
    GETX(bool, bSongPkg, 0x1a0);
    GETSTR(szFreeBeginTime, 0x1a1);
    GETSTR(szFreeEndTime, 0x1e1);
    GETX(int16_t, ush4KeyEasy, 0x221);
    GETX(int16_t, ush4KeyNormal, 0x223);
    GETX(int16_t, ush4KeyHard, 0x225);
    GETX(int16_t, ush5KeyEasy, 0x227);
    GETX(int16_t, ush5KeyNormal, 0x229);
    GETX(int16_t, ush5KeyHard, 0x22b);
    GETX(int16_t, ush6KeyEasy, 0x22d);
    GETX(int16_t, ush6KeyNormal, 0x22f);
    GETX(int16_t, ush6KeyHard, 0x231);
    GETX(int32_t, iPrice, 0x233);
    GETSTR(szNoteNumber, 0x237);
    GETSTR(szProductID, 0x2b7);
    GETX(int32_t, iVipFlag, 0x337);
    GETX(bool, bIsHide, 0x33b);
    GETX(bool, bIsReward, 0x33c);
    GETX(bool, bIsLevelReward, 0x33d);

#undef GETSTR
#undef GETX
}

QByteArray RmeSong::RmeSongClientItemStruct::toByteArray() const
{
    QByteArray arr;
    arr.resize(0x33e);
    memset(arr.data(), 0, 0x33e);

#define SETX(name, offset)                                                    \
    do {                                                                      \
        size_t length = sizeof(this->m_##name);                               \
        const char *data = reinterpret_cast<const char *>(&(this->m_##name)); \
        for (size_t i = 0; i < length; ++i)                                   \
            arr[offset + i] = data[i];                                        \
    } while (false)

#define SETSTR(name, offset, len)                     \
    do {                                              \
        QByteArray dataArr = this->m_##name.toUtf8(); \
        const char *data = dataArr.constData();       \
        for (int i = 0; i < len; ++i) {               \
            if (i < dataArr.length())                 \
                arr[offset + i] = data[i];            \
            else                                      \
                arr[offset + i] = '\0';               \
        }                                             \
    } while (false)

    SETX(ushSongID, 0x0);
    SETX(iVersion, 0x2);
    SETSTR(szSongName, 0x6, 0x40);
    SETSTR(szPath, 0x46, 0x40);
    SETSTR(szArtist, 0x86, 0x40);
    SETSTR(szComposer, 0xc6, 0x40);
    SETSTR(szSongTime, 0x106, 0x40);
    SETX(iGameTime, 0x146);
    SETX(iRegion, 0x14a);
    SETX(iStyle, 0x14e);
    SETX(ucIsNew, 0x152);
    SETX(ucIsHot, 0x154);
    SETX(ucIsRecommend, 0x156);
    SETSTR(szBPM, 0x158, 0x40);
    SETX(ucIsOpen, 0x198);
    SETX(ucCanBuy, 0x19a);
    SETX(iOrderIndex, 0x19b);
    SETX(bIsFree, 0x19f);
    SETX(bSongPkg, 0x1a0);
    SETSTR(szFreeBeginTime, 0x1a1, 0x40);
    SETSTR(szFreeEndTime, 0x1e1, 0x40);
    SETX(ush4KeyEasy, 0x221);
    SETX(ush4KeyNormal, 0x223);
    SETX(ush4KeyHard, 0x225);
    SETX(ush5KeyEasy, 0x227);
    SETX(ush5KeyNormal, 0x229);
    SETX(ush5KeyHard, 0x22b);
    SETX(ush6KeyEasy, 0x22d);
    SETX(ush6KeyNormal, 0x22f);
    SETX(ush6KeyHard, 0x231);
    SETX(iPrice, 0x233);
    SETSTR(szNoteNumber, 0x237, 0x80);
    SETSTR(szProductID, 0x2b7, 0x80);
    SETX(iVipFlag, 0x337);
    SETX(bIsHide, 0x33b);
    SETX(bIsReward, 0x33c);
    SETX(bIsLevelReward, 0x33d);

#undef SETSTR
#undef SETX

    return arr;
}

void RmeSong::RmeSongClientItemStruct::parseMap(const QVariantMap &map)
{
#define GETSTR(name) \
    this->m_##name = map[QStringLiteral("m_" #name)].toString()
#define GETINT(name) \
    this->m_##name = map[QStringLiteral("m_" #name)].toString().trimmed().toInt()
#define GETHEX(name) \
    this->m_##name = map[QStringLiteral("m_" #name)].toString().trimmed().mid(2).toInt(nullptr, 16)

    GETINT(ushSongID);
    GETINT(iVersion);
    GETSTR(szSongName);
    GETSTR(szPath);
    GETSTR(szArtist);
    GETSTR(szComposer);
    GETSTR(szSongTime);
    GETINT(iGameTime);
    GETINT(iRegion);
    GETINT(iStyle);
    GETINT(ucIsNew);
    GETINT(ucIsHot);
    GETINT(ucIsRecommend);
    GETSTR(szBPM);
    GETINT(ucIsOpen);
    GETHEX(ucCanBuy);
    GETINT(iOrderIndex);
    GETHEX(bIsFree);
    GETHEX(bSongPkg);
    GETSTR(szFreeBeginTime);
    GETSTR(szFreeEndTime);
    GETINT(ush4KeyEasy);
    GETINT(ush4KeyNormal);
    GETINT(ush4KeyHard);
    GETINT(ush5KeyEasy);
    GETINT(ush5KeyNormal);
    GETINT(ush5KeyHard);
    GETINT(ush6KeyEasy);
    GETINT(ush6KeyNormal);
    GETINT(ush6KeyHard);
    GETINT(iPrice);
    GETSTR(szNoteNumber);
    GETSTR(szProductID);
    GETINT(iVipFlag);
    GETHEX(bIsHide);
    GETHEX(bIsReward);
    GETHEX(bIsLevelReward);

#undef GETSTR
#undef GETINT
#undef GETHEX
}

QVariantMap RmeSong::RmeSongClientItemStruct::toMap() const
{
    QVariantMap map;

#define SETSTR(name) \
    map[QStringLiteral("m_" #name)] = this->m_##name
#define SETINT(name) \
    map[QStringLiteral("m_" #name)] = QString::number(static_cast<int>(this->m_##name)) + QStringLiteral(" ")
#define SETHEX(name) \
    map[QStringLiteral("m_" #name)] = QStringLiteral("0x") + QString::number(static_cast<int>(this->m_##name), 16) + QStringLiteral(" ")

    SETINT(ushSongID);
    SETINT(iVersion);
    SETSTR(szSongName);
    SETSTR(szPath);
    SETSTR(szArtist);
    SETSTR(szComposer);
    SETSTR(szSongTime);
    SETINT(iGameTime);
    SETINT(iRegion);
    SETINT(iStyle);
    SETINT(ucIsNew);
    SETINT(ucIsHot);
    SETINT(ucIsRecommend);
    SETSTR(szBPM);
    SETINT(ucIsOpen);
    SETHEX(ucCanBuy);
    SETINT(iOrderIndex);
    SETHEX(bIsFree);
    SETHEX(bSongPkg);
    SETSTR(szFreeBeginTime);
    SETSTR(szFreeEndTime);
    SETINT(ush4KeyEasy);
    SETINT(ush4KeyNormal);
    SETINT(ush4KeyHard);
    SETINT(ush5KeyEasy);
    SETINT(ush5KeyNormal);
    SETINT(ush5KeyHard);
    SETINT(ush6KeyEasy);
    SETINT(ush6KeyNormal);
    SETINT(ush6KeyHard);
    SETINT(iPrice);
    SETSTR(szNoteNumber);
    SETSTR(szProductID);
    SETINT(iVipFlag);
    SETHEX(bIsHide);
    SETHEX(bIsReward);
    SETHEX(bIsLevelReward);

#undef SETSTR
#undef SETINT
#undef SETHEX

    return map;
}

bool RmeSong::RmeSongClientItemStruct::isHidden() const
{
    return m_bIsHide;
}

bool RmeSong::RmeSongClientItemStruct::isReward() const
{
    return m_bIsReward && !isLevel();
}

bool RmeSong::RmeSongClientItemStruct::isDown() const
{
    return m_ucIsOpen == 0;
}

bool RmeSong::RmeSongClientItemStruct::isBuy() const
{
    return m_ucCanBuy;
}

bool RmeSong::RmeSongClientItemStruct::isFree() const
{
    return !isBuy() && !isDown() && !isHidden() && !isLevel() && !isReward();
}

bool RmeSong::RmeSongClientItemStruct::isLevel() const
{
    // BIG IMPORTANT HACK!!!!
    // RM used No251-262 as level reward, and m_bIsLevelReward doesn't make sense anymore.
    return m_ushSongID >= 251 && m_ushSongID <= 262;
}

bool RmeSong::RmeSongClientItemStruct::sortByID(const RmeSong::RmeSongClientItemStruct &a, const RmeSong::RmeSongClientItemStruct &b)
{
    return a.m_ushSongID < b.m_ushSongID;
}

RmeSong::RmePapaSongClientItemStruct::RmePapaSongClientItemStruct()
    : m_ushSongID(0)
    , m_iVersion(0)
    , m_cDifficulty(0)
    , m_cLevel(0)
    , m_iGameTime(0)
    , m_iOrderIndex(0)
    , m_ucIsOpen(0)
    , m_ucIsFree(0)
    , m_ucIsHide(0)
    , m_ucIsReward(0)
    , m_ucIsLevelReward(0)
    , m_iSongType(0)
{
}

void RmeSong::RmePapaSongClientItemStruct::parseByteArray(const QByteArray &arr)
{
    Q_ASSERT(arr.length() == 0x169);
    const char *data = arr.constData();

#define GETX(type, name, offset) \
    this->m_##name = *(reinterpret_cast<const type *>(data + offset))
#define GETSTR(name, offset) \
    this->m_##name = QString::fromUtf8(data + offset)

    GETX(int16_t, ushSongID, 0x0);
    GETX(int32_t, iVersion, 0x2);
    GETSTR(szSongName, 0x6);
    GETX(int8_t, cDifficulty, 0x46);
    GETX(int8_t, cLevel, 0x47);
    GETSTR(szPath, 0x48);
    GETSTR(szArtist, 0x88);
    GETSTR(szSongTime, 0xc8);
    GETX(int32_t, iGameTime, 0x108);
    GETSTR(szRegion, 0x10c);
    GETSTR(szStyle, 0x120);
    GETSTR(szBPM, 0x134);
    GETSTR(szNoteNumber, 0x148);
    GETX(int32_t, iOrderIndex, 0x15c);
    GETX(int8_t, ucIsOpen, 0x160);
    GETX(int8_t, ucIsFree, 0x161);
    GETX(int8_t, ucIsHide, 0x162);
    GETX(int8_t, ucIsReward, 0x163);
    GETX(int8_t, ucIsLevelReward, 0x164);
    GETX(int32_t, iSongType, 0x165);

#undef GETSTR
#undef GETX
}

QByteArray RmeSong::RmePapaSongClientItemStruct::toByteArray() const
{
    QByteArray arr;
    arr.resize(0x169);
    memset(arr.data(), 0, 0x169);

#define SETX(name, offset)                                                    \
    do {                                                                      \
        size_t length = sizeof(this->m_##name);                               \
        const char *data = reinterpret_cast<const char *>(&(this->m_##name)); \
        for (size_t i = 0; i < length; ++i)                                   \
            arr[offset + i] = data[i];                                        \
    } while (false)

#define SETSTR(name, offset, len)                     \
    do {                                              \
        QByteArray dataArr = this->m_##name.toUtf8(); \
        const char *data = dataArr.constData();       \
        for (int i = 0; i < len; ++i) {               \
            if (i < dataArr.length())                 \
                arr[offset + i] = data[i];            \
            else                                      \
                arr[offset + i] = '\0';               \
        }                                             \
    } while (false)

    SETX(ushSongID, 0x0);
    SETX(iVersion, 0x2);
    SETSTR(szSongName, 0x6, 0x40);
    SETX(cDifficulty, 0x46);
    SETX(cLevel, 0x47);
    SETSTR(szPath, 0x48, 0x40);
    SETSTR(szArtist, 0x88, 0x40);
    SETSTR(szSongTime, 0xc8, 0x40);
    SETX(iGameTime, 0x108);
    SETSTR(szRegion, 0x10c, 0x14);
    SETSTR(szStyle, 0x120, 0x14);
    SETSTR(szBPM, 0x134, 0x14);
    SETSTR(szNoteNumber, 0x148, 0x14);
    SETX(iOrderIndex, 0x15c);
    SETX(ucIsOpen, 0x160);
    SETX(ucIsFree, 0x161);
    SETX(ucIsHide, 0x162);
    SETX(ucIsReward, 0x163);
    SETX(ucIsLevelReward, 0x164);
    SETX(iSongType, 0x165);

#undef SETSTR
#undef SETX

    return arr;
}

void RmeSong::RmePapaSongClientItemStruct::parseMap(const QVariantMap &map)
{
#define GETSTR(name) \
    this->m_##name = map[QStringLiteral("m_" #name)].toString()
#define GETINT(name) \
    this->m_##name = map[QStringLiteral("m_" #name)].toString().trimmed().toInt()
#define GETHEX(name) \
    this->m_##name = map[QStringLiteral("m_" #name)].toString().trimmed().mid(2).toInt(nullptr, 16)

    GETINT(ushSongID);
    GETINT(iVersion);
    GETSTR(szSongName);
    GETHEX(cDifficulty);
    GETHEX(cLevel);
    GETSTR(szPath);
    GETSTR(szArtist);
    GETSTR(szSongTime);
    GETINT(iGameTime);
    GETSTR(szRegion);
    GETSTR(szStyle);
    GETSTR(szBPM);
    GETSTR(szNoteNumber);
    GETINT(iOrderIndex);
    GETHEX(ucIsOpen);
    GETHEX(ucIsFree);
    GETHEX(ucIsHide);
    GETHEX(ucIsReward);
    GETHEX(ucIsLevelReward);
    GETINT(iSongType);

#undef GETSTR
#undef GETINT
#undef GETHEX
}

QVariantMap RmeSong::RmePapaSongClientItemStruct::toMap() const
{
    QVariantMap map;

#define SETSTR(name) \
    map[QStringLiteral("m_" #name)] = this->m_##name
#define SETINT(name) \
    map[QStringLiteral("m_" #name)] = QString::number(static_cast<int>(this->m_##name)) + QStringLiteral(" ")
#define SETHEX(name) \
    map[QStringLiteral("m_" #name)] = QStringLiteral("0x") + QString::number(static_cast<int>(this->m_##name), 16) + QStringLiteral(" ")

    SETINT(ushSongID);
    SETINT(iVersion);
    SETSTR(szSongName);
    SETHEX(cDifficulty);
    SETHEX(cLevel);
    SETSTR(szPath);
    SETSTR(szArtist);
    SETSTR(szSongTime);
    SETINT(iGameTime);
    SETSTR(szRegion);
    SETSTR(szStyle);
    SETSTR(szBPM);
    SETSTR(szNoteNumber);
    SETINT(iOrderIndex);
    SETHEX(ucIsOpen);
    SETHEX(ucIsFree);
    SETHEX(ucIsHide);
    SETHEX(ucIsReward);
    SETHEX(ucIsLevelReward);
    SETINT(iSongType);

#undef SETSTR
#undef SETINT
#undef SETHEX

    return map;
}

bool RmeSong::RmePapaSongClientItemStruct::sortByID(const RmeSong::RmePapaSongClientItemStruct &a, const RmeSong::RmePapaSongClientItemStruct &b)
{
    return a.m_ushSongID < b.m_ushSongID;
}
