#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

#include <ncurses.h>

typedef struct {
    gint x;
    gint y;
} position;

static void g_list_free_position(gpointer data) {
    g_list_free_full(data, g_free);
}

static inline void 
print_grid(const gchar **grid, guint width, guint height) 
{
    guint screen_width = 0, screen_height = 0;
    getmaxyx(stdscr, screen_height, screen_width);

    for (guint y=0; y<MIN(height, screen_height); y++) {
        for (guint x=0; x<MIN(width, screen_width); x++) {
            move(y, x);
            gchar c = grid[y][x];
            if (c == '.') {
                attron(COLOR_PAIR(1));
                addch(c | A_DIM);
                attroff(COLOR_PAIR(1));
            } else if (c == '#') {
                attron(COLOR_PAIR(3));
                addch(c | A_BOLD);
                attroff(COLOR_PAIR(3));
            } else if (c == '+' || c == 'o') {
                attron(COLOR_PAIR(2));
                addch(c | A_BOLD);
                attroff(COLOR_PAIR(2));
            } 
        }
    }

    refresh();
}

static inline int 
simulate_grid(gchar **grid, gint width, gint height, gint min_x, gint min_y, gboolean part2) 
{
    guint n_sand = 1;
    gint sand_x = 500, sand_y = 0;
    // Set sand
    grid[sand_y-min_y][sand_x-min_x] = '+';
    while (TRUE) {
        print_grid((const gchar **)grid, width, height);
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
            if (part2 && grid[0][500] == 'o') {
                return n_sand - 1;
            }
            sand_x = 500;
            sand_y = 0;
        }
        g_usleep(5000);
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

    // Ncurses setup
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    clear();

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

    guint width = max_x - min_x + 1;
    guint height = max_y - min_y + 1;

    // Allocate char grid

    gchar ** grid = g_malloc0_n(height, sizeof(gchar*));
    for (guint i=0; i<height; i++) {
        grid[i] = g_malloc0_n(width, sizeof(gchar));
        memset(grid[i], '.', width);
    }

    // Draw rocks
    for (guint i=0; i<paths->len; i++) {
        GList *list = g_ptr_array_index(paths, i);
        for (GList *l=list; l; l=l->next) {
            position *pos = l->data;
            grid[pos->y-min_y][pos->x-min_x] = '#';
            // Get previous position if any
            if (l->prev) {
                position *prev = l->prev->data;
                if (prev->x == pos->x) {
                    // Vertical
                    if (prev->y < pos->y) {
                        for (guint y=prev->y+1; y<pos->y; y++) {
                            grid[y-min_y][pos->x-min_x] = '#';
                        }
                    } else {
                        for (guint y=pos->y+1; y<prev->y; y++) {
                            grid[y-min_y][pos->x-min_x] = '#';
                        }
                    }
                } else {
                    // Horizontal
                    if (prev->x < pos->x) {
                        for (guint x=prev->x+1; x<pos->x; x++) {
                            grid[pos->y-min_y][x-min_x] = '#';
                        }
                    } else {
                        for (guint x=pos->x+1; x<prev->x; x++) {
                            grid[pos->y-min_y][x-min_x] = '#';
                        }
                    }
                }
            }
        }
    }

    // Part I
    guint part1 = simulate_grid(grid, width, height, min_x, min_y, FALSE);

    endwin();

    g_print("Part I: %d\n", part1);

    // Free grids
    for (guint i=0; i<height; i++) {
        g_free(grid[i]);
    }
    g_free(grid);
 
    return 0;
}
