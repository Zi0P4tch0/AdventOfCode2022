#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"

struct range {
    guint left;
    guint right;
};

inline gboolean 
range_contains_range(const struct range *this_range, const struct range *other_range) 
{
    return other_range->left >= this_range->left && other_range->right <= this_range->right;
}

inline gboolean 
range_overlaps_range(const struct range *this_range, const struct range *other_range) 
{
    // glib needs sets
    for (guint j=this_range->left; j<=this_range->right; j++) {
        for (guint k=other_range->left; k<=other_range->right; k++) {
            if (j == k) {
                return TRUE;
            }
        }
    }
    return FALSE;
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

    BENCHMARK_START(day4);
    
    // Read file
    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    // Part I & II
    guint part1 = 0, part2 = 0;

    for (guint i=0; i<n_lines; i++) {
        const gchar *current_line = *(lines+i);
        g_autostrvfree gchar **tokens = g_strsplit(current_line, ",", 0);
        struct range left = {.left = 0, .right = 0}, right = { .left = 0, .right = 0};
        g_autostrvfree gchar **left_tokens = g_strsplit(tokens[0], "-", 0);
        left.left = g_ascii_strtoull(left_tokens[0], NULL, 10);
        left.right = g_ascii_strtoull(left_tokens[1], NULL, 10);
        g_autostrvfree gchar **right_tokens = g_strsplit(tokens[1], "-", 0);
        right.left = g_ascii_strtoull(right_tokens[0], NULL, 10);
        right.right = g_ascii_strtoull(right_tokens[1], NULL, 10);
        if (range_contains_range(&left, &right) || range_contains_range(&right, &left)) {
            part1++;
        }
        if (range_overlaps_range(&left, &right)) {
            part2++;
        }
    }

    g_print("Part I: %d.\n", part1);
    g_print("Part II: %d.\n", part2);

    BENCHMARK_END(day4);

    return 0;
}