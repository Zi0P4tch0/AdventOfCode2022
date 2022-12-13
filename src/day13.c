#include <stdio.h>
#include <glib.h>

#include "mpc.h"

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

typedef struct {
    gboolean is_int;
    union {
       gint integer;
       GPtrArray *list;
    };
} list_item;

static void 
list_item_free(gpointer data)
{
    list_item *item = (list_item*)data;
    if (!item->is_int) {
        g_ptr_array_free(item->list, TRUE);
    }
    g_free(item);
}

typedef struct {
    list_item *one;
    list_item *two;
} list_pair;

static void 
list_pair_free(gpointer data)
{
    list_pair *pair = (list_pair *)data;
    list_item_free(pair->one);
    list_item_free(pair->two);
    g_free(pair);
}

static list_item* 
list_item_listify(list_item *item)
{
    if (item->is_int) {

        list_item *self_copy = g_new0(list_item, 1);
        self_copy->is_int = TRUE;
        self_copy->integer = item->integer;

        list_item *list = g_new0(list_item, 1);
        list->is_int = FALSE;
        list->list = g_ptr_array_new_with_free_func(list_item_free);
        g_ptr_array_add(list->list, self_copy);
        return list;
    }

    return NULL;
}

static gchar*
list_item_print(list_item *item)
{
    if (item->is_int) {
        return g_strdup_printf("%d", item->integer);
    } else {
        GString *str = g_string_new("[");
        for (guint i = 0; i < item->list->len; i++) {
            list_item *sub_item = g_ptr_array_index(item->list, i);
            g_autofree gchar *sub_str = list_item_print(sub_item);
            g_string_append_printf(str, "%s", sub_str);
            if (i < item->list->len - 1) {
                g_string_append(str, ", ");
            }
        }
        g_string_append(str, "]");
        return g_string_free(str, FALSE);
    }
}

typedef enum {
    RESULT_FALSE = 0,
    RESULT_TRUE = 1,
    RESULT_UNKNOWN
} result;

static result
list_item_pair_is_valid(list_item *lhs, list_item *rhs)
{
    //g_autofree gchar *lhs_str = list_item_print(lhs);
    //g_autofree gchar *rhs_str = list_item_print(rhs);

    //g_print("Comparing %s to %s...\n", lhs_str, rhs_str);

    if (lhs->is_int && rhs->is_int && lhs->integer < rhs->integer) {
        return RESULT_TRUE;
    } else if (lhs->is_int && rhs->is_int && lhs->integer > rhs->integer) {
        return RESULT_FALSE;
    } 

    if (!lhs->is_int && !rhs->is_int) {
        if (lhs->list->len == 0 && rhs->list->len != 0) {
            return RESULT_TRUE;
        }
        if (lhs->list->len > 0 && rhs->list->len == 0) {
            return RESULT_FALSE;
        }
        guint lhs_index = 0;
        guint rhs_index = 0;
        while (lhs_index < lhs->list->len && rhs_index < rhs->list->len) {
            list_item *lhs_item = g_ptr_array_index(lhs->list, lhs_index);
            list_item *rhs_item = g_ptr_array_index(rhs->list, rhs_index);
            result lhs_rhs_result = list_item_pair_is_valid(lhs_item, rhs_item);
            if (lhs_rhs_result != RESULT_UNKNOWN) {
                return lhs_rhs_result;
            }
            lhs_index++;
            rhs_index++;
            if (lhs_index >= lhs->list->len && rhs_index < rhs->list->len) {
                return RESULT_TRUE;
            } 
            if (lhs_index < lhs->list->len && rhs_index >= rhs->list->len) {
                return RESULT_FALSE;
            } 
        }
        return RESULT_UNKNOWN;
    } 

     if (lhs->is_int && !rhs->is_int) {
        list_item *lhs_as_list = list_item_listify(lhs);
        gboolean result = list_item_pair_is_valid(lhs_as_list, rhs);
        list_item_free(lhs_as_list);
        return result;
    } 
    
    if (!lhs->is_int && rhs->is_int) {
        list_item *rhs_as_list = list_item_listify(rhs);
        gboolean result = list_item_pair_is_valid(lhs, rhs_as_list);
        list_item_free(rhs_as_list);
        return result;
    }

    return RESULT_UNKNOWN;

}

