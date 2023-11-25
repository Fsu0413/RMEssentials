#ifndef CHARTVIEW_H__INCLUDED
#define CHARTVIEW_H__INCLUDED

#include <RmEss/RmeChart>
#include <RmEss/RmeGlobal>

#include <QAbstractTableModel>
#include <QDialog>
#include <QList>

class QLineEdit;

class ChartViewerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ChartViewerModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setChart(RmeChart *chart);
    RmeChart *chart();
    const RmeChart *chart() const;

    enum
    {
        ChartDrawRole = Qt::UserRole + 1,
    };

    int keys() const;

public slots:
    void switchTickTimestamp();

private:
    RmeChart *m_chart;
    bool m_isTick;
    bool m_remastered;
};

class ChartViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ChartViewer(QWidget *parent = nullptr);

private:
    QLineEdit *m_fileName;
    ChartViewerModel *m_currentChartModel;

    QLineEdit *m_keyCount;
    QLineEdit *m_totalTime;
    QLineEdit *m_bpm;
    QLineEdit *m_totalKeyAmount;

private slots:
    void selectFile();

    void chartReloaded();
};

#endif
