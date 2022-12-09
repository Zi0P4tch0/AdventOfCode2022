#ifndef IO_H
#define IO_H

#include <glib.h>

gchar**
read_file(const gchar *path, 
          guint       *n_lines);

#endif
