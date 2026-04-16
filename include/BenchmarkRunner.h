#ifndef FINALPROJECT_BENCHMARK_RUNNER_H
#define FINALPROJECT_BENCHMARK_RUNNER_H

#include <vector>

#include "BenchmarkTypes.h"

namespace finalproject {

BenchmarkConfig makeDefaultConfig();
std::vector<BenchmarkResult> runBenchmarks(const BenchmarkConfig& config);

}

#endif
