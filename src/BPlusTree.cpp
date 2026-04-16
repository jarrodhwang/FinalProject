#include "BPlusTree.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace finalproject {

BPlusTree::Node::Node(bool isLeaf) : leaf(isLeaf), next(nullptr) {
}

BPlusTree::InsertResult::InsertResult()
    : inserted(false),
      split(false),
      promotedKey(0),
      rightNode(nullptr) {
}

BPlusTree::BPlusTree(int order)
    : root_(nullptr),
      order_(order),
      maxKeys_(order - 1),
      size_(0),
      stats_() {
    if (order_ < 3) {
        throw std::invalid_argument("B+ Tree order must be at least 3.");
    }
}

void BPlusTree::countNodeTouch(std::size_t visits) const {
    stats_.nodeVisits += visits;
}

BPlusTree::InsertResult BPlusTree::splitLeaf(Node* node) {
    InsertResult result;
    std::unique_ptr<Node> sibling(new Node(true));
    std::size_t middle = node->keys.size() / 2;

    sibling->keys.assign(node->keys.begin() + static_cast<std::ptrdiff_t>(middle), node->keys.end());
    sibling->values.assign(node->values.begin() + static_cast<std::ptrdiff_t>(middle), node->values.end());

    node->keys.erase(node->keys.begin() + static_cast<std::ptrdiff_t>(middle), node->keys.end());
    node->values.erase(node->values.begin() + static_cast<std::ptrdiff_t>(middle), node->values.end());

    sibling->next = node->next;
    node->next = sibling.get();

    result.split = true;
    result.promotedKey = sibling->keys.front();
    result.rightNode = std::move(sibling);

    stats_.nodeSplits++;
    countNodeTouch(2);
    return result;
}

BPlusTree::InsertResult BPlusTree::splitInternal(Node* node) {
    InsertResult result;
    std::unique_ptr<Node> sibling(new Node(false));
    std::size_t middle = node->keys.size() / 2;

    result.split = true;
    result.promotedKey = node->keys[middle];

    sibling->keys.assign(node->keys.begin() + static_cast<std::ptrdiff_t>(middle) + 1, node->keys.end());
    sibling->children.assign(
        std::make_move_iterator(node->children.begin() + static_cast<std::ptrdiff_t>(middle) + 1),
        std::make_move_iterator(node->children.end()));

    node->keys.erase(node->keys.begin() + static_cast<std::ptrdiff_t>(middle), node->keys.end());
    node->children.erase(node->children.begin() + static_cast<std::ptrdiff_t>(middle) + 1,
                         node->children.end());

    result.rightNode = std::move(sibling);

    stats_.nodeSplits++;
    countNodeTouch(2);
    return result;
}

BPlusTree::InsertResult BPlusTree::insertRecursive(Node* node, int key, int value) {
    countNodeTouch();

    if (node->leaf) {
        InsertResult result;
        std::vector<int>::iterator position = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        std::size_t index = static_cast<std::size_t>(position - node->keys.begin());

        if (index < node->keys.size() && node->keys[index] == key) {
            node->values[index] = value;
            return result;
        }

        node->keys.insert(position, key);
        node->values.insert(node->values.begin() + static_cast<std::ptrdiff_t>(index), value);
        size_++;
        result.inserted = true;

        if (node->keys.size() > static_cast<std::size_t>(maxKeys_)) {
            InsertResult splitResult = splitLeaf(node);
            splitResult.inserted = true;
            return splitResult;
        }

        return result;
    }

    std::size_t childIndex = static_cast<std::size_t>(
        std::upper_bound(node->keys.begin(), node->keys.end(), key) - node->keys.begin());

    InsertResult childResult = insertRecursive(node->children[childIndex].get(), key, value);
    if (!childResult.split) {
        return childResult;
    }

    node->keys.insert(node->keys.begin() + static_cast<std::ptrdiff_t>(childIndex), childResult.promotedKey);
    node->children.insert(node->children.begin() + static_cast<std::ptrdiff_t>(childIndex) + 1,
                          std::move(childResult.rightNode));

    if (node->keys.size() > static_cast<std::size_t>(maxKeys_)) {
        InsertResult ownSplit = splitInternal(node);
        ownSplit.inserted = childResult.inserted;
        return ownSplit;
    }

    childResult.split = false;
    childResult.promotedKey = 0;
    return childResult;
}

const BPlusTree::Node* BPlusTree::findLeafForKey(int key) const {
    const Node* current = root_.get();

    while (current != nullptr && !current->leaf) {
        countNodeTouch();
        std::size_t childIndex = static_cast<std::size_t>(
            std::upper_bound(current->keys.begin(), current->keys.end(), key) - current->keys.begin());
        current = current->children[childIndex].get();
    }

    if (current != nullptr) {
        countNodeTouch();
    }

    return current;
}

bool BPlusTree::insert(int key, int value) {
    if (root_ == nullptr) {
        root_.reset(new Node(true));
        root_->keys.push_back(key);
        root_->values.push_back(value);
        size_ = 1;
        countNodeTouch();
        return true;
    }

    InsertResult result = insertRecursive(root_.get(), key, value);
    if (result.split) {
        std::unique_ptr<Node> newRoot(new Node(false));
        newRoot->keys.push_back(result.promotedKey);
        newRoot->children.push_back(std::move(root_));
        newRoot->children.push_back(std::move(result.rightNode));
        root_ = std::move(newRoot);
    }

    return result.inserted;
}

bool BPlusTree::search(int key, int& value) const {
    const Node* leaf = findLeafForKey(key);
    if (leaf == nullptr) {
        return false;
    }

    std::vector<int>::const_iterator position = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    std::size_t index = static_cast<std::size_t>(position - leaf->keys.begin());

    if (index < leaf->keys.size() && leaf->keys[index] == key) {
        value = leaf->values[index];
        return true;
    }

    return false;
}

std::vector<std::pair<int, int>> BPlusTree::rangeQuery(int low, int high) const {
    std::vector<std::pair<int, int>> result;
    if (root_ == nullptr || low > high) {
        return result;
    }

    const Node* leaf = findLeafForKey(low);
    while (leaf != nullptr) {
        for (std::size_t index = 0; index < leaf->keys.size(); index++) {
            if (leaf->keys[index] > high) {
                return result;
            }

            if (leaf->keys[index] >= low) {
                result.push_back(std::make_pair(leaf->keys[index], leaf->values[index]));
            }
        }

        leaf = leaf->next;
        if (leaf != nullptr) {
            countNodeTouch();
        }
    }

    return result;
}

std::string BPlusTree::name() const {
    return "B+ Tree";
}

int BPlusTree::order() const {
    return order_;
}

int BPlusTree::height() const {
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

TreeStats BPlusTree::stats() const {
    return stats_;
}

}
