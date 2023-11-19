#ifndef CHARTVIEW_H__INCLUDED
#define CHARTVIEW_H__INCLUDED

#include <RmEss/RmeChart>
#include <RmEss/RmeGlobal>

#include <QAbstractTableModel>
#include <QDialog>
#include <QList>

class QLineEdit;
class QTableView;

class ChartViewerModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ChartViewerModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setChart(RmeChart *chart);

private:
    RmeChart *m_chart;
    QList<RmeChartNote> m_chartNotes;
};

class ChartViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ChartViewer(QWidget *parent = nullptr);

private:
    QLineEdit *m_fileName;
    QTableView *m_chartTable;
    ChartViewerModel *m_currentChartModel;

private slots:
    void selectFile();
};

#endif
