function generate_figures()
%GENERATE_FIGURES Create report-ready figures from benchmark_results.csv.

resultsPath = fullfile('..', 'results', 'benchmark_results.csv');
if ~isfile(resultsPath)
    error('Results file not found: %s', resultsPath);
end

outputDir = fullfile('..', 'results', 'figures');
if ~isfolder(outputDir)
    mkdir(outputDir);
end

data = readtable(resultsPath, 'TextType', 'string');
structures = unique(data.structure_name, 'stable');

makeTimeFigure(data, structures, outputDir);
makeVisitFigure(data, structures, outputDir);
makeStructureFigure(data, structures, outputDir);

fprintf('Saved MATLAB figures to %s\n', outputDir);
end

function makeTimeFigure(data, structures, outputDir)
figure('Name', 'Timing Metrics', 'Color', 'w');
tiledlayout(2, 2, 'Padding', 'compact', 'TileSpacing', 'compact');

plotMetric(data, structures, 'build_time_ms', 'Build Time (ms)');
plotMetric(data, structures, 'successful_search_time_ms', 'Successful Search Time (ms)');
plotMetric(data, structures, 'unsuccessful_search_time_ms', 'Unsuccessful Search Time (ms)');
plotMetric(data, structures, 'range_query_time_ms', 'Range Query Time (ms)');

exportgraphics(gcf, fullfile(outputDir, 'timing_metrics.png'));
end

function makeVisitFigure(data, structures, outputDir)
figure('Name', 'Node Visit Metrics', 'Color', 'w');
tiledlayout(2, 2, 'Padding', 'compact', 'TileSpacing', 'compact');

plotMetric(data, structures, 'average_build_node_visits', 'Average Insert Node Visits');
plotMetric(data, structures, 'average_successful_search_node_visits', 'Average Successful Search Node Visits');
plotMetric(data, structures, 'average_unsuccessful_search_node_visits', 'Average Unsuccessful Search Node Visits');
plotMetric(data, structures, 'average_range_query_node_visits', 'Average Range Query Node Visits');

exportgraphics(gcf, fullfile(outputDir, 'node_visit_metrics.png'));
end

function makeStructureFigure(data, structures, outputDir)
figure('Name', 'Structure Metrics', 'Color', 'w');
tiledlayout(1, 3, 'Padding', 'compact', 'TileSpacing', 'compact');

plotMetric(data, structures, 'average_node_splits', 'Average Node Splits');
plotMetric(data, structures, 'tree_height', 'Tree Height');
plotMetric(data, structures, 'average_range_result_count', 'Average Range Result Count');

exportgraphics(gcf, fullfile(outputDir, 'structure_metrics.png'));
end

function plotMetric(data, structures, metricName, chartTitle)
nexttile;
hold on;
for i = 1:numel(structures)
    rows = data(data.structure_name == structures(i), :);
    rows = sortrows(rows, 'dataset_size');
    plot(rows.dataset_size, rows.(metricName), '-o', 'LineWidth', 1.8, 'MarkerSize', 6, ...
         'DisplayName', char(structures(i)));
end
grid on;
xlabel('Dataset Size');
ylabel(strrep(metricName, '_', ' '));
title(chartTitle);
legend('Location', 'northwest');
hold off;
end
