#ifndef FINALPROJECT_DATASET_GENERATOR_H
#define FINALPROJECT_DATASET_GENERATOR_H

#include <cstddef>

#include "BenchmarkTypes.h"

namespace finalproject {

BenchmarkDataset generateDataset(std::size_t datasetSize, unsigned int seed);

}

#endif
