#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

#define PRIORITY(c) (c < 'a' ? c - 'A' + 27 : c - 'a' + 1)

////////////////
// ENTRYPOINT //
///////////////

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        g_print("Usage: %s INPUT_FILE.\n", argv[0]);
        return 1;
    }

    BENCHMARK_START(day3);
    
    // Read file
    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    // Part I
    guint part1 =0;

    for (guint i=0; i<n_lines; i++) {
        guint line_length = strlen(*(lines+i));
        g_autofree gchar *first_comparment = g_strndup(lines[i], line_length/2);
        g_autofree gchar *second_comparment = g_strndup(lines[i] + line_length/2, line_length/2);
        gchar common_item = 0;
        for (guint j=0; j<line_length/2; j++) {
            for (guint k=0; k<line_length/2; k++) {
                if (first_comparment[j] == second_comparment[k]) {
                    common_item = first_comparment[j];
                    break;
                }
            }
        }
        part1 += PRIORITY(common_item);
    }

    g_print("Part I: %d.\n", part1);

    // Part II
    guint part2 =0;

    for (guint i=0; i<n_lines; i+=3) {
        const gchar *first_line = lines[i];
        const gchar *second_line = lines[i+1];
        const gchar *third_line = lines[i+2];
        gchar common_item = 0;
        for (guint a=0; a<strlen(first_line); a++) {
            for (guint b=0; b<strlen(second_line); b++) {
                for (guint c=0; c<strlen(third_line); c++) {
                    if (first_line[a] == second_line[b] && second_line[b] == third_line[c]) {
                        common_item = first_line[a];
                        break;
                    }
                }
            }
        }
        part2 += PRIORITY(common_item);
    }

    g_print("Part II: %d.\n", part2);
 
    BENCHMARK_END(day3);

    return 0;
}