//////////
// GLib //
//////////

static gint 
g_ptr_array_packets_sort(gconstpointer a, 
                         gconstpointer b)
{
    const struct list_item *lhs = *(list_item**)a;
    const struct list_item *rhs = *(list_item**)b;

    if (list_item_pair_is_valid(lhs, rhs) == RESULT_TRUE) {
        return -1;
    } else {
        return 1;
    }
}

/////////
// MPC //
/////////

static mpc_val_t*
int_fold_f(int n, mpc_val_t **xs)
{
    g_autofree gchar *str = g_malloc0(n + 1);
    for (int i = 0; i < n; i++) {
        str[i] = ((gchar**)xs)[i][0];
        free(xs[i]);
    }
    str[n] = '\0';

    list_item *item = g_new0(list_item, 1);
    item->is_int = TRUE;
    item->integer = g_ascii_strtoll(str, NULL, 10);
    return item;
}

static mpc_val_t*
list_fold_f(int n, mpc_val_t **xs)
{
    free(xs[0]);
    free(xs[2]);

    list_item *item = g_new0(list_item, 1);
    item->is_int = FALSE;
    item->list = (GPtrArray *)xs[1];
    return item;
}

static mpc_val_t*
pair_fold_f(int n, mpc_val_t **xs)
{
    free(xs[1]);
    free(xs[3]);

    list_pair *pair = g_new0(list_pair, 1);
    pair->one = xs[0];
    pair->two = xs[2];
    return pair;
}

static mpc_val_t*
input_inner_fold_f(int n, mpc_val_t **xs)
{
    GPtrArray *list = g_ptr_array_new_with_free_func(list_pair_free);
    for (int i = 0; i < n; i++) {
        g_ptr_array_add(list, xs[i]);
    }
    return list;
}

static mpc_val_t*
many_int_or_list_fold_f(int n, mpc_val_t **xs)
{
    GPtrArray *list = g_ptr_array_new_with_free_func(list_item_free);
    for (int i = 0; i < n; i++) {
        g_ptr_array_add(list, xs[i]);
    }
    return list;
}

