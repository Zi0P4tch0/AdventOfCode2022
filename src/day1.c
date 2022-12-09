#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

static gint 
g_array_sort_guint_desc(gconstpointer a, 
                        gconstpointer b)
{
    const guint lhs = *(guint*)a;
    const guint rhs = *(guint*)b;

    if (lhs == rhs) return 0;
    if (lhs > rhs) {
        return -1;
    } else {
        return 1;
    }
}

////////////////
// ENTRYPOINT //
///////////////

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        g_print("Usage: %s INPUT_FILE.\n", argv[0]);
        return 1;
    }

    BENCHMARK_START(day1);
    
    // Read file
    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    // Processing

    g_autoptr(GArray) elves = g_array_new(FALSE, TRUE, sizeof(guint));
    guint tmp = 0;

    for (int i=0; i<n_lines; i++) {
        const gchar *line = lines[i];
        if (!strcmp(line, "")) {
            g_array_append_val(elves, tmp);
            tmp = 0;
        } else {
            tmp += GUINT_FROM_STR(line);
        }
    }
    g_array_append_val(elves, tmp);

    g_array_sort(elves, g_array_sort_guint_desc);

    // Part I

    guint part1 = g_array_index(elves, guint, 0);

    g_print("Part I: %d.\n", part1);

    // Part II

    guint part2 = g_array_index(elves, guint, 0) + g_array_index(elves, guint, 1) + g_array_index(elves, guint, 2);

    g_print("Part II: %d.\n", part2);
 
    BENCHMARK_END(day1);

    return 0;
}
