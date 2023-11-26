#ifndef CHARTSCENE_H__INCLUDED
#define CHARTSCENE_H__INCLUDED

#include <QByteArray>
#include <QGraphicsScene>
#include <QMap>

class QPixmap;
class QGraphicsItem;
class QGraphicsRectItem;
class ChartViewerModel;

class ChartScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit ChartScene(QObject *parent = nullptr);
    ~ChartScene() override;

    void setChartModel(ChartViewerModel *m);
    void setTickPerGrid(int tpg);

public slots:
    void redrawChart();

private:
    void drawNote(int64_t tick, int track, int z);
    void drawSlideNote(int64_t tick, int track, int z);
    void drawSlideEnd(int64_t tick, int track, int toTrack, int z);
    void drawSlideMiddle(int64_t tick, int track, int toTrack, int z);
    void drawSlideFrom(int64_t tick, int track, int toTrack, int z);
    void drawSlideTo(int64_t tick, int track, int toTrack, int z);
    void drawLongPressEnd(int64_t tick, int track, int z);
    void drawLongPress(int64_t tick, int64_t dur, int track, int z);

private:
    QGraphicsItem *createNoteItem(const QByteArray &noteType);

    QGraphicsRectItem *rootItem;

    QMap<QByteArray, QPixmap *> notePixmaps;
    QList<QGraphicsItem *> lineItems;
    QList<QGraphicsItem *> noteItems;

    int tickPerGrid;
    ChartViewerModel *model;
};

#endif
