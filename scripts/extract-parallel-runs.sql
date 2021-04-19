SELECT
	algorithm.*,
	benchmark.stage,
	parallelBenchmark.numberOfThreads,
	parallelBenchmark.averageDurationPerIteration,
	parallelBenchmark.throughput,
	parallelBenchmark.totalIterations,
	parallelBenchmark.averageIterationsPerThread
FROM
	benchmark
	INNER JOIN algorithm ON algorithm.id = benchmark.algorithm
	INNER JOIN parallelBenchmark ON parallelBenchmark.benchmark = benchmark.id
ORDER BY benchmark.stage, numberOfThreads
