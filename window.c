/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of Viewnior.
 *
 * Viewnior is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 * Viewnior is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Viewnior.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window.h"
#include "config.h"

#include "uni-scroll-win.h"
#include "uni-anim-view.h"
#include "vnr-tools.h"
#include "vnr-message-area.h"
#include "vnr-properties-dialog.h"
#include "vnr-crop.h"
#include "uni-exiv2.hpp"
#include "uni-utils.h"
#include "dialog.h"
#include "list.h"

#include <etkaction.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Timeout to hide the toolbar in fullscreen mode
#define FULLSCREEN_TIMEOUT 1000
#define DARK_BACKGROUND_COLOR "#222222"

G_DEFINE_TYPE(VnrWindow, window, GTK_TYPE_WINDOW)

// Window creation ------------------------------------------------------------

GtkWindow* window_new();
static void window_class_init(VnrWindowClass *klass);
static void window_init(VnrWindow *window);
static void _window_load_accel_map();

// Window destruction ---------------------------------------------------------

static void _window_on_destroy(GtkWidget *widget, gpointer user_data);
static void _window_on_realize(GtkWidget *widget, gpointer user_data);
static void _window_save_accel_map();
static void window_dispose(GObject *object);
static void window_finalize(GObject *object);

// Window ---------------------------------------------------------------------

static gint _window_on_key_press(GtkWidget *widget, GdkEventKey *event);
static gboolean _window_on_change_state(GtkWidget *widget,
                                        GdkEventWindowState *event,
                                        gpointer user_data);
static void _view_on_zoom_changed(UniImageView *view, VnrWindow *window);

// Open / Close ---------------------------------------------------------------

static void _window_action_openfile(VnrWindow *window, GtkWidget *widget);
static void _window_action_opendir(VnrWindow *window, GtkWidget *widget);
static void _window_update_openwith_menu(VnrWindow *window);
static void _on_openwith(VnrWindow *window, gpointer user_data);

// Actions --------------------------------------------------------------------

static gboolean _window_next_image_src(VnrWindow *window);
static void _window_action_rename(VnrWindow *window, GtkWidget *widget);
static void _window_action_move_to(VnrWindow *window, GtkWidget *widget);
static void _window_action_move(VnrWindow *window, GtkWidget *widget);
static void _window_move(VnrWindow *window);
static gboolean _window_select_directory(VnrWindow *window);
static GSList* _window_file_chooser(VnrWindow *window,
                                    const gchar *title,
                                    GtkFileChooserAction action,
                                    gboolean multiple);
static void _window_action_delete(VnrWindow *window, GtkWidget *widget);
static gboolean _window_delete_item(VnrWindow *window);
static void _window_hide_cursor(VnrWindow *window);
static void _window_show_cursor(VnrWindow *window);
static void _window_action_properties(VnrWindow *window, GtkWidget *widget);
static void _window_action_preferences(VnrWindow *window, GtkWidget *widget);

// Private Actions ------------------------------------------------------------

static void _window_rotate_pixbuf(VnrWindow *window, GdkPixbufRotation angle);
static void _window_flip_pixbuf(VnrWindow *window, gboolean horizontal);
static void _window_action_save_image(VnrWindow *window, GtkWidget *widget);

// Fullscreen -----------------------------------------------------------------

static void _window_fullscreen(VnrWindow *window);
static void _window_unfullscreen(VnrWindow *window);
static gboolean _on_fullscreen_motion(GtkWidget *widget,
                                      GdkEventMotion *ev,
                                      VnrWindow *window);
static void _window_fullscreen_set_timeout(VnrWindow *window);
static gboolean _on_fullscreen_timeout(VnrWindow *window);
static void _window_fullscreen_unset_timeout(VnrWindow *window);
static gboolean _on_leave_image_area(GtkWidget *widget,
                                     GdkEventCrossing *ev,
                                     VnrWindow *window);

//static GtkWidget* _window_get_fs_toolitem(VnrWindow *window);
//static void _on_fullscreen_leave(GtkButton *button, VnrWindow *window);
//static void _on_spin_value_change(GtkSpinButton *spinbutton,
//                                  VnrWindow *window);
//static void _on_toggle_show_next(GtkToggleButton *togglebutton,
//                                 VnrWindow *window);

// ----------------------------------------------------------------------------

static void _action_crop(GtkAction *action, VnrWindow *window);
static void _action_resize(GtkToggleAction *action, VnrWindow *window);
static gboolean _file_size_is_small(char *filename);
static void _on_update_preview(GtkFileChooser *file_chooser, gpointer data);
static void _on_file_open_dialog_response(GtkWidget *dialog,
                                          gint response_id,
                                          VnrWindow *window);
static void _window_update_fs_filename_label(VnrWindow *window);

// Slideshow ------------------------------------------------------------------

static void _window_action_slideshow(VnrWindow *window);
static void _window_slideshow_stop(VnrWindow *window);
static void _window_slideshow_start(VnrWindow *window);
//static void _window_slideshow_restart(VnrWindow *window);
static void _window_slideshow_allow(VnrWindow *window);
void window_slideshow_deny(VnrWindow *window);

// DnD ------------------------------------------------------------------------

static void _window_set_drag(VnrWindow *window);
static void _view_on_drag_begin(GtkWidget *widget,
                                GdkDragContext *drag_context,
                                GtkSelectionData *data,
                                guint info,
                                guint time,
                                gpointer user_data);
static void _window_drag_data_received(GtkWidget *widget,
                                       GdkDragContext *context,
                                       gint x, gint y,
                                       GtkSelectionData *selection_data,
                                       guint info, guint time);

typedef enum
{
    WINDOW_ACTION_OPENFILE = 1,
    WINDOW_ACTION_OPENDIR,
    WINDOW_ACTION_OPENWITH,
    WINDOW_ACTION_RENAME,
    WINDOW_ACTION_MOVETO,
    WINDOW_ACTION_MOVE,
    WINDOW_ACTION_DELETE,
    WINDOW_ACTION_PROPERTIES,
    WINDOW_ACTION_PREFERENCES,
    WINDOW_ACTION_ITEM1,
    WINDOW_ACTION_ITEM2,
    WINDOW_ACTION_ITEM3,
    WINDOW_ACTION_ITEM4,
    WINDOW_ACTION_ITEM5,
    WINDOW_ACTION_ITEM6,
    WINDOW_ACTION_ITEM7,
    WINDOW_ACTION_ITEM8,

} WindowAction;

static EtkActionEntry _window_actions[] =
{
    {WINDOW_ACTION_OPENFILE,
     "<Actions>/AppWindow/OpenFile", "<Control>O",
     ETK_MENU_ITEM_IMAGE, N_("Open _Image..."),
     N_("Open an Image"),
     "gtk-file",
     G_CALLBACK(_window_action_openfile)},

    {WINDOW_ACTION_OPENDIR,
     "<Actions>/AppWindow/OpenDir", "<Control>F",
     ETK_MENU_ITEM_IMAGE, N_("Open _Folder..."),
     N_("Open a Folder"),
     "gtk-directory",
     G_CALLBACK(_window_action_opendir)},

    {WINDOW_ACTION_OPENWITH,
     "<Actions>/AppWindow/OpenWith", "",
     ETK_MENU_ITEM, N_("Open _With"),
     N_("Open the selected image with a different application"),
     NULL,
     NULL},

    {WINDOW_ACTION_RENAME,
     "<Actions>/AppWindow/Rebame", "F2",
     ETK_MENU_ITEM, N_("Rename"),
     N_("Rename the current file"),
     NULL,
     G_CALLBACK(_window_action_rename)},

    {WINDOW_ACTION_MOVETO,
     "<Actions>/AppWindow/MoveTo", "F7",
     ETK_MENU_ITEM, N_("Move to..."),
     N_("Move the current file..."),
     NULL,
     G_CALLBACK(_window_action_move_to)},

    {WINDOW_ACTION_MOVE,
     "<Actions>/AppWindow/Move", "F8",
     ETK_MENU_ITEM, N_("Move"),
     N_("Move the current file"),
     NULL,
     G_CALLBACK(_window_action_move)},

    {WINDOW_ACTION_DELETE,
     "<Actions>/AppWindow/Delete", "Delete",
     ETK_MENU_ITEM, N_("_Delete"),
     N_("Delete the current file"),
     NULL,
     G_CALLBACK(_window_action_delete)},

    {WINDOW_ACTION_PROPERTIES,
     "<Actions>/AppWindow/Properties", "<Control>Return",
     ETK_MENU_ITEM, N_("_Properties..."),
     N_("Show information about the current file"),
     NULL,
     G_CALLBACK(_window_action_properties)},

    {WINDOW_ACTION_PREFERENCES,
     "<Actions>/AppWindow/Preferences", "",
     ETK_MENU_ITEM, N_("_Preferences..."),
     N_("User preferences for Viewnior"),
     NULL,
     G_CALLBACK(_window_action_preferences)},

    {0},
};


