
#include "chartview.h"

#include <RmEss/RmeChart>
#include <RmEss/RmeCrypt>
#include <RmEss/RmeDownloader>
#include <RmEss/RmeUtils>

#include <QDir>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableWidget>
#include <QVBoxLayout>

#include <type_traits>

namespace {
enum ChartFormat
{
    ChartUnknown,

    ChartImd,
    ChartImdJson,
    ChartRmp,

};

ChartFormat getOpenFileName(QWidget *parent, const QString &dir, QString &filePath)
{
    QString selectedFilter;
    filePath = QFileDialog::getOpenFileName(parent, QString(), dir,
                                            ChartViewer::tr("IMD Files") + QStringLiteral(" (*.imd)") + QStringLiteral(";;") + ChartViewer::tr("IMD Json files")
                                                + QStringLiteral(" (*.imd.json)") + QStringLiteral(";;") + ChartViewer::tr("RMP files") + QStringLiteral(" (*.rmp)"),
                                            &selectedFilter);

    if (filePath.isNull())
        return ChartUnknown;

    if (selectedFilter.contains(QStringLiteral("*.imd.json"))) // must be put before ".imd"
        return ChartImdJson;
    if (selectedFilter.contains(QStringLiteral("*.imd")))
        return ChartImd;
    if (selectedFilter.contains(QStringLiteral("*.rmp")))
        return ChartRmp;

    return ChartUnknown;
}

template <typename T> inline constexpr int sgn(T t1)
{
    return t1 == 0 ? 0 : (t1 > 0 ? 1 : -1);
}
}

ChartViewer::ChartViewer(QWidget *parent)
    : QDialog(parent)
    , m_currentChartModel(nullptr)
{
    setWindowTitle(tr("Rhythm Master Chart Viewer"));

    m_fileName = new QLineEdit;
    m_fileName->setPlaceholderText(tr("Browse the file using the Browse button"));
    m_fileName->setReadOnly(true);

    QPushButton *browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &ChartViewer::selectFile);

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(new QLabel(tr("File:")));
    layout1->addWidget(m_fileName);
    layout1->addWidget(browseButton);

    m_currentChartModel = new ChartViewerModel(this);
    connect(m_currentChartModel, &ChartViewerModel::modelReset, this, &ChartViewer::chartReloaded);
    QTableView *chartTable = new QTableView;
    chartTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    chartTable->setModel(m_currentChartModel);

    QGridLayout *midLayout = new QGridLayout;
    midLayout->addWidget(new QLabel(tr("Key count:")), 0, 0);
    m_keyCount = new QLineEdit;
    m_keyCount->setValidator(new QIntValidator(4, 6));
    m_keyCount->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    midLayout->addWidget(m_keyCount, 0, 1);
    midLayout->addWidget(new QLabel(tr("Total time:")), 1, 0);
    m_totalTime = new QLineEdit;
    m_totalTime->setValidator(new QIntValidator(1, 2147483647));
    m_totalTime->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    midLayout->addWidget(m_totalTime, 1, 1);
    midLayout->addWidget(new QLabel(tr("Tempo:")), 2, 0);
    m_bpm = new QLineEdit;
    m_bpm->setValidator(new QDoubleValidator(1., 1000000., 1));
    m_bpm->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    midLayout->addWidget(m_bpm, 2, 1);
    midLayout->addWidget(new QLabel(tr("Total key amount:")), 3, 0);
    m_totalKeyAmount = new QLineEdit;
    m_totalKeyAmount->setReadOnly(true);
    m_totalKeyAmount->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    midLayout->addWidget(m_totalKeyAmount, 3, 1);
    QPushButton *switchButton = new QPushButton(tr("Switch timestamp / tick"));
    connect(switchButton, &QPushButton::clicked, m_currentChartModel, &ChartViewerModel::switchTickTimestamp);
    switchButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    midLayout->addWidget(switchButton, 4, 0, 1, 2);
    QWidget *spaceWidget = new QWidget;
    spaceWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    midLayout->addWidget(spaceWidget, 5, 0, 1, 2);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(layout1);
    leftLayout->addWidget(chartTable);

    QHBoxLayout *totalLayout = new QHBoxLayout;
    totalLayout->addLayout(leftLayout);
    totalLayout->addLayout(midLayout);

    setLayout(totalLayout);
}

