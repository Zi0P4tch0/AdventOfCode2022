#ifndef BENCHMARK_H
#define BENCHMARK_H

#define BENCHMARK_START(id) \
gint64 __benchmark_start_##id = g_get_monotonic_time();

#define BENCHMARK_END(id) \
gint64 __benchmark_duration_##id = g_get_monotonic_time() - __benchmark_start_##id; \
g_print("Elapsed time for '" # id "': %ldus (%.2fms).\n", __benchmark_duration_##id, __benchmark_duration_##id / (double)1000.0);

#endif