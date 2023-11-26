
#include "chartscene.h"

#include "chartviewermodel.h"

#include <RmEss/RmeChart>

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QPixmap>

namespace {
template <typename T> inline constexpr int sgn(T t1)
{
    return t1 == 0 ? 0 : (t1 > 0 ? 1 : -1);
}
}

ChartScene::ChartScene(QObject *parent)
    : QGraphicsScene(QRectF(0, 0, 450, 10000), parent)
    , rootItem(nullptr)
    , model(nullptr)
    , tickPerGrid(12)
{
    rootItem = addRect(0, 0, 450, 100, Qt::NoPen, Qt::white);
    rootItem->setTransform(QTransform(75, 0, 0, -(25. / qreal(tickPerGrid)), 0, 10000 - 25));
    rootItem->setZValue(-1000000);
    rootItem->show();

#define LoadPicture(P)                                                  \
    do {                                                                \
        QString P##filename = QStringLiteral(":/chartview/" #P ".png"); \
        QPixmap *newp##P = new QPixmap;                                 \
        if (!newp##P->load(P##filename, "PNG")) {                       \
            qDebug() << P##filename << "can't be loaded";               \
            delete newp##P;                                             \
        } else {                                                        \
            notePixmaps[#P] = newp##P;                                  \
        }                                                               \
    } while (false)

    LoadPicture(slideDown);
    LoadPicture(slideDownFinal);
    LoadPicture(slideHorizontal);
    LoadPicture(slideLeft);
    LoadPicture(slideLeftArrow);
    LoadPicture(slideNote);
    LoadPicture(slideRight);
    LoadPicture(slideRightArrow);
    LoadPicture(slideUp);
    LoadPicture(slideVertical);
    LoadPicture(note);

#undef LoadPicture
}

ChartScene::~ChartScene()
{
    qDeleteAll(noteItems);
    qDeleteAll(notePixmaps);
}

void ChartScene::setChartModel(ChartViewerModel *m)
{
    if (model != nullptr)
        disconnect(model, &QAbstractTableModel::modelReset, this, &ChartScene::redrawChart);
    model = m;
    connect(model, &QAbstractTableModel::modelReset, this, &ChartScene::redrawChart);
}

void ChartScene::setTickPerGrid(int tpg)
{
    tickPerGrid = tpg;
    redrawChart();
}

void ChartScene::redrawChart()
{
    if (model->chart() == nullptr)
        return;

    qlonglong height = RmeChartNote::timestampToTick(model->chart()->totalTime, model->chart()->bpm) / qreal(tickPerGrid) * 25 + 250;
    setSceneRect(0, 0, 450, height);
    rootItem->setRect(0, 0, 450, height);
    rootItem->setTransform(QTransform(75, 0, 0, -(25. / qreal(tickPerGrid)), 0, height - 25));

    qDeleteAll(lineItems);
    lineItems.clear();

    QPen verticalLinePen;
    verticalLinePen.setWidthF(1. / 75);
    verticalLinePen.setColor(Qt::black);
    for (int i : {0, 1, 2, 3, 4, 5, 6}) {
        QGraphicsLineItem *line = addLine(i, 0, i, height / 25 * qreal(tickPerGrid), verticalLinePen);
        line->setZValue(-999999);
        line->setParentItem(rootItem);
        line->show();

        lineItems << line;
    }

    QPen horizontalLinePen;
    horizontalLinePen.setWidthF(qreal(tickPerGrid) / 25);
    horizontalLinePen.setColor(Qt::lightGray);
    QPen horizontalSegmentLinePen = horizontalLinePen;
    horizontalSegmentLinePen.setColor(Qt::black);
    for (int i = -tickPerGrid; i < height / 25 * qreal(tickPerGrid); i += tickPerGrid) {
        QGraphicsLineItem *line = addLine(0, i, 6, i, ((i + tickPerGrid) % 48 == 0) ? horizontalSegmentLinePen : horizontalLinePen);
        line->setZValue(-999999);
        line->setParentItem(rootItem);
        line->show();

        lineItems << line;
    }

    qDeleteAll(noteItems);
    noteItems.clear();

    int row = model->rowCount(QModelIndex());

    for (int i = 0; i < row; ++i) {
        ChartViewerModel::NoteType nt = static_cast<ChartViewerModel::NoteType>(model->data(model->index(i, 0), ChartViewerModel::ChartDrawRole).toInt());
        int64_t tick = model->data(model->index(i, 1), ChartViewerModel::ChartDrawRole).value<qint64>();
        int track = model->data(model->index(i, 2), ChartViewerModel::ChartDrawRole).toInt();
        switch (nt) {
        case ChartViewerModel::SingleClick: {
            drawNote(tick, track, -i);
            break;
        }
        case ChartViewerModel::SingleSlide: {
            int toTrack = model->data(model->index(i, 3), ChartViewerModel::ChartDrawRole).toInt();
            drawSlideMiddle(tick, track, toTrack, -i);
            drawSlideEnd(tick, track, toTrack, -i);
            drawSlideNote(tick, track, -i);
            break;
        }
        case ChartViewerModel::SingleLongPress: {
            int64_t dur = model->data(model->index(i, 4), ChartViewerModel::ChartDrawRole).value<qint64>();
            drawLongPressEnd(tick + dur, track, -i);
            drawLongPress(tick, dur, track, -i);
            drawSlideNote(tick, track, -i);
            break;
        }
        case ChartViewerModel::LongPressContinueChangeTrack: {
            int toTrack = model->data(model->index(i, 3), ChartViewerModel::ChartDrawRole).toInt();
            drawSlideFrom(tick, track, toTrack, -i);
            drawSlideMiddle(tick, track, toTrack, -i);
            drawSlideTo(tick, track, toTrack, -i);
            break;
        }
        case ChartViewerModel::LongPressContinueTargetTrack: {
            int64_t dur = model->data(model->index(i, 4), ChartViewerModel::ChartDrawRole).value<qint64>();
            drawLongPress(tick, dur, track, -i);
            break;
        }
        case ChartViewerModel::LongPressStartChangeTrack: {
            int toTrack = model->data(model->index(i, 3), ChartViewerModel::ChartDrawRole).toInt();
            drawSlideMiddle(tick, track, toTrack, -i);
            drawSlideTo(tick, track, toTrack, -i);
            drawSlideNote(tick, track, -i);
            break;
        }
        case ChartViewerModel::LongPressStart: {
            int64_t dur = model->data(model->index(i, 4), ChartViewerModel::ChartDrawRole).value<qint64>();
            drawSlideNote(tick, track, -i);
            drawLongPress(tick, dur, track, -i);
            break;
        }
        case ChartViewerModel::LongPressEndWithSlide: {
            int toTrack = model->data(model->index(i, 3), ChartViewerModel::ChartDrawRole).toInt();
            drawSlideFrom(tick, track, toTrack, -i);
            drawSlideEnd(tick, track, toTrack, -i);
            drawSlideMiddle(tick, track, toTrack, -i);
            break;
        }
        case ChartViewerModel::LongPressEnd: {
            int64_t dur = model->data(model->index(i, 4), ChartViewerModel::ChartDrawRole).value<qint64>();
            drawLongPressEnd(tick + dur, track, -i);
            drawLongPress(tick, dur, track, -i);
            break;
        }
        default:
            break;
        }
    }
}

void ChartScene::drawNote(int64_t tick, int track, int z)
{
    QGraphicsItem *item = createNoteItem("note");
    item->setPos(track - 1, tick);
    item->setZValue(z);
    item->show();
    noteItems << item;
}

void ChartScene::drawSlideNote(int64_t tick, int track, int z)
{
    QGraphicsItem *item = createNoteItem("slideNote");
    item->setPos(track - 1, tick);
    item->setZValue(z);
    item->show();
    noteItems << item;
}

void ChartScene::drawSlideEnd(int64_t tick, int track, int toTrack, int z)
{
    QByteArray arrowType = ((toTrack - track > 0) ? "slideLeftArrow" : "slideRightArrow");

    QGraphicsItem *item = createNoteItem(arrowType);
    item->setPos(toTrack - 1, tick);
    item->setZValue(z);
    item->show();
    noteItems << item;
}

void ChartScene::drawSlideMiddle(int64_t tick, int track, int toTrack, int z)
{
    int targetDirection = sgn(toTrack - track);

    for (int middleTrack = track + targetDirection; qAbs(middleTrack - toTrack) > 0; middleTrack += targetDirection) {
        QGraphicsItem *item = createNoteItem("slideHorizontal");
        item->setPos(middleTrack - 1, tick);
        item->setZValue(z);
        item->show();
        noteItems << item;
    }
}

void ChartScene::drawSlideFrom(int64_t tick, int track, int toTrack, int z)
{
    {
        QGraphicsItem *item = createNoteItem("slideDown");
        item->setPos(track - 1, tick);
        item->setZValue(z);
        item->show();
        noteItems << item;
    }
    {
        QByteArray arrowType = ((toTrack - track > 0) ? "slideRight" : "slideLeft");

        QGraphicsItem *item = createNoteItem(arrowType);
        item->setPos(track - 1, tick);
        item->setZValue(z);
        item->show();
        noteItems << item;
    }
}

void ChartScene::drawSlideTo(int64_t tick, int track, int toTrack, int z)
{
    {
        QByteArray arrowType = ((toTrack - track > 0) ? "slideLeft" : "slideRight");

        QGraphicsItem *item = createNoteItem(arrowType);
        item->setPos(toTrack - 1, tick);
        item->setZValue(z);
        item->show();
        noteItems << item;
    }
    {
        QGraphicsItem *item = createNoteItem("slideUp");
        item->setPos(toTrack - 1, tick);
        item->setZValue(z);
        item->show();
        noteItems << item;
    }
}

void ChartScene::drawLongPressEnd(int64_t tick, int track, int z)
{
    QGraphicsItem *item = createNoteItem("slideDownFinal");
    item->setPos(track - 1, tick);
    item->setZValue(z);
    item->show();
    noteItems << item;
}

void ChartScene::drawLongPress(int64_t tick, int64_t dur, int track, int z)
{
    for (int64_t t = tick + dur - tickPerGrid; t > tick; t -= tickPerGrid) {
        QGraphicsItem *item = createNoteItem("slideVertical");
        item->setPos(track - 1, t);
        item->setZValue(z);
        item->show();
        noteItems << item;
    }
}

QGraphicsItem *ChartScene::createNoteItem(const QByteArray &noteType)
{
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(rootItem);
    item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    item->setTransform(QTransform(1. / 75, 0, 0, -(qreal(tickPerGrid) / 25), 0, 0));
    item->setPixmap(*(notePixmaps.value(noteType, nullptr)));

    return item;
}
