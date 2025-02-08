#ifndef ETKWIDGETLIST_H
#define ETKWIDGETLIST_H

#include <gtk/gtk.h>

GList* etk_widget_list_add(GList *list, GtkWidget *widget);
GList* etk_widget_list_free(GList *list);
void etk_widget_list_set_sensitive(GList *list, gboolean sensitive);

#endif // ETKWIDGETLIST_H

