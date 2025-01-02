#include "graphicsview.h"
#include <QDebug>
#include <QGraphicsLineItem>

MovableCircle::MovableCircle(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
    : QGraphicsEllipseItem(x, y, width, height, parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    // Rellenar el círculo de color azul macizo
    setBrush(Qt::blue);
}

QVariant MovableCircle::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();

        // Restringir el movimiento dentro de la circunferencia
        qreal radius = 100; // Radio de la circunferencia
        qreal distanceSquared = newPos.x() * newPos.x() + newPos.y() * newPos.y();
        if (distanceSquared > radius * radius) {
            // Si el círculo intenta salir de la circunferencia, lo mantenemos dentro
            qreal angle = atan2(newPos.y(), newPos.x());
            newPos.setX(radius * cos(angle));
            newPos.setY(radius * sin(angle));
        }

        // Emitir la señal con los valores mapeados
        int xValue = static_cast<int>((newPos.x() + 100) * 16383 / 200);
        int yValue = static_cast<int>((newPos.y() + 100) * 16383 / 200);
        emit positionChanged(xValue, yValue);

        return newPos;
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

void MovableCircle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mousePressEvent(event);
    emit pressed(); // Emitir señal cuando se presiona el círculo
}

void MovableCircle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mouseReleaseEvent(event);
    emit released(); // Emitir señal cuando se suelta el círculo
}

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , isDragging(false)
{
    QGraphicsScene *scene = new QGraphicsScene(this);
    setScene(scene);

    // Dibujar las líneas de los ejes (en color gris y limitadas al radio de la circunferencia)
    qreal radius = 100;     // Radio de la circunferencia
    QPen axisPen(Qt::gray); // Color gris para los ejes
    QGraphicsLineItem *xAxis = new QGraphicsLineItem(-radius, 0, radius, 0); // Línea horizontal
    QGraphicsLineItem *yAxis = new QGraphicsLineItem(0, -radius, 0, radius); // Línea vertical
    xAxis->setPen(axisPen);
    yAxis->setPen(axisPen);
    scene->addItem(xAxis);
    scene->addItem(yAxis);

    // Dibujar la circunferencia
    QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(-radius,
                                                            -radius,
                                                            2 * radius,
                                                            2 * radius);
    circle->setPen(QPen(Qt::black)); // Borde negro para la circunferencia
    scene->addItem(circle);

    // Dibujar el círculo pequeño (puntero)
    pointer = new MovableCircle(-10, -10, 20, 20);
    pointer = new MovableCircle(-15, -15, 30, 30); // Círculo más grande
    scene->addItem(pointer);                       // Añadir el círculo pequeño después de los ejes

    // Conectar las señales del círculo a los slots
    connect(pointer, &MovableCircle::released, this, &GraphicsView::returnToCenter);
    connect(pointer, &MovableCircle::pressed, this, [this]() {
        isDragging = true; // Indicar que se está arrastrando
        timer->stop();     // Detener el temporizador mientras se arrastra
    });
    connect(pointer, &MovableCircle::positionChanged, this, &GraphicsView::updatePosition);

    // Establecer el rectángulo de la escena
    scene->setSceneRect(-radius, -radius, 2 * radius, 2 * radius);

    // Configurar el temporizador para regresar a (0, 0)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GraphicsView::moveToCenter);
}

MovableCircle *GraphicsView::getPointer() const
{
    return pointer; // Devolver el puntero al círculo pequeño
}

void GraphicsView::returnToCenter()
{
    isDragging = false; // Indicar que se dejó de arrastrar
    timer->start(10);   // Iniciar el temporizador cuando se suelta el ratón
}

void GraphicsView::moveToCenter()
{
    if (isDragging)
        return; // Ignorar el temporizador si se está arrastrando

    QPointF currentPos = pointer->pos();
    if (currentPos == QPointF(0, 0)) {
        timer->stop(); // Detener el temporizador si ya está en (0, 0)
        return;
    }

    // Mover el círculo hacia (0, 0) suavemente
    QPointF newPos = currentPos * 0.9; // Reducir la posición en un 10%
    pointer->setPos(newPos);

    // Mapear las coordenadas al rango 0-16383
    int xValue = mapToRange(newPos.x(), -100, 100, 0, 16383);
    int yValue = mapToRange(newPos.y(), -100, 100, 0, 16383);
    qDebug() << "X:" << xValue << "Y:" << yValue;
}

void GraphicsView::updatePosition(int x, int y)
{
    // Mostrar los valores en la consola
    qDebug() << "X:" << x << "Y:" << y;
}

int GraphicsView::mapToRange(qreal value, qreal inMin, qreal inMax, int outMin, int outMax)
{
    return static_cast<int>((value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin);
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    int radius = 100; // The joystick max movement radius
    keysPressed.insert(event->key());

    int dx = 0, dy = 0;

    // Set position directly to the edge based on arrow key input
    if (keysPressed.contains(Qt::Key_Up))
        dy = -radius;
    if (keysPressed.contains(Qt::Key_Down))
        dy = radius;
    if (keysPressed.contains(Qt::Key_Left))
        dx = -radius;
    if (keysPressed.contains(Qt::Key_Right))
        dx = radius;

    if (dx != 0 || dy != 0) {
        movePointerToEdge(dx, dy);
        timer->stop(); // Stop auto-return while key is pressed
    }
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    keysPressed.remove(event->key());

    // Restart auto-return only if no arrow keys are still pressed
    if (keysPressed.isEmpty()) {
        returnToCenter();
    }
}

void GraphicsView::movePointerToEdge(int dx, int dy)
{
    QPointF newPos(dx, dy);

    // Ensure diagonal movement stays within the radius
    if (std::hypot(newPos.x(), newPos.y()) > 100) {
        double angle = std::atan2(newPos.y(), newPos.x());
        newPos.setX(100 * std::cos(angle));
        newPos.setY(100 * std::sin(angle));
    }

    pointer->setPos(newPos);
}
