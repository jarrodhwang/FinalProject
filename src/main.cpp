#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "BenchmarkRunner.h"
#include "ResultPrinter.h"

namespace {

std::vector<std::string> splitCommaSeparated(const std::string& text) {
    std::vector<std::string> parts;
    std::stringstream input(text);
    std::string piece;

    while (std::getline(input, piece, ',')) {
        if (!piece.empty()) {
            parts.push_back(piece);
        }
    }

    return parts;
}

std::vector<std::size_t> parseSizeList(const std::string& text) {
    std::vector<std::size_t> sizes;
    std::vector<std::string> parts = splitCommaSeparated(text);

    for (std::size_t index = 0; index < parts.size(); index++) {
        sizes.push_back(static_cast<std::size_t>(std::stoull(parts[index])));
    }

    return sizes;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [--sizes 1000,5000,10000,50000] [--order 8]\n";
    std::cout << "       [--repeats 5] [--csv results/benchmark_results.csv] [--seed 225]\n";
}

bool parseArguments(int argc, char* argv[], finalproject::BenchmarkConfig& config) {
    for (int index = 1; index < argc; index++) {
        std::string argument = argv[index];

        if (argument == "--help") {
            printUsage(argv[0]);
            return false;
        }

        if (index + 1 >= argc) {
            throw std::runtime_error("Missing value for argument: " + argument);
        }

        std::string value = argv[++index];

        if (argument == "--sizes") {
            config.datasetSizes = parseSizeList(value);
        } else if (argument == "--order") {
            config.order = std::stoi(value);
        } else if (argument == "--repeats") {
            config.repeats = std::stoi(value);
        } else if (argument == "--csv") {
            config.csvPath = value;
        } else if (argument == "--seed") {
            config.baseSeed = static_cast<unsigned int>(std::stoul(value));
        } else {
            throw std::runtime_error("Unknown argument: " + argument);
        }
    }

    if (config.datasetSizes.empty()) {
        throw std::runtime_error("At least one dataset size is required.");
    }

    if (config.order < 3) {
        throw std::runtime_error("Tree order must be at least 3.");
    }

    if (config.repeats <= 0) {
        throw std::runtime_error("Repeats must be positive.");
    }

    return true;
}

}

int main(int argc, char* argv[]) {
    finalproject::BenchmarkConfig config = finalproject::makeDefaultConfig();

    try {
        if (!parseArguments(argc, argv, config)) {
            return 0;
        }

        finalproject::printBenchmarkIntro(config);
        std::vector<finalproject::BenchmarkResult> results = finalproject::runBenchmarks(config);
        finalproject::printBenchmarkTables(results, config);
        finalproject::printSummaryCharts(results, config);
        finalproject::writeResultsToCsv(results, config.csvPath);

        std::cout << "\nCSV results written to " << config.csvPath << "\n";
        std::cout << "Interpretation: B+ Tree range queries benefit from linked leaves, which is why B+ Trees are common in database index implementations.\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
