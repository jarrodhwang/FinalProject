#ifndef FINALPROJECT_BTREE_H
#define FINALPROJECT_BTREE_H

#include <memory>
#include <utility>
#include <vector>

#include "IndexStructure.h"

namespace finalproject {

class BTree : public IndexStructure {
private:
    struct Node {
        bool leaf;
        std::vector<int> keys;
        std::vector<int> values;
        std::vector<std::unique_ptr<Node>> children;

        explicit Node(bool isLeaf);
    };

    std::unique_ptr<Node> root_;
    int order_;
    int maxKeys_;
    std::size_t size_;
    mutable TreeStats stats_;

    void splitChild(Node* parent, std::size_t childIndex);
    bool insertNonFull(Node* node, int key, int value);
    bool searchRecursive(const Node* node, int key, int& value) const;
    void rangeQueryRecursive(const Node* node, int low, int high,
                             std::vector<std::pair<int, int>>& result) const;
    void countNodeTouch(std::size_t visits = 1) const;

public:
    explicit BTree(int order);

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
