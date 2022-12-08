#ifndef MACROS_H
#define MACROS_H

#include <glib.h>

#define GINT_FROM_STR(s) ((gint)g_ascii_strtoll(s, NULL, 10))
#define GUINT_FROM_STR(s) ((guint)g_ascii_strtoull(s, NULL, 10))

#endif
