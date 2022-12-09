#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"

#define FILESYSTEM_NAME_MAX_SIZE 32
#define FILESYSTEM_MAX_SPACE ((guint)70000000)
#define FILESYSTEM_MIN_SPACE_FOR_UPDATE ((guint)30000000)

struct filesystem_node_data {
    gchar name[FILESYSTEM_NAME_MAX_SIZE+1];
    gboolean is_dir;    
    guint size;    
};

inline static struct filesystem_node_data*
filesystem_node_new (gchar   *name, 
                     gboolean is_dir, 
                     guint    size) 
{
    struct filesystem_node_data *new = g_malloc0(sizeof(struct filesystem_node_data));
    strcpy(new->name, name);
    new->is_dir = is_dir;
    new->size = (is_dir ? 0 : size);
    return new;
}

static gchar*
filesystem_print(GNode *node, 
                 guint  indent_level) 
{
    struct filesystem_node_data *data = node->data;
    gchar *desc = NULL;;
    if (data->is_dir) {
        desc = g_strdup_printf("%*s- %s (dir)\n", indent_level, "", data->name);
    } else {
        desc = g_strdup_printf("%*s- %s (file, size=%d)\n", indent_level, "", data->name, data->size);
    }
    for (guint i=0; i<g_node_n_children(node); i++) {
        GNode *child = g_node_nth_child(node, i);
        g_autofree gchar *child_desc = filesystem_print(child, indent_level+2);
        gchar *new_desc = g_strdup_printf("%s%s", desc, child_desc);
        g_free(desc);
        desc = new_desc;
    }
    return desc;
}

static guint 
filesystem_node_total_size(GNode *node) 
{
    struct filesystem_node_data *node_data = node->data;
    if (node_data->is_dir == FALSE) {
        return node_data->size;
    } else {
        guint children_size = 0;
        for (guint i=0; i<g_node_n_children(node); i++) {
            GNode *child = g_node_nth_child(node, i);
            children_size += filesystem_node_total_size(child);
        }
        return children_size;
    }
}

static gboolean 
g_node_traverse_fn_part1(GNode   *node, 
                         gpointer data) 
{
    g_assert(data != NULL);
    guint *part1 = (guint *)data;
    struct filesystem_node_data *node_data = node->data;
    if (node_data->is_dir) {
        guint total_size = filesystem_node_total_size(node);
        if (total_size <= 100000) {
            *part1 += total_size;
        }
    }
    return FALSE;
}

static gboolean 
g_node_traverse_fn_dirs(GNode   *node, 
                        gpointer data) 
{
    g_assert(data != NULL);
    GPtrArray **result = (GPtrArray **)data;
    if (*result == NULL) {
        *result = g_ptr_array_new();
    }
    struct filesystem_node_data *node_data = node->data;
    if (node_data->is_dir && node->parent != NULL) {
        g_ptr_array_add(*result, node);
    }
    return FALSE;
}

static gint 
g_ptr_array_sort_dirs_by_size_asc(gconstpointer a, 
                                  gconstpointer b)
{
    const GNode* lhs = *(GNode**)a;
    const GNode* rhs = *(GNode**)b;

    guint lhs_total_size = filesystem_node_total_size((GNode*)lhs);
    guint rhs_total_size = filesystem_node_total_size((GNode*)rhs);

    if (lhs_total_size == rhs_total_size) {
        return 0;
    } else if (lhs_total_size < rhs_total_size) {
        return -1;
    } else {
        return 1;
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

    // Parse file

    g_autoptr(GNode) root_node = NULL;
    GNode *current_node = NULL;
    g_autoptr(GPtrArray) tracked_filesystem_entries = g_ptr_array_new_with_free_func(g_free);

    for(guint i=0; i<n_lines; i++) {
        const gchar *current_line = *(lines + i);
        g_autostrvfree gchar **tokens = g_strsplit(current_line, " ", 0);
        if (!(strcmp("ls", tokens[1]))) { 
            continue; 
        }
        if (!(strcmp("cd", tokens[1]))) {
            if (!strcmp("..", tokens[2])) {
                current_node = current_node->parent;
            } else {
                if (!root_node) {

                    // cd "/"
                    g_assert(!strcmp("/", tokens[2]));

                    struct filesystem_node_data *data = filesystem_node_new("/", TRUE, 0);
                    g_ptr_array_add(tracked_filesystem_entries, data);
                    root_node = g_node_new(data);
                    current_node = root_node;

                } else {
                    
                    // Cd another directory
                    // The filesystem should exist already, because of ls outputs.

                    GNode *existing_node = NULL;
                    for (guint i=0; i<g_node_n_children(current_node); i++) {
                        GNode *child = g_node_nth_child(current_node, i);
                        struct filesystem_node_data *child_data = (struct filesystem_node_data *)child->data;
                        if (!strcmp(child_data->name, tokens[2])) {
                            existing_node = child;
                            break;
                        }
                    }

                    g_assert(existing_node != NULL);
                    current_node = existing_node;
  
                }
            }
        }
        if (!(strcmp("dir", tokens[0]))) {
            struct filesystem_node_data *data = filesystem_node_new(tokens[1], TRUE, 0);
            g_ptr_array_add(tracked_filesystem_entries, data);
            g_node_append_data(current_node, data);
        }
        guint file_size = (guint)g_ascii_strtoull(tokens[0], NULL, 10);
        if (file_size) {
            struct filesystem_node_data *data = filesystem_node_new(tokens[1], FALSE, file_size);
            g_ptr_array_add(tracked_filesystem_entries, data);
            g_node_append_data(current_node, data);
        }
    }

    // Print layout

    //g_autofree gchar *t = filesystem_print(root_node, 0);
    //printf("%s", t);

    // Part I
    BENCHMARK_START(day7_part1);

    guint part1 = 0;
    g_node_traverse(root_node, G_PRE_ORDER, G_TRAVERSE_ALL, -1, g_node_traverse_fn_part1, &part1);
    
    BENCHMARK_END(day7_part1);

    g_print("Part I: %d.\n", part1);

    // Part II
    BENCHMARK_START(day7_part2);

    // Get directories
    g_autoptr(GPtrArray) dir_nodes = NULL;
    g_node_traverse(root_node, G_PRE_ORDER, G_TRAVERSE_ALL, -1, g_node_traverse_fn_dirs, &dir_nodes);
    
    // Sort them from the smallest to the largest
    g_ptr_array_sort(dir_nodes, g_ptr_array_sort_dirs_by_size_asc); 

    // Select the directory that - when removed - frees enough space to perform an update.
    guint root_size = filesystem_node_total_size(root_node);

    for(guint i=0; i<dir_nodes->len; i++) {
        const GNode *current_dir_node = g_ptr_array_index(dir_nodes, i);
        guint current_dir_size = filesystem_node_total_size((GNode*)current_dir_node);
        if (FILESYSTEM_MAX_SPACE - root_size + current_dir_size >= FILESYSTEM_MIN_SPACE_FOR_UPDATE) {
            BENCHMARK_END(day7_part2);
            g_print("Part II: %d.\n", current_dir_size);
            break;
        }
    }

    return 0;
}
