#ifndef MEM_H
#define MEM_H

#include <glib.h>

static inline void
g_autoptr_cleanup_strvfree(void *p)
{
  gchar ***pp = (gchar***)p;
  g_strfreev(*pp);
}

#define g_autostrvfree __attribute__((cleanup(g_autoptr_cleanup_strvfree)))

#endif
