#ifndef CHARTVIEW_H__INCLUDED
#define CHARTVIEW_H__INCLUDED

#include <RmEss/RmeGlobal>

#include <QDialog>
#include <QList>

class QLineEdit;
class QTableView;
class ChartViewerModel;

class ChartViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ChartViewer(QWidget *parent = nullptr);

private:
    QLineEdit *m_fileName;
    ChartViewerModel *m_currentChartModel;
    QTableView *m_chartView;
    QLineEdit *m_keyCount;
    QLineEdit *m_totalTime;
    QLineEdit *m_bpm;
    QLineEdit *m_totalKeyAmount;

private slots:
    void selectFile();
    void chartReloaded();

protected:
    void showEvent(QShowEvent *event) override;
};

#endif