// Window creation ------------------------------------------------------------

GtkWindow* window_new()
{
    return (GtkWindow*) g_object_new(VNR_TYPE_WINDOW, NULL);
}

static void window_class_init(VnrWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->dispose = window_dispose;
    gobject_class->finalize = window_finalize;

    GtkWidgetClass *gtkwidget_class = GTK_WIDGET_CLASS(klass);
    gtkwidget_class->key_press_event = _window_on_key_press;
    gtkwidget_class->drag_data_received = _window_drag_data_received;
}

static void window_init(VnrWindow *window)
{
    g_assert(window->filelist == NULL);

    window->filelist = NULL;
    window->movedir = NULL;
    window->mode = WINDOW_MODE_NORMAL;
    window->accel_group = etk_actions_init(GTK_WINDOW(window), _window_actions);
    window->prefs = (VnrPrefs*) vnr_prefs_new(GTK_WIDGET(window));
    window->can_edit = false;
    window->list_image = NULL;

    window->writable_format_name = NULL;
    window->cursor_is_hidden = FALSE;
    window->disable_autohide = FALSE;

    window->fs_toolitem = NULL;
    window->fs_source = NULL;
    window->sl_timeout = 5;
    window->slideshow = TRUE;

    gtk_window_set_title((GtkWindow*) window, "Viewnior");
    gtk_window_set_default_icon_name("viewnior");

    window->layout_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_container_add(GTK_CONTAINER(window), window->layout_box);
    gtk_widget_show(window->layout_box);

    // create popup menu.
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item = NULL;

    window->popup_menu = menu;
    gtk_menu_set_accel_group(GTK_MENU(menu), window->accel_group);

    etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                  WINDOW_ACTION_OPENFILE,
                                  _window_actions,
                                  G_OBJECT(window));

    etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                  WINDOW_ACTION_OPENDIR,
                                  _window_actions,
                                  G_OBJECT(window));

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_OPENWITH,
                                         _window_actions,
                                         G_OBJECT(window));
    window->openwith_item = item;

    etk_menu_append_separator(GTK_MENU_SHELL(menu));

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_RENAME,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_MOVETO,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_MOVE,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_DELETE,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_PROPERTIES,
                                         _window_actions,
                                         G_OBJECT(window));
    window->list_image = etk_widget_list_add(window->list_image, item);

    etk_menu_append_separator(GTK_MENU_SHELL(menu));

    item = etk_menu_item_new_from_action(GTK_MENU_SHELL(menu),
                                         WINDOW_ACTION_PREFERENCES,
                                         _window_actions,
                                         G_OBJECT(window));

    gtk_widget_show_all(menu);
    gtk_widget_hide(window->openwith_item);

    etk_widget_list_set_sensitive(window->list_image, false);

    //gtk_action_group_set_sensitive(window->action_wallpaper, FALSE);
    //gtk_action_group_set_sensitive(window->actions_collection, FALSE);
    //gtk_action_group_set_sensitive(window->actions_static_image, FALSE);
    //gtk_action_group_set_sensitive(window->action_save, FALSE);
    //gtk_action_group_set_sensitive(window->actions_bars, TRUE);

    //gtk_widget_hide(_window_get_fs_toolitem(window));

    // Apply auto-resize preference
    //action = gtk_action_group_get_action(window->actions_image,
    //                                     "ViewResizeWindow");

    //if (window->prefs->auto_resize)
    //    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), TRUE);

    window->msg_area = vnr_message_area_new();
    VNR_MESSAGE_AREA(window->msg_area)->vnr_win = window;
    gtk_box_pack_start(GTK_BOX(window->layout_box),
                       window->msg_area, FALSE, FALSE, 0);
    gtk_widget_show(GTK_WIDGET(window->msg_area));

    window->view = uni_anim_view_new();
    gtk_widget_set_can_focus(window->view, TRUE);
    window->scroll_view = uni_scroll_win_new(UNI_IMAGE_VIEW(window->view));

    gtk_box_pack_end(GTK_BOX(window->layout_box),
                     window->scroll_view, TRUE, TRUE, 0);
    gtk_widget_show_all(GTK_WIDGET(window->scroll_view));

    gtk_widget_grab_focus(window->view);

    // Initialize slideshow timeout
    window->sl_timeout = window->prefs->slideshow_timeout;

    // Care for Properties dialog
    window->props_dlg = vnr_properties_dialog_new(
            window,
            NULL,
            //gtk_action_group_get_action(window->actions_collection, "GoNext"),
            NULL);
            //gtk_action_group_get_action(window->actions_collection, "GoPrevious"));

    window_preferences_apply(window);

    _window_set_drag(window);

    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(_window_on_destroy), NULL);

    g_signal_connect(G_OBJECT(window), "realize",
                     G_CALLBACK(_window_on_realize), NULL);

    g_signal_connect(G_OBJECT(window), "window-state-event",
                     G_CALLBACK(_window_on_change_state), NULL);

    g_signal_connect(G_OBJECT(window->view), "zoom_changed",
                     G_CALLBACK(_view_on_zoom_changed), window);

    g_signal_connect(G_OBJECT(window->view), "drag-data-get",
                     G_CALLBACK(_view_on_drag_begin), window);

    _window_load_accel_map();
}

#if 0
#endif

static void _window_load_accel_map()
{
    gchar *accelfile = g_build_filename(g_get_user_config_dir(), PACKAGE,
                                        "accel_map", NULL);
    //gtk_accel_map_load(accelfile);

    g_free(accelfile);
}


// Window destruction ---------------------------------------------------------

static void _window_on_destroy(GtkWidget *widget, gpointer user_data)
{
    _window_save_accel_map();
    vnr_prefs_save(VNR_WINDOW(widget)->prefs);
    gtk_main_quit();
}

static void _window_save_accel_map()
{
    gchar *accelfile = g_build_filename(g_get_user_config_dir(), PACKAGE,
                                        "accel_map", NULL);
    //gtk_accel_map_save(accelfile);

    g_free(accelfile);
}

static void window_dispose(GObject *object)
{
    // do something

    VnrWindow *window = VNR_WINDOW(object);

    window->accel_group = etk_actions_dispose(GTK_WINDOW(window),
                                              window->accel_group);
    window->list_image = etk_widget_list_free(window->list_image);

    G_OBJECT_CLASS(window_parent_class)->dispose(object);
}

static void window_finalize(GObject *object)
{
    VnrWindow *window = VNR_WINDOW(object);
    if (window->movedir)
        g_free(window->movedir);

    window->filelist = vnr_list_free(window->filelist);

    G_OBJECT_CLASS(window_parent_class)->finalize(object);
}

static void _window_on_realize(GtkWidget *widget, gpointer user_data)
{
    g_signal_handlers_disconnect_by_func(widget,
                                         _window_on_realize,
                                         user_data);

    VnrWindow *window = VNR_WINDOW(widget);

    if (vnr_message_area_is_critical(VNR_MESSAGE_AREA(window->msg_area)))
        return;

    if (window->prefs->start_maximized)
    {
        window_file_load(window, FALSE);
    }
    else
    {
        GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(widget));

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS

        GdkRectangle monitor;
        gdk_screen_get_monitor_geometry(
                            screen,
                            gdk_screen_get_monitor_at_window(screen,
                            gtk_widget_get_window(widget)),
                            &monitor);

        G_GNUC_END_IGNORE_DEPRECATIONS

        window->max_width = monitor.width * 0.9 - 100;
        window->max_height = monitor.height * 0.9 - 100;

        window_file_load(window, TRUE);
    }

    VnrFile *current = window_list_get_current(window);

    if (!current)
        return;

    if (window->prefs->start_fullscreen)
    {
        _window_fullscreen(window);
    }
    else if (window->prefs->start_slideshow)
    {
        _window_fullscreen(window);
        window->mode = WINDOW_MODE_NORMAL;
        _window_slideshow_allow(window);
        _window_slideshow_start(window);
    }
}


// ----------------------------------------------------------------------------

