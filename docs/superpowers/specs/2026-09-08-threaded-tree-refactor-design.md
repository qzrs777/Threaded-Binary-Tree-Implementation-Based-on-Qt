# Threaded Binary Tree Refactor Design

## Objective

Refactor the Qt threaded-binary-tree assignment so that tree algorithms are independent of the GUI, insertion and deletion preserve inorder-thread invariants, repeated display values are safe, and the core behavior can be verified automatically.

## Architecture

The project will be separated into three responsibilities:

- `ThreadedBinaryTree`: owns nodes and implements structural mutations, threading, traversal, lookup, and invariant validation.
- `TreeNodeItem` / `TreeEdgeItem`: render immutable node snapshots and emit selection events by stable node ID.
- `MainWindow`: validates user input, calls the model, reports errors, and redraws the current model state.

Nodes will have a stable integer ID distinct from their display value. The tree owns nodes through an internal `std::unordered_map<NodeId, std::unique_ptr<Node>>`; the root and node links are non-owning pointers into that store. This permits classic `left/right + ltag/rtag` threaded-tree representation without ambiguous memory ownership.

## Mutation Strategy

Correctness is prioritized over incremental micro-optimization. Each insertion or deletion will:

1. remove existing thread links while preserving real-child links;
2. mutate the ordinary binary-tree topology;
3. rebuild inorder predecessor and successor threads in one traversal;
4. validate the resulting invariants before returning success.

Insertion into an occupied side will be rejected instead of silently inserting a node between the parent and existing subtree. Deletion will use ordinary binary-tree cases: leaf, one child, and two children. A two-child deletion will replace the target's display value with its inorder predecessor's value and then remove that predecessor structurally.

## Invariants

Validation will check that:

- every `LINK` points to a real child;
- every `THREAD` is null or points to the immediate inorder predecessor/successor;
- the real-child graph is connected and acyclic from the root;
- every owned node is reachable exactly once;
- recursive inorder traversal equals threaded inorder traversal.

## User Interface

The GUI will retain node selection, left/right insertion, and deletion. Improvements include:

- selection by node ID so duplicate values remain unambiguous;
- explicit validation for empty and non-integer input;
- clear errors for an occupied child position;
- inorder traversal display to demonstrate the purpose of threading;
- solid child edges and directed red dashed thread edges with a legend;
- layout based on inorder rank and depth, avoiding the current fixed-offset collapse on deeper trees.

## Tests

A standalone core test executable will cover:

- empty and single-node trees;
- left and right insertion;
- duplicate display values with distinct IDs;
- leaf deletion;
- deletion of root/non-root nodes with one child;
- deletion of nodes with two children, including immediate and deep predecessors;
- rejection of insertion into an occupied side;
- equality of recursive and threaded inorder traversal after every mutation;
- invariant validation across mixed insertion/deletion sequences.

The Qt executable and core tests will be built with CMake/CTest. The obsolete console prototype will be removed from the maintained implementation after its useful behavior is covered by the model and tests.
