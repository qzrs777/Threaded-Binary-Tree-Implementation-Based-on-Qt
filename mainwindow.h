#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "threadedbinarytree.h"

#include <QHash>
#include <QMainWindow>
#include <optional>

class QLabel;
class QGraphicsScene;
class QGraphicsView;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QString;
class TreeNodeItem;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onAddButtonClicked();
    void onDeleteButtonClicked();
    void onNodeClicked(NodeId id);

private:
    void drawTree();
    void assignDepths(const ThreadedNode* node, int depth, QHash<NodeId, int>& depths) const;
    void drawEdge(NodeId from, NodeId to, bool thread, bool predecessorThread = false);
    void updateSelectionText();
    void updateTraversalText();
    void showInputError(const QString& message);

    ThreadedBinaryTree tree_;
    std::optional<NodeId> selectedNodeId_;
    QHash<NodeId, TreeNodeItem*> nodeItems_;

    QGraphicsScene* scene_;
    QGraphicsView* view_;
    QLineEdit* nodeInput_;
    QRadioButton* leftOption_;
    QRadioButton* rightOption_;
    QPushButton* addButton_;
    QPushButton* deleteButton_;
    QLabel* selectionLabel_;
    QLabel* traversalLabel_;
};

#endif