static gint _window_on_key_press(GtkWidget *widget, GdkEventKey *event)
{
    // Modified version of eog's eog_window_key_press

    VnrWindow *window = VNR_WINDOW(widget);
    gint result = FALSE;

    GtkWidget *toolbar_focus_child = NULL;
    GtkWidget *msg_area_focus_child = gtk_container_get_focus_child(
                                            GTK_CONTAINER(window->msg_area));
    switch (event->keyval)
    {
    case GDK_KEY_Left:
        if (!uni_scroll_win_image_fits(UNI_SCROLL_WIN(window->scroll_view)))
            break; // let scrollview handle the key

        if (toolbar_focus_child != NULL || msg_area_focus_child != NULL)
            break;

        if (event->state & GDK_CONTROL_MASK)
        {
            _window_rotate_pixbuf(window, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
            result = TRUE;
            break;
        }

        window_prev(window);
        result = TRUE;
        break;

    case GDK_KEY_Right:
        if (!uni_scroll_win_image_fits(UNI_SCROLL_WIN(window->scroll_view)))
            break; // let scrollview handle the key

        if (toolbar_focus_child != NULL || msg_area_focus_child != NULL)
            break;

        if (event->state & GDK_CONTROL_MASK)
        {
            _window_rotate_pixbuf(window, GDK_PIXBUF_ROTATE_CLOCKWISE);
            result = TRUE;
            break;
        }

        window_next(window, TRUE);
        result = TRUE;
        break;

    case GDK_KEY_Up:
        if (event->state & GDK_CONTROL_MASK)
        {
            _window_flip_pixbuf(window, TRUE);
            result = TRUE;
        }
        break;

    case GDK_KEY_Down:
        if (event->state & GDK_CONTROL_MASK)
        {
            _window_flip_pixbuf(window, FALSE);
            result = TRUE;
        }
        break;

    case GDK_KEY_Escape:
    case 'q':
        if (window->mode != WINDOW_MODE_NORMAL)
            _window_unfullscreen(window);
        else
            gtk_main_quit();
        break;

    case GDK_KEY_space:
        if (toolbar_focus_child != NULL || msg_area_focus_child != NULL)
            break;
        window_next(window, TRUE);
        result = TRUE;
        break;

    case GDK_KEY_BackSpace:
        window_prev(window);
        result = TRUE;
        break;

    case GDK_KEY_Home:
        window_first(window);
        result = TRUE;
        break;

    case GDK_KEY_End:
        window_last(window);
        result = TRUE;
        break;

    case GDK_KEY_F6:
        _window_action_slideshow(window);
        result = TRUE;
        break;

    case GDK_KEY_F11:
        window_fullscreen_toggle(window);
        result = TRUE;
        break;

    case 'h':
        _window_flip_pixbuf(window, TRUE);
        break;

    case 'v':
        _window_flip_pixbuf(window, FALSE);
        break;

    case 'c':
        _action_crop(NULL, window);
        break;
    }

    if (result == FALSE
        && GTK_WIDGET_CLASS(window_parent_class)->key_press_event)
    {
        result =
        (*GTK_WIDGET_CLASS(window_parent_class)->key_press_event) (widget, event);
    }

    return result;
}

static gboolean _window_on_change_state(GtkWidget *widget,
                                        GdkEventWindowState *event,
                                        gpointer user_data)
{
    if (event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
    {
        /* Detect maximized state only */
        if (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED)
        {
            VNR_WINDOW(widget)->prefs->start_maximized = TRUE;
        }
        else
        {
            VNR_WINDOW(widget)->prefs->start_maximized = FALSE;
        }

        vnr_prefs_save(VNR_WINDOW(widget)->prefs);
    }

    return TRUE;
}

static void _view_on_zoom_changed(UniImageView *view, VnrWindow *window)
{
    /* Change the info, only if there is an image
     * (vnr_window_close isn't called on the current image) */

    VnrFile *current = window_list_get_current(window);
    if (!current)
        return;

    gint total = 0;
    gint position = vnr_list_get_position(window->filelist, &total);

    char *buf = g_strdup_printf("%s%s - %i/%i - %ix%i - %i%%",
                                (window->modifications) ? "*" : "",
                                current->display_name,
                                position,
                                total,
                                window->current_image_width,
                                window->current_image_height,
                                (int) (view->zoom * 100.));

    gtk_window_set_title(GTK_WINDOW(window), buf);

    //gint context_id = gtk_statusbar_get_context_id(
    //            GTK_STATUSBAR(window->statusbar), "statusbar");
    //gtk_statusbar_pop(GTK_STATUSBAR(window->statusbar),
    //                  GPOINTER_TO_INT(context_id));
    //gtk_statusbar_push(GTK_STATUSBAR(window->statusbar),
    //                   GPOINTER_TO_INT(context_id), buf);

    g_free(buf);
}


// ----------------------------------------------------------------------------

void window_list_set(VnrWindow *window, GList *list)
{
    window->filelist = vnr_list_free(window->filelist);

    if (g_list_length(g_list_first(list)) > 1)
    {
        //gtk_action_group_set_sensitive(window->actions_collection, true);

        _window_slideshow_allow(window);
    }
    else
    {
        //gtk_action_group_set_sensitive(window->actions_collection, false);

        window_slideshow_deny(window);
    }

    window->filelist = list;
}

VnrFile* window_list_get_current(VnrWindow *window)
{
    if (window->filelist == NULL)
        return NULL;

    return VNR_FILE(window->filelist->data);
}


// Open / Close ---------------------------------------------------------------

static void _window_action_openfile(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
                                _("Open Image"),
                                GTK_WINDOW(window),
                                GTK_FILE_CHOOSER_ACTION_OPEN,
                                "gtk-cancel", GTK_RESPONSE_CANCEL,
                                "gtk-open", GTK_RESPONSE_ACCEPT,
                                NULL);

    GtkFileFilter *img_filter = gtk_file_filter_new();
    g_assert(img_filter != NULL);
    gtk_file_filter_add_pixbuf_formats(img_filter);
    gtk_file_filter_add_mime_type(img_filter, "image/vnd.microsoft.icon");
    gtk_file_filter_set_name(img_filter, _("All Images"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), img_filter);

    GtkFileFilter *all_filter = gtk_file_filter_new();
    g_assert(all_filter != NULL);
    gtk_file_filter_add_pattern(all_filter, "*");
    gtk_file_filter_set_name(all_filter, _("All Files"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all_filter);

    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), img_filter);

    GtkWidget *preview = gtk_image_new();
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
    g_signal_connect(GTK_FILE_CHOOSER(dialog), "update-preview",
                     G_CALLBACK(_on_update_preview), preview);

    VnrFile *current = window_list_get_current(window);
    if (current)
    {
        gchar *dirname = g_path_get_dirname(current->path);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    g_signal_connect(dialog, "response",
                     G_CALLBACK(_on_file_open_dialog_response), window);

    gtk_widget_show_all(GTK_WIDGET(dialog));

    // This only works when here.
    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog), window->prefs->show_hidden);
}

static void _window_action_opendir(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
                                _("Open Folder"),
                                GTK_WINDOW(window),
                                GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                "gtk-cancel", GTK_RESPONSE_CANCEL,
                                "gtk-open", GTK_RESPONSE_ACCEPT,
                                NULL);

    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    VnrFile *current = window_list_get_current(window);
    if (current)
    {
        gchar *dirname = g_path_get_dirname(current->path);
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dirname);
        g_free(dirname);
    }

    g_signal_connect(dialog, "response",
                     G_CALLBACK(_on_file_open_dialog_response),
                     window);

    gtk_widget_show_all(GTK_WIDGET(dialog));

    // This only works when here
    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),
                                     window->prefs->show_hidden);
}

void window_open_list(VnrWindow *window, GSList *uri_list)
{
    GList *file_list = NULL;
    GError *error = NULL;

    if (g_slist_length(uri_list) == 1)
    {
        file_list = vnr_list_new_for_path(uri_list->data, window->prefs->show_hidden, &error);
    }
    else
    {
        file_list = vnr_list_new_multiple(uri_list, window->prefs->show_hidden, &error);
    }

    if (error != NULL && file_list != NULL)
    {
        window_file_close(window);

        //gtk_action_group_set_sensitive(window->actions_collection, FALSE);

        window_slideshow_deny(window);
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE, error->message, TRUE);

        window_list_set(window, file_list); // TRUE);
    }
    else if (error != NULL)
    {
        window_file_close(window);
        window_slideshow_deny(window);
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE, error->message, TRUE);
    }
    else if (file_list == NULL)
    {
        window_file_close(window);

        //gtk_action_group_set_sensitive(window->actions_collection, FALSE);

        window_slideshow_deny(window);
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area), TRUE,
                              _("The given locations contain no images."),
                              TRUE);
    }
    else
    {
        window_list_set(window, file_list); // TRUE);

        if (!window->cursor_is_hidden)
            vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

        //gdk_flush();

        window_file_close(window);
        window_file_load(window, FALSE);

        if (!window->cursor_is_hidden)
            vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);
    }
}

