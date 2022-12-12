#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

#define PT1_ROUNDS ((guint)20)
#define PT2_ROUNDS ((guint)10000)
#define MONKEY_ITEMS_TYPE gulong

static gulong
worry_op_sum(gulong lhs, gulong rhs)
{
    return lhs + rhs;
}

static gulong
worry_op_mul(gulong lhs, gulong rhs)
{
    return lhs * rhs;
}

static gulong 
worry_op_sq(gulong lhs, gulong rhs)
{
    return lhs * lhs;
}

typedef gulong (*monkey_worry_op)(gulong lhs, gulong rhs);

struct monkey {
    GArray *items;
    gulong inspections;
    monkey_worry_op worry_op;
    guint worry_op_rhs;
    guint test;
    guint target_monkey_if_true;
    guint target_monkey_if_false;
};

static inline struct monkey*
monkey_new() 
{
    struct monkey *new_monkey = g_malloc0(sizeof(*new_monkey));
    new_monkey->items = g_array_new(FALSE, TRUE, sizeof(long long));
    return new_monkey;
}

static void
monkey_free(gpointer data) 
{
    struct monkey *monkey = (struct monkey *)data;
    g_array_free(monkey->items, TRUE);
    g_free(monkey);
}

static inline gpointer
monkey_copy(gconstpointer src, gpointer user_data)
{
    const struct monkey *src_monkey = src;
    struct monkey *dst_monkey = monkey_new();
    dst_monkey->items =g_array_copy(src_monkey->items);
    dst_monkey->inspections = src_monkey->inspections;
    dst_monkey->worry_op = src_monkey->worry_op;
    dst_monkey->worry_op_rhs = src_monkey->worry_op_rhs;
    dst_monkey->test = src_monkey->test;
    dst_monkey->target_monkey_if_true = src_monkey->target_monkey_if_true;
    dst_monkey->target_monkey_if_false = src_monkey->target_monkey_if_false;
    return dst_monkey;
}

static gint 
g_ptr_array_monkeys_sort_by_inspections(gconstpointer a, 
                                        gconstpointer b)
{
    const struct monkey *lhs = *(struct monkey**)a;
    const struct monkey *rhs = *(struct monkey**)b;

    if (lhs->inspections < rhs->inspections) {
        return 1;
    } else if (lhs->inspections > rhs->inspections) {
        return -1;
    } else {
        return 0;
    }
}

static void
do_the_monkey_business(GPtrArray *monkeys, guint rounds, gboolean apply_relief)
{
    guint lcm = 1;

    for (guint m=0; m<monkeys->len; m++) {
        struct monkey * current_monkey = g_ptr_array_index(monkeys, m);
        lcm *= current_monkey->test;
    }

    for (guint round=0; round<rounds; round++) {
                
        for (guint m=0; m<monkeys->len; m++) {
            
            struct monkey * current_monkey = g_ptr_array_index(monkeys, m);

            for (guint i=0; i<current_monkey->items->len; i++) {

                current_monkey->inspections++;

                MONKEY_ITEMS_TYPE current_item = g_array_index(current_monkey->items, MONKEY_ITEMS_TYPE, i);

                current_item = current_monkey->worry_op(current_item, current_monkey->worry_op_rhs);

                if (apply_relief) {
                    current_item /= 3;
                }

                current_item %= lcm;

                struct monkey *next_monkey = NULL;
    
                if (current_item % current_monkey->test == 0) {
                    next_monkey = g_ptr_array_index(monkeys, current_monkey->target_monkey_if_true);
                } else {
                    next_monkey = g_ptr_array_index(monkeys, current_monkey->target_monkey_if_false);
                }

                g_array_append_val(next_monkey->items, current_item);

            }

            g_array_remove_range(current_monkey->items, 0, current_monkey->items->len);

        }

    }

    g_ptr_array_sort(monkeys, g_ptr_array_monkeys_sort_by_inspections);

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

    g_autoptr(GPtrArray) monkeys = g_ptr_array_new_with_free_func(monkey_free);

    struct monkey *current_monkey = NULL;

    for (guint i=0; i<n_lines; i++) {
        if (!strcmp("", lines[i])) {
            continue;
        } else {
            g_autostrvfree gchar **tokens = g_strsplit(lines[i], " ", 0);
            if (!strcmp("Monkey", tokens[0])) {
                if (current_monkey != NULL) {
                    g_ptr_array_add(monkeys, current_monkey);
                }
                current_monkey = monkey_new();
            } else if (!strcmp("Starting", tokens[2])) {
                for (guint j=4; j<g_strv_length(tokens); j++) {
                    
                    gchar *current_token = tokens[j];
                    current_token[strcspn(current_token, ",")] = 0;
                    
                    MONKEY_ITEMS_TYPE item = g_ascii_strtoull(current_token, NULL, 10);
                    
                    g_array_append_val(current_monkey->items, item);
                }
            } else if (!strcmp("Operation:", tokens[2])) {
                if (tokens[6][0] == '+') {
                    current_monkey->worry_op = worry_op_sum;
                    current_monkey->worry_op_rhs = GUINT_FROM_STR(tokens[7]);
                } else if (tokens[6][0] == '*') {
                    current_monkey->worry_op = worry_op_mul;
                    if (!strcmp("old", tokens[7])) {
                        current_monkey->worry_op = worry_op_sq;
                    } else {
                        current_monkey->worry_op_rhs = GUINT_FROM_STR(tokens[7]);
                    }
                }      
            } else if (!strcmp("Test:", tokens[2])) {
                current_monkey->test = GUINT_FROM_STR(tokens[5]);
            } else if (!strcmp("true:", tokens[5])) {
                current_monkey->target_monkey_if_true = GUINT_FROM_STR(tokens[9]);
            } else if (!strcmp("false:", tokens[5])) {
                current_monkey->target_monkey_if_false = GUINT_FROM_STR(tokens[9]);
            }
        }
    }

    g_ptr_array_add(monkeys, current_monkey);
    current_monkey = NULL;

    g_autoptr(GPtrArray) monkeys_copy = g_ptr_array_copy(monkeys, monkey_copy, NULL);

    // Part I

    BENCHMARK_START(day11_part1);

    do_the_monkey_business(monkeys, PT1_ROUNDS, TRUE);

    BENCHMARK_END(day11_part1);

    struct monkey *first_monkey = g_ptr_array_index(monkeys, 0);
    struct monkey *second_monkey = g_ptr_array_index(monkeys, 1);

    g_print("Part I: %lu.\n", first_monkey->inspections * second_monkey->inspections);

    // Part II

    BENCHMARK_START(day11_part2);

    do_the_monkey_business(monkeys_copy, PT2_ROUNDS, FALSE);

    BENCHMARK_END(day11_part2);

    first_monkey = g_ptr_array_index(monkeys_copy, 0);
    second_monkey = g_ptr_array_index(monkeys_copy, 1);

    g_print("Part I: %lu.\n", first_monkey->inspections * second_monkey->inspections);

    return 0;
}
