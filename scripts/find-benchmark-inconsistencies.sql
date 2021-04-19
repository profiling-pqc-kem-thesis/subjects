SELECT
	algorithm.*,
	benchmark.benchmark_type,
	benchmark.stage,
	COUNT(1) as occurances
FROM
	environment
	INNER JOIN benchmark ON benchmark.environment = environment.id
	INNER JOIN algorithm ON algorithm.id = benchmark.algorithm
GROUP BY algorithm.id, benchmark.benchmark_type, benchmark.stage
