#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"

#define PART_I_MARKER_LENGTH  ((guint)4)
#define PART_II_MARKER_LENGTH ((guint)14)

static gint 
find_marker(const gchar *input, 
            guint        marker_length) 
{
    g_assert(strlen(input) > marker_length);
    for (guint i=marker_length-1; i<strlen(input); i++) {
        guint lhs = i-marker_length+1;
        g_autofree gchar *slice = g_strndup(input+lhs, i-lhs+1);
        gboolean unique = TRUE;
        for(guint j=0; j<marker_length; j++) {
            for(guint k=0; k<marker_length; k++) {
                if (k != j && slice[k] == slice[j]) {
                    unique = FALSE;
                    break;
                }
            }
        }
        if (unique) {
            return i+1;
        }
    }
    return -1;
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

    // Read file

    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    // Part I and II

    gchar *input = lines[0];
    BENCHMARK_START(day6_part1);
    printf("Part I: %d.\n", find_marker(input, PART_I_MARKER_LENGTH));
    BENCHMARK_END(day6_part1);
    BENCHMARK_START(day6_part2);
    printf("Part II: %d.\n", find_marker(input, PART_II_MARKER_LENGTH));
    BENCHMARK_END(day6_part2);

    return 0;
}
