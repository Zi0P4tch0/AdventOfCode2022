#include "io.h"

gchar**
read_file(const gchar *path, 
          guint       *n_lines)
{
    gchar **read_file(const gchar *path, guint *n_lines);
    g_autofree gchar *contents = NULL;
    g_file_get_contents(path, &contents, NULL, NULL);
    if (contents && n_lines) {
        gchar **lines = g_strsplit(contents, "\n", 0);
        *n_lines = g_strv_length(lines); 
        return lines;
    }
    return NULL;
}
