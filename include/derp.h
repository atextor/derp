#ifndef __DERP_H
#define __DERP_H

struct DerpPlugin {
	char* name;
	void (*create_plugin)(void);
};

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

