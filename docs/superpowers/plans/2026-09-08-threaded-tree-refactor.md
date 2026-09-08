# Threaded Binary Tree Refactor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the coupled Qt/tree implementation with a tested threaded-tree model and a GUI that safely visualizes and edits it.

**Architecture:** A Qt-independent `ThreadedBinaryTree` owns all nodes and rebuilds inorder threads after structural mutations. `MainWindow` acts as controller, while `TreeNodeItem` and `TreeEdgeItem` only render model snapshots and emit stable node IDs.

**Tech Stack:** C++17, CMake, Qt 5/6 Widgets, QGraphicsScene, CTest

**Spec:** `docs/superpowers/specs/2026-09-08-threaded-tree-refactor-design.md`

## Global Constraints

- Keep compatibility with both Qt 5 and Qt 6 Widgets.
- Use the classic `left/right` pointer plus `ltag/rtag` representation in the model.
- Own nodes centrally with `std::unordered_map<NodeId, std::unique_ptr<Node>>`.
- Reject insertion when the requested child position is occupied.
- Rebuild and validate inorder threads after every successful mutation.
- Do not make Qt types a dependency of the core tree model or its tests.

---

### Task 1: Tested threaded-tree core

**Files:**
- Create: `threadedbinarytree.h`
- Create: `threadedbinarytree.cpp`
- Create: `tests/threadedbinarytree_tests.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces: `using NodeId = std::uint64_t`, `enum class LinkTag { Child, Thread }`, and `struct ThreadedNode`.
- Produces: `ThreadedBinaryTree::createRoot(int)`, `insertLeft(NodeId,int)`, `insertRight(NodeId,int)`, `erase(NodeId)`, `find(NodeId)`, `inorderNodes()`, `threadedInorderNodes()`, and `validate()`.

- [ ] **Step 1: Write failing core tests**

```cpp
ThreadedBinaryTree tree;
auto root = tree.createRoot(10);
auto left = tree.insertLeft(root, 5);
auto right = tree.insertRight(root, 15);
expectValues(tree.threadedInorderNodes(), {5, 10, 15});
EXPECT_FALSE(tree.insertLeft(root, 4).has_value());
EXPECT_TRUE(tree.validate().empty());
```

Add cases for duplicate values, leaf deletion, one-child root/non-root deletion, immediate/deep predecessor deletion, and mixed mutations.

- [ ] **Step 2: Run the core test target and verify it fails because the model is absent**

Run: `cmake -S . -B build -DBUILD_TESTING=ON && cmake --build build --config Release --target threaded_tree_tests`

Expected: configuration or compilation fails because `threadedbinarytree.h/.cpp` are not implemented.

- [ ] **Step 3: Implement the model and ownership rules**

```cpp
struct ThreadedNode {
    NodeId id;
    int value;
    ThreadedNode* left = nullptr;
    ThreadedNode* right = nullptr;
    LinkTag ltag = LinkTag::Thread;
    LinkTag rtag = LinkTag::Thread;
};

