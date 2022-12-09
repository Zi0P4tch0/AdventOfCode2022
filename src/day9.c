#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

#define PT1_TAILS ((guint)1)
#define PT2_TAILS ((guint)9)

struct position {
    gint x;
    gint y;
};

#define POS_IS_EQUAL(lhs, rhs) (lhs.x == rhs.x && lhs.y == rhs.y)

static inline void
knot_next_position(const struct position *leader, struct position *follower) {
    if (ABS(leader->x - follower->x) <= 1 && ABS(leader->y - follower->y) <= 1) { return; }
    gboolean jump = FALSE;
    if (leader->y == follower->y && ABS(leader->x - follower->x) <= 2) {
        x:;
        if (leader->x > follower->x) { 
            follower->x++; 
        }
        else { 
            follower->x--; 
        }
        if (jump) {
            goto y;
        }
    }
    else if (leader->x == follower->x && ABS(leader->y - follower->y) <= 2) {
        y:;
        if (leader->y > follower->y) { 
            follower->y++; 
        }
        else { 
            follower->y--; 
        }
    } else {
        jump = TRUE;
        goto x;
    }
}

static inline gboolean
g_array_contains_position(const GArray *array, const struct position *pos) {
    for (guint i=0; i<array->len; i++) {
        struct position current_pos = g_array_index(array, struct position, i);
        if (POS_IS_EQUAL(current_pos, (*pos))) {
            return TRUE;
        }
    }
    return FALSE;
}

static inline void
draw(const struct position *head_pos, const struct position *tails, gsize n_tails, gint size) {
    for (gint y=-size; y<size; y++) {
        for (gint x=-size; x<size; x++) {
            struct position current_pos = {.x = x, .y = y};
            gchar output = '.';
            if (current_pos.x == 0 && current_pos.y == 0) {
                output = 's';
            }
            for (guint t=0; t<n_tails; t++) {
                const struct position *current_tail = tails + t;
                if (POS_IS_EQUAL(current_pos, (*current_tail))) {
                    output = (n_tails == 1 ? 'T' : '1' + t);
                }
            }
            if (POS_IS_EQUAL(current_pos, (*head_pos))) {
                output = 'H';
            }
            printf("%c", output);
        }
        printf("\n");
    }
}

static guint
process(gchar **lines, guint n_lines, struct position *head, struct position *tails, guint n_tails) 
{
    g_autoptr(GArray) visited = g_array_new(FALSE, TRUE, sizeof(struct position));
    for (guint i=0; i<n_lines; i++) {
        const gchar *current_line = *(lines+i);
        g_autostrvfree gchar **tokens = g_strsplit(current_line, " ", 0);
        guint n_steps = GUINT_FROM_STR(tokens[1]);
        for (guint step=0; step<n_steps; step++) {
            switch (tokens[0][0]) {
                case 'U':
                    head->y++;
                    break;
                case 'L':
                    head->x--;
                    break;
                case 'R':
                    head->x++;
                    break;
                default:
                    head->y--;
                    break;
            }
            for (guint t=0; t<n_tails; t++) {
                const struct position *leader = (t == 0 ? head : tails + (t - 1));
                struct position *follower =  tails + t;
                knot_next_position(leader, follower);
            }

            if (!g_array_contains_position(visited, tails + (n_tails - 1))) {
                g_array_append_val(visited, *(tails + (n_tails - 1)));
            }
        }
    }
    return visited->len;
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

    struct position head = {.x = 0, .y = .0};
    struct position tails[PT2_TAILS];
    memset(tails, 0, sizeof(struct position) * PT2_TAILS);
    
    BENCHMARK_START(day1_part1);

    // Part I

    guint part1 = process(lines, n_lines, &head, tails, PT1_TAILS);

    BENCHMARK_END(day1_part1);

    printf("Part I: %d.\n", part1);

    // Cleanup

    memset(&head, 0, sizeof(struct position));
    memset(tails, 0, sizeof(struct position) * PT2_TAILS);

     // Part II

    BENCHMARK_START(day1_part2);

    guint part2 = process(lines, n_lines, &head, tails, PT2_TAILS);

    BENCHMARK_END(day1_part2);

    printf("Part II: %d.\n", part2);
 
    return 0;
}
