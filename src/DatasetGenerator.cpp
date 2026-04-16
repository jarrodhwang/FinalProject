#include "DatasetGenerator.h"

#include <algorithm>
#include <random>

namespace finalproject {

int makeValueForKey(int key) {
    return key * 5 + 11;
}

BenchmarkDataset generateDataset(std::size_t datasetSize, unsigned int seed) {
    BenchmarkDataset dataset;
    dataset.insertKeys.resize(datasetSize);
    dataset.successfulSearchQueries.resize(datasetSize);
    dataset.unsuccessfulSearchQueries.resize(datasetSize);

    for (std::size_t index = 0; index < datasetSize; index++) {
        dataset.insertKeys[index] = static_cast<int>(index * 3 + 1);
        dataset.unsuccessfulSearchQueries[index] = static_cast<int>(index * 3 + 2);
    }

    std::mt19937 generator(seed);
    std::shuffle(dataset.insertKeys.begin(), dataset.insertKeys.end(), generator);

    dataset.successfulSearchQueries = dataset.insertKeys;
    std::shuffle(dataset.successfulSearchQueries.begin(), dataset.successfulSearchQueries.end(), generator);
    std::shuffle(dataset.unsuccessfulSearchQueries.begin(), dataset.unsuccessfulSearchQueries.end(), generator);

    std::vector<int> sortedKeys = dataset.insertKeys;
    std::sort(sortedKeys.begin(), sortedKeys.end());

    std::size_t queryCount = std::min<std::size_t>(256, std::max<std::size_t>(64, datasetSize / 20));
    std::size_t width = std::max<std::size_t>(16, datasetSize / 200);
    if (!sortedKeys.empty()) {
        width = std::min<std::size_t>(width, sortedKeys.size() - 1);
    }

    std::uniform_int_distribution<std::size_t> startPick(0, sortedKeys.empty() ? 0 : sortedKeys.size() - 1);
    for (std::size_t query = 0; query < queryCount && !sortedKeys.empty(); query++) {
        std::size_t lowIndex = startPick(generator);
        std::size_t highIndex = std::min<std::size_t>(sortedKeys.size() - 1, lowIndex + width);
        dataset.rangeQueries.push_back(std::make_pair(sortedKeys[lowIndex], sortedKeys[highIndex]));
    }

    return dataset;
}

}
