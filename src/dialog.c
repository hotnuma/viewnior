/*
 * from Thunar by XFCE developpers
 */

#include "dialog.h"
#include "config.h"

//#include <stdlib.h>
#include <libintl.h>
//#include <glib/gi18n.h>
#define _(String) gettext (String)

#include "xfce-filename-input.h"

// dialog_file_rename
static void _dialog_select_filename(GtkWidget *entry, const gchar *filename);

gchar* dialog_file_rename(GtkWindow *window, const gchar *filename)
{
    //g_return_val_if_fail(parent == NULL
    //                     || GDK_IS_SCREEN(parent)
    //                     || GTK_IS_WINDOW(parent), FALSE);

    //IconFactory *icon_factory;
    //ThunarJob         *job = NULL;
    // get the filename of the file
    //const gchar       *filename;
    //filename = th_file_get_display_name(file);

    // create a new dialog window
    gchar *title = g_strdup_printf(_("Rename \"%s\""), filename);
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
                                        title,
                                        window,
                                        GTK_DIALOG_MODAL
                                        | GTK_DIALOG_DESTROY_WITH_PARENT,
                                        _("_Cancel"),
                                        GTK_RESPONSE_CANCEL,
                                        _("_Rename"),
                                        GTK_RESPONSE_OK,
                                        NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    g_free(title);

    // move the dialog to the appropriate screen
    //GtkWindow *window;
    //GdkScreen *screen = util_parse_parent(parent, &window);
    //if (window == NULL && screen != NULL)
    //    gtk_window_set_screen(GTK_WINDOW(dialog), screen);

    GtkWidget         *grid;
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 3);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 6);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), grid, TRUE, TRUE, 0);
    gtk_widget_show(grid);

    //GtkIconTheme      *icon_theme;
    //icon_theme = gtk_icon_theme_get_for_screen(gtk_widget_get_screen(dialog));
    //icon_factory = iconfact_get_for_icon_theme(icon_theme);
    //GdkPixbuf         *icon;
    //icon = iconfact_load_file_icon(icon_factory, file, FILE_ICON_STATE_DEFAULT, 48);
    //g_object_unref(G_OBJECT(icon_factory));

    //GtkWidget         *image;
    //image = gtk_image_new_from_pixbuf(icon);
    //gtk_widget_set_margin_start(GTK_WIDGET(image), 6);
    //gtk_widget_set_margin_end(GTK_WIDGET(image), 6);
    //gtk_widget_set_margin_top(GTK_WIDGET(image), 6);
    //gtk_widget_set_margin_bottom(GTK_WIDGET(image), 6);
    //gtk_grid_attach(GTK_GRID(grid), image, 0, 0, 1, 2);
    //g_object_unref(G_OBJECT(icon));
    //gtk_widget_show(image);

    GtkWidget         *label;
    label = gtk_label_new(_("Enter the new name:"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
    gtk_widget_show(label);

    // set up the widget for entering the filename
    XfceFilenameInput *filename_input;
    filename_input = g_object_new(XFCE_TYPE_FILENAME_INPUT, "original-filename", filename, NULL);
    gtk_widget_set_hexpand(GTK_WIDGET(filename_input), TRUE);
    gtk_widget_set_valign(GTK_WIDGET(filename_input), GTK_ALIGN_CENTER);

        /* connect to signals so that the sensitivity of the Create button is updated according to whether there
     * is a valid file name entered */
    g_signal_connect_swapped(
        filename_input, "text-invalid",
        G_CALLBACK(xfce_filename_input_desensitise_widget),
        gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK));

    g_signal_connect_swapped(
        filename_input, "text-valid",
        G_CALLBACK(xfce_filename_input_sensitise_widget),
        gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK));

    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(filename_input), 1, 1, 1, 1);

    //etk_label_set_a11y_relation(
    //                GTK_LABEL(label),
    //                GTK_WIDGET(xfce_filename_input_get_entry(filename_input)));

    gtk_widget_show_all(GTK_WIDGET(filename_input));

    // ensure that the sensitivity of the Create button is set correctly
    xfce_filename_input_check(filename_input);

    // select the filename without the extension
    GtkEntry *entry = xfce_filename_input_get_entry(filename_input);
    _dialog_select_filename(GTK_WIDGET(entry), filename);

    // get the size the entry requires to render the full text
    PangoLayout *layout = gtk_entry_get_layout(entry);

    gint layout_width;
    pango_layout_get_pixel_size(layout, &layout_width, NULL);
    gint layout_offset;
    gtk_entry_get_layout_offsets(entry, &layout_offset, NULL);
    layout_width +=(layout_offset * 2) +(12 * 4) + 48; // 12px free space in entry

    gint parent_width = 500;

    // parent window width
    if (window != NULL)
    {
        // keep below 90% of the parent window width
        gtk_window_get_size(GTK_WINDOW(window), &parent_width, NULL);
        parent_width *= 0.90f;
    }

    // resize the dialog to make long names fit as much as possible
    gtk_window_set_default_size(GTK_WINDOW(dialog),
                                CLAMP(layout_width, 300, parent_width),
                                -1);

    // automatically close the dialog when the file is destroyed
    //g_signal_connect_swapped(G_OBJECT(file), "destroy",
    //                         G_CALLBACK(gtk_widget_destroy), dialog);

    // run the dialog
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK)
    {
        // hide the dialog
        gtk_widget_hide(dialog);

        // determine the new filename
        const gchar *text = xfce_filename_input_get_text(filename_input);

        // check if we have a new name here
        if (g_strcmp0(filename, text) != 0)
        {
            // try to rename the file
            //job = io_rename_file(file, text);
        }
    }

    // cleanup
    if (response == GTK_RESPONSE_NONE)
        return NULL;

    // unregister handler
    //g_signal_handlers_disconnect_by_func(G_OBJECT(file),
    //                                     gtk_widget_destroy,
    //                                     dialog);

    gtk_widget_destroy(dialog);

    return NULL;
}

static void _dialog_select_filename(GtkWidget *entry, const gchar *filename)
{
    //const gchar *filename;
    // check if we have a directory here
    //if (th_file_is_directory(file))
    //{
    //    gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
    //    return;
    //}

    //filename = th_file_get_display_name(file);

 #if 0
    // check if the filename contains an extension
    const gchar *ext = util_str_get_extension(filename);
    if (ext == NULL)
        return;

    // grab focus to the entry first, else the selection will be altered later
    gtk_widget_grab_focus(entry);

    // determine the UTF-8 char offset
    glong offset = g_utf8_pointer_to_offset(filename, ext);

    // select the text prior to the dot
    if (offset > 0)
        gtk_editable_select_region(GTK_EDITABLE(entry), 0, offset);
#endif

}


