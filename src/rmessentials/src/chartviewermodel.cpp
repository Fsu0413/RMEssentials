
#include "chartviewermodel.h"

#include <RmEss/RmeChart>

#include <QColor>

#include <type_traits>

namespace {
template <typename T> inline constexpr int sgn(T t1)
{
    return t1 == 0 ? 0 : (t1 > 0 ? 1 : -1);
}
}

ChartViewerModel::ChartViewerModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_chart(nullptr)
    , m_isTick(false)
    , m_remastered(false)
{
}

int ChartViewerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    if (m_chart == nullptr)
        return 0;

    return m_chart->notes.length();
}

int ChartViewerModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 5;
}

QVariant ChartViewerModel::data(const QModelIndex &index, int role) const
{
    if (m_chart == nullptr)
        return {};

    int column = index.column();
    int row = index.row();

    const RmeChartNote &note = m_chart->notes.at(row);
    switch (column) {
    case 0: {
        if (role == Qt::TextAlignmentRole)
            return static_cast<std::underlying_type_t<Qt::AlignmentFlag> >(Qt::AlignLeft | Qt::AlignVCenter);

        NoteType nt = NoteTypeUnknown;

        if (note.attr == 0) {
            nt = SingleClick;
        } else if (note.attr == 3) {
            if (note.timeDur == 0) {
                if (note.isEnd)
                    nt = SingleSlide;
                else
                    nt = LongPressStartChangeTrack;
            } else {
                if (note.isEnd)
                    nt = SingleLongPress;
                else
                    nt = LongPressStart;
            }
        } else if (note.attr == 4) {
            if (note.timeDur == 0) {
                if (note.isEnd)
                    nt = LongPressEndWithSlide;
                else
                    nt = LongPressContinueChangeTrack;
            } else {
                if (note.isEnd)
                    nt = LongPressEnd;
                else
                    nt = LongPressContinueTargetTrack;
            }
        }
        if (role == Qt::DisplayRole)
            return noteTypeStr(nt);
        else if (role == ChartViewerModel::ChartDrawRole)
            return static_cast<int>(nt);

        break;
    }
    case 1: {
        if (role == Qt::DisplayRole) {
            if (m_isTick)
                return QString::number(RmeChartNote::timestampToTick(note.timestamp, m_chart->bpm));
            return QString::number(note.timestamp);
        }
        if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;
        if (role == ChartViewerModel::ChartDrawRole)
            return (int64_t)(RmeChartNote::timestampToTick(note.timestamp, m_chart->bpm));
        break;
    }
    case 2: {
        if (role == Qt::DisplayRole)
            return QString::number(note.track - 2);
        if (role == ChartViewerModel::ChartDrawRole)
            return (int)(note.track - 2);
        if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;
        break;
    }
    case 3: {
        if ((note.attr != 0) && (note.timeDur == 0)) {
            if (role == Qt::DisplayRole)
                return QString::number(note.toTrack - 2);

            if (role == ChartViewerModel::ChartDrawRole) {
                if (m_remastered && note.isEnd)
                    return (int)(note.track + sgn(note.toTrack - note.track) - 2);
                else
                    return (int)(note.toTrack - 2);
            }
            if (role == Qt::TextAlignmentRole)
                return Qt::AlignCenter;
            if (role == Qt::BackgroundRole) {
                if (note.isEnd && qAbs(note.toTrack - note.track) > 1)
                    return QColor(Qt::red);
            }
        } else {
            if (role == Qt::DisplayRole)
                return tr("{N/A}");
            if (role == Qt::TextAlignmentRole)
                return Qt::AlignCenter;
            if (role == Qt::BackgroundRole)
                return QColor(Qt::gray);
            if (role == Qt::ForegroundRole)
                return QColor(Qt::darkGray);
        }
        break;
    }
    case 4: {
        if (note.timeDur != 0) {
            if (role == Qt::DisplayRole) {
                if (m_isTick) {
                    unsigned int tickBegin = RmeChartNote::timestampToTick(note.timestamp, m_chart->bpm);
                    unsigned int tickEnd = RmeChartNote::timestampToTick(note.timestamp + note.timeDur, m_chart->bpm);
                    return QString::number(tickEnd - tickBegin);
                }

                return QString::number(note.timeDur);
            }
            if (role == ChartViewerModel::ChartDrawRole) {
                unsigned int tickBegin = RmeChartNote::timestampToTick(note.timestamp, m_chart->bpm);
                unsigned int tickEnd = RmeChartNote::timestampToTick(note.timestamp + note.timeDur, m_chart->bpm);
                return (int64_t)(tickEnd - tickBegin);
            }
            if (role == Qt::TextAlignmentRole)
                return Qt::AlignCenter;
        } else {
            if (role == Qt::DisplayRole)
                return tr("{N/A}");
            if (role == Qt::TextAlignmentRole)
                return Qt::AlignCenter;
            if (role == Qt::BackgroundRole)
                return QColor(Qt::gray);
            if (role == Qt::ForegroundRole)
                return QColor(Qt::darkGray);
        }
        break;
    }
    }

    return {};
}

QVariant ChartViewerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0:
                return tr("Note Type");
            case 1:
                if (m_isTick)
                    return tr("Tick");

                return tr("Timestamp");
            case 2:
                return tr("Track");
            case 3:
                return tr("Target track"); // tr("Target Track (of slide)");
            case 4:
                if (m_isTick)
                    return tr("Tick Duration"); // tr("Tick Duration (of long press)");

                return tr("Time Duration"); // tr("Time Duration (of long press)");
            }
        } else {
            return QString::number(section + 1);
        }
    }

    return {};
}

void ChartViewerModel::setChart(RmeChart *chart)
{
    beginResetModel();
    if (m_chart != nullptr) {
        delete m_chart;
        m_chart = nullptr;
    }
    m_chart = chart;
    endResetModel();
}

RmeChart *ChartViewerModel::chart()
{
    return m_chart;
}

const RmeChart *ChartViewerModel::chart() const
{
    return m_chart;
}

int ChartViewerModel::keys() const
{
    int maxTrack = 4;

    if (m_chart == nullptr)
        return maxTrack;

    foreach (const RmeChartNote &n, m_chart->notes)
        maxTrack = qMax(maxTrack, (int)(n.track - 2));

    return qMin(maxTrack, 6);
}

QString ChartViewerModel::noteTypeStr(NoteType nt)
{
    static const QMap<NoteType, QString> noteTypeMap {
        std::make_pair(SingleClick, tr("Single Click")),
        std::make_pair(SingleSlide, tr("Single Slide")),
        std::make_pair(SingleLongPress, tr("Single Long Press")),
        std::make_pair(LongPressContinueChangeTrack, tr("Long Press Continue Change Track")),
        std::make_pair(LongPressContinueTargetTrack, tr("Long Press Continue Target Track")),
        std::make_pair(LongPressStartChangeTrack, tr("Long Press Start Change Track")),
        std::make_pair(LongPressStart, tr("Long Press Start")),
        std::make_pair(LongPressEndWithSlide, tr("Long Press End With Slide")),
        std::make_pair(LongPressEnd, tr("Long Press End Without Slide")),
    };
    return noteTypeMap.value(nt, tr("Unknown"));
}

void ChartViewerModel::switchTickTimestamp()
{
    beginResetModel();
    m_isTick = !m_isTick;
    endResetModel();
}

void ChartViewerModel::switchLegacyRemastered()
{
    beginResetModel();
    m_remastered = !m_remastered;
    endResetModel();
}
