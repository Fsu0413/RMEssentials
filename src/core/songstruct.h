#ifndef __SONGSTRUCT_H__
#define __SONGSTRUCT_H__

#include <QString>
#include <QVariantMap>

namespace RMSong
{
struct SongClientHeaderStruct
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
};

bool ByteArray2Header(const QByteArray &arr, SongClientHeaderStruct &header);
bool Header2ByteArray(const SongClientHeaderStruct &header, QByteArray &arr);
bool Map2Header(const QVariantMap &arr, SongClientHeaderStruct &header);
bool Herader2Map(const SongClientHeaderStruct &header, QVariantMap &arr);

struct SongClientItemStruct
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
    bool m_bIsLevelReward; // obsolete?
};

bool ByteArray2Song(const QByteArray &arr, SongClientItemStruct &song);
bool Song2ByteArray(const SongClientItemStruct &song, QByteArray &arr);
bool Map2Song(const QVariantMap &arr, SongClientItemStruct &song);
bool Song2Map(const SongClientItemStruct &song, QVariantMap &arr);
bool IsHidden(const SongClientItemStruct &song);
bool IsReward(const SongClientItemStruct &song);
bool IsDown(const SongClientItemStruct &song);
bool IsBuy(const SongClientItemStruct &song);
bool IsFree(const SongClientItemStruct &song);
bool IsLevel(const SongClientItemStruct &song);

bool sortByID(const SongClientItemStruct &a, const SongClientItemStruct &b);

struct PapaSongClientItemStruct
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
    char m_ucIsLevelReward; // obsolete?
    int m_iSongType;
};

bool ByteArray2Song(const QByteArray &arr, PapaSongClientItemStruct &song);
bool Song2ByteArray(const PapaSongClientItemStruct &song, QByteArray &arr);
bool Map2Song(const QVariantMap &arr, PapaSongClientItemStruct &song);
bool Song2Map(const PapaSongClientItemStruct &song, QVariantMap &arr);

bool sortByID(const PapaSongClientItemStruct &a, const PapaSongClientItemStruct &b);

}

#endif
