#ifndef FINALPROJECT_BPLUS_TREE_H
#define FINALPROJECT_BPLUS_TREE_H

#include <memory>
#include <utility>
#include <vector>

#include "IndexStructure.h"

namespace finalproject {

class BPlusTree : public IndexStructure {
private:
    struct Node {
        bool leaf;
        std::vector<int> keys;
        std::vector<int> values;
        std::vector<std::unique_ptr<Node>> children;
        Node* next;

        explicit Node(bool isLeaf);
    };

    struct InsertResult {
        bool inserted;
        bool split;
        int promotedKey;
        std::unique_ptr<Node> rightNode;

        InsertResult();
    };

    std::unique_ptr<Node> root_;
    int order_;
    int maxKeys_;
    std::size_t size_;
    mutable TreeStats stats_;

    InsertResult insertRecursive(Node* node, int key, int value);
    InsertResult splitLeaf(Node* node);
    InsertResult splitInternal(Node* node);
    const Node* findLeafForKey(int key) const;
    void countNodeTouch(std::size_t visits = 1) const;

public:
    explicit BPlusTree(int order);

    bool insert(int key, int value) override;
    bool search(int key, int& value) const override;
    std::vector<std::pair<int, int>> rangeQuery(int low, int high) const override;
    std::string name() const override;
    int order() const override;
    int height() const override;
    TreeStats stats() const override;
};

}

#endif
