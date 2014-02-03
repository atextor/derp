#ifndef _DERP_H_
#define _DERP_H_

#include <glib.h>
#include <stdbool.h>

#if defined _WIN32 || defined __CYGWIN__
  #define EXPORT __declspec(dllexport)
#else
  #if __GNUC__ >= 4
    #define EXPORT __attribute__ ((visibility ("default")))
  #else
    #define EXPORT
  #endif
#endif

struct _DerpPlugin {
	char* name;
	void (*create_plugin)(void);
};

typedef struct _DerpPlugin DerpPlugin;

bool derp_assert_fact(char* fact);
bool derp_register_rule();
int derp_get_facts_size();
GSList* derp_get_facts();
GSList* derp_get_rules();
GSList* derp_get_rule_definition(char* rulename);


/*struct DerpPlugin* derp_init_plugin(void);*/

/*
typedef struct _GkrellmMonitor
        {
        gchar           *name;
        gint            id;
        void            (*create_monitor)(GtkWidget *, gint);
        void            (*update_monitor)(void);
        void            (*create_config)(GtkWidget *);
        void            (*apply_config)(void);

        void            (*save_user_config)(FILE *);
        void            (*load_user_config)(gchar *);
        gchar           *config_keyword;

        void            (*undef2)(void);
        void            (*undef1)(void);
        GkrellmMonprivate *privat;

        gint            insert_before_id;               // If plugin, insert before this mon

        void            *handle;                                // If monitor is a plugin.
        gchar           *path;                                  //      "
        }
        GkrellmMonitor;
*/


#endif

