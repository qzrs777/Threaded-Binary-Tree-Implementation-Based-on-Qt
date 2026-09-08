#ifndef TREEEDGEITEM_H
#define TREEEDGEITEM_H

#include <QGraphicsPathItem>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

enum class TreeEdgeKind {
    Child,
    Thread
};

class TreeEdgeItem : public QGraphicsPathItem {
public:
    TreeEdgeItem(const QPainterPath& path, TreeEdgeKind kind, QGraphicsItem* parent = nullptr);

    TreeEdgeKind kind() const;

protected:
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

private:
    TreeEdgeKind kind_;
};

#endif
