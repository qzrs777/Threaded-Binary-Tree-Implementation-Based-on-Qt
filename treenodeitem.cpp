#include "treenodeitem.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QPen>

TreeNodeItem::TreeNodeItem(NodeId id, int value, QGraphicsItem* parent)
    : QObject(nullptr),
      QGraphicsEllipseItem(-23, -23, 46, 46, parent),
      id_(id),
      textItem_(new QGraphicsTextItem(QString::number(value), this)) {
    setBrush(QColor("#f8fafc"));
    setPen(QPen(QColor("#334155"), 2));
    setZValue(1);
    setCursor(Qt::PointingHandCursor);

    QFont font = textItem_->font();
    font.setBold(true);
    textItem_->setFont(font);
    textItem_->setDefaultTextColor(QColor("#0f172a"));
    const QRectF bounds = textItem_->boundingRect();
    textItem_->setPos(-bounds.width() / 2.0, -bounds.height() / 2.0);
}

NodeId TreeNodeItem::nodeId() const {
    return id_;
}

void TreeNodeItem::setHighlighted(bool highlighted) {
    setBrush(highlighted ? QColor("#fde68a") : QColor("#f8fafc"));
    setPen(QPen(highlighted ? QColor("#d97706") : QColor("#334155"), highlighted ? 3 : 2));
}

void TreeNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    emit clicked(id_);
    QGraphicsEllipseItem::mousePressEvent(event);
}
