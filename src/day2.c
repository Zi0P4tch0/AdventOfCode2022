#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"
#include "macros.h"

enum shape {
    ROCK = 1,
    PAPER = 2,
    SCISSORS = 3
};

enum outcome {
    LOSS = 0,
    DRAW = 3,
    WIN = 6
};

inline enum shape
shape_from_char(gchar input)
 {
    if (input == 'A' || input == 'X') {
        return ROCK;
    } else if (input == 'B' || input == 'Y') {
        return PAPER;
    } 
    return SCISSORS;
}


inline enum outcome
outcome_from_char(gchar input)
 {
    if (input == 'X') {
        return LOSS;
    } else if (input == 'Y') {
        return DRAW;
    } 
    return WIN;
}

inline enum shape
shape_for_outcome(enum shape lhs, enum outcome outcome) {
    switch (outcome) {
        case DRAW:
            return lhs;
        case LOSS:
            if (lhs == ROCK) { return SCISSORS; }
            if (lhs == PAPER) { return ROCK; }
            return PAPER;
        case WIN:
            if (lhs == ROCK) { return PAPER; }
            if (lhs == PAPER) { return SCISSORS; }
            return ROCK;
    }
}

inline guint
score_for_round(enum shape lhs, enum shape rhs) 
{
    if (lhs == rhs) { return 3 + ((guint)rhs); }
    switch (lhs) {
        case ROCK:
            return (rhs == SCISSORS ? 0 : 6) + ((guint)rhs);
        case PAPER:
            return (rhs == ROCK ? 0 : 6) + ((guint)rhs);
        case SCISSORS:
            return (rhs == PAPER ? 0 : 6) + ((guint)rhs);
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

    BENCHMARK_START(day2);
    
    // Read file
    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    // Part I && II
    guint part1 = 0;
    guint part2 = 0;

    for (guint i=0; i<n_lines; i++) {
        const gchar *current_line = lines[i];
        g_autostrvfree gchar **tokens = g_strsplit(current_line, " ", 0);
        enum shape lhs = shape_from_char(tokens[0][0]);
        enum shape rhs = shape_from_char(tokens[1][0]);
        enum outcome outcome = outcome_from_char(tokens[1][0]);
        enum shape pt2_rhs = shape_for_outcome(lhs, outcome);
        part1 += score_for_round(lhs, rhs);
        part2 += ((guint)outcome) + ((guint)pt2_rhs);
    }

    g_print("Part I: %d.\n", part1);
    g_print("Part II: %d.\n", part2);
 
    BENCHMARK_END(day2);

    return 0;
}