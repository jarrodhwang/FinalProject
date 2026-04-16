#include "BenchmarkRunner.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>

#include "BPlusTree.h"
#include "BTree.h"
#include "DatasetGenerator.h"
#include "IndexStructure.h"

namespace finalproject {
namespace {

struct ResultAccumulator {
    BenchmarkResult result;
};

double safeDivide(double total, double count) {
    if (count == 0.0) {
        return 0.0;
    }

    return total / count;
}

template <typename Work>
double measureMilliseconds(Work work) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    work();
    std::chrono::steady_clock::time_point finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

bool validateRangeResult(const std::vector<std::pair<int, int>>& rows,
                         const std::vector<int>& sortedKeys,
                         int low,
                         int high,
                         long long& checksum) {
    std::vector<int>::const_iterator expectedBegin = std::lower_bound(sortedKeys.begin(), sortedKeys.end(), low);
    std::vector<int>::const_iterator expectedEnd = std::upper_bound(sortedKeys.begin(), sortedKeys.end(), high);
    std::size_t expectedCount = static_cast<std::size_t>(expectedEnd - expectedBegin);

    if (rows.size() != expectedCount) {
        return false;
    }

    for (std::size_t index = 0; index < rows.size(); index++) {
        int expectedKey = *(expectedBegin + static_cast<std::ptrdiff_t>(index));
        int expectedValue = makeValueForKey(expectedKey);
        if (rows[index].first != expectedKey || rows[index].second != expectedValue) {
            return false;
        }
        checksum += rows[index].second;
    }

    return true;
}

}

BenchmarkConfig makeDefaultConfig() {
    BenchmarkConfig config;
    config.datasetSizes = {1000, 5000, 10000, 50000};
    config.repeats = 5;
    config.order = 8;
    config.baseSeed = 225;
    config.csvPath = "results/benchmark_results.csv";
    return config;
}

std::vector<BenchmarkResult> runBenchmarks(const BenchmarkConfig& config) {
    std::vector<std::function<std::unique_ptr<IndexStructure>(int)>> factories;
    factories.push_back([](int order) {
        return std::unique_ptr<IndexStructure>(new BTree(order));
    });
    factories.push_back([](int order) {
        return std::unique_ptr<IndexStructure>(new BPlusTree(order));
    });

    std::vector<BenchmarkResult> results;
    std::size_t validationErrors = 0;
    long long checksum = 0;

    for (std::size_t sizeIndex = 0; sizeIndex < config.datasetSizes.size(); sizeIndex++) {
        std::size_t datasetSize = config.datasetSizes[sizeIndex];
        std::vector<ResultAccumulator> accumulators(factories.size());

        for (std::size_t tableIndex = 0; tableIndex < factories.size(); tableIndex++) {
            std::unique_ptr<IndexStructure> table = factories[tableIndex](config.order);
            accumulators[tableIndex].result.structureName = table->name();
            accumulators[tableIndex].result.datasetSize = datasetSize;
            accumulators[tableIndex].result.order = config.order;
            accumulators[tableIndex].result.repeats = config.repeats;
        }

        for (int repeat = 0; repeat < config.repeats; repeat++) {
            unsigned int seed = config.baseSeed
                + static_cast<unsigned int>(datasetSize * 13)
                + static_cast<unsigned int>(repeat * 101);
            BenchmarkDataset dataset = generateDataset(datasetSize, seed);
            std::vector<int> sortedKeys = dataset.insertKeys;
            std::sort(sortedKeys.begin(), sortedKeys.end());

            for (std::size_t tableIndex = 0; tableIndex < factories.size(); tableIndex++) {
                std::unique_ptr<IndexStructure> table = factories[tableIndex](config.order);

                TreeStats beforeBuild = table->stats();
                double buildTime = measureMilliseconds([&]() {
                    for (std::size_t index = 0; index < dataset.insertKeys.size(); index++) {
                        table->insert(dataset.insertKeys[index], makeValueForKey(dataset.insertKeys[index]));
                    }
                });
                TreeStats afterBuild = table->stats();

                TreeStats beforeSuccessfulSearch = table->stats();
                double successfulSearchTime = measureMilliseconds([&]() {
                    for (std::size_t index = 0; index < dataset.successfulSearchQueries.size(); index++) {
                        int value = 0;
                        bool found = table->search(dataset.successfulSearchQueries[index], value);
                        if (!found || value != makeValueForKey(dataset.successfulSearchQueries[index])) {
                            validationErrors++;
                        }
                        checksum += value;
                    }
                });
                TreeStats afterSuccessfulSearch = table->stats();

                TreeStats beforeUnsuccessfulSearch = table->stats();
                double unsuccessfulSearchTime = measureMilliseconds([&]() {
                    for (std::size_t index = 0; index < dataset.unsuccessfulSearchQueries.size(); index++) {
                        int value = 0;
                        bool found = table->search(dataset.unsuccessfulSearchQueries[index], value);
                        if (found) {
                            validationErrors++;
                            checksum += value;
                        }
                    }
                });
                TreeStats afterUnsuccessfulSearch = table->stats();

                std::size_t totalRangeResultCount = 0;
                TreeStats beforeRangeQueries = table->stats();
                double rangeQueryTime = measureMilliseconds([&]() {
                    for (std::size_t index = 0; index < dataset.rangeQueries.size(); index++) {
                        std::vector<std::pair<int, int>> rows = table->rangeQuery(
                            dataset.rangeQueries[index].first, dataset.rangeQueries[index].second);
                        totalRangeResultCount += rows.size();
                        if (!validateRangeResult(rows, sortedKeys,
                                                 dataset.rangeQueries[index].first,
                                                 dataset.rangeQueries[index].second,
                                                 checksum)) {
                            validationErrors++;
                        }
                    }
                });
                TreeStats afterRangeQueries = table->stats();

                ResultAccumulator& accumulator = accumulators[tableIndex];
                accumulator.result.rangeQueryCount = dataset.rangeQueries.size();
                accumulator.result.buildTimeMs += buildTime;
                accumulator.result.successfulSearchTimeMs += successfulSearchTime;
                accumulator.result.unsuccessfulSearchTimeMs += unsuccessfulSearchTime;
                accumulator.result.rangeQueryTimeMs += rangeQueryTime;
                accumulator.result.treeHeight += static_cast<double>(table->height());
                accumulator.result.averageNodeSplits += static_cast<double>(afterBuild.nodeSplits - beforeBuild.nodeSplits);
                accumulator.result.averageBuildNodeVisits += safeDivide(
                    static_cast<double>(afterBuild.nodeVisits - beforeBuild.nodeVisits),
                    static_cast<double>(dataset.insertKeys.size()));
                accumulator.result.averageSuccessfulSearchNodeVisits += safeDivide(
                    static_cast<double>(afterSuccessfulSearch.nodeVisits - beforeSuccessfulSearch.nodeVisits),
                    static_cast<double>(dataset.successfulSearchQueries.size()));
                accumulator.result.averageUnsuccessfulSearchNodeVisits += safeDivide(
                    static_cast<double>(afterUnsuccessfulSearch.nodeVisits - beforeUnsuccessfulSearch.nodeVisits),
                    static_cast<double>(dataset.unsuccessfulSearchQueries.size()));
                accumulator.result.averageRangeQueryNodeVisits += safeDivide(
                    static_cast<double>(afterRangeQueries.nodeVisits - beforeRangeQueries.nodeVisits),
                    static_cast<double>(dataset.rangeQueries.size()));
                accumulator.result.averageRangeResultCount += safeDivide(
                    static_cast<double>(totalRangeResultCount),
                    static_cast<double>(dataset.rangeQueries.size()));
            }
        }

        for (std::size_t tableIndex = 0; tableIndex < accumulators.size(); tableIndex++) {
            BenchmarkResult result = accumulators[tableIndex].result;
            double repeats = static_cast<double>(config.repeats);

            result.buildTimeMs = safeDivide(result.buildTimeMs, repeats);
            result.successfulSearchTimeMs = safeDivide(result.successfulSearchTimeMs, repeats);
            result.unsuccessfulSearchTimeMs = safeDivide(result.unsuccessfulSearchTimeMs, repeats);
            result.rangeQueryTimeMs = safeDivide(result.rangeQueryTimeMs, repeats);
            result.treeHeight = safeDivide(result.treeHeight, repeats);
            result.averageNodeSplits = safeDivide(result.averageNodeSplits, repeats);
            result.averageBuildNodeVisits = safeDivide(result.averageBuildNodeVisits, repeats);
            result.averageSuccessfulSearchNodeVisits = safeDivide(result.averageSuccessfulSearchNodeVisits, repeats);
            result.averageUnsuccessfulSearchNodeVisits = safeDivide(result.averageUnsuccessfulSearchNodeVisits, repeats);
            result.averageRangeQueryNodeVisits = safeDivide(result.averageRangeQueryNodeVisits, repeats);
            result.averageRangeResultCount = safeDivide(result.averageRangeResultCount, repeats);

            results.push_back(result);
        }
    }

    if (validationErrors > 0) {
        throw std::runtime_error("Benchmark validation failed because one or more tree operations returned unexpected results.");
    }

    if (checksum == -1) {
        throw std::runtime_error("Unexpected checksum state.");
    }

    return results;
}

}
