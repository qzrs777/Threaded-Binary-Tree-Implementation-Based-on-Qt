#include "threadedbinarytree.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
int failures = 0;

void check(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

std::vector<int> values(const std::vector<const ThreadedNode*>& nodes) {
    std::vector<int> result;
    for (const auto* node : nodes) result.push_back(node->value);
    return result;
}

void expectValid(const ThreadedBinaryTree& tree, const std::string& context) {
    const auto validation = tree.validate();
    check(validation.empty(), context + ": " + validation);
    check(values(tree.inorderNodes()) == values(tree.threadedInorderNodes()),
          context + ": recursive and threaded traversals differ");
}

void testEmptyAndSingleNode() {
    ThreadedBinaryTree tree;
    check(tree.empty(), "new tree should be empty");
    expectValid(tree, "empty tree");
    const auto root = tree.createRoot(10);
    check(tree.rootId() == root, "root ID should be stable");
    check(values(tree.threadedInorderNodes()) == std::vector<int>{10}, "single-node traversal");
    expectValid(tree, "single-node tree");
}

void testInsertionAndOccupiedSide() {
    ThreadedBinaryTree tree;
    const auto root = tree.createRoot(10);
    const auto left = tree.insertLeft(root, 5);
    const auto right = tree.insertRight(root, 15);
    check(left && right, "empty child positions accept insertion");
    check(values(tree.threadedInorderNodes()) == std::vector<int>({5, 10, 15}), "inorder sequence");
    check(!tree.insertLeft(root, 4), "occupied left side is rejected");
    check(values(tree.threadedInorderNodes()) == std::vector<int>({5, 10, 15}), "rejection is atomic");
    expectValid(tree, "insertions");
}

void testDuplicateValuesUseDistinctIds() {
    ThreadedBinaryTree tree;
    const auto root = tree.createRoot(7);
    const auto left = tree.insertLeft(root, 7);
    check(left && *left != root, "duplicate values receive distinct IDs");
    check(tree.find(root) != tree.find(*left), "lookup uses ID rather than value");
    expectValid(tree, "duplicate values");
}

void testLeafDeletion() {
    ThreadedBinaryTree tree;
    const auto root = tree.createRoot(10);
    const auto left = tree.insertLeft(root, 5);
    tree.insertRight(root, 15);
    check(left && tree.erase(*left), "leaf deletion succeeds");
    check(values(tree.threadedInorderNodes()) == std::vector<int>({10, 15}), "leaf disappears");
    check(tree.find(*left) == nullptr, "deleted leaf is released");
    expectValid(tree, "leaf deletion");
}

void testRootDeletionWithOneChild() {
    ThreadedBinaryTree leftTree;
    const auto leftRoot = leftTree.createRoot(2);
    const auto left = leftTree.insertLeft(leftRoot, 1);
    check(left && leftTree.erase(leftRoot), "delete root with left child");
    check(leftTree.rootId() == *left, "left child becomes root");
    check(values(leftTree.threadedInorderNodes()) == std::vector<int>{1}, "no stale successor");
    expectValid(leftTree, "left-child root deletion");

    ThreadedBinaryTree rightTree;
    const auto rightRoot = rightTree.createRoot(2);
    const auto right = rightTree.insertRight(rightRoot, 3);
    check(right && rightTree.erase(rightRoot), "delete root with right child");
    check(rightTree.rootId() == *right, "right child becomes root");
    check(values(rightTree.threadedInorderNodes()) == std::vector<int>{3}, "no stale predecessor");
    expectValid(rightTree, "right-child root deletion");
}

void testNonRootDeletionWithOneChild() {
    ThreadedBinaryTree tree;
    const auto root = tree.createRoot(10);
    const auto left = tree.insertLeft(root, 5);
    const auto child = tree.insertRight(*left, 7);
    check(child && tree.erase(*left), "delete non-root with one child");
    check(values(tree.threadedInorderNodes()) == std::vector<int>({7, 10}), "child replaces parent");
    expectValid(tree, "non-root one-child deletion");
}

void testTwoChildDeletion() {
    ThreadedBinaryTree immediate;
    const auto root = immediate.createRoot(10);
    const auto left = immediate.insertLeft(root, 5);
    immediate.insertRight(root, 15);
    check(left && immediate.erase(root), "two-child deletion with immediate predecessor");
    check(immediate.find(root) == nullptr, "target ID is removed");
    check(values(immediate.threadedInorderNodes()) == std::vector<int>({5, 15}), "order is preserved");
    expectValid(immediate, "immediate predecessor");

    ThreadedBinaryTree deep;
    const auto deepRoot = deep.createRoot(20);
    const auto n10 = deep.insertLeft(deepRoot, 10);
    deep.insertRight(deepRoot, 30);
    const auto n15 = deep.insertRight(*n10, 15);
    deep.insertLeft(*n15, 13);
    check(deep.erase(deepRoot), "two-child deletion with deep predecessor");
    check(deep.find(deepRoot) == nullptr, "deep case removes target ID");
    check(values(deep.threadedInorderNodes()) == std::vector<int>({10, 13, 15, 30}), "all nodes remain");
    expectValid(deep, "deep predecessor");
}

void testMixedMutations() {
    ThreadedBinaryTree tree;
    const auto root = tree.createRoot(8);
    const auto n4 = tree.insertLeft(root, 4);
    const auto n12 = tree.insertRight(root, 12);
    const auto n2 = tree.insertLeft(*n4, 2);
    tree.insertRight(*n4, 6);
    tree.insertLeft(*n12, 10);
    tree.insertRight(*n12, 14);
    expectValid(tree, "seven-node tree");
    check(tree.erase(*n4), "delete two-child non-root");
    check(tree.erase(*n2), "delete leaf by stable ID");
    check(values(tree.threadedInorderNodes()) == std::vector<int>({6, 8, 10, 12, 14}), "mixed sequence");
    expectValid(tree, "mixed mutations");
}
} // namespace

int main() {
    testEmptyAndSingleNode();
    testInsertionAndOccupiedSide();
    testDuplicateValuesUseDistinctIds();
    testLeafDeletion();
    testRootDeletionWithOneChild();
    testNonRootDeletionWithOneChild();
    testTwoChildDeletion();
    testMixedMutations();
    if (failures) {
        std::cerr << failures << " test assertion(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All threaded binary tree tests passed\n";
    return EXIT_SUCCESS;
}
