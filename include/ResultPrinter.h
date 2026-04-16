#ifndef FINALPROJECT_RESULT_PRINTER_H
#define FINALPROJECT_RESULT_PRINTER_H

#include <string>
#include <vector>

#include "BenchmarkTypes.h"

namespace finalproject {

void printBenchmarkIntro(const BenchmarkConfig& config);
void printBenchmarkTables(const std::vector<BenchmarkResult>& results, const BenchmarkConfig& config);
void printSummaryCharts(const std::vector<BenchmarkResult>& results, const BenchmarkConfig& config);
void writeResultsToCsv(const std::vector<BenchmarkResult>& results, const std::string& csvPath);

}

#endif
