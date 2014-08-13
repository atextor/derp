#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <glib.h>
#include "oo.h"

typedef GSList GSList_DerpPlugin;

struct DerpPlugin {
	const struct Class* class;
	gchar* name;
	gchar* file_name;
	void (*start_plugin)(void);
	void (*shutdown_plugin)(void);
	void (*callback)(gchar* rule, GHashTable* arguments);
};

extern const void* DerpPlugin;

#endif
