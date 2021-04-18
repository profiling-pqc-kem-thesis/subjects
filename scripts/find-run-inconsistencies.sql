SELECT
	algorithm.*,
	environment.name,
	SUM(case when benchmark_type = "sequential" then 1 else 0 end) AS "sequential",
	SUM(case when benchmark_type = "parallel" then 1 else 0 end) AS "parallel",
	SUM(case when benchmark_type = "heap" then 1 else 0 end) AS "heap",
	SUM(case when benchmark_type = "stack" then 1 else 0 end) AS "stack",
	SUM(case when benchmark_type = "micro" then 1 else 0 end) AS "micro"
FROM
	algorithm
	INNER JOIN benchmark ON benchmark.algorithm = algorithm.id
	INNER JOIN environment ON environment.id = benchmark.environment
GROUP BY benchmark.algorithm, environment.name
ORDER BY algorithm.name
