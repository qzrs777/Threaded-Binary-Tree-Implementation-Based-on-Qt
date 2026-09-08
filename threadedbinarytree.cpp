#include "threadedbinarytree.h"

#include <sstream>
#include <unordered_set>

ThreadedNode* ThreadedBinaryTree::createNode(int value) {
    const NodeId id = nextId_++;
    auto node = std::make_unique<ThreadedNode>();
    node->id = id;
    node->value = value;
    auto* raw = node.get();
    nodes_.emplace(id, std::move(node));
    return raw;
}

NodeId ThreadedBinaryTree::createRoot(int value) {
    nodes_.clear();
    root_ = createNode(value);
    rebuildThreads();
    return root_->id;
}

std::optional<NodeId> ThreadedBinaryTree::insertLeft(NodeId parentId, int value) {
    auto* parent = findMutable(parentId);
    if (!parent || parent->ltag == LinkTag::Child) return std::nullopt;

    clearThreads();
    parent = findMutable(parentId);
    auto* node = createNode(value);
    parent->left = node;
    rebuildThreads();
    return node->id;
}

std::optional<NodeId> ThreadedBinaryTree::insertRight(NodeId parentId, int value) {
    auto* parent = findMutable(parentId);
    if (!parent || parent->rtag == LinkTag::Child) return std::nullopt;

    clearThreads();
    parent = findMutable(parentId);
    auto* node = createNode(value);
    parent->right = node;
    rebuildThreads();
    return node->id;
}

bool ThreadedBinaryTree::erase(NodeId id) {
    if (!findMutable(id)) return false;
    clearThreads();

    ThreadedNode** targetLink = findStructuralLink(id);
    if (!targetLink || !*targetLink) {
        rebuildThreads();
        return false;
    }

    ThreadedNode* target = *targetLink;
    if (!target->left) {
        *targetLink = target->right;
    } else if (!target->right) {
        *targetLink = target->left;
    } else {
        ThreadedNode* predecessorParent = target;
        ThreadedNode* predecessor = target->left;
        while (predecessor->right) {
            predecessorParent = predecessor;
            predecessor = predecessor->right;
        }

        if (predecessorParent != target) {
            predecessorParent->right = predecessor->left;
            predecessor->left = target->left;
        }
        predecessor->right = target->right;
        *targetLink = predecessor;
    }

    nodes_.erase(id);
    rebuildThreads();
    return true;
}

bool ThreadedBinaryTree::empty() const { return root_ == nullptr; }
std::size_t ThreadedBinaryTree::size() const { return nodes_.size(); }
NodeId ThreadedBinaryTree::rootId() const { return root_ ? root_->id : 0; }
const ThreadedNode* ThreadedBinaryTree::root() const { return root_; }

const ThreadedNode* ThreadedBinaryTree::find(NodeId id) const {
    const auto it = nodes_.find(id);
    return it == nodes_.end() ? nullptr : it->second.get();
}

ThreadedNode* ThreadedBinaryTree::findMutable(NodeId id) {
    const auto it = nodes_.find(id);
    return it == nodes_.end() ? nullptr : it->second.get();
}

ThreadedNode** ThreadedBinaryTree::findStructuralLink(NodeId id) {
    return findStructuralLinkFrom(&root_, id);
}

ThreadedNode** ThreadedBinaryTree::findStructuralLinkFrom(ThreadedNode** link, NodeId id) {
    if (!link || !*link) return nullptr;
    if ((*link)->id == id) return link;
    if (auto* found = findStructuralLinkFrom(&(*link)->left, id)) return found;
    return findStructuralLinkFrom(&(*link)->right, id);
}

void ThreadedBinaryTree::clearThreads() {
    for (auto& entry : nodes_) {
        auto* node = entry.second.get();
        if (node->ltag == LinkTag::Thread) node->left = nullptr;
        if (node->rtag == LinkTag::Thread) node->right = nullptr;
    }
}

void ThreadedBinaryTree::collectPlainInorder(ThreadedNode* node, std::vector<ThreadedNode*>& nodes) {
    if (!node) return;
    collectPlainInorder(node->left, nodes);
    nodes.push_back(node);
    collectPlainInorder(node->right, nodes);
}

