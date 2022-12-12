#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

struct node {
    guint row;
    guint column;
    gchar value;
    gboolean visited;
    struct node *previous;
};

typedef gboolean (*GFilterFunc)(gconstpointer data, gpointer user_data);

static GPtrArray*
g_ptr_array_filter(const GPtrArray *array,
                   GFilterFunc      func,
                   gpointer         user_data)
{
    g_return_val_if_fail(array != NULL, NULL);
    g_return_val_if_fail(func != NULL, NULL);

    GPtrArray *result = g_ptr_array_new();

    for (guint i=0; i<array->len; i++) {
        if (func(g_ptr_array_index(array, i), user_data)) {
            g_ptr_array_add(result, g_ptr_array_index(array, i));
        }
    }

    return result;
}

static gboolean
node_is_reachable_and_not_visited(gconstpointer data, gpointer user_data)
{
    const struct node *node = data;
    const struct node *source_node = user_data;

    if (node->visited) {
        return FALSE;
    }

    gboolean is_in_reach = (
        node->row == source_node->row-1 && node->column == source_node->column ||
        node->row == source_node->row && node->column == source_node->column+1 ||
        node->row == source_node->row+1 && node->column == source_node->column ||
        node->row == source_node->row && node->column == source_node->column-1
    );

    gchar source_value = source_node->value == 'S' ? 'a' : (source_node->value == 'E' ? 'z' : source_node->value);
    gchar node_value = node->value == 'S' ? 'a' : (node->value == 'E' ? 'z' : node->value);

    gboolean can_be_traversed = (
        node_value < source_value  ||
        node_value == source_value ||
        node_value == source_value+1
    );

    return is_in_reach && can_be_traversed;
}

static void 
bfs(GPtrArray *all_nodes, struct node *start)
{

    g_autoptr(GQueue) to_visit = g_queue_new();

    g_queue_push_tail(to_visit, start);
    start->visited = TRUE;

    while (to_visit->length > 0) {
        
        struct node *current_node = g_queue_pop_head(to_visit);

        g_autoptr(GPtrArray) reachable_nodes = g_ptr_array_filter(all_nodes, node_is_reachable_and_not_visited, current_node);

        for (guint i=0; i<reachable_nodes->len; i++) {
            struct node *reachable_node = g_ptr_array_index(reachable_nodes, i);
            reachable_node->previous = current_node;

            g_queue_push_tail(to_visit, reachable_node);
            reachable_node->visited = TRUE;
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

    guint columns = strlen(lines[0]);

    g_autoptr(GPtrArray) nodes = g_ptr_array_new_with_free_func(g_free);
    g_autoptr(GPtrArray) a_nodes = g_ptr_array_new();

    // Find start and end, and create nodes

    struct node *start = NULL;
    struct node *end = NULL;

    for (guint row=0; row<n_lines; row++) {
        for (guint column=0; column<columns; column++) {
            struct node *this_node = g_malloc0(sizeof(struct node));
            this_node->row = row;
            this_node->column = column;
            this_node->value = lines[row][column];
            if (this_node->value == 'a') {
                g_ptr_array_add(a_nodes, this_node);
            } else if (this_node->value == 'S') {
                start = this_node;
            } else if (this_node->value == 'E') {
                end = this_node;
            } 
            g_ptr_array_add(nodes, this_node);
        }
    }

    // Part I

    BENCHMARK_START(day12_part1);

    guint part1 = 0;
    bfs(nodes, start);
    
    struct node *it = end;
    while (it->previous) {
        part1++;
        it = it->previous;
    }

    BENCHMARK_END(day12_part1);
  
    g_print("Part I: %u\n", part1);

    // Part II

     BENCHMARK_START(day12_part2);

    guint part2 = G_MAXUINT;

    for (guint i=0; i<a_nodes->len; i++) {
        // Reset nodes
        for (guint j=0; j<nodes->len; j++) {
            struct node *node = g_ptr_array_index(nodes, j);
            node->previous = NULL;
            node->visited = FALSE;
        }
        struct node *a_node = g_ptr_array_index(a_nodes, i);
        bfs(nodes, a_node);
        struct node *it = end;
        guint steps = 0;
        while (it->previous) {
            steps++;
            it = it->previous;
        }
        if (steps != 0) {
            part2 = MIN(part2, steps);
        }
    }

    BENCHMARK_END(day12_part2);

    g_print("Part II: %u\n", part2);
 
    return 0;
}
