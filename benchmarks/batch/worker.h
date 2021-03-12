#ifndef WORKER_H
#define WORKER_H
#include <unistd.h>

pid_t spawn_worker(const int *values, int id);
#endif