void ThreadedBinaryTree::rebuildThreads() {
    if (!root_) return;
    std::vector<ThreadedNode*> ordered;
    collectPlainInorder(root_, ordered);
    for (std::size_t i = 0; i < ordered.size(); ++i) {
        auto* node = ordered[i];
        if (node->left) {
            node->ltag = LinkTag::Child;
        } else {
            node->ltag = LinkTag::Thread;
            node->left = i == 0 ? nullptr : ordered[i - 1];
        }
        if (node->right) {
            node->rtag = LinkTag::Child;
        } else {
            node->rtag = LinkTag::Thread;
            node->right = i + 1 < ordered.size() ? ordered[i + 1] : nullptr;
        }
    }
}

void ThreadedBinaryTree::collectInorder(const ThreadedNode* node,
                                        std::vector<const ThreadedNode*>& nodes) {
    if (!node) return;
    if (node->ltag == LinkTag::Child) collectInorder(node->left, nodes);
    nodes.push_back(node);
    if (node->rtag == LinkTag::Child) collectInorder(node->right, nodes);
}

std::vector<const ThreadedNode*> ThreadedBinaryTree::inorderNodes() const {
    std::vector<const ThreadedNode*> result;
    collectInorder(root_, result);
    return result;
}

std::vector<const ThreadedNode*> ThreadedBinaryTree::threadedInorderNodes() const {
    std::vector<const ThreadedNode*> result;
    const ThreadedNode* current = root_;
    while (current && current->ltag == LinkTag::Child) current = current->left;
    while (current) {
        result.push_back(current);
        if (current->rtag == LinkTag::Thread) {
            current = current->right;
        } else {
            current = current->right;
            while (current && current->ltag == LinkTag::Child) current = current->left;
        }
    }
    return result;
}

std::string ThreadedBinaryTree::validate() const {
    if (!root_) return nodes_.empty() ? std::string{} : "owned nodes exist without a root";

    std::unordered_set<const ThreadedNode*> owned;
    for (const auto& entry : nodes_) owned.insert(entry.second.get());
    if (!owned.count(root_)) return "root is not owned by the tree";

    std::unordered_set<const ThreadedNode*> visited;
    std::string graphError;
    const auto walk = [&](const auto& self, const ThreadedNode* node) -> void {
        if (!node || !graphError.empty()) return;
        if (!owned.count(node)) {
            graphError = "a child link points to an unowned node";
            return;
        }
        if (!visited.insert(node).second) {
            graphError = "the real-child graph contains a cycle or repeated node";
            return;
        }
        if (node->ltag == LinkTag::Child) {
            if (!node->left) graphError = "a left child tag has a null pointer";
            else self(self, node->left);
        }
        if (node->rtag == LinkTag::Child) {
            if (!node->right) graphError = "a right child tag has a null pointer";
            else self(self, node->right);
        }
    };
    walk(walk, root_);
    if (!graphError.empty()) return graphError;
    if (visited.size() != nodes_.size()) return "an owned node is unreachable from the root";

    const auto ordered = inorderNodes();
    for (std::size_t i = 0; i < ordered.size(); ++i) {
        const auto* node = ordered[i];
        if (node->ltag == LinkTag::Thread) {
            const auto* expected = i == 0 ? nullptr : ordered[i - 1];
            if (node->left != expected) return "an inorder predecessor thread is incorrect";
        }
        if (node->rtag == LinkTag::Thread) {
            const auto* expected = i + 1 < ordered.size() ? ordered[i + 1] : nullptr;
            if (node->right != expected) return "an inorder successor thread is incorrect";
        }
    }

    const auto threaded = threadedInorderNodes();
    if (threaded.size() != ordered.size()) return "threaded traversal visits the wrong node count";
    for (std::size_t i = 0; i < ordered.size(); ++i) {
        if (threaded[i] != ordered[i]) return "threaded traversal differs from recursive inorder";
    }
    return {};
}
