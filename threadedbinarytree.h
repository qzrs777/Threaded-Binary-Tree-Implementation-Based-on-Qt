#ifndef THREADEDBINARYTREE_H
#define THREADEDBINARYTREE_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using NodeId = std::uint64_t;

enum class LinkTag {
    Child,
    Thread
};

struct ThreadedNode {
    NodeId id = 0;
    int value = 0;
    ThreadedNode* left = nullptr;
    ThreadedNode* right = nullptr;
    LinkTag ltag = LinkTag::Thread;
    LinkTag rtag = LinkTag::Thread;
};

class ThreadedBinaryTree {
public:
    NodeId createRoot(int value);
    std::optional<NodeId> insertLeft(NodeId parentId, int value);
    std::optional<NodeId> insertRight(NodeId parentId, int value);
    bool erase(NodeId id);

    bool empty() const;
    std::size_t size() const;
    NodeId rootId() const;
    const ThreadedNode* root() const;
    const ThreadedNode* find(NodeId id) const;

    std::vector<const ThreadedNode*> inorderNodes() const;
    std::vector<const ThreadedNode*> threadedInorderNodes() const;
    std::string validate() const;

private:
    ThreadedNode* createNode(int value);
    ThreadedNode* findMutable(NodeId id);
    ThreadedNode** findStructuralLink(NodeId id);
    ThreadedNode** findStructuralLinkFrom(ThreadedNode** link, NodeId id);

    void clearThreads();
    void rebuildThreads();
    static void collectPlainInorder(ThreadedNode* node, std::vector<ThreadedNode*>& nodes);
    static void collectInorder(const ThreadedNode* node, std::vector<const ThreadedNode*>& nodes);

    std::unordered_map<NodeId, std::unique_ptr<ThreadedNode>> nodes_;
    ThreadedNode* root_ = nullptr;
    NodeId nextId_ = 1;
};

#endif

