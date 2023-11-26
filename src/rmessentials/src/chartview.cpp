
#include "chartview.h"
#include "chartscene.h"
#include "chartviewermodel.h"

#include <RmEss/RmeChart>
#include <RmEss/RmeCrypt>
#include <RmEss/RmeDownloader>
#include <RmEss/RmeUtils>

#include <QComboBox>
#include <QDir>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTableView>
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

    QGridLayout *leftLayout = new QGridLayout;

    m_fileName = new QLineEdit;
    m_fileName->setPlaceholderText(tr("Browse the file using the Browse button"));
    m_fileName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_fileName->setReadOnly(true);

    QPushButton *browseButton = new QPushButton(tr("Browse..."));
    connect(browseButton, &QPushButton::clicked, this, &ChartViewer::selectFile);

    leftLayout->addWidget(new QLabel(tr("File:")), 0, 0);
    leftLayout->addWidget(m_fileName, 0, 1);
    leftLayout->addWidget(browseButton, 0, 2);

    leftLayout->addWidget(new QLabel(tr("Key count:")), 1, 0);
    m_keyCount = new QLineEdit;
    m_keyCount->setValidator(new QIntValidator(4, 6, this));
    m_keyCount->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(m_keyCount, 1, 1, 1, 2);
    leftLayout->addWidget(new QLabel(tr("Total time:")), 2, 0);
    m_totalTime = new QLineEdit;
    m_totalTime->setValidator(new QIntValidator(1, 2147483647, this));
    m_totalTime->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(m_totalTime, 2, 1, 1, 2);
    leftLayout->addWidget(new QLabel(tr("Tempo:")), 3, 0);
    m_bpm = new QLineEdit;
    m_bpm->setValidator(new QDoubleValidator(1., 625., 1, this));
    m_bpm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(m_bpm, 3, 1, 1, 2);
    leftLayout->addWidget(new QLabel(tr("Total key amount:")), 4, 0);
    m_totalKeyAmount = new QLineEdit;
    m_totalKeyAmount->setReadOnly(true);
    m_totalKeyAmount->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(m_totalKeyAmount, 4, 1, 1, 2);
    m_currentChartModel = new ChartViewerModel(this);
    connect(m_currentChartModel, &ChartViewerModel::modelReset, this, &ChartViewer::chartReloaded);
    m_chartTableView = new QTableView;
    m_chartTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_chartTableView->setModel(m_currentChartModel);
    leftLayout->addWidget(m_chartTableView, 5, 0, 1, 3);
    QPushButton *switchButton = new QPushButton(tr("Switch timestamp / tick"));
    connect(switchButton, &QPushButton::clicked, m_currentChartModel, &ChartViewerModel::switchTickTimestamp);
    switchButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    leftLayout->addWidget(switchButton, 6, 0, 1, 3);

    QGridLayout *rightLayout = new QGridLayout;

    rightLayout->addWidget(new QLabel(tr("Tick per Grid:")), 0, 0, 1, 1);
    m_tickPerGridBox = new QComboBox;
    m_tickPerGridBox->setEditable(false);
    for (int i = 1; i < 48; ++i) {
        if (48 % i == 0)
            m_tickPerGridBox->addItem(QString::number(i));
    }
    m_tickPerGridBox->setCurrentText(QStringLiteral("12"));
    connect(m_tickPerGridBox, &QComboBox::currentIndexChanged, this, &ChartViewer::setTickPerGrid);
    m_tickPerGridBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rightLayout->addWidget(m_tickPerGridBox, 0, 1, 1, 1);
    QPushButton *switchButton2 = new QPushButton(tr("Switch Old / Remastered"));
    connect(switchButton2, &QPushButton::clicked, m_currentChartModel, &ChartViewerModel::switchLegacyRemastered);
    switchButton2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    rightLayout->addWidget(switchButton2, 0, 2, 1, 1);

    m_chartGraphicsView = new QGraphicsView;
    m_chartGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chartScene = new ChartScene;
    m_chartGraphicsView->setScene(m_chartScene);
    m_chartScene->setChartModel(m_currentChartModel);
    rightLayout->addWidget(m_chartGraphicsView, 1, 0, 1, 3);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(leftLayout);
    layout->addLayout(rightLayout);

    setLayout(layout);
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

void ChartViewer::setTickPerGrid()
{
    int tpg = m_tickPerGridBox->currentText().toInt();
    m_chartScene->setTickPerGrid(tpg);
}

void ChartViewer::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    QFontMetrics fm(font());
    int totalWidth = fm.horizontalAdvance(QStringLiteral("8888")) * 1.3;

    int noteTypeWidth = 0;
    // clang-format off
    for (ChartViewerModel::NoteType nt : {
                                          ChartViewerModel::SingleClick,
                                          ChartViewerModel::SingleSlide,
                                          ChartViewerModel::SingleLongPress,
                                          ChartViewerModel::LongPressContinueChangeTrack,
                                          ChartViewerModel::LongPressContinueTargetTrack,
                                          ChartViewerModel::LongPressStartChangeTrack,
                                          ChartViewerModel::LongPressStart,
                                          ChartViewerModel::LongPressEndWithSlide,
                                          ChartViewerModel::LongPressEnd
                                         })
        noteTypeWidth = qMax(noteTypeWidth, fm.horizontalAdvance(ChartViewerModel::noteTypeStr(nt)));
    // clang-format on

    noteTypeWidth *= 1.1;
    m_chartTableView->setColumnWidth(0, noteTypeWidth);
    totalWidth += noteTypeWidth;

    for (int i : {1, 2, 3, 4}) {
        int currentWidth = fm.horizontalAdvance(m_currentChartModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString()) * 1.3;
        totalWidth += currentWidth;
        m_chartTableView->setColumnWidth(i, currentWidth);
    }

    m_chartTableView->setMinimumWidth(totalWidth);
    m_chartGraphicsView->setFixedWidth(450 + m_chartGraphicsView->verticalScrollBar()->width());
}
