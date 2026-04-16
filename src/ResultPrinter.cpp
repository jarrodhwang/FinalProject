#include "ResultPrinter.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace finalproject {
namespace {

std::string formatNumber(double value, int precision = 3) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

void printBarChart(const std::vector<BenchmarkResult>& rows,
                   const std::string& title,
                   double BenchmarkResult::*metric) {
    std::cout << "\n" << title << "\n";

    double maxValue = 0.0;
    for (std::size_t index = 0; index < rows.size(); index++) {
        maxValue = std::max(maxValue, rows[index].*metric);
    }

    const int maxBarWidth = 36;
    for (std::size_t index = 0; index < rows.size(); index++) {
        double value = rows[index].*metric;
        int width = 0;

        if (maxValue > 0.0) {
            width = static_cast<int>((value / maxValue) * maxBarWidth);
        }
        if (value > 0.0 && width == 0) {
            width = 1;
        }

        std::cout << std::left << std::setw(12) << rows[index].structureName << " | "
                  << std::string(width, '#')
                  << std::string(maxBarWidth - width, ' ')
                  << " " << formatNumber(value) << " ms\n";
    }
}

}

void printBenchmarkIntro(const BenchmarkConfig& config) {
    std::cout << "CMPT 225 Final Project: Database Indexing with B-Tree and B+ Tree\n";
    std::cout << "Goal: compare search, insertion, and range query behavior for database-style workloads.\n";
    std::cout << "Configured tree order: " << config.order << "\n";
    std::cout << "Dataset sizes: ";

    for (std::size_t index = 0; index < config.datasetSizes.size(); index++) {
        if (index > 0) {
            std::cout << ", ";
        }
        std::cout << config.datasetSizes[index];
    }

    std::cout << "\nRepeats per dataset: " << config.repeats << "\n";
    std::cout << "CSV export path: " << config.csvPath << "\n";
}

void printBenchmarkTables(const std::vector<BenchmarkResult>& results, const BenchmarkConfig& config) {
    for (std::size_t sizeIndex = 0; sizeIndex < config.datasetSizes.size(); sizeIndex++) {
        std::size_t datasetSize = config.datasetSizes[sizeIndex];
        std::vector<BenchmarkResult> rows;

        for (std::size_t index = 0; index < results.size(); index++) {
            if (results[index].datasetSize == datasetSize) {
                rows.push_back(results[index]);
            }
        }

        std::cout << "\n===============================================================\n";
        std::cout << "Dataset Size = " << datasetSize;
        if (!rows.empty()) {
            std::cout << " | Range Queries = " << rows[0].rangeQueryCount;
        }
        std::cout << "\n===============================================================\n";

        std::cout << std::left << std::setw(12) << "Structure"
                  << std::right << std::setw(10) << "Build"
                  << std::setw(10) << "Search+"
                  << std::setw(10) << "Search-"
                  << std::setw(10) << "Range"
                  << std::setw(8) << "Height"
                  << std::setw(9) << "Splits"
                  << std::setw(10) << "InsVis"
                  << std::setw(10) << "SucVis"
                  << std::setw(10) << "FailVis"
                  << std::setw(10) << "RngVis"
                  << std::setw(10) << "RngRows"
                  << "\n";
        std::cout << std::string(119, '-') << "\n";

        for (std::size_t index = 0; index < rows.size(); index++) {
            const BenchmarkResult& row = rows[index];
            std::cout << std::left << std::setw(12) << row.structureName
                      << std::right << std::setw(10) << formatNumber(row.buildTimeMs)
                      << std::setw(10) << formatNumber(row.successfulSearchTimeMs)
                      << std::setw(10) << formatNumber(row.unsuccessfulSearchTimeMs)
                      << std::setw(10) << formatNumber(row.rangeQueryTimeMs)
                      << std::setw(8) << formatNumber(row.treeHeight, 1)
                      << std::setw(9) << formatNumber(row.averageNodeSplits, 1)
                      << std::setw(10) << formatNumber(row.averageBuildNodeVisits, 2)
                      << std::setw(10) << formatNumber(row.averageSuccessfulSearchNodeVisits, 2)
                      << std::setw(10) << formatNumber(row.averageUnsuccessfulSearchNodeVisits, 2)
                      << std::setw(10) << formatNumber(row.averageRangeQueryNodeVisits, 2)
                      << std::setw(10) << formatNumber(row.averageRangeResultCount, 1)
                      << "\n";
        }
    }
}

void printSummaryCharts(const std::vector<BenchmarkResult>& results, const BenchmarkConfig& config) {
    if (config.datasetSizes.empty()) {
        return;
    }

    std::size_t focusDataset = *std::max_element(config.datasetSizes.begin(), config.datasetSizes.end());
    std::vector<BenchmarkResult> rows;

    for (std::size_t index = 0; index < results.size(); index++) {
        if (results[index].datasetSize == focusDataset) {
            rows.push_back(results[index]);
        }
    }

    if (rows.empty()) {
        return;
    }

    std::cout << "\nSummary charts focus on the largest dataset size (" << focusDataset << ").\n";
    printBarChart(rows, "Build Time Chart", &BenchmarkResult::buildTimeMs);
    printBarChart(rows, "Successful Search Time Chart", &BenchmarkResult::successfulSearchTimeMs);
    printBarChart(rows, "Range Query Time Chart", &BenchmarkResult::rangeQueryTimeMs);
}

void writeResultsToCsv(const std::vector<BenchmarkResult>& results, const std::string& csvPath) {
    std::filesystem::path outputPath(csvPath);
    if (outputPath.has_parent_path()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }

    std::ofstream out(csvPath.c_str());
    out << "structure_name,dataset_size,order,repeats,range_query_count,build_time_ms,"
           "successful_search_time_ms,unsuccessful_search_time_ms,range_query_time_ms,tree_height,"
           "average_node_splits,average_build_node_visits,average_successful_search_node_visits,"
           "average_unsuccessful_search_node_visits,average_range_query_node_visits,average_range_result_count\n";

    for (std::size_t index = 0; index < results.size(); index++) {
        const BenchmarkResult& row = results[index];
        out << row.structureName << ","
            << row.datasetSize << ","
            << row.order << ","
            << row.repeats << ","
            << row.rangeQueryCount << ","
            << formatNumber(row.buildTimeMs) << ","
            << formatNumber(row.successfulSearchTimeMs) << ","
            << formatNumber(row.unsuccessfulSearchTimeMs) << ","
            << formatNumber(row.rangeQueryTimeMs) << ","
            << formatNumber(row.treeHeight, 4) << ","
            << formatNumber(row.averageNodeSplits, 4) << ","
            << formatNumber(row.averageBuildNodeVisits, 4) << ","
            << formatNumber(row.averageSuccessfulSearchNodeVisits, 4) << ","
            << formatNumber(row.averageUnsuccessfulSearchNodeVisits, 4) << ","
            << formatNumber(row.averageRangeQueryNodeVisits, 4) << ","
            << formatNumber(row.averageRangeResultCount, 4) << "\n";
    }
}

}
