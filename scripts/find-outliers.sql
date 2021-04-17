SELECT
	algorithm.*,
	benchmark.stage,
	microBenchmarkMeasurement.region,
	microBenchmarkEvent.event,
	SUM(case when microBenchmarkEvent.value = -1 then 1 else 0 end) AS "outlier_underflow",
	SUM(case when microBenchmarkEvent.value = -2 then 1 else 0 end) AS "outlier_overflow",
	SUM(case when microBenchmarkEvent.value = -3 then 1 else 0 end) AS "outlier_zero",
	MIN(case when microBenchmarkEvent.value >= 0 then microBenchmarkEvent.value else 9223372036854775807 end) AS "min",
	MAX(case when microBenchmarkEvent.value >= 0 then microBenchmarkEvent.value else 0 end) AS "max",
	SUM(case when microBenchmarkEvent.value >= 0 then 1 else 0 end) AS "values"
FROM
	algorithm
	INNER JOIN benchmark ON benchmark.algorithm = algorithm.id
	INNER JOIN microBenchmark ON microBenchmark.benchmark = benchmark.id
	INNER JOIN microBenchmarkMeasurement ON microBenchmarkMeasurement.microBenchmark = microBenchmark.id
	INNER JOIN microBenchmarkEvent ON microBenchmarkEvent.microBenchmarkMeasurement = microBenchmarkMeasurement.id
GROUP BY
	algorithm.id, microBenchmarkMeasurement.region, microBenchmarkEvent.event