gboolean window_file_load(VnrWindow *window, gboolean fit_to_screen)
{
    g_return_val_if_fail(window != NULL, false);

    VnrFile *current = window_list_get_current(window);
    if (!current)
        return false;

    _window_update_fs_filename_label(window);

    GError *error = NULL;
    GdkPixbufAnimation *pixbuf =
            gdk_pixbuf_animation_new_from_file(current->path, &error);

    GdkPixbufFormat *format;
    UniFittingMode last_fit_mode;

    if (error != NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE, error->message, TRUE);

        if (gtk_widget_get_visible(window->props_dlg))
            vnr_properties_dialog_clear(VNR_PROPERTIES_DIALOG(window->props_dlg));
        return FALSE;
    }

    if (vnr_message_area_is_visible(VNR_MESSAGE_AREA(window->msg_area)))
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));
    }

    etk_widget_list_set_sensitive(window->list_image, true);
    //gtk_action_group_set_sensitive(window->actions_image, TRUE);
    //gtk_action_group_set_sensitive(window->action_wallpaper, TRUE);

    format = gdk_pixbuf_get_file_info(current->path, NULL, NULL);

    g_free(window->writable_format_name);
    if (gdk_pixbuf_format_is_writable(format))
        window->writable_format_name = gdk_pixbuf_format_get_name(format);
    else
        window->writable_format_name = NULL;

    vnr_tools_apply_embedded_orientation(&pixbuf);
    window->current_image_width = gdk_pixbuf_animation_get_width(pixbuf);
    window->current_image_height = gdk_pixbuf_animation_get_height(pixbuf);
    window->modifications = 0;

    if (fit_to_screen)
    {
        gint img_h, img_w; /* Width and Height of the pixbuf */

        img_w = window->current_image_width;
        img_h = window->current_image_height;

        vnr_tools_fit_to_size(&img_w, &img_h, window->max_width, window->max_height);

        gtk_window_resize(GTK_WINDOW(window),
                          img_w,
                          img_h /*+ _window_get_top_widgets_height(window)*/);
    }

    last_fit_mode = UNI_IMAGE_VIEW(window->view)->fitting;

    // returns true if the image is static
    window->can_edit = uni_anim_view_set_anim(UNI_ANIM_VIEW(window->view), pixbuf);

    if (window->mode != WINDOW_MODE_NORMAL && window->prefs->fit_on_fullscreen)
    {
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view), VNR_PREFS_ZOOM_FIT);
    }
    else if (window->prefs->zoom == VNR_PREFS_ZOOM_LAST_USED)
    {
        uni_image_view_set_fitting(UNI_IMAGE_VIEW(window->view), last_fit_mode);
        _view_on_zoom_changed(UNI_IMAGE_VIEW(window->view), window);
    }
    else
    {
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view), window->prefs->zoom);
    }

    if (window->prefs->auto_resize)
    {
        _action_resize(NULL, window);
    }

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update(VNR_PROPERTIES_DIALOG(window->props_dlg));

    _window_update_openwith_menu(window);

    g_object_unref(pixbuf);

    return TRUE;
}

void window_file_close(VnrWindow *window)
{
    gtk_window_set_title(GTK_WINDOW(window), "Viewnior");
    uni_anim_view_set_anim(UNI_ANIM_VIEW(window->view), NULL);

    //gtk_action_group_set_sensitive(window->actions_static_image, FALSE);
    _window_update_openwith_menu(window);
    window->can_edit = false;

    etk_widget_list_set_sensitive(window->list_image, false);
    //gtk_action_group_set_sensitive(window->actions_image, FALSE);
    //gtk_action_group_set_sensitive(window->action_wallpaper, FALSE);
}

static void _window_update_openwith_menu(VnrWindow *window)
{
    // Modified version of eog's eog_window_update_openwith_menu

    gtk_widget_hide(window->openwith_item);

    VnrFile *current = window_list_get_current(window);
    if (!current)
        return;

    GFile *file = g_file_new_for_path((gchar*) current->path);
    GFileInfo *file_info = g_file_query_info(
                            file,
                            G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                            0, NULL, NULL);
    g_object_unref(file);

    if (!file_info)
        return;

    const gchar *mime_type = g_file_info_get_content_type(file_info);

    if (mime_type == NULL)
    {
        g_object_unref(file_info);
        return;
    }

    GList *apps = g_app_info_get_all_for_type(mime_type);
    g_object_unref(file_info);

    if (!apps)
        return;

    GtkWidget *menu = gtk_menu_new();
    gint count = 0;

    for (GList *iter = apps; iter; iter = iter->next)
    {
        GAppInfo *app = iter->data;

        // do not include viewnior itself
        if (g_ascii_strcasecmp(g_app_info_get_executable(app),
                               g_get_prgname()) == 0)
        {
            g_object_unref(app);
            continue;
        }

        gchar *label = g_strdup(g_app_info_get_name(app));
        gchar *tooltip = g_strdup_printf(
                                _("Use \"%s\" to open the selected image"),
                                g_app_info_get_name(app));

        GtkWidget *item = etk_menu_item_new(GTK_MENU_SHELL(menu),
                                            label,
                                            tooltip,
                                            NULL,
                                            G_CALLBACK(_on_openwith),
                                            G_OBJECT(window));

        //printf("add item : %s\n", label);

        g_free(label);
        g_free(tooltip);

        g_object_set_data_full(G_OBJECT(item),
                               "app",
                               app,
                               (GDestroyNotify) g_object_unref);

        ++count;
    }

    g_list_free(apps);

    // see launcher.c: 816
    // launcher_append_open_section
    // gtk_menu_item_set_submenu
    // Sets or replaces the menu item’s submenu,
    // or removes it when a NULL submenu is passed.

    if (!count)
    {
        gtk_widget_destroy(menu);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->openwith_item), NULL);

        return;
    }

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(window->openwith_item), menu);
    gtk_widget_show_all(window->openwith_item);
}

static void _on_openwith(VnrWindow *window, gpointer user_data)
{
    g_return_if_fail(VNR_IS_WINDOW(window) || !GTK_IS_WIDGET(user_data));

    VnrFile *current = window_list_get_current(window);
    if (!current)
        return;

    GFile *file = g_file_new_for_path((gchar*) current->path);
    GList *files = g_list_append(NULL, file);

    GtkWidget *item = GTK_WIDGET(user_data);
    GAppInfo *app = g_object_get_data(G_OBJECT(item), "app");
    if (app)
        g_app_info_launch(app, files, NULL, NULL);

    g_object_unref(file);
    g_list_free(files);
}


// ----------------------------------------------------------------------------

gboolean window_prev(VnrWindow *window)
{
    GList *prev;

    /* Don't reload current image
     * if the list contains only one(or no) image */
    if (g_list_length(g_list_first(window->filelist)) < 2)
        return FALSE;

    if (window->mode == WINDOW_MODE_SLIDESHOW)
        g_source_remove(window->sl_source_tag);

    prev = g_list_previous(window->filelist);
    if (prev == NULL)
    {
        prev = g_list_last(window->filelist);
    }

    window->filelist = prev;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_flush();

    window_file_load(window, FALSE);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    if (window->mode == WINDOW_MODE_SLIDESHOW)
    {
        window->sl_source_tag =
            g_timeout_add_seconds(window->sl_timeout,
                                  (GSourceFunc) _window_next_image_src,
                                  window);
    }

    return TRUE;
}

gboolean window_next(VnrWindow *window, gboolean rem_timeout)
{
    GList *next;

    // don't reload current image if the list contains only one(or no) image

    if (g_list_length(g_list_first(window->filelist)) < 2)
        return FALSE;

    if (window->mode == WINDOW_MODE_SLIDESHOW && rem_timeout)
        g_source_remove(window->sl_source_tag);

    next = g_list_next(window->filelist);
    if (next == NULL)
    {
        next = g_list_first(window->filelist);
    }

    window->filelist = next;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_flush();

    window_file_load(window, FALSE);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    if (window->mode == WINDOW_MODE_SLIDESHOW && rem_timeout)
    {
        window->sl_source_tag =
            g_timeout_add_seconds(window->sl_timeout,
                                  (GSourceFunc) _window_next_image_src,
                                  window);
    }

    return TRUE;
}

