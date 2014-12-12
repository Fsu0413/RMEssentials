#ifndef __SONGSTRUCT_H__
#define __SONGSTRUCT_H__

#include <QString>

class QByteArray;

namespace RMSong {
    struct SongStruct {
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

    bool Array2Song(const QByteArray &arr, SongStruct &song);
    bool Song2Array(const SongStruct &song, QByteArray &arr);
    bool IsHidden(const SongStruct &song);
    bool IsReward(const SongStruct &song);
    bool IsDown(const SongStruct &song);
    bool IsBuy(const SongStruct &song);
    bool IsFree(const SongStruct &song);
    bool IsLevel(const SongStruct &song);

    bool sortByID(const SongStruct &a, const SongStruct &b);
}

#endif