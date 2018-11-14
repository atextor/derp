#ifndef _DERP_H_
#define _DERP_H_

#include <glib.h>
#include "typealiases.h"
#include "oo.h"
#include "plugin.h"
#include "triple.h"
#include "rule.h"
#include "assertion.h"
#include "retraction.h"
#include "action.h"

// Derp types
typedef enum {
	DERP_LOG_WARNING,
	DERP_LOG_ERROR,
	DERP_LOG_INFO,
	DERP_LOG_DEBUG
} derp_log_level;

// Plugin Descriptor is not a class
struct _DerpPluginDescriptor {
	gchar* name;
	void (*start_plugin)(struct DerpPlugin* self);
	void (*shutdown_plugin)(void);
	void (*callback)(gchar* rule, GHashTable* arguments);
};

typedef struct _DerpPluginDescriptor DerpPluginDescriptor;

// Derp functions
void derp_log(derp_log_level level, char* fmt, ...);

#endif
