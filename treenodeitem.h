#ifndef TREENODEITEM_H
#define TREENODEITEM_H

#include "threadedbinarytree.h"

#include <QGraphicsEllipseItem>
#include <QObject>

class QGraphicsSceneMouseEvent;
class QGraphicsTextItem;

class TreeNodeItem : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT

public:
    TreeNodeItem(NodeId id, int value, QGraphicsItem* parent = nullptr);

    NodeId nodeId() const;
    void setHighlighted(bool highlighted);

signals:
    void clicked(NodeId id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    NodeId id_;
    QGraphicsTextItem* textItem_;
};

#endif
