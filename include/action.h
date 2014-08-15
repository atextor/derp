#ifndef _ACTION_H_
#define _ACTION_H_

#include <glib.h>
#include "typealiases.h"
#include "oo.h"
#include "plugin.h"
#include "rule.h"

struct DerpAction {
	const struct Object _;
	struct DerpPlugin* callee;
	GSList_String* vars;
	struct DerpRule* rule;
};

extern const void* DerpAction;

#endif

