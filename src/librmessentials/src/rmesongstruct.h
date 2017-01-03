#ifndef __SONGSTRUCT_H__
#define __SONGSTRUCT_H__

#include "rmeglobal.h"

#include <QString>
#include <QVariantMap>

#if 0
class LIBRMESSENTIALS_EXPORT RmeSongClientStruct
#endif

namespace RmeSong {
struct LIBRMESSENTIALS_EXPORT RmeSongClientHeaderStruct
{
    int Magic;
    int Version;
    int Unit;
    int Count;
    QString MetalibHash;
    int ResVersion;
    static const QString CreateTime; // CreateTime?? better static?
    QString ResEncoding;
    QString ContentHash;
    int DataOffset;

    RmeSongClientHeaderStruct();

    void parseByteArray(const QByteArray &arr);
    QByteArray toByteArray() const;
    void parseMap(const QVariantMap &map);
    QVariantMap toMap() const;
};

struct LIBRMESSENTIALS_EXPORT RmeSongClientItemStruct
{
    short m_ushSongID;
    int m_iVersion; // what's this?
    QString m_szSongName;
    QString m_szPath;
    QString m_szArtist;
    QString m_szComposer;
    QString m_szSongTime;
    int m_iGameTime;
    int m_iRegion;
    int m_iStyle;
    short m_ucIsNew;
    short m_ucIsHot;
    short m_ucIsRecommend;
    QString m_szBPM;
    short m_ucIsOpen;
    bool m_ucCanBuy;
    int m_iOrderIndex;
    bool m_bIsFree;
    bool m_bSongPkg;
    QString m_szFreeBeginTime;
    QString m_szFreeEndTime;
    short m_ush4KeyEasy;
    short m_ush4KeyNormal;
    short m_ush4KeyHard;
    short m_ush5KeyEasy;
    short m_ush5KeyNormal;
    short m_ush5KeyHard;
    short m_ush6KeyEasy;
    short m_ush6KeyNormal;
    short m_ush6KeyHard;
    int m_iPrice;
    QString m_szNoteNumber;
    QString m_szProductID;
    int m_iVipFlag;
    bool m_bIsHide;
    bool m_bIsReward;
    bool m_bIsLevelReward;

    RmeSongClientItemStruct();

    void parseByteArray(const QByteArray &arr);
    QByteArray toByteArray() const;
    void parseMap(const QVariantMap &map);
    QVariantMap toMap() const;

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
    short m_ushSongID;
    int m_iVersion; // what's this?
    QString m_szSongName;
    char m_cDifficulty;
    char m_cLevel;
    QString m_szPath;
    QString m_szArtist;
    QString m_szSongTime;
    int m_iGameTime;
    QString m_szRegion;
    QString m_szStyle;
    QString m_szBPM;
    QString m_szNoteNumber;
    int m_iOrderIndex;
    char m_ucIsOpen;
    char m_ucIsFree;
    char m_ucIsHide;
    char m_ucIsReward;
    char m_ucIsLevelReward;
    int m_iSongType;

    RmePapaSongClientItemStruct();

    void parseByteArray(const QByteArray &arr);
    QByteArray toByteArray() const;
    void parseMap(const QVariantMap &map);
    QVariantMap toMap() const;

    static bool sortByID(const RmePapaSongClientItemStruct &a, const RmePapaSongClientItemStruct &b);
};
}

#endif
