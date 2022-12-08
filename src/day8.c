#include <stdio.h>
#include <glib.h>

#include "benchmark.h"
#include "io.h"
#include "mem.h"

////////////////
// ENTRYPOINT //
////////////////

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        g_print("Usage: %s INPUT_FILE.\n", argv[0]);
        return 1;
    }

    BENCHMARK_START(day8);
    
    // Read file
    guint n_lines = 0;
    g_autostrvfree gchar **lines = read_file(argv[1], &n_lines);

    if (!lines) {
        g_printerr("Unable to read input file.\n");
        return 1;
    }

    guint width = strlen(*(lines));

    // Part I
    guint part1 = (width * 2) + ((n_lines * 2) - 4);

    for (guint row=1; row<n_lines-1; row++) {
        for (guint column=1; column<width-1; column++) {
       
            // Check top
            for (guint rr=0; rr<row; rr++) {
                if (lines[rr][column] >= lines[row][column]) {
                    // Higher or same tree blocking line of sight - go next
                    goto right;
                }
            }
            goto increment_part1;
            // Check right
            right:;
            for (guint cc=width-1; cc>column; cc--) {
                if (lines[row][cc] >= lines[row][column]) {
                    goto bottom;
                }
            }
            goto increment_part1;
            // Check bottom
            bottom:;
            for (guint rr=n_lines-1; rr>row; rr--) {
                if (lines[rr][column] >= lines[row][column]) {
                    goto left;
                }
            }
            goto increment_part1;
            // Check left
            left:;
            for (guint cc=0; cc<column; cc++) {
                if (lines[row][cc] >= lines[row][column]) {
                    goto nexttree;
                }
            }
            increment_part1:;
            part1++;
            nexttree:;
        }
    }

    g_print("Part I: %d.\n", part1);

    // Part II
    guint part2 = 0;

    for (guint row=0; row<n_lines; row++) {
        for (guint column=0; column<width; column++) {
       
            guint top = 0, left = 0, bottom = 0, right = 0;

            // Check top
            for (gint rr=row-1; rr>=0; rr--) {
                top++;
                if (lines[rr][column] >= lines[row][column]) {
                    break;
                } 
            }
            // Check right
            for (gint cc=column+1; cc<((gint)width); cc++) {
                right++;
                if (lines[row][cc] >= lines[row][column]) {
                    break;
                } 
            }
            // Check bottom
            for (gint rr=row+1; rr<((gint)n_lines); rr++) {
                bottom++;
                if (lines[rr][column] >= lines[row][column]) {
                    break;
                } 
            }
            // Check left
            for (gint cc=column-1; cc>=0; cc--) {
                left++;
                if (lines[row][cc] >= lines[row][column]) {
                    break;
                } 
            }

            part2 = MAX(part2, top * right * left * bottom);

        }
        
    }

    g_print("Part II: %d.\n", part2);

    BENCHMARK_END(day8);

    return 0;
}
