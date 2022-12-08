#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"

////////////////
// ENTRYPOINT //
///////////////

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        g_print("Usage: %s INPUT_FILE.\n", argv[0]);
        return 1;
    }

    BENCHMARK_START(day5);

    // Read file

    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }
    
    // Find separator in input

    guint separator_index = 0;
    for (guint i=0; i<n_lines; i++) {
        if (strlen(*(lines+i)) == 0) {
            separator_index = i;
            break;
        }
    }

    // Count stacks (this only works when stacks < 10)
    const gchar *stacks_line = lines[separator_index - 1];
    guint n_stacks = 0;
    gchar *it = (gchar *)stacks_line;
    while (*it != '\0') {
        if (*it != ' ') n_stacks++;
        it++;
    }

    // Create stacks (using GQueue)
    g_autoptr(GPtrArray) stacks = g_ptr_array_new_full(n_stacks, (GDestroyNotify) g_queue_free);
    for (guint i=0; i<n_stacks; i++) {
        GQueue *new_queue = g_queue_new();
        g_ptr_array_add(stacks, new_queue);
    }

    // Populates stacks
    for (gint i=separator_index-2; i>=0; i--) {
        const gchar *current_line = lines[i];
        for(guint stack=0; stack<n_stacks; stack++) {
            gchar cargo = current_line[(4*stack)+1];
            if (cargo != ' ') {
                GQueue *current_queue = g_ptr_array_index(stacks, stack);
                g_queue_push_tail(current_queue, GINT_TO_POINTER((gint)cargo));
            }
        }
    }

    g_autoptr(GPtrArray) stacks_copy = g_ptr_array_copy(stacks, (GCopyFunc)g_queue_copy, NULL);

    // Part I
    for (guint i=separator_index+1; i<n_lines; i++) {
        g_autostrvfree gchar **tokens = g_strsplit(lines[i], " ", 0);
        guint count = g_ascii_strtoull(tokens[1], NULL, 10);
        guint from = g_ascii_strtoull(tokens[3], NULL, 10);
        guint to = g_ascii_strtoull(tokens[5], NULL, 10);
        GQueue *from_queue = g_ptr_array_index(stacks, from-1);
        GQueue *to_queue = g_ptr_array_index(stacks, to-1);
        for (guint j=0; j<count; j++) {
            gint cargo = GPOINTER_TO_INT(g_queue_pop_tail(from_queue));
            g_queue_push_tail(to_queue, GINT_TO_POINTER(cargo));
        }
    }

    g_autofree gchar *p1_cargo = g_malloc0_n(n_stacks+1, sizeof(gchar));
    for (guint stack=0; stack<n_stacks; stack++) {
        GQueue *current_queue = g_ptr_array_index(stacks, stack);
        gint cargo = GPOINTER_TO_INT(g_queue_peek_tail(current_queue));
        *(p1_cargo+stack) = (gchar)cargo;
    }
    *(p1_cargo+n_stacks) = '\0';

    g_print("Part I: %s.\n", p1_cargo);

    // Part II
    for (guint i=separator_index+1; i<n_lines; i++) {
        g_autostrvfree gchar **tokens = g_strsplit(lines[i], " ", 0);
        guint count = g_ascii_strtoull(tokens[1], NULL, 10);
        guint from = g_ascii_strtoull(tokens[3], NULL, 10);
        guint to = g_ascii_strtoull(tokens[5], NULL, 10);
        GQueue *from_queue = g_ptr_array_index(stacks_copy, from-1);
        GQueue *to_queue = g_ptr_array_index(stacks_copy, to-1);
        g_autoptr(GArray) tmp = g_array_sized_new(FALSE, TRUE, sizeof(gint), count);
        for (guint j=0; j<count; j++) {
            gint cargo = GPOINTER_TO_INT(g_queue_pop_tail(from_queue));
            g_array_append_val(tmp, cargo);
        }
        guint counter = 0;
        while (counter < count) {
            gint cargo = g_array_index(tmp, gint, tmp->len - 1 - counter);
            g_queue_push_tail(to_queue, GINT_TO_POINTER(cargo));
            counter++;
        }
    }

    g_autofree gchar *p2_cargo = g_malloc0_n(n_stacks+1, sizeof(gchar));
    for (guint stack=0; stack<n_stacks; stack++) {
        GQueue *current_queue = g_ptr_array_index(stacks_copy, stack);
        gint cargo = GPOINTER_TO_INT(g_queue_peek_tail(current_queue));
        *(p2_cargo+stack) = (gchar)cargo;
    }
    *(p2_cargo+n_stacks) = '\0';

    g_print("Part II: %s.\n", p2_cargo);

    BENCHMARK_END(day5);

    return 0;
}