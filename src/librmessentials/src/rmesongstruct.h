#ifndef __SONGSTRUCT_H__
#define __SONGSTRUCT_H__

#include "rmeglobal.h"

#include <QJsonObject>
#include <QString>
#include <QVariantMap>
#include <cstdint>

#if 0
class LIBRMESSENTIALS_EXPORT RmeSongClientStruct
#endif

namespace RmeSong {
struct LIBRMESSENTIALS_EXPORT RmeSongClientHeaderStruct
{
    int32_t Magic;
    int32_t Version;
    int32_t Unit;
    int32_t Count;
    QString MetalibHash;
    int32_t ResVersion;
    static const QString CreateTime; // CreateTime?? better static?
    QString ResEncoding;
    QString ContentHash;
    int32_t DataOffset;

    RmeSongClientHeaderStruct();

    void parseByteArray(const QByteArray &arr);
    QByteArray toByteArray() const;

    void parseMap(const QVariantMap &map);
    QVariantMap toMap() const;
};

struct LIBRMESSENTIALS_EXPORT RmeSongClientItemStruct
{
    int16_t m_ushSongID;
    int32_t m_iVersion; // what's this?
    QString m_szSongName;
    QString m_szPath;
    QString m_szArtist;
    QString m_szComposer;
    QString m_szSongTime;
    int32_t m_iGameTime;
    int32_t m_iRegion;
    int32_t m_iStyle;
    int16_t m_ucIsNew;
    int16_t m_ucIsHot;
    int16_t m_ucIsRecommend;
    QString m_szBPM;
    int16_t m_ucIsOpen;
    bool m_ucCanBuy;
    int32_t m_iOrderIndex;
    bool m_bIsFree;
    bool m_bSongPkg;
    QString m_szFreeBeginTime;
    QString m_szFreeEndTime;
    int16_t m_ush4KeyEasy;
    int16_t m_ush4KeyNormal;
    int16_t m_ush4KeyHard;
    int16_t m_ush5KeyEasy;
    int16_t m_ush5KeyNormal;
    int16_t m_ush5KeyHard;
    int16_t m_ush6KeyEasy;
    int16_t m_ush6KeyNormal;
    int16_t m_ush6KeyHard;
    int32_t m_iPrice;
    QString m_szNoteNumber;
    QString m_szProductID;
    int32_t m_iVipFlag;
    bool m_bIsHide;
    bool m_bIsReward;
    bool m_bIsLevelReward;

    RmeSongClientItemStruct();

    void parseByteArray(const QByteArray &arr);
    QByteArray toByteArray() const;

    void parseMap(const QVariantMap &map);
    QVariantMap toMap() const;

    QJsonObject createPatch(const RmeSongClientItemStruct &orig) const;
    bool applyPatch(const QJsonObject &patch);

    void prepareForUserMakingNotes();

    bool isHidden() const;
    bool isReward() const;
    bool isDown() const;
    bool isBuy() const;
    bool isFree() const;
    bool isLevel() const;

    static bool sortByID(const RmeSongClientItemStruct &a, const RmeSongClientItemStruct &b);
};

struct LIBRMESSENTIALS_EXPORT RmePapaSongClientItemStruct
{
    int16_t m_ushSongID;
    int32_t m_iVersion; // what's this?
    QString m_szSongName;
    int8_t m_cDifficulty;
    int8_t m_cLevel;
    QString m_szPath;
    QString m_szArtist;
    QString m_szSongTime;
    int32_t m_iGameTime;
    QString m_szRegion;
    QString m_szStyle;
    QString m_szBPM;
    QString m_szNoteNumber;
    int32_t m_iOrderIndex;
    int8_t m_ucIsOpen;
    int8_t m_ucIsFree;
    int8_t m_ucIsHide;
    int8_t m_ucIsReward;
    int8_t m_ucIsLevelReward;
    int32_t m_iSongType;

    RmePapaSongClientItemStruct();

    void parseByteArray(const QByteArray &arr);
    QByteArray toByteArray() const;

    void parseMap(const QVariantMap &map);
    QVariantMap toMap() const;

    QJsonObject createPatch(const RmePapaSongClientItemStruct &orig) const;
    bool applyPatch(const QJsonObject &patch);

    static bool sortByID(const RmePapaSongClientItemStruct &a, const RmePapaSongClientItemStruct &b);
};
}

#endif
