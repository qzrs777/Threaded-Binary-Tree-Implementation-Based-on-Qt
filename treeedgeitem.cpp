#include "treeedgeitem.h"

#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <cmath>

TreeEdgeItem::TreeEdgeItem(const QPainterPath& path, TreeEdgeKind kind, QGraphicsItem* parent)
    : QGraphicsPathItem(path, parent), kind_(kind) {
    QPen edgePen(kind == TreeEdgeKind::Thread ? QColor("#dc2626") : QColor("#64748b"));
    edgePen.setWidthF(kind == TreeEdgeKind::Thread ? 1.5 : 2.0);
    edgePen.setStyle(kind == TreeEdgeKind::Thread ? Qt::DashLine : Qt::SolidLine);
    edgePen.setCapStyle(Qt::RoundCap);
    setPen(edgePen);
    setZValue(-1);
}

TreeEdgeKind TreeEdgeItem::kind() const {
    return kind_;
}

void TreeEdgeItem::paint(QPainter* painter,
                         const QStyleOptionGraphicsItem* option,
                         QWidget* widget) {
    QGraphicsPathItem::paint(painter, option, widget);
    if (kind_ != TreeEdgeKind::Thread || path().isEmpty()) return;

    const QPointF tip = path().pointAtPercent(1.0);
    const QPointF before = path().pointAtPercent(0.96);
    const double angle = std::atan2(tip.y() - before.y(), tip.x() - before.x());
    constexpr double arrowSize = 8.0;
    constexpr double pi = 3.14159265358979323846;
    const QPointF p1 = tip - QPointF(std::cos(angle - pi / 6.0) * arrowSize,
                                     std::sin(angle - pi / 6.0) * arrowSize);
    const QPointF p2 = tip - QPointF(std::cos(angle + pi / 6.0) * arrowSize,
                                     std::sin(angle + pi / 6.0) * arrowSize);
    painter->setPen(Qt::NoPen);
    painter->setBrush(pen().color());
    painter->drawPolygon(QPolygonF() << tip << p1 << p2);
}
