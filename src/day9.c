#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

////////////////
// ENTRYPOINT //
///////////////

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        g_print("Usage: %s INPUT_FILE.\n", argv[0]);
        return 1;
    }

    BENCHMARK_START(day9);
    
    // Read file
    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    // Processing
 
    BENCHMARK_END(day9);

    return 0;
}
