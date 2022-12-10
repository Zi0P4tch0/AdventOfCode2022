#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

#define N_CYCLES ((guint)6)
#define CRT_WIDTH ((guint)40)
#define CRT_HEIGHT ((guint)6)

struct cpu {
    gint reg_X;
};

enum op {
    OP_NOOP,
    OP_ADD
};

struct cpu_op {
    enum op op;
    gint value;
};

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

    guint cycles[N_CYCLES] = {20, 60, 100, 140, 180, 220};

    g_autoptr(GArray) ops = g_array_new(FALSE, TRUE, sizeof(struct cpu_op));

    for (guint i=0; i<n_lines; i++) {
        const gchar *current_line = lines[i];
        g_autostrvfree gchar **tokens = g_strsplit(current_line, " ", 0);
        guint n_tokens = g_strv_length(tokens);
        struct cpu_op current_op = {.op = OP_NOOP, .value = 0};
        if (n_tokens == 2) {
            g_array_append_val(ops, current_op);
            current_op.op = OP_ADD;
            current_op.value = GINT_FROM_STR(tokens[1]);
            g_array_append_val(ops, current_op);
        } else {
            g_array_append_val(ops, current_op);
        }
    }

    struct cpu cpu = { .reg_X = 1 };

    gchar crt[CRT_HEIGHT][CRT_WIDTH] = { 0 };
    memset(crt, '.', sizeof(crt));

    gint part1 = 0;

    for (guint i=0; i<ops->len; i++) {
        for (guint c=0; c<N_CYCLES; c++) {
            if (cycles[c] == i+1) {
                part1 += (i+1) * cpu.reg_X;
                break;
            }
        }
    
        gint crt_row = i/CRT_WIDTH;
        gint crt_column = i%CRT_WIDTH;

        for (gint s=cpu.reg_X - 1; s<=cpu.reg_X + 1; s++) {
            if (crt_column == s) {
                crt[crt_row][crt_column] = '#';
            }
        }

        struct cpu_op current_op = g_array_index(ops, struct cpu_op, i);
        if (current_op.op == OP_ADD) {
            cpu.reg_X += current_op.value;
        }
    }

    g_print("Part I: %d.\nPart II:\n", part1);

    for (guint row=0; row<CRT_HEIGHT; row++) {
        for (guint column=0; column<CRT_WIDTH; column++) {
            g_print("%c", crt[row][column]);
        }
        g_print("\n");
    }
 
    return 0;
}