static void g_ptr_array_dtor(mpc_val_t *val)
{
    g_ptr_array_free(val, TRUE);
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
    
    // MPC

    mpc_parser_t *p_int = mpc_new("int");
    mpc_parser_t *p_list = mpc_new("list");
    mpc_parser_t *p_int_or_list = mpc_new("int_or_list");
    mpc_parser_t *p_many_int_or_list = mpc_new("many_int_or_list");
    mpc_parser_t *p_pair = mpc_new("pair");
    mpc_parser_t *p_input = mpc_new("input");
    
    // int: /[0-9]+/
    mpc_define(p_int, mpc_many1(int_fold_f, mpc_digit()));
    // int_or_list: <int> | <list>
    mpc_define(p_int_or_list, mpc_or(2, p_int, p_list));
    // many_int_or_list: (<int_or_list> ','?)*
    mpc_define(p_many_int_or_list,
        mpc_many(many_int_or_list_fold_f,
            mpc_and(
                2, 
                mpcf_fst_free, 
                p_int_or_list, 
                mpc_maybe(mpc_char(',')), 
                list_item_free, 
                free
            )
        )
    );
    // list: '[' <many_int_or_list> ']'
    mpc_define(p_list, 
        mpc_and(
            3, 
            list_fold_f, 
            mpc_char('['),
            p_many_int_or_list, 
            mpc_char(']'),
            free, 
            g_ptr_array_dtor, 
            free
        )
    );
    // pair: <list> '\n' <list> '\n'?
    mpc_define(p_pair, 
        mpc_and(
            4, 
            pair_fold_f, 
            p_list, 
            mpc_newline(), 
            p_list, 
            mpc_maybe(mpc_newline()), 
            list_item_free, 
            free, 
            list_item_free,
            free
        )
    );
    // input: /^/ (<pair> '\n'?)+ /$/
    mpc_define(p_input, mpc_and(
        3,
        mpcf_snd_free,
        mpc_soi(),
        mpc_many1(
            input_inner_fold_f, 
            mpc_and(
                2, 
                mpcf_fst_free, 
                p_pair, 
                mpc_maybe(mpc_newline()), 
                list_pair_free, 
                free
            )
        ),
        mpc_eoi(),
        free,
        g_ptr_array_dtor,
        free
    ));

    mpc_optimise(p_input);

    // Parse input

    mpc_result_t mpc_res;
    FILE *fp = fopen(argv[1], "r");
    g_autoptr(GPtrArray) pairs = NULL;

    BENCHMARK_START(day13_parsing);

    if (mpc_parse_file(argv[1], fp, p_input, &mpc_res)) {
        pairs = mpc_res.output;
    } else {
        mpc_err_print(mpc_res.error);
        mpc_err_delete(mpc_res.error);
        fclose(fp);
        mpc_cleanup(6, p_int, p_list, p_int_or_list, p_many_int_or_list, p_pair, p_input);
        return 1;
    }

    BENCHMARK_END(day13_parsing);
    
    // Part I
    BENCHMARK_START(day13_part1);

    guint part1 = 0;
    for (guint i = 0; i < pairs->len; i++) {
        list_pair *pair = g_ptr_array_index(pairs, i);
        if (list_item_pair_is_valid(pair->one, pair->two) == RESULT_TRUE) {
            part1 += (i+1);
        }
    }

    BENCHMARK_END(day13_part1);

    g_print("Part I: %u\n", part1);

    // Part II

    BENCHMARK_START(day13_part2);

    // Get all packets
    g_autoptr(GPtrArray) all_packets = g_ptr_array_new();
    for (guint i = 0; i < pairs->len; i++) {
        list_pair *pair = g_ptr_array_index(pairs, i);
        g_ptr_array_add(all_packets, pair->one);
        g_ptr_array_add(all_packets, pair->two);
    }

    // add [[2]]
    const gchar *two_marker = "[[2]]";
    memset(&mpc_res, 0, sizeof(mpc_res));
    mpc_parse("<two_marker>", two_marker, p_list, &mpc_res);
    g_assert(mpc_res.output != NULL);
    list_item *two_marker_item = mpc_res.output;
    g_ptr_array_add(all_packets, two_marker_item);

    // add [[6]]
    const gchar *six_marker = "[[6]]";
    memset(&mpc_res, 0, sizeof(mpc_res));
    mpc_parse("<six_marker>", six_marker, p_list, &mpc_res);
    g_assert(mpc_res.output != NULL);
    list_item *six_marker_item = mpc_res.output;
    g_ptr_array_add(all_packets, six_marker_item);

    // Sort them
    g_ptr_array_sort(all_packets, g_ptr_array_packets_sort);

    guint part2 = 1;
    for (guint i = 0; i < all_packets->len; i++) {
        list_item *packet = g_ptr_array_index(all_packets, i);
        if (packet == two_marker_item || packet == six_marker_item) {
            part2 *= (i+1);
        }
    }

    BENCHMARK_END(day13_part2);

    g_print("Part II: %u.\n", part2);

    // Cleanup
    
    list_item_free(two_marker_item);
    list_item_free(six_marker_item);
    mpc_cleanup(6, p_int, p_list, p_int_or_list, p_many_int_or_list, p_pair, p_input);

    return 0;
}
