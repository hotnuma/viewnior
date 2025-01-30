#ifndef LIST_H
#define LIST_H

#include <glib.h>

G_BEGIN_DECLS

GList* vnr_list_new(gchar *filepath, gboolean include_hidden, GError **error);
GList* vnr_list_new_for_path(gchar *filepath, gboolean include_hidden, GError **error);
GList* vnr_list_new_multiple(GSList *uri_list, gboolean include_hidden, GError **error);
GList* vnr_list_free(GList *list);
GList* vnr_list_sort(GList *list);
gint vnr_list_get_position(GList *list, gint *total);

G_END_DECLS

#endif // LIST_H


