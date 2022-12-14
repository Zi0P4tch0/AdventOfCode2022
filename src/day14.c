#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

typedef struct {
    gint x;
    gint y;
} position;

static void g_list_free_position(gpointer data) {
    g_list_free_full(data, g_free);
}

static inline int 
simulate_grid(gchar **grid, gint width, gint height, gint min_x, gint min_y, gboolean part2) 
{
    guint n_sand = 1;
    gint sand_x = 500, sand_y = 0;
    // Set sand
    grid[sand_y-min_y][sand_x-min_x] = '+';
    while (TRUE) {
        if (grid[sand_y-min_y][sand_x-min_x] == 'o') {
            return n_sand - 1;
        }
        if (sand_y+1 >= height+min_y) {
            return n_sand - 1;
        }
        if (grid[sand_y+1-min_y][sand_x-min_x] == '.') {
            sand_y++;
            grid[sand_y+-min_y][sand_x-min_x] = '+';
            grid[sand_y-min_y-1][sand_x-min_x] = '.';
        } else if (grid[sand_y+1-min_y][sand_x-1-min_x] == '.') {
            sand_x--;
            sand_y++;
            grid[sand_y-min_y][sand_x-min_x] = '+';
            grid[sand_y-min_y-1][sand_x-min_x+1] = '.';
        } else if (grid[sand_y+1-min_y][sand_x+1-min_x] == '.') {
            sand_x++;
            sand_y++;
            grid[sand_y-min_y][sand_x-min_x] = '+';
            grid[sand_y-min_y-1][sand_x-min_x-1] = '.';
        } else {
            if (!part2 && grid[sand_y+1-min_y][sand_x-min_x] == '#') {
               gint left_pos_x = sand_x-min_x-1, left_pos_y = sand_y+1-min_y;
               if (left_pos_x < 0) {
                    return n_sand - 1;
               }
               gint right_pos_x = sand_x-min_x+1, right_pos_y = sand_y+1-min_y;
                if (right_pos_x >= width) {
                      return n_sand - 1;
                }
            } 
            // Settle down
            grid[sand_y-min_y][sand_x-min_x] = 'o';
            n_sand++;
            sand_x = 500;
            sand_y = 0;
        }
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
    
    // Read file
    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    // Processing

    g_autoptr(GPtrArray) paths = g_ptr_array_new_with_free_func(g_list_free_position);

    for (guint i=0; i<n_lines; i++) {
        const gchar *line = lines[i];
        g_autostrvfree gchar **tokens = g_strsplit(line, "->", 0);
        GList *list = NULL;
        for (guint t=0; t<g_strv_length(tokens); t++) {
            const gchar *token = g_strstrip(tokens[t]);
            g_autostrvfree gchar **coordinates = g_strsplit(token, ",", 0);
            position *pos = g_new(position, 1);
            pos->x = GINT_FROM_STR(coordinates[0]);
            pos->y = GINT_FROM_STR(coordinates[1]);
            list = g_list_append(list, pos);
        }
        g_ptr_array_add(paths, list);
    }

    // Calculate grid size

    guint min_x = 500, max_x = 0, min_y = 0, max_y = 0;

    for (guint i=0; i<paths->len; i++) {
        GList *list = g_ptr_array_index(paths, i);
        for (GList *l=list; l; l=l->next) {
            position *pos = l->data;
            if (pos->x < min_x) min_x = pos->x;
            if (pos->x > max_x) max_x = pos->x;
            if (pos->y > max_y) max_y = pos->y;
        }
    }

    guint height = max_y - min_y + 1;
    guint width = 3 * height;

    // Allocate char grid

    gchar ** grid = g_malloc0_n(height, sizeof(gchar*));
    for (guint i=0; i<height; i++) {
        grid[i] = g_malloc0_n(width, sizeof(gchar));
        memset(grid[i], '.', width);
    }

    // Draw rocks
    gint x_shift = width / 3 - (max_x - min_x) / 2;

    for (guint i=0; i<paths->len; i++) {
        GList *list = g_ptr_array_index(paths, i);
        for (GList *l=list; l; l=l->next) {
            position *pos = l->data;
            grid[pos->y-min_y][pos->x-min_x+x_shift] = '#';
            // Get previous position if any
            if (l->prev) {
                position *prev = l->prev->data;
                if (prev->x == pos->x) {
                    // Vertical
                    if (prev->y < pos->y) {
                        for (guint y=prev->y+1; y<pos->y; y++) {
                            grid[y-min_y][pos->x-min_x+x_shift] = '#';
                        }
                    } else {
                        for (guint y=pos->y+1; y<prev->y; y++) {
                            grid[y-min_y][pos->x-min_x+x_shift] = '#';
                        }
                    }
                } else {
                    // Horizontal
                    if (prev->x < pos->x) {
                        for (guint x=prev->x+1; x<pos->x; x++) {
                            grid[pos->y-min_y][x-min_x+x_shift] = '#';
                        }
                    } else {
                        for (guint x=pos->x+1; x<prev->x; x++) {
                            grid[pos->y-min_y][x-min_x+x_shift] = '#';
                        }
                    }
                }
            }
        }
    }

    // Make a copy for part II

    gchar ** grid2 = g_malloc0_n(height+2, sizeof(gchar*));
    for (guint i=0; i<height+2; i++) {
        grid2[i] = g_malloc0_n(width, sizeof(gchar));
        if (i ==height) {
            memset(grid2[i], '.', width); // Air
        } else if (i==height+1) {
            memset(grid2[i], '#', width); // Floor
        } else {
            memcpy(grid2[i], grid[i], width);
        }
    }

    // Part I
    BENCHMARK_START(day14_part1);

    guint part1 = simulate_grid(grid, width, height, min_x - x_shift, min_y, FALSE);

    BENCHMARK_END(day14_part1);

    // Part II

    BENCHMARK_START(day14_part2);

    guint part2 = simulate_grid(grid2, width, height+2, min_x - x_shift, min_y, TRUE);

    BENCHMARK_END(day14_part2);

    g_print("Part I: %d\n", part1);
    g_print("Part II: %d\n", part2);

    // Free grids
    for (guint i=0; i<height; i++) {
        g_free(grid[i]);
    }
    g_free(grid);

    for (guint i=0; i<height+2; i++) {
        g_free(grid2[i]);
    }
    g_free(grid2);
 
    return 0;
}