static gboolean _window_next_image_src(VnrWindow *window)
{
    if (g_list_length(g_list_first(window->filelist)) <= 1)
        return FALSE;
    else
        window_next(window, FALSE);

    window->sl_source_tag =
        g_timeout_add_seconds(window->sl_timeout,
                              (GSourceFunc) _window_next_image_src,
                              window);

    return FALSE;
}

gboolean window_first(VnrWindow *window)
{
    GList *prev = g_list_first(window->filelist);

    if (vnr_message_area_is_critical(VNR_MESSAGE_AREA(window->msg_area)))
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));
    }

    window->filelist = prev;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_flush();

    window_file_load(window, FALSE);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    return TRUE;
}

gboolean window_last(VnrWindow *window)
{
    GList *prev = g_list_last(window->filelist);

    if (vnr_message_area_is_critical(VNR_MESSAGE_AREA(window->msg_area)))
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));
    }

    window->filelist = prev;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_flush();

    window_file_load(window, FALSE);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    return TRUE;
}

static void _window_action_rename(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    VnrFile *current = window_list_get_current(window);

    if (!current || window->mode != WINDOW_MODE_NORMAL)
        return;

    gboolean result = dialog_file_rename(GTK_WINDOW(window), current);

    if (!result)
        return;

    vnr_list_sort(window->filelist);
    _view_on_zoom_changed(UNI_IMAGE_VIEW(window->view), window);
}

static void _window_action_move_to(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    if (window_list_get_current(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    _window_select_directory(window);

    if (window->movedir == NULL)
        return;

    _window_move(window);
}

static void _window_action_move(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    if (window_list_get_current(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    if (window->movedir == NULL)
        _window_select_directory(window);

    if (window->movedir == NULL)
        return;

    _window_move(window);
}

static void _window_move(VnrWindow *window)
{
    g_return_if_fail(window != NULL);

    VnrFile *current = window_list_get_current(window);
    if (!current || window->mode != WINDOW_MODE_NORMAL)
        return;

    const gchar *display_name = current->display_name;
    gchar *newpath = g_build_filename(window->movedir, display_name, NULL);

    if (g_strcmp0(current->path, newpath) == 0)
        goto cleanup;

    //printf("move %s to %s\n", file->path, newpath);

    gboolean ret = vnr_file_rename(current, newpath);

    if (ret)
    {
        _window_delete_item(window);

        window_file_close(window);
        window_file_load(window, FALSE);
    }

cleanup:

    g_free(newpath);
}

static gboolean _window_select_directory(VnrWindow *window)
{
    g_return_val_if_fail(window != NULL, false);

    GSList *list = _window_file_chooser(window,
                                        "bla",
                                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                        false);
    if (!list)
        return false;

    if (window->movedir)
    {
        g_free(window->movedir);
        window->movedir = NULL;
    }

    window->movedir = g_strdup((const char*) list->data);
    g_slist_free_full(list, g_free);

    return true;
}

static GSList* _window_file_chooser(VnrWindow *window,
                                    const gchar *title,
                                    GtkFileChooserAction action,
                                    gboolean multiple)
{
    // https://docs.gtk.org/gtk3/class.FileChooserDialog.html

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
                                title,
                                GTK_WINDOW(window),
                                action,
                                "gtk-cancel", GTK_RESPONSE_CANCEL,
                                "gtk-open", GTK_RESPONSE_ACCEPT,
                                NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_show_hidden(chooser, window->prefs->show_hidden);
    gtk_file_chooser_set_select_multiple(chooser, multiple);

    VnrFile *current = window_list_get_current(window);
    if (current)
    {
        gchar *dirname = g_path_get_dirname(current->path);
        gtk_file_chooser_set_current_folder(chooser, dirname);
        g_free(dirname);
    }

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));

    GSList *uri_list = NULL;

    if (res == GTK_RESPONSE_ACCEPT)
        uri_list = gtk_file_chooser_get_filenames(chooser);

    gtk_widget_destroy(dialog);

    return uri_list;
}

static void _window_action_delete(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    VnrFile *current = window_list_get_current(window);

    if (!current || window->mode != WINDOW_MODE_NORMAL)
        return;

    gboolean restart_slideshow = FALSE;

    if (window->mode == WINDOW_MODE_SLIDESHOW)
    {
        _window_slideshow_stop(window);
        restart_slideshow = TRUE;
    }

    gboolean cursor_was_hidden = FALSE;
    if (window->cursor_is_hidden)
    {
        cursor_was_hidden = TRUE;
        _window_show_cursor(window);
    }

    window->disable_autohide = TRUE;
    gboolean restart_autohide_timeout = FALSE;

    if (window->fs_source != NULL)
        restart_autohide_timeout = TRUE;

    const gchar *file_path = current->path;

    gchar *prompt = NULL;
    gchar *markup = NULL;
    GtkWidget *dlg = NULL;

    if (window->prefs->confirm_delete)
    {
        gchar *warning = NULL;
        warning = _("If you delete an item, it will be permanently lost.");

        /* I18N: The '%s' is replaced with the name of the file to be deleted. */
        prompt = g_strdup_printf(_("Are you sure you want to\n"
                                 "permanently delete \"%s\"?"),
                                 current->display_name);
        markup = g_markup_printf_escaped("<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s",
                                         prompt, warning);

        dlg = gtk_message_dialog_new(GTK_WINDOW(window),
                                     GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_WARNING,
                                     GTK_BUTTONS_NONE,
                                     NULL);

        gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dlg),
                                      markup);

        gtk_dialog_add_buttons(GTK_DIALOG(dlg),
                               "gtk-cancel", GTK_RESPONSE_CANCEL,
                               "gtk-delete", GTK_RESPONSE_YES,
                               NULL);
    }

    if (!window->prefs->confirm_delete
        || gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_YES)
    {
        GFile *file = g_file_new_for_path(file_path);

        GError *error = NULL;
        g_file_trash(file, NULL, &error);

        if (error != NULL)
        {
            vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area), TRUE,
                                  error->message, FALSE);
            restart_slideshow = FALSE;
        }
        else
        {
            gboolean ret = _window_delete_item(window);

            if (!ret)
            {
                restart_slideshow = FALSE;
            }
            else
            {
                if (window->prefs->confirm_delete && !window->cursor_is_hidden)
                    vnr_tools_set_cursor(GTK_WIDGET(dlg), GDK_WATCH, true);

                //gdk_flush();

                window_file_close(window);
                window_file_load(window, FALSE);

                if (window->prefs->confirm_delete && !window->cursor_is_hidden)
                    vnr_tools_set_cursor(GTK_WIDGET(dlg), GDK_LEFT_PTR, false);
            }
        }
    }

    window->disable_autohide = FALSE;

    if (restart_slideshow)
        _window_slideshow_start(window);

    if (cursor_was_hidden)
        _window_hide_cursor(window);

    if (restart_autohide_timeout)
        _window_fullscreen_set_timeout(window);

    if (window->prefs->confirm_delete)
    {
        g_free(prompt);
        g_free(markup);
        gtk_widget_destroy(dlg);
    }
}

static gboolean _window_delete_item(VnrWindow *window)
{
    GList *next = vnr_list_delete_item(window->filelist);
    window->filelist = NULL; // ensure we won't free it

    if (next == NULL)
    {
        window_file_close(window);

        //gtk_action_group_set_sensitive(window->actions_collection, FALSE);

        window_slideshow_deny(window);

        window_list_set(window, NULL);

        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area), TRUE,
                              _("The given locations contain no images."),
                              TRUE);

//        restart_slideshow = FALSE;

        if (gtk_widget_get_visible(window->props_dlg))
            vnr_properties_dialog_clear(VNR_PROPERTIES_DIALOG(window->props_dlg));

        return false;
    }

    window_list_set(window, next);

    return true;
}

static void _window_hide_cursor(VnrWindow *window)
{
    vnr_tools_set_cursor(GTK_WIDGET(window), GDK_BLANK_CURSOR, true);
    window->cursor_is_hidden = TRUE;
}

static void _window_show_cursor(VnrWindow *window)
{
    vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, true);
    window->cursor_is_hidden = FALSE;
}

static void _window_action_properties(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    if (window_list_get_current(window) == NULL
        || window->mode != WINDOW_MODE_NORMAL)
        return;

    vnr_properties_dialog_show(VNR_PROPERTIES_DIALOG(window->props_dlg));
}

