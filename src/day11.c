#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

#define PT1_ROUNDS ((guint)20)
#define PT2_ROUNDS ((guint)10000)

#define WORRY_OP_RHS_SELF ((gint)-1)

struct monkey {
    GArray *items;
    gulong inspections;
    gboolean worry_op_is_sum;
    guint worry_op_rhs;
    guint test;
    guint target_monkey_if_true;
    guint target_monkey_if_false;
};

static inline struct monkey*
monkey_new() 
{
    struct monkey *new_monkey = g_malloc0(sizeof(*new_monkey));
    new_monkey->items = g_array_new(FALSE, TRUE, sizeof(guint));
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
    dst_monkey->worry_op_is_sum = src_monkey->worry_op_is_sum;
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

    return mpz_cmp(lhs->inspections, rhs->inspections) * -1;
}

static void
do_the_monkey_business(GPtrArray *monkeys, guint rounds)
{

    for (guint round=0; round<rounds; round++) {
        
        g_print("Round %u/%u.\n", round, rounds);
        
        for (guint m=0; m<monkeys->len; m++) {

            g_print("Monkey %u/%u.\n", m, monkeys->len);
            
            struct monkey * current_monkey = g_ptr_array_index(monkeys, m);

            for (guint i=0; i<current_monkey->items->len; i++) {

                mpz_add_ui(current_monkey->inspections, current_monkey->inspections, one);

                struct mpz_t_wrapper *current_item_ptr = g_ptr_array_index(current_monkey->items, i);

                if (current_monkey->worry_op_rhs == WORRY_OP_RHS_SELF) {

                    if (current_monkey->worry_op_is_sum) {
                        mpz_add(current_item_ptr->value, current_item_ptr->value, current_item_ptr->value);
                    } else {
                        mpz_t tmp;
                        mpz_init(tmp);
                        mpz_mul(tmp, current_item_ptr->value, current_item_ptr->value);
                        mpz_set(current_item_ptr->value, tmp);
                        mpz_clear(tmp);
                    }

                }  else {
                    
                    mpz_init_set_ui(rhs, current_monkey->worry_op_rhs);
                    
                    if (current_monkey->worry_op_is_sum) {
                        mpz_add(current_item_ptr->value, current_item_ptr->value, rhs);
                    } else {
                        mpz_t tmp;
                        mpz_init(tmp);
                        mpz_mul(tmp, current_item_ptr->value, rhs);
                        mpz_set(current_item_ptr->value, tmp);
                        mpz_clear(tmp);
                    }
    
                }

                mpz_div(current_item_ptr->value, current_item_ptr->value, three);

                struct monkey *next_monkey = NULL;
    
                if (mpz_divisible_ui_p(current_item_ptr->value, current_monkey->test)) {
                    next_monkey = g_ptr_array_index(monkeys, current_monkey->target_monkey_if_true);
                } else {
                    next_monkey = g_ptr_array_index(monkeys, current_monkey->target_monkey_if_false);
                }

                struct mpz_t_wrapper *next_item_ptr = g_malloc0(sizeof(*next_item_ptr));
                mpz_init_set(next_item_ptr->value, current_item_ptr->value);
                g_ptr_array_add(next_monkey->items, next_item_ptr);

            }

            g_ptr_array_free(current_monkey->items, TRUE);
            current_monkey->items = g_ptr_array_new_with_free_func(mpz_t_wrapper_free);

        }

    }

    g_ptr_array_sort(monkeys, g_ptr_array_monkeys_sort_by_inspections);

    mpz_clear(one);
    mpz_clear(three);
    mpz_clear(rhs);

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
                    
                    gint item = GINT_FROM_STR(current_token);
                    
                    struct mpz_t_wrapper *wrapper = g_malloc0(sizeof(*wrapper));
                    mpz_init_set_ui(wrapper->value, item);
                    g_ptr_array_add(current_monkey->items, wrapper);
                }
            } else if (!strcmp("Operation:", tokens[2])) {
                current_monkey->worry_op_is_sum = tokens[6][0] == '+';
                current_monkey->worry_op_rhs = (!strcmp("old", tokens[7]) ? WORRY_OP_RHS_SELF : GUINT_FROM_STR(tokens[7]));
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

    do_the_monkey_business(monkeys, PT1_ROUNDS);
    
    struct monkey *first_monkey = g_ptr_array_index(monkeys, 0);
    struct monkey *second_monkey = g_ptr_array_index(monkeys, 1);

    {
        mpz_t output;
        mpz_init(output);
        mpz_mul(output, first_monkey->inspections, second_monkey->inspections);
        g_autofree gchar *output_str = g_malloc0(mpz_sizeinbase(output, 10) + 2);
        mpz_get_str(output_str, 10, output);
        mpz_clear(output);

        g_print("Part I: %s.\n", output_str);
    }

    // Part II

    do_the_monkey_business(monkeys_copy, PT2_ROUNDS);
    
    first_monkey = g_ptr_array_index(monkeys_copy, 0);
    second_monkey = g_ptr_array_index(monkeys_copy, 1);

    {
        mpz_t output;
        mpz_init(output);
        mpz_mul(output, first_monkey->inspections, second_monkey->inspections);
        g_autofree gchar *output_str = g_malloc0(mpz_sizeinbase(output, 10) + 2);
        mpz_get_str(output_str, 10, output);
        mpz_clear(output);

        g_print("Part II: %s.\n", output_str);
    }

    return 0;
}
