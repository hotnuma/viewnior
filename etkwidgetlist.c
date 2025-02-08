#include "etkwidgetlist.h"

GList* etk_widget_list_add(GList *list, GtkWidget *widget)
{
    g_return_val_if_fail(widget != NULL, NULL);

    return g_list_append(list, widget);
}

GList* etk_widget_list_free(GList *list)
{
    if (!list)
        return NULL;

    g_list_free(list);

    return NULL;
}

void etk_widget_list_set_sensitive(GList *list, gboolean sensitive)
{
    while (list)
    {
        gtk_widget_set_sensitive(GTK_WIDGET(list->data), sensitive);
        list = list->next;
    }
}


