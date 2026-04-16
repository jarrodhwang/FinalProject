#include "BTree.h"

#include <algorithm>
#include <stdexcept>

namespace finalproject {

BTree::Node::Node(bool isLeaf) : leaf(isLeaf) {
}

BTree::BTree(int order)
    : root_(nullptr),
      order_(order),
      maxKeys_(order - 1),
      size_(0),
      stats_() {
    if (order_ < 3) {
        throw std::invalid_argument("B-Tree order must be at least 3.");
    }
}

void BTree::countNodeTouch(std::size_t visits) const {
    stats_.nodeVisits += visits;
}

void BTree::splitChild(Node* parent, std::size_t childIndex) {
    Node* child = parent->children[childIndex].get();
    std::unique_ptr<Node> sibling(new Node(child->leaf));

    std::size_t middle = child->keys.size() / 2;
    int promotedKey = child->keys[middle];
    int promotedValue = child->values[middle];

    sibling->keys.assign(child->keys.begin() + static_cast<std::ptrdiff_t>(middle) + 1, child->keys.end());
    sibling->values.assign(child->values.begin() + static_cast<std::ptrdiff_t>(middle) + 1, child->values.end());

    child->keys.erase(child->keys.begin() + static_cast<std::ptrdiff_t>(middle), child->keys.end());
    child->values.erase(child->values.begin() + static_cast<std::ptrdiff_t>(middle), child->values.end());

    if (!child->leaf) {
        sibling->children.assign(
            std::make_move_iterator(child->children.begin() + static_cast<std::ptrdiff_t>(middle) + 1),
            std::make_move_iterator(child->children.end()));
        child->children.erase(child->children.begin() + static_cast<std::ptrdiff_t>(middle) + 1,
                              child->children.end());
    }

    parent->keys.insert(parent->keys.begin() + static_cast<std::ptrdiff_t>(childIndex), promotedKey);
    parent->values.insert(parent->values.begin() + static_cast<std::ptrdiff_t>(childIndex), promotedValue);
    parent->children.insert(parent->children.begin() + static_cast<std::ptrdiff_t>(childIndex) + 1,
                            std::move(sibling));

    stats_.nodeSplits++;
    countNodeTouch(2);
}

bool BTree::insertNonFull(Node* node, int key, int value) {
    countNodeTouch();

    std::vector<int>::iterator position = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    std::size_t index = static_cast<std::size_t>(position - node->keys.begin());

    if (node->leaf) {
        if (index < node->keys.size() && node->keys[index] == key) {
            node->values[index] = value;
            return false;
        }

        node->keys.insert(position, key);
        node->values.insert(node->values.begin() + static_cast<std::ptrdiff_t>(index), value);
        size_++;
        return true;
    }

    if (index < node->keys.size() && node->keys[index] == key) {
        node->values[index] = value;
        return false;
    }

    if (node->children[index]->keys.size() == static_cast<std::size_t>(maxKeys_)) {
        splitChild(node, index);

        if (key == node->keys[index]) {
            node->values[index] = value;
            return false;
        }

        if (key > node->keys[index]) {
            index++;
        }
    }

    return insertNonFull(node->children[index].get(), key, value);
}

bool BTree::searchRecursive(const Node* node, int key, int& value) const {
    if (node == nullptr) {
        return false;
    }

    countNodeTouch();

    std::vector<int>::const_iterator position = std::lower_bound(node->keys.begin(), node->keys.end(), key);
    std::size_t index = static_cast<std::size_t>(position - node->keys.begin());

    if (index < node->keys.size() && node->keys[index] == key) {
        value = node->values[index];
        return true;
    }

    if (node->leaf) {
        return false;
    }

    return searchRecursive(node->children[index].get(), key, value);
}

void BTree::rangeQueryRecursive(const Node* node, int low, int high,
                                std::vector<std::pair<int, int>>& result) const {
    if (node == nullptr || low > high) {
        return;
    }

    countNodeTouch();

    for (std::size_t index = 0; index < node->keys.size(); index++) {
        if (!node->leaf) {
            bool overlapsChild = false;
            if (index == 0) {
                overlapsChild = low < node->keys[index];
            } else {
                overlapsChild = (low < node->keys[index]) && (high > node->keys[index - 1]);
            }

            if (overlapsChild) {
                rangeQueryRecursive(node->children[index].get(), low, high, result);
            }
        }

        if (node->keys[index] >= low && node->keys[index] <= high) {
            result.push_back(std::make_pair(node->keys[index], node->values[index]));
        }

        if (node->keys[index] > high) {
            return;
        }
    }

    if (!node->leaf && !node->children.empty()) {
        if (node->keys.empty() || high > node->keys.back()) {
            rangeQueryRecursive(node->children.back().get(), low, high, result);
        }
    }
}

bool BTree::insert(int key, int value) {
    if (root_ == nullptr) {
        root_.reset(new Node(true));
        root_->keys.push_back(key);
        root_->values.push_back(value);
        size_ = 1;
        countNodeTouch();
        return true;
    }

    if (root_->keys.size() == static_cast<std::size_t>(maxKeys_)) {
        std::unique_ptr<Node> newRoot(new Node(false));
        newRoot->children.push_back(std::move(root_));
        splitChild(newRoot.get(), 0);
        root_ = std::move(newRoot);
    }

    return insertNonFull(root_.get(), key, value);
}

bool BTree::search(int key, int& value) const {
    return searchRecursive(root_.get(), key, value);
}

std::vector<std::pair<int, int>> BTree::rangeQuery(int low, int high) const {
    std::vector<std::pair<int, int>> result;
    rangeQueryRecursive(root_.get(), low, high, result);
    return result;
}

std::string BTree::name() const {
    return "B-Tree";
}

int BTree::order() const {
    return order_;
}

int BTree::height() const {
    int levels = 0;
    const Node* current = root_.get();

    while (current != nullptr) {
        levels++;
        if (current->leaf) {
            break;
        }
        current = current->children.front().get();
    }

    return levels;
}

TreeStats BTree::stats() const {
    return stats_;
}

}
