#ifndef CHARTVIEW_H__INCLUDED
#define CHARTVIEW_H__INCLUDED

#include <RmEss/RmeGlobal>

#include <QDialog>
#include <QList>

class QLineEdit;
class QTableView;
class ChartViewerModel;
class QGraphicsView;
class ChartScene;
class QComboBox;

class ChartViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ChartViewer(QWidget *parent = nullptr);

private:
    QLineEdit *m_fileName;
    ChartViewerModel *m_currentChartModel;
    QTableView *m_chartTableView;
    QGraphicsView *m_chartGraphicsView;
    ChartScene *m_chartScene;
    QLineEdit *m_keyCount;
    QLineEdit *m_totalTime;
    QLineEdit *m_bpm;
    QLineEdit *m_totalKeyAmount;
    QComboBox *m_tickPerGridBox;

private slots:
    void selectFile();
    void chartReloaded();
    void setTickPerGrid();

protected:
    void showEvent(QShowEvent *event) override;
};

#endif