static void _window_action_preferences(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;
    g_return_if_fail(window != NULL);

    if (window->mode != WINDOW_MODE_NORMAL)
        return;

    vnr_prefs_show_dialog(window->prefs);
}


// Pixbuf ---------------------------------------------------------------------

static void _window_rotate_pixbuf(VnrWindow *window,
                                  GdkPixbufRotation angle)
{
    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_display_flush(display);

    // Stop slideshow while editing the image
    _window_slideshow_stop(window);

    GdkPixbuf *result = gdk_pixbuf_rotate_simple(
                            UNI_IMAGE_VIEW(window->view)->pixbuf,
                            angle);

    if (result == NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE, _("Not enough virtual memory."),
                              FALSE);
        return;
    }

    uni_anim_view_set_static(UNI_ANIM_VIEW(window->view), result);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    g_object_unref(result);

    window->current_image_width = gdk_pixbuf_get_width(result);
    window->current_image_height = gdk_pixbuf_get_height(result);

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update_image(VNR_PROPERTIES_DIALOG(window->props_dlg));

    /* Extra conditions. Rotating 180 degrees is also flipping horizontal and vertical */
    if ((window->modifications & (4)) ^ ((angle == GDK_PIXBUF_ROTATE_CLOCKWISE) << 2))
        window->modifications ^= 3;

    window->modifications ^= 4;

    //gtk_action_group_set_sensitive(window->action_save, window->modifications);

    if (window->modifications == 0 && window->prefs->behavior_modify != VNR_PREFS_MODIFY_IGNORE)
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));
        return;
    }

    if (window->writable_format_name == NULL)
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE,
                              _("Image modifications cannot be saved.\nWriting in this format is not supported."),
                              FALSE);
    else if (window->prefs->behavior_modify == VNR_PREFS_MODIFY_SAVE)
        _window_action_save_image(window, NULL);
    else if (window->prefs->behavior_modify == VNR_PREFS_MODIFY_ASK)
        vnr_message_area_show_with_button(VNR_MESSAGE_AREA(window->msg_area),
                                          FALSE,
                                          _("Save modifications?\nThis will overwrite the image and may reduce its quality!"),
                                          FALSE, "gtk-save",
                                          G_CALLBACK(_window_action_save_image));
}

static void _window_flip_pixbuf(VnrWindow *window, gboolean horizontal)
{
    if (!window->can_edit)
        return;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_flush();

    GdkPixbuf *result = gdk_pixbuf_flip(
                                UNI_IMAGE_VIEW(window->view)->pixbuf,
                                horizontal);

    if (result == NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE, _("Not enough virtual memory."),
                              FALSE);
        return;
    }

    uni_anim_view_set_static(UNI_ANIM_VIEW(window->view), result);

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update_image(VNR_PROPERTIES_DIALOG(window->props_dlg));

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    g_object_unref(result);

    /* Extra conditions. Rotating 180 degrees is also flipping horizontal and vertical */
    window->modifications ^= (window->modifications & 4) ? 1 + horizontal : 2 - horizontal;

    //gtk_action_group_set_sensitive(window->action_save,
    //                               window->modifications);

    if (window->modifications == 0)
    {
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));
        return;
    }

    if (window->writable_format_name == NULL)
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE,
                              _("Image modifications cannot be saved.\nWriting in this format is not supported."),
                              FALSE);
    else if (window->prefs->behavior_modify == VNR_PREFS_MODIFY_SAVE)
        _window_action_save_image(window, NULL);
    else if (window->prefs->behavior_modify == VNR_PREFS_MODIFY_ASK)
        vnr_message_area_show_with_button(VNR_MESSAGE_AREA(window->msg_area),
                                          FALSE,
                                          _("Save modifications?\nThis will overwrite the image and may reduce its quality!"),
                                          FALSE, "gtk-save",
                                          G_CALLBACK(_window_action_save_image));
}

static void _window_action_save_image(VnrWindow *window, GtkWidget *widget)
{
    (void) widget;

    VnrFile *current = window_list_get_current(window);
    if (!current)
        return;

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_WATCH, true);

    //gdk_flush();

    if (window->prefs->behavior_modify == VNR_PREFS_MODIFY_ASK)
        vnr_message_area_hide(VNR_MESSAGE_AREA(window->msg_area));

    /* Store exiv2 metadata to cache, so we can restore it afterwards */
    uni_read_exiv2_to_cache(current->path);

    GError *error = NULL;

    if (g_strcmp0(window->writable_format_name, "jpeg") == 0)
    {
        gchar *quality;
        quality = g_strdup_printf("%i", window->prefs->jpeg_quality);

        gdk_pixbuf_save(uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view)),
                        current->path, "jpeg",
                        &error, "quality", quality, NULL);
        g_free(quality);
    }
    else if (g_strcmp0(window->writable_format_name, "png") == 0)
    {
        gchar *compression;
        compression = g_strdup_printf("%i", window->prefs->png_compression);

        gdk_pixbuf_save(uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view)),
                        current->path, "png",
                        &error, "compression", compression, NULL);
        g_free(compression);
    }
    else
    {
        gdk_pixbuf_save(uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view)),
                        current->path,
                        window->writable_format_name, &error, NULL);
    }

    uni_write_exiv2_from_cache(current->path);

    if (!window->cursor_is_hidden)
        vnr_tools_set_cursor(GTK_WIDGET(window), GDK_LEFT_PTR, false);

    if (error != NULL)
    {
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area), TRUE,
                              error->message, FALSE);
        return;
    }

    if (window->prefs->reload_on_save)
    {
        window_file_load(window, FALSE);
        return;
    }

    window->modifications = 0;

    //gtk_action_group_set_sensitive(window->action_save, FALSE);

    if (window->prefs->behavior_modify != VNR_PREFS_MODIFY_ASK)
        _view_on_zoom_changed(UNI_IMAGE_VIEW(window->view), window);

    if (gtk_widget_get_visible(window->props_dlg))
        vnr_properties_dialog_update(VNR_PROPERTIES_DIALOG(window->props_dlg));
}


// Fullscreen -----------------------------------------------------------------

void window_fullscreen_toggle(VnrWindow *window)
{
    if (window->mode == WINDOW_MODE_NORMAL)
        _window_fullscreen(window);
    else
        _window_unfullscreen(window);
}

static void _window_fullscreen(VnrWindow *window)
{
    if (window_list_get_current(window) == NULL)
        return;

    gtk_window_fullscreen(GTK_WINDOW(window));

    window->mode = WINDOW_MODE_FULLSCREEN;

    GdkRGBA color;
    gdk_rgba_parse(&color, "black");

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_widget_override_background_color(window->view,
                                         GTK_STATE_FLAG_NORMAL,
                                         &color);
    G_GNUC_END_IGNORE_DEPRECATIONS

    if (window->prefs->fit_on_fullscreen)
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view),
                                     VNR_PREFS_ZOOM_FIT);

    _window_update_fs_filename_label(window);

    //gtk_widget_hide(window->statusbar);
    //gtk_widget_show(window->properties_button);

    if (window->fs_toolitem)
        gtk_widget_show(window->fs_toolitem);

    _window_slideshow_stop(window);

    /* Reset timeouts for the toolbar autohide when the mouse
     * moves over the UniImageviewer.
     * "after" because it must be called after the uniImageView's
     * callback(when the image is dragged).*/
    g_signal_connect_after(window->view,
                           "motion-notify-event",
                           G_CALLBACK(_on_fullscreen_motion),
                           window);

    g_signal_connect(window->msg_area,
                     "enter-notify-event",
                     G_CALLBACK(_on_leave_image_area),
                     window);

    _window_fullscreen_set_timeout(window);
}

static void _window_unfullscreen(VnrWindow *window)
{
    if (window->mode == WINDOW_MODE_NORMAL)
        return;

    _window_slideshow_stop(window);
    window->mode = WINDOW_MODE_NORMAL;

    gtk_window_unfullscreen(GTK_WINDOW(window));

    if (window->prefs->dark_background)
    {
        GdkRGBA color;
        gdk_rgba_parse(&color, DARK_BACKGROUND_COLOR);

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gtk_widget_override_background_color(window->view,
                                             GTK_STATE_FLAG_NORMAL,
                                             &color);
        G_GNUC_END_IGNORE_DEPRECATIONS
    }
    else
    {
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gtk_widget_override_background_color(window->view,
                                             GTK_STATE_FLAG_NORMAL, NULL);
        G_GNUC_END_IGNORE_DEPRECATIONS
    }

    if (window->prefs->fit_on_fullscreen)
    {
        uni_image_view_set_zoom_mode(UNI_IMAGE_VIEW(window->view),
                                     window->prefs->zoom);
    }

    if (window->fs_toolitem)
        gtk_widget_hide(window->fs_toolitem);

    //if (!window->prefs->show_statusbar)
    //    gtk_widget_hide(window->statusbar);
    //else
    //    gtk_widget_show(window->statusbar);

    g_signal_handlers_disconnect_by_func(window->view,
                                         G_CALLBACK(_on_fullscreen_motion),
                                         window);

    g_signal_handlers_disconnect_by_func(window->msg_area,
                                         G_CALLBACK(_on_leave_image_area),
                                         window);

    _window_fullscreen_unset_timeout(window);
    _window_show_cursor(window);
}

