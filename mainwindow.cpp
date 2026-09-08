#include "mainwindow.h"

#include "treeedgeitem.h"
#include "treenodeitem.h"

#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRadioButton>
#include <QStringList>
#include <QVBoxLayout>
#include <QtMath>

namespace {
constexpr qreal kHorizontalSpacing = 105.0;
constexpr qreal kVerticalSpacing = 95.0;
constexpr qreal kNodeRadius = 23.0;

QPointF shortenedPoint(const QPointF& from, const QPointF& to, qreal distance) {
    const QPointF delta = to - from;
    const qreal length = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    if (length <= distance || qFuzzyIsNull(length)) return to;
    return to - delta / length * distance;
}
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      scene_(new QGraphicsScene(this)),
      view_(new QGraphicsView(scene_, this)),
      nodeInput_(new QLineEdit(this)),
      leftOption_(new QRadioButton(tr("左孩子"), this)),
      rightOption_(new QRadioButton(tr("右孩子"), this)),
      addButton_(new QPushButton(tr("添加节点"), this)),
      deleteButton_(new QPushButton(tr("删除选中节点"), this)),
      selectionLabel_(new QLabel(this)),
      traversalLabel_(new QLabel(this)) {
    setWindowTitle(tr("中序线索二叉树可视化"));
    resize(1000, 700);

    view_->setRenderHint(QPainter::Antialiasing);
    view_->setDragMode(QGraphicsView::ScrollHandDrag);
    view_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    nodeInput_->setPlaceholderText(tr("输入整数节点值"));
    nodeInput_->setMaximumWidth(180);
    leftOption_->setChecked(true);

    auto* controls = new QHBoxLayout;
    controls->addWidget(new QLabel(tr("节点值："), this));
    controls->addWidget(nodeInput_);
    controls->addWidget(leftOption_);
    controls->addWidget(rightOption_);
    controls->addWidget(addButton_);
    controls->addWidget(deleteButton_);
    controls->addStretch();

    auto* information = new QVBoxLayout;
    information->addWidget(selectionLabel_);
    information->addWidget(traversalLabel_);
    auto* legend = new QLabel(tr("图例：实线 = 真实孩子　　红色虚线箭头 = 中序前驱/后继线索"), this);
    legend->setStyleSheet("color:#475569;");
    information->addWidget(legend);

    auto* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(view_, 1);
    mainLayout->addLayout(controls);
    mainLayout->addLayout(information);

    auto* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(addButton_, &QPushButton::clicked, this, &MainWindow::onAddButtonClicked);
    connect(deleteButton_, &QPushButton::clicked, this, &MainWindow::onDeleteButtonClicked);
    connect(nodeInput_, &QLineEdit::returnPressed, this, &MainWindow::onAddButtonClicked);

    selectedNodeId_ = tree_.createRoot(1);
    drawTree();
}

void MainWindow::assignDepths(const ThreadedNode* node,
                              int depth,
                              QHash<NodeId, int>& depths) const {
    if (!node) return;
    depths.insert(node->id, depth);
    if (node->ltag == LinkTag::Child) assignDepths(node->left, depth + 1, depths);
    if (node->rtag == LinkTag::Child) assignDepths(node->right, depth + 1, depths);
}

void MainWindow::drawTree() {
    scene_->clear();
    nodeItems_.clear();

    if (selectedNodeId_ && !tree_.find(*selectedNodeId_)) selectedNodeId_.reset();
    const auto ordered = tree_.inorderNodes();
    if (ordered.empty()) {
        scene_->addText(tr("树为空：输入整数后点击“添加节点”创建根节点"));
        updateSelectionText();
        updateTraversalText();
        return;
    }

    QHash<NodeId, int> depths;
    assignDepths(tree_.root(), 0, depths);
    for (int i = 0; i < static_cast<int>(ordered.size()); ++i) {
        const auto* node = ordered[static_cast<std::size_t>(i)];
        auto* item = new TreeNodeItem(node->id, node->value);
        item->setPos(60.0 + i * kHorizontalSpacing,
                     60.0 + depths.value(node->id) * kVerticalSpacing);
        item->setHighlighted(selectedNodeId_ && *selectedNodeId_ == node->id);
        connect(item, &TreeNodeItem::clicked, this, &MainWindow::onNodeClicked);
        nodeItems_.insert(node->id, item);
    }

    for (const auto* node : ordered) {
        if (node->ltag == LinkTag::Child) drawEdge(node->id, node->left->id, false);
        if (node->rtag == LinkTag::Child) drawEdge(node->id, node->right->id, false);
    }
    for (const auto* node : ordered) {
        if (node->ltag == LinkTag::Thread && node->left) {
            drawEdge(node->id, node->left->id, true, true);
        }
        if (node->rtag == LinkTag::Thread && node->right) {
            drawEdge(node->id, node->right->id, true, false);
        }
    }
    for (auto* item : nodeItems_) scene_->addItem(item);

    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-55, -55, 55, 55));
    view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    updateSelectionText();
    updateTraversalText();
}