void ChartViewer::selectFile()
{
    QString dir;
    QDir d_detect(RmeDownloader::songDownloadPath());
    if (d_detect.exists())
        dir = d_detect.absolutePath();
    else
        dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    QString selected;
    ChartFormat format = getOpenFileName(this, dir, selected);
    if (format == ChartUnknown)
        return;

    QFile f(selected);
    if (!f.open(QFile::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), tr("The file can't be opened. Please double check."));
        return;
    }
    QByteArray ba = f.readAll();
    f.close();

    if (format == ChartRmp) {
        QFileInfo fi(selected);
        QString name = fi.baseName();

        QByteArray rmpKey = RmeUtils::rmpKeyForChart(name);
        QByteArray decrypted = RmeCrypt::decryptFull(ba, rmpKey);

        if (decrypted.isEmpty()) {
            QMessageBox::critical(this, tr("Error"), tr("RMP chart decrypt failed."));
            return;
        }
        ba = decrypted;
        format = ChartImdJson;
    }

    RmeChart *newChart = nullptr;

    switch (format) {
    case ChartRmp: // dead code here. ChartRmp will be replaced by ChartImdJson during decrypt.
    case ChartImdJson: {
        QJsonDocument doc = QJsonDocument::fromJson(ba);
        if (!doc.isObject()) {
            QMessageBox::critical(this, tr("Error"), tr("Can't decode JSON file."));
            return;
        }

        bool ok = false;
        newChart = new RmeChart(RmeChart::fromJson(doc.object(), &ok));
        if (!ok) {
            QMessageBox::critical(this, tr("Error"), tr("JSON file is not a valid chart."));
            delete newChart;
            return;
        }
        break;
    }
    case ChartImd: {
        bool ok = false;
        newChart = new RmeChart(RmeChart::fromImd(ba, &ok));
        if (!ok) {
            QMessageBox::critical(this, tr("Error"), tr("IMD file is not a valid chart."));
            delete newChart;
            return;
        }
        break;
    }
    default:
        break;
    }

    if (newChart == nullptr) // dead code!
        return;

    m_fileName->setText(selected);
    m_currentChartModel->setChart(newChart);
}

void ChartViewer::chartReloaded()
{
    if (m_currentChartModel->chart() == nullptr)
        return;

    m_keyCount->setText(QString::number(m_currentChartModel->keys()));
    m_totalTime->setText(QString::number(m_currentChartModel->chart()->totalTime));
    m_bpm->setText(QString::number(m_currentChartModel->chart()->bpm));
    m_totalKeyAmount->setText(QString::number(m_currentChartModel->chart()->calculateTotalKeyAmount()));
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

        if (note.attr == 0) {
            if (role == Qt::DisplayRole)
                return tr("Single Click");
            if (role == ChartViewerModel::ChartDrawRole)
                return 0;
        } else if (note.attr == 3) {
            if (note.timeDur == 0) {
                if (note.isEnd) {
                    if (role == Qt::DisplayRole)
                        return tr("Single Slide");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 1;
                } else {
                    if (role == Qt::DisplayRole)
                        return tr("Long Press Start Change Track");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 61;
                }
            } else {
                if (note.isEnd) {
                    if (role == Qt::DisplayRole)
                        return tr("Single Long Press");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 2;
                } else {
                    if (role == Qt::DisplayRole)
                        return tr("Long Press Start");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 62;
                }
            }
        } else if (note.attr == 4) {
            if (note.timeDur == 0) {
                if (note.isEnd) {
                    if (role == Qt::DisplayRole)
                        return tr("Long Press End With Slide");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 101;
                } else {
                    if (role == Qt::DisplayRole)
                        return tr("Long Press Continue Change Track");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 21;
                }
            } else {
                if (note.isEnd) {
                    if (role == Qt::DisplayRole)
                        return tr("Long Press End Without Slide");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 102;
                } else {
                    if (role == Qt::DisplayRole)
                        return tr("Long Press Continue Target Track");
                    if (role == ChartViewerModel::ChartDrawRole)
                        return 22;
                }
            }
        }
        if (role == Qt::DisplayRole)
            return tr("Unknown");
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
            return QString::number(RmeChartNote::timestampToTick(note.timestamp, m_chart->bpm));
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
                return QString::number(tickEnd - tickBegin);
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
                return tr("Target Track (of slide)");
            case 4:
                if (m_isTick)
                    return tr("Duration (of long press) Tick");

                return tr("Duration (of long press) Time");
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

void ChartViewerModel::switchTickTimestamp()
{
    beginResetModel();
    m_isTick = !m_isTick;
    endResetModel();
}