static gboolean _on_fullscreen_motion(GtkWidget *widget,
                                      GdkEventMotion *ev,
                                      VnrWindow *window)
{
    if (window->disable_autohide)
        return FALSE;

    if (window->cursor_is_hidden)
        _window_show_cursor(window);

    _window_fullscreen_set_timeout(window);

    return FALSE;
}

static void _window_fullscreen_set_timeout(VnrWindow *window)
{
    _window_fullscreen_unset_timeout(window);

    window->fs_source = g_timeout_source_new(FULLSCREEN_TIMEOUT);
    g_source_set_callback(window->fs_source,
                          (GSourceFunc)_on_fullscreen_timeout,
                          window, NULL);

    g_source_attach(window->fs_source, NULL);
}

static gboolean _on_fullscreen_timeout(VnrWindow *window)
{
    // hides the toolbar
    _window_fullscreen_unset_timeout(window);

    if (window->disable_autohide)
        return FALSE;

    _window_hide_cursor(window);

    return FALSE;
}

static void _window_fullscreen_unset_timeout(VnrWindow *window)
{
    if (window->fs_source != NULL)
    {
        g_source_unref(window->fs_source);
        g_source_destroy(window->fs_source);
        window->fs_source = NULL;
    }
}

static gboolean _on_leave_image_area(GtkWidget *widget,
                                     GdkEventCrossing *ev,
                                     VnrWindow *window)
{
    _window_fullscreen_unset_timeout(window);
    return FALSE;
}


// ----------------------------------------------------------------------------

#if 0
static GtkWidget* _window_get_fs_toolitem(VnrWindow *window)
{
    if (window->fs_toolitem != NULL)
        return window->fs_toolitem;

    // Tool item, that contains the hbox
    GtkToolItem *item = gtk_tool_item_new();
    gtk_tool_item_set_expand(item, TRUE);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(item), box);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    GtkWidget *widget = gtk_button_new_from_stock("gtk-leave-fullscreen");
    G_GNUC_END_IGNORE_DEPRECATIONS

    g_signal_connect(widget, "clicked",
                     G_CALLBACK(_on_fullscreen_leave), window);
    gtk_box_pack_end(GTK_BOX(box), widget, FALSE, FALSE, 0);

    // create label for the current image's filename
    widget = gtk_label_new(NULL);
    gtk_label_set_ellipsize(GTK_LABEL(widget), PANGO_ELLIPSIZE_END);
    gtk_label_set_selectable(GTK_LABEL(widget), TRUE);
    window->fs_filename_label = widget;
    gtk_box_pack_end(GTK_BOX(box), widget, TRUE, TRUE, 10);

    widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

    widget = gtk_check_button_new_with_label(_("Show next image after: "));
    g_signal_connect(widget, "toggled",
                     G_CALLBACK(_on_toggle_show_next), window);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    window->toggle_btn = widget;

    // create spin button to adjust slideshow's timeout
    GtkAdjustment *spinner_adj =
        (GtkAdjustment*) gtk_adjustment_new(
                                    window->prefs->slideshow_timeout,
                                    1.0, 30.0, 1.0, 1.0, 0);

    widget = gtk_spin_button_new(spinner_adj, 1.0, 0);
    gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(widget), TRUE);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(widget),
                                      GTK_UPDATE_ALWAYS);
    g_signal_connect(widget, "value-changed",
                     G_CALLBACK(_on_spin_value_change), window);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
    window->sl_timeout_widget = widget;

    window->fs_seconds_label = gtk_label_new(ngettext(" second", " seconds", 5));
    gtk_box_pack_start(GTK_BOX(box), window->fs_seconds_label, FALSE, FALSE, 0);

    window->fs_toolitem = GTK_WIDGET(item);

    gtk_widget_show_all(window->fs_toolitem);

    return window->fs_toolitem;
}

static void _on_fullscreen_leave(GtkButton *button, VnrWindow *window)
{
    _window_unfullscreen(window);
}

static void _on_spin_value_change(GtkSpinButton *spinbutton,
                                  VnrWindow *window)
{
    int new_value = gtk_spin_button_get_value_as_int(spinbutton);

    if (new_value != window->prefs->slideshow_timeout)
        vnr_prefs_set_slideshow_timeout(window->prefs, new_value);

    gtk_label_set_text(GTK_LABEL(window->fs_seconds_label),
                       ngettext(" second", " seconds", new_value));
    window->sl_timeout = new_value;
    _window_slideshow_restart(window);
}

static void _on_toggle_show_next(GtkToggleButton *togglebutton,
                                 VnrWindow *window)
{
    if (!window->slideshow)
        return;

    if (window->mode == WINDOW_MODE_FULLSCREEN)
        _window_slideshow_start(window);
    else if (window->mode == WINDOW_MODE_SLIDESHOW)
        _window_slideshow_stop(window);
}

#endif


// ----------------------------------------------------------------------------

static void _window_update_fs_filename_label(VnrWindow *window)
{
    VnrFile *current = window_list_get_current(window);

    if (!current || window->mode == WINDOW_MODE_NORMAL)
        return;

    gint total = 0;
    gint position = vnr_list_get_position(window->filelist, &total);

    char *buf = g_strdup_printf("%s - %i/%i",
                                current->display_name,
                                position,
                                total);

    if (window->fs_toolitem)
        gtk_label_set_text(GTK_LABEL(window->fs_filename_label), buf);

    g_free(buf);
}


// ----------------------------------------------------------------------------

static void _window_action_slideshow(VnrWindow *window)
{
    g_return_if_fail(window != NULL);

    if (!window->slideshow)
        return;

    VnrFile *current = window_list_get_current(window);
    if (!current)
        return;

    if (window->mode != WINDOW_MODE_SLIDESHOW)
    {
        // Uncomment to force Fullscreen along with Slideshow
        if (window->mode == WINDOW_MODE_NORMAL)
            _window_fullscreen(window);

        _window_slideshow_start(window);
    }
    else if (window->mode == WINDOW_MODE_SLIDESHOW)
    {
        // Uncomment to force Fullscreen along with Slideshow
        _window_unfullscreen(window);

        _window_slideshow_stop(window);
    }
}

static void _window_slideshow_start(VnrWindow *window)
{
    if (!window->slideshow)
        return;

    if (window->mode == WINDOW_MODE_SLIDESHOW)
        return;

    window->mode = WINDOW_MODE_SLIDESHOW;

    window->sl_source_tag =
        g_timeout_add_seconds(window->sl_timeout,
                              (GSourceFunc) _window_next_image_src,
                              window);


    window->slideshow = FALSE;

    if (window->fs_toolitem)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window->toggle_btn),
                                     TRUE);
    }

    //GtkAction *action = gtk_action_group_get_action(
    //                            window->actions_collection,
    //                            "ViewSlideshow");
    //gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), TRUE);

    window->slideshow = TRUE;
}

static void _window_slideshow_stop(VnrWindow *window)
{
    if (!window->slideshow)
        return;

    if (window->mode != WINDOW_MODE_SLIDESHOW)
        return;

    window->slideshow = FALSE;

    if (window->fs_toolitem)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window->toggle_btn),
                                     FALSE);
    }

    //GtkAction *action = gtk_action_group_get_action(
    //                            window->actions_collection,
    //                            "ViewSlideshow");
    //gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action), FALSE);

    window->slideshow = TRUE;

    window->mode = WINDOW_MODE_FULLSCREEN;

    g_source_remove(window->sl_source_tag);
}

#if 0
static void _window_slideshow_restart(VnrWindow *window)
{
    if (!window->slideshow)
        return;

    if (window->mode != WINDOW_MODE_SLIDESHOW)
        return;

    g_source_remove(window->sl_source_tag);
    window->sl_source_tag =
        g_timeout_add_seconds(
                        window->sl_timeout,
                        (GSourceFunc) _window_next_image_src,
                        window);
}
#endif