void MainWindow::drawEdge(NodeId from, NodeId to, bool thread, bool predecessorThread) {
    auto* fromItem = nodeItems_.value(from, nullptr);
    auto* toItem = nodeItems_.value(to, nullptr);
    if (!fromItem || !toItem) return;

    const QPointF fromCenter = fromItem->pos();
    const QPointF toCenter = toItem->pos();
    const QPointF start = shortenedPoint(toCenter, fromCenter, kNodeRadius);
    const QPointF end = shortenedPoint(fromCenter, toCenter, kNodeRadius + (thread ? 6.0 : 0.0));
    QPainterPath path(start);
    if (thread) {
        const QPointF middle = (start + end) / 2.0;
        const qreal bend = predecessorThread ? 42.0 : -42.0;
        path.quadTo(QPointF(middle.x(), middle.y() + bend), end);
    } else {
        path.lineTo(end);
    }
    scene_->addItem(new TreeEdgeItem(path, thread ? TreeEdgeKind::Thread : TreeEdgeKind::Child));
}

void MainWindow::onAddButtonClicked() {
    bool ok = false;
    const int value = nodeInput_->text().trimmed().toInt(&ok);
    if (!ok) {
        showInputError(tr("请输入有效的整数节点值。"));
        return;
    }

    if (tree_.empty()) {
        selectedNodeId_ = tree_.createRoot(value);
        nodeInput_->clear();
        drawTree();
        return;
    }
    if (!selectedNodeId_) {
        showInputError(tr("请先在图中选择父节点。"));
        return;
    }

    std::optional<NodeId> inserted;
    if (leftOption_->isChecked()) {
        inserted = tree_.insertLeft(*selectedNodeId_, value);
    } else if (rightOption_->isChecked()) {
        inserted = tree_.insertRight(*selectedNodeId_, value);
    } else {
        showInputError(tr("请选择左孩子或右孩子。"));
        return;
    }
    if (!inserted) {
        showInputError(tr("该方向已经存在真实孩子，请选择其他位置。"));
        return;
    }

    selectedNodeId_ = *inserted;
    nodeInput_->clear();
    drawTree();
}

void MainWindow::onDeleteButtonClicked() {
    if (!selectedNodeId_) {
        showInputError(tr("请先选择需要删除的节点。"));
        return;
    }
    if (!tree_.erase(*selectedNodeId_)) {
        showInputError(tr("选中的节点已不存在。"));
        selectedNodeId_.reset();
        drawTree();
        return;
    }
    selectedNodeId_.reset();
    drawTree();
}

void MainWindow::onNodeClicked(NodeId id) {
    if (!tree_.find(id)) return;
    selectedNodeId_ = id;
    for (auto it = nodeItems_.begin(); it != nodeItems_.end(); ++it) {
        it.value()->setHighlighted(it.key() == id);
    }
    updateSelectionText();
}

void MainWindow::updateSelectionText() {
    if (!selectedNodeId_) {
        selectionLabel_->setText(tr("当前选择：无"));
        return;
    }
    const auto* node = tree_.find(*selectedNodeId_);
    if (!node) {
        selectionLabel_->setText(tr("当前选择：无"));
        return;
    }
    selectionLabel_->setText(tr("当前选择：值 %1（节点ID %2）")
                                 .arg(node->value)
                                 .arg(static_cast<qulonglong>(node->id)));
}

void MainWindow::updateTraversalText() {
    QStringList values;
    for (const auto* node : tree_.threadedInorderNodes()) values << QString::number(node->value);
    traversalLabel_->setText(tr("中序线索遍历：%1").arg(values.isEmpty() ? tr("（空）") : values.join(" → ")));
}

void MainWindow::showInputError(const QString& message) {
    QMessageBox::warning(this, tr("操作失败"), message);
}
