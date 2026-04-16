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
style = lightPlotStyle();

makeTimeFigure(data, structures, outputDir, style);
makeVisitFigure(data, structures, outputDir, style);
makeStructureFigure(data, structures, outputDir, style);

fprintf('Saved MATLAB figures to %s\n', outputDir);
end

function makeTimeFigure(data, structures, outputDir, style)
figure('Name', 'Timing Metrics', 'Color', style.figureColor);
tiledlayout(2, 2, 'Padding', 'compact', 'TileSpacing', 'compact');

plotMetric(data, structures, 'build_time_ms', 'Build Time (ms)', style);
plotMetric(data, structures, 'successful_search_time_ms', 'Successful Search Time (ms)', style);
plotMetric(data, structures, 'unsuccessful_search_time_ms', 'Unsuccessful Search Time (ms)', style);
plotMetric(data, structures, 'range_query_time_ms', 'Range Query Time (ms)', style);

exportgraphics(gcf, fullfile(outputDir, 'timing_metrics.png'));
end

function makeVisitFigure(data, structures, outputDir, style)
figure('Name', 'Node Visit Metrics', 'Color', style.figureColor);
tiledlayout(2, 2, 'Padding', 'compact', 'TileSpacing', 'compact');

plotMetric(data, structures, 'average_build_node_visits', 'Average Insert Node Visits', style);
plotMetric(data, structures, 'average_successful_search_node_visits', 'Average Successful Search Node Visits', style);
plotMetric(data, structures, 'average_unsuccessful_search_node_visits', 'Average Unsuccessful Search Node Visits', style);
plotMetric(data, structures, 'average_range_query_node_visits', 'Average Range Query Node Visits', style);

exportgraphics(gcf, fullfile(outputDir, 'node_visit_metrics.png'));
end

function makeStructureFigure(data, structures, outputDir, style)
figure('Name', 'Structure Metrics', 'Color', style.figureColor);
tiledlayout(1, 3, 'Padding', 'compact', 'TileSpacing', 'compact');

plotMetric(data, structures, 'average_node_splits', 'Average Node Splits', style);
plotMetric(data, structures, 'tree_height', 'Tree Height', style);
plotMetric(data, structures, 'average_range_result_count', 'Average Range Result Count', style);

exportgraphics(gcf, fullfile(outputDir, 'structure_metrics.png'));
end

function plotMetric(data, structures, metricName, chartTitle, style)
nexttile;
ax = gca;
styleAxes(ax, style);
hold on;
seriesData = cell(numel(structures), 1);
for i = 1:numel(structures)
    rows = data(data.structure_name == structures(i), :);
    rows = sortrows(rows, 'dataset_size');
    seriesData{i}.x = rows.dataset_size;
    seriesData{i}.y = rows.(metricName);
end

hasOverlap = numel(seriesData) == 2 ...
    && isequal(seriesData{1}.x, seriesData{2}.x) ...
    && isequal(seriesData{1}.y, seriesData{2}.y);

for i = 1:numel(structures)
    lineStyle = '-';
    marker = 'o';
    markerFaceColor = style.lineColors(i, :);
    lineWidth = 1.8;

    if hasOverlap
        if i == 1
            lineStyle = '--';
            marker = 's';
            markerFaceColor = style.axesColor;
            lineWidth = 2.2;
        else
            lineWidth = 1.6;
        end
    end

    plot(seriesData{i}.x, seriesData{i}.y, 'LineStyle', lineStyle, 'Marker', marker, ...
         'LineWidth', lineWidth, 'MarkerSize', 6, 'Color', style.lineColors(i, :), ...
         'MarkerFaceColor', markerFaceColor, 'DisplayName', char(structures(i)));
end
grid(ax, 'on');
xlabel('Dataset Size', 'Color', style.textColor);
ylabel(strrep(metricName, '_', ' '), 'Color', style.textColor);
title(chartTitle, 'Color', style.textColor);
legend('Location', 'northwest', 'TextColor', style.textColor, ...
    'Color', style.axesColor, 'EdgeColor', style.gridColor);
hold off;
end

function styleAxes(ax, style)
set(ax, 'Color', style.axesColor, ...
    'XColor', style.textColor, ...
    'YColor', style.textColor, ...
    'GridColor', style.gridColor, ...
    'MinorGridColor', style.gridColor, ...
    'GridAlpha', 0.35, ...
    'MinorGridAlpha', 0.2, ...
    'Box', 'on', ...
    'Layer', 'top', ...
    'FontName', 'Helvetica');
if isprop(ax, 'Toolbar')
    ax.Toolbar.Visible = 'off';
end
end

function style = lightPlotStyle()
style.figureColor = [1.0, 1.0, 1.0];
style.axesColor = [1.0, 1.0, 1.0];
style.textColor = [0.0, 0.0, 0.0];
style.gridColor = [0.8, 0.8, 0.8];
style.lineColors = [
    0.00, 0.45, 0.74
    0.85, 0.33, 0.10
];
end