static void _window_slideshow_allow(VnrWindow *window)
{
    if (window->slideshow)
        return;

    window->slideshow = TRUE;

    if (window->fs_toolitem)
        gtk_widget_set_sensitive(window->toggle_btn, TRUE);
}

void window_slideshow_deny(VnrWindow *window)
{
    if (!window->slideshow)
        return;

    window->slideshow = FALSE;

    if (window->fs_toolitem)
        gtk_widget_set_sensitive(window->toggle_btn, FALSE);
}


// private signal handlers ----------------------------------------------------

static void _action_resize(GtkToggleAction *action, VnrWindow *window)
{
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS

    if (action != NULL && !gtk_toggle_action_get_active(action))
    {
        window->prefs->auto_resize = FALSE;
        return;
    }

    G_GNUC_END_IGNORE_DEPRECATIONS

    gint img_h, img_w; /* Width and Height of the pixbuf */

    img_w = window->current_image_width;
    img_h = window->current_image_height;

    if (img_w == 0 || img_h == 0)
        return;

    window->prefs->auto_resize = TRUE;

    vnr_tools_fit_to_size(&img_w, &img_h, window->max_width, window->max_height);
    gtk_window_resize(GTK_WINDOW(window),
                      img_w,
                      img_h /*+ _window_get_top_widgets_height(window)*/);
}

static void _action_crop(GtkAction *action, VnrWindow *window)
{
    VnrCrop *crop;

//    if (!gtk_action_group_get_sensitive(window->actions_static_image))
//        return;

    crop = (VnrCrop *)vnr_crop_new(window);

    if (!vnr_crop_run(crop))
    {
        g_object_unref(crop);
        return;
    }

    GdkPixbuf *cropped;
    GdkPixbuf *original;

    original = uni_image_view_get_pixbuf(UNI_IMAGE_VIEW(window->view));

    cropped = gdk_pixbuf_new(gdk_pixbuf_get_colorspace(original),
                             gdk_pixbuf_get_has_alpha(original),
                             gdk_pixbuf_get_bits_per_sample(original),
                             crop->area.width, crop->area.height);

    gdk_pixbuf_copy_area((const GdkPixbuf *)original, crop->area.x, crop->area.y,
                         crop->area.width, crop->area.height, cropped, 0, 0);

    uni_anim_view_set_static(UNI_ANIM_VIEW(window->view), cropped);

    g_object_unref(cropped);

    window->modifications |= 8;

    window->current_image_width = crop->area.width;
    window->current_image_height = crop->area.height;

    //gtk_action_group_set_sensitive(window->action_save, TRUE);

    if (window->writable_format_name == NULL)
        vnr_message_area_show(VNR_MESSAGE_AREA(window->msg_area),
                              TRUE,
                              _("Image modifications cannot be saved.\nWriting in this format is not supported."),
                              FALSE);
    else if (window->prefs->behavior_modify == VNR_PREFS_MODIFY_SAVE)
        _window_action_save_image(window, NULL);
    else if (window->prefs->behavior_modify == VNR_PREFS_MODIFY_ASK)
        vnr_message_area_show_with_button(VNR_MESSAGE_AREA(window->msg_area),
                                          FALSE,
                                          _("Save modifications?\nThis will overwrite the image and may reduce its quality!"),
                                          FALSE, "gtk-save",
                                          G_CALLBACK(_window_action_save_image));

    g_object_unref(crop);
}


// ----------------------------------------------------------------------------

static void _on_file_open_dialog_response(GtkWidget *dialog,
                                          gint response_id,
                                          VnrWindow *window)
{
    if (response_id == GTK_RESPONSE_ACCEPT)
    {
        GSList *uri_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        g_return_if_fail(uri_list != NULL);
        window_open_list(window, uri_list);
        g_slist_free_full(uri_list, g_free);
    }

    gtk_widget_destroy(dialog);
}

static gboolean _file_size_is_small(char *filename)
{

    struct stat st;
    int four_mb = 4 * 1024 * 1024;

    if (filename != NULL && stat(filename, &st) == 0)
    {
        return st.st_size < four_mb;
    }
    return FALSE;
}

static void _on_update_preview(GtkFileChooser *file_chooser, gpointer data)
{
    GtkWidget *preview = GTK_WIDGET(data);
    char *filename = gtk_file_chooser_get_preview_filename(file_chooser);
    gboolean has_preview = FALSE;

    if (_file_size_is_small(filename))
    {
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(filename, 256, 256, NULL);
        has_preview = pixbuf != NULL;

        gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
        if (pixbuf)
        {
            g_object_unref(pixbuf);
        }
    }
    gtk_file_chooser_set_preview_widget_active(file_chooser, has_preview);
    if (filename != NULL)
    {
        g_free(filename);
    }
}

void window_preferences_apply(VnrWindow *window)
{
    if (window->prefs->dark_background)
    {
        // https://stackoverflow.com/questions/36520637/
        GdkRGBA color;
        gdk_rgba_parse(&color, DARK_BACKGROUND_COLOR);

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS

        gtk_widget_override_background_color(window->view,
                                             GTK_STATE_FLAG_NORMAL,
                                             &color);

        G_GNUC_END_IGNORE_DEPRECATIONS
    }

    if (window->prefs->smooth_images
        && UNI_IMAGE_VIEW(window->view)->interp != GDK_INTERP_BILINEAR)
    {
        UNI_IMAGE_VIEW(window->view)->interp = GDK_INTERP_BILINEAR;
        gtk_widget_queue_draw(window->view);
    }
    else if (!window->prefs->smooth_images
             && UNI_IMAGE_VIEW(window->view)->interp != GDK_INTERP_NEAREST)
    {
        UNI_IMAGE_VIEW(window->view)->interp = GDK_INTERP_NEAREST;
        gtk_widget_queue_draw(window->view);
    }

    if (window->fs_toolitem)
    {
        gint val = gtk_spin_button_get_value_as_int(
                        GTK_SPIN_BUTTON(window->sl_timeout_widget));

        if (val != window->prefs->slideshow_timeout)
        {
            gtk_spin_button_set_value(
                        GTK_SPIN_BUTTON(window->sl_timeout_widget),
                        (gdouble) window->prefs->slideshow_timeout);
        }
    }
}


// Dnd ------------------------------------------------------------------------

static void _window_drag_data_received(GtkWidget *widget,
                                       GdkDragContext *context,
                                       gint x, gint y,
                                       GtkSelectionData *selection_data,
                                       guint info, guint time)
{
    GSList *uri_list = NULL;
    GdkAtom target = gtk_selection_data_get_target(selection_data);
    GdkDragAction suggested_action;

    if (!gtk_targets_include_uri(&target, 1))
        return;

    if (gtk_drag_get_source_widget(context))
        return;

    suggested_action = gdk_drag_context_get_suggested_action(context);
    if (suggested_action == GDK_ACTION_COPY || suggested_action == GDK_ACTION_ASK)
    {
        const guchar *data = gtk_selection_data_get_data(selection_data);

        uri_list = vnr_tools_parse_uri_string_list_to_file_list((gchar *)data);
        if (uri_list == NULL)
        {
            window_file_close(VNR_WINDOW(widget));

            //gtk_action_group_set_sensitive(VNR_WINDOW(widget)->actions_collection, FALSE);

            window_slideshow_deny(VNR_WINDOW(widget));
            vnr_message_area_show(VNR_MESSAGE_AREA(VNR_WINDOW(widget)->msg_area), TRUE,
                                  _("The given locations contain no images."),
                                  TRUE);
            return;
        }

        window_open_list(VNR_WINDOW(widget), uri_list);
    }
}

static void _window_set_drag(VnrWindow *window)
{
    gtk_drag_dest_set(GTK_WIDGET(window),
                      GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP,
                      NULL, 0,
                      GDK_ACTION_COPY | GDK_ACTION_ASK);
    gtk_drag_dest_add_uri_targets(GTK_WIDGET(window));
}

static void _view_on_drag_begin(GtkWidget *widget,
                                  GdkDragContext *drag_context,
                                  GtkSelectionData *data,
                                  guint info,
                                  guint time,
                                  gpointer user_data)
{
    (void) widget;
    (void) drag_context;
    (void) info;
    (void) time;

    VnrFile *current = window_list_get_current(VNR_WINDOW(user_data));
    if (!current)
        return;

    gchar *uris[2];

    uris[0] = g_filename_to_uri((gchar*) current->path, NULL, NULL);
    uris[1] = NULL;

    gtk_selection_data_set_uris(data, uris);

    g_free(uris[0]);
}