class ThreadedBinaryTree {
public:
    NodeId createRoot(int value);
    std::optional<NodeId> insertLeft(NodeId parent, int value);
    std::optional<NodeId> insertRight(NodeId parent, int value);
    bool erase(NodeId id);
    const ThreadedNode* find(NodeId id) const;
    std::vector<const ThreadedNode*> inorderNodes() const;
    std::vector<const ThreadedNode*> threadedInorderNodes() const;
    std::string validate() const;
};
```

Before mutation, `clearThreads()` sets thread pointers to null. After mutation, `rebuildThreads()` collects recursive inorder nodes and assigns missing-child predecessor/successor pointers. `erase()` performs ordinary structural deletion and removes the detached node from the ownership map.

- [ ] **Step 4: Build and run the core tests**

Run: `ctest --test-dir build -C Release --output-on-failure`

Expected: all threaded-tree core cases pass.

- [ ] **Step 5: Commit the model and tests**

```bash
git add CMakeLists.txt threadedbinarytree.h threadedbinarytree.cpp tests/threadedbinarytree_tests.cpp
git commit -m "refactor: add tested threaded tree model"
```

### Task 2: Decouple Qt graphics items from the tree model

**Files:**
- Modify: `treenodeitem.h`
- Modify: `treenodeitem.cpp`
- Modify: `treeedgeitem.h`
- Modify: `treeedgeitem.cpp`

**Interfaces:**
- Consumes: `NodeId` from `threadedbinarytree.h`.
- Produces: `TreeNodeItem(NodeId id, int value)`, `NodeId nodeId() const`, and signal `clicked(NodeId id)`.
- Produces: `TreeEdgeItem(QPainterPath, EdgeKind)` with `EdgeKind::Child` and `EdgeKind::Thread`.

- [ ] **Step 1: Change node items to store only ID and display value**

```cpp
class TreeNodeItem : public QObject, public QGraphicsEllipseItem {
    Q_OBJECT
public:
    TreeNodeItem(NodeId id, int value, QGraphicsItem* parent = nullptr);
    NodeId nodeId() const;
signals:
    void clicked(NodeId id);
};
```

Remove all child pointers and link tags from the graphics item.

- [ ] **Step 2: Make edge type explicit and draw thread direction**

Child edges use a solid dark pen. Thread edges use a red dashed pen and a small arrowhead at the destination so predecessor/successor direction is visible.

- [ ] **Step 3: Build the Qt target to expose controller integration failures**

Run: `cmake --build build --config Release --target untitled1`

Expected: compilation fails in `MainWindow` because it still calls removed `TreeNodeItem` tree methods.

- [ ] **Step 4: Commit the graphics-item separation**

```bash
git add treenodeitem.h treenodeitem.cpp treeedgeitem.h treeedgeitem.cpp
git commit -m "refactor: separate tree graphics items from model"
```

### Task 3: Rebuild the Qt controller and visualization

**Files:**
- Modify: `mainwindow.h`
- Modify: `mainwindow.cpp`
- Modify: `main.cpp`

**Interfaces:**
- Consumes: `ThreadedBinaryTree` mutation/traversal APIs and `TreeNodeItem::clicked(NodeId)`.
- Produces: input validation, model-backed selection, inorder traversal display, and rank/depth layout.

- [ ] **Step 1: Replace GUI-owned tree pointers with the model**

```cpp
ThreadedBinaryTree tree_;
std::optional<NodeId> selectedNodeId_;
QHash<NodeId, TreeNodeItem*> nodeItems_;
```

Initialize the model with root value `1`; redraw solely from model nodes.

- [ ] **Step 2: Implement rank/depth layout and edge rendering**

Assign each node's horizontal coordinate from its index in recursive inorder traversal and its vertical coordinate from real-child depth. Draw child edges first, then thread edges, and add a legend describing both styles.

- [ ] **Step 3: Route insert/delete actions through the model**

Parse input with `QString::toInt(&ok)`. Reject empty/non-integer input, missing selection, missing direction, and occupied child positions with specific messages. After each successful operation, redraw and update the traversal label from `threadedInorderNodes()`.

- [ ] **Step 4: Build and smoke-test the application**

Run: `cmake --build build --config Release --target untitled1`

Expected: the GUI builds; selecting nodes, inserting on empty sides, deleting all structural cases, and displaying inorder traversal use the model rather than graphics pointers.

- [ ] **Step 5: Commit the controller refactor**

```bash
git add mainwindow.h mainwindow.cpp main.cpp
git commit -m "refactor: drive Qt visualization from tree model"
```

### Task 4: Documentation and final verification

**Files:**
- Modify: `README.md`
- Modify: `CMakeLists.txt`
- Retain as unbuilt reference: `first.cpp`

**Interfaces:**
- Consumes: final model, GUI, and test commands.
- Produces: reproducible build/test instructions and an explanation of tree/thread edge semantics.

- [ ] **Step 1: Document the architecture and operations**

Describe prerequisites, CMake commands, insertion semantics, deletion behavior, stable IDs, automatic rethreading, traversal display, and line-style legend. Mark `first.cpp` as a legacy console prototype not included in the maintained target.

- [ ] **Step 2: Run a clean Release build and complete tests**

Run: `cmake -S . -B build-final -DBUILD_TESTING=ON && cmake --build build-final --config Release && ctest --test-dir build-final -C Release --output-on-failure`

Expected: both GUI and test targets build and all tests pass.

- [ ] **Step 3: Inspect repository changes**

Run: `git status --short && git diff --check && git log --oneline -5`

Expected: no whitespace errors or untracked build products; only intended source, tests, documentation, and plan changes are committed.

- [ ] **Step 4: Commit documentation and push**

```bash
git add README.md CMakeLists.txt docs/superpowers/plans/2026-09-08-threaded-tree-refactor.md
git commit -m "docs: document threaded tree application"
git push origin main
```
