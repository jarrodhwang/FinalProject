#ifndef FINALPROJECT_BENCHMARK_TYPES_H
#define FINALPROJECT_BENCHMARK_TYPES_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace finalproject {

struct TreeStats {
    std::size_t nodeSplits{0};
    std::size_t nodeVisits{0};
};

struct BenchmarkDataset {
    std::vector<int> insertKeys;
    std::vector<int> successfulSearchQueries;
    std::vector<int> unsuccessfulSearchQueries;
    std::vector<std::pair<int, int>> rangeQueries;
};

struct BenchmarkConfig {
    std::vector<std::size_t> datasetSizes;
    int repeats{5};
    int order{8};
    unsigned int baseSeed{225};
    std::string csvPath;
};

struct BenchmarkResult {
    std::string structureName;
    std::size_t datasetSize{0};
    int order{0};
    int repeats{0};
    std::size_t rangeQueryCount{0};
    double buildTimeMs{0.0};
    double successfulSearchTimeMs{0.0};
    double unsuccessfulSearchTimeMs{0.0};
    double rangeQueryTimeMs{0.0};
    double treeHeight{0.0};
    double averageNodeSplits{0.0};
    double averageBuildNodeVisits{0.0};
    double averageSuccessfulSearchNodeVisits{0.0};
    double averageUnsuccessfulSearchNodeVisits{0.0};
    double averageRangeQueryNodeVisits{0.0};
    double averageRangeResultCount{0.0};
};

int makeValueForKey(int key);

}

#endif
