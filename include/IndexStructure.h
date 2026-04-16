#ifndef FINALPROJECT_INDEX_STRUCTURE_H
#define FINALPROJECT_INDEX_STRUCTURE_H

#include <string>
#include <utility>
#include <vector>

#include "BenchmarkTypes.h"

namespace finalproject {

class IndexStructure {
public:
    virtual ~IndexStructure() = default;

    virtual bool insert(int key, int value) = 0;
    virtual bool search(int key, int& value) const = 0;
    virtual std::vector<std::pair<int, int>> rangeQuery(int low, int high) const = 0;
    virtual std::string name() const = 0;
    virtual int order() const = 0;
    virtual int height() const = 0;
    virtual TreeStats stats() const = 0;
};

}

#endif
