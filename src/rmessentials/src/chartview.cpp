
#include "chartview.h"

#include <RmEss/RmeChart>
#include <RmEss/RmeCrypt>
#include <RmEss/RmeDownloader>
#include <RmEss/RmeUtils>

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTableWidget>
#include <QVBoxLayout>

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

    m_currentChartModel = new ChartViewerModel;
    m_chartTable = new QTableView;
    m_chartTable->setModel(m_currentChartModel);

    QVBoxLayout *alllayout = new QVBoxLayout;
    alllayout->addLayout(layout1);
    alllayout->addWidget(m_chartTable);

    setLayout(alllayout);
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

ChartViewerModel::ChartViewerModel()
    : m_chart(nullptr)
{
}

int ChartViewerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    if (m_chart == nullptr)
        return 0;

    return m_chartNotes.length();
}

int ChartViewerModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 5;
}

QVariant ChartViewerModel::data(const QModelIndex &index, int role) const
{
    int column = index.column();
    int row = index.row();

    const RmeChartNote &note = m_chartNotes.at(row);
    switch (column) {
    case 0: {
        if (role == Qt::DisplayRole) {
            QString type = QStringLiteral("Unknown");
            if (note.attr == 0) {
                type = QStringLiteral("Single Click");
            } else if (note.attr == 3) {
                if (note.timeDur == 0) {
                    if (note.isEnd)
                        type = QStringLiteral("Single Slide");
                    else
                        type = QStringLiteral("Long Press Start Change Track");
                } else {
                    if (note.isEnd)
                        type = QStringLiteral("Single Long Press");
                    else
                        type = QStringLiteral("Long Press Start");
                }
            } else if (note.attr == 4) {
                if (note.timeDur == 0) {
                    if (note.isEnd)
                        type = QStringLiteral("Long Press End With Slide");
                    else
                        type = QStringLiteral("Long Press Continue Change Track");
                } else {
                    if (note.isEnd)
                        type = QStringLiteral("Long Press End Without Slide");
                    else
                        type = QStringLiteral("Long Press Continue Target Track");
                }
            }
            return type;
        }
        break;
    }
    case 1: {
        if (role == Qt::DisplayRole)
            return QString::number(note.timestamp);
        break;
    }
    case 2: {
        if (role == Qt::DisplayRole)
            return QString::number(note.track - 2);
        break;
    }
    case 3: {
        if ((note.attr != 0) && (note.timeDur == 0)) {
            if (role == Qt::DisplayRole)
                return QString::number(note.toTrack - 2);
        } else {
            if (role == Qt::DisplayRole)
                return QStringLiteral("{N/A}");
            if (role == Qt::BackgroundRole)
                return QColor(Qt::gray);
            if (role == Qt::ForegroundRole)
                return QColor(Qt::darkGray);
        }
        break;
    }
    case 4: {
        if (note.timeDur != 0) {
            if (role == Qt::DisplayRole)
                return QString::number(note.timeDur);
        } else {
            if (role == Qt::DisplayRole)
                return QStringLiteral("{N/A}");
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
                return tr("Type");
            case 1:
                return tr("Timestamp");
            case 2:
                return tr("Track");
            case 3:
                return tr("ToTrack");
            case 4:
                return tr("Duration");
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
    m_chartNotes = QList(m_chart->notes.begin(), m_chart->notes.end());
    endResetModel();
}
