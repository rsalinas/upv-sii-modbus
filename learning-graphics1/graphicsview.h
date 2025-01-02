#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QObject>
#include <QSet>
#include <QTimer>

class MovableCircle : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    MovableCircle(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void released();
    void pressed();
    void positionChanged(int x, int y);
};

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(QWidget *parent = nullptr);
    MovableCircle *getPointer() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void returnToCenter();
    void moveToCenter();
    void updatePosition(int x, int y);

private:
    MovableCircle *pointer;
    QTimer *timer;
    bool isDragging;
    QSet<int> keysPressed;

    void movePointerToEdge(int dx, int dy);
    int mapToRange(qreal value, qreal inMin, qreal inMax, int outMin, int outMax);
};

#endif // GRAPHICSVIEW_H
