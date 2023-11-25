#ifndef CHARTVIEWERMODEL_H__INCLUDED
#define CHARTVIEWERMODEL_H__INCLUDED

#include <QAbstractTableModel>

class RmeChart;

class ChartViewerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum
    {
        ChartDrawRole = Qt::UserRole + 1,
    };

    enum NoteType
    {
        SingleClick = 0, // 0x0
        SingleSlide = 1, // 0x1
        SingleLongPress = 2, // 0x2
        LongPressContinueChangeTrack = 3, // 0x21
        LongPressContinueTargetTrack = 4, // 0x22
        LongPressStartChangeTrack = 5, // 0x61
        LongPressStart = 6, // 0x62
        LongPressEndWithSlide = 7, // 0xA1, so-called "Hook"
        LongPressEnd = 8, // 0xA2

        NoteTypeUnknown = 255,
    };

    ChartViewerModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setChart(RmeChart *chart);
    RmeChart *chart();
    const RmeChart *chart() const;

    int keys() const;

    static QString noteTypeStr(NoteType nt);

public slots:
    void switchTickTimestamp();

private:
    RmeChart *m_chart;
    bool m_isTick;
    bool m_remastered;
};

#endif
