function generate_tables()
%GENERATE_TABLES Create CSV summary tables from benchmark_results.csv.

resultsPath = fullfile('..', 'results', 'benchmark_results.csv');
if ~isfile(resultsPath)
    error('Results file not found: %s', resultsPath);
end

outputDir = fullfile('..', 'results');
data = readtable(resultsPath, 'TextType', 'string');
data = sortrows(data, {'dataset_size', 'structure_name'});

writePivotTable(data, 'build_time_ms', fullfile(outputDir, 'build_time_table.csv'));
writePivotTable(data, 'successful_search_time_ms', fullfile(outputDir, 'successful_search_time_table.csv'));
writePivotTable(data, 'unsuccessful_search_time_ms', fullfile(outputDir, 'unsuccessful_search_time_table.csv'));
writePivotTable(data, 'range_query_time_ms', fullfile(outputDir, 'range_query_time_table.csv'));
writePivotTable(data, 'average_range_query_node_visits', fullfile(outputDir, 'range_query_node_visits_table.csv'));

fprintf('Saved MATLAB summary tables to %s\n', outputDir);
end

function writePivotTable(data, valueField, outputPath)
subset = data(:, {'dataset_size', 'structure_name', valueField});
subset.structure_column = makeStructureColumnNames(subset.structure_name);
subset = subset(:, {'dataset_size', 'structure_column', valueField});
pivot = unstack(subset, valueField, 'structure_column');
writetable(pivot, outputPath);
end

function structureColumns = makeStructureColumnNames(structureNames)
structureColumns = replace(structureNames, "B+ Tree", "BPlusTree");
structureColumns = replace(structureColumns, "B-Tree", "BTree");
structureColumns = matlab.lang.makeValidName(structureColumns, 'ReplacementStyle', 'delete');
end
