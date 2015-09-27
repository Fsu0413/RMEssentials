#ifndef __SONGSTRUCT_H__
#define __SONGSTRUCT_H__

namespace RMSong {
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

    bool Array2Song(const QByteArray &arr, SongClientItemStruct &song);
    bool Song2Array(const SongClientItemStruct &song, QByteArray &arr);
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

    bool Array2Song(const QByteArray &arr, PapaSongClientItemStruct &song);
    bool Song2Array(const PapaSongClientItemStruct &song, QByteArray &arr);

    bool sortByID(const PapaSongClientItemStruct &a, const PapaSongClientItemStruct &b);

}

#endif
