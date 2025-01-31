/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of Viewnior.
 *
 * Viewnior is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Viewnior is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Viewnior.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VNR_WINDOW_H__
#define __VNR_WINDOW_H__

#include "preferences.h"
#include "file.h"

G_BEGIN_DECLS

typedef struct _VnrWindow VnrWindow;

#define VNR_TYPE_WINDOW (window_get_type())
G_DECLARE_FINAL_TYPE(VnrWindow, window, VNR, WINDOW, GtkWindow)

typedef enum
{
    WINDOW_MODE_NORMAL,
    WINDOW_MODE_FULLSCREEN,
    WINDOW_MODE_SLIDESHOW,
} WindowMode;

struct _VnrWindow
{
    GtkWindow __parent__;

    // Interface
    GtkUIManager *ui_manager;

    GtkWidget *layout;
    GtkWidget *button_menu;
    GtkWidget *toolbar;
    GtkWidget *statusbar;

    GtkWidget *properties_button;
    GtkWidget *popup_menu;
    GtkWidget *msg_area;
    GtkWidget *props_dlg;

    GtkWidget *view;
    GtkWidget *scroll_view;

    // Parameters
    VnrPrefs *prefs;

    gint max_width;
    gint max_height;
    gchar *writable_format_name;

    gint current_image_height;
    gint current_image_width;

    WindowMode mode;
    guint8 modifications;
    gboolean cursor_is_hidden;

    GtkActionGroup *actions_window;
    GtkActionGroup *actions_image;
    GtkActionGroup *actions_static_image;
    GtkActionGroup *actions_collection;
    GtkActionGroup *action_save;
    GtkActionGroup *action_properties;
    GtkActionGroup *actions_bars;
    GtkActionGroup *actions_open_with;
    GtkActionGroup *action_wallpaper;

    guint open_with_menu_id;

    // Data
    gchar *movedir;
    GList *filelist;

    // Fullscreen (fs) variables
    GtkWidget *fs_controls;
    GtkWidget *toggle_btn;
    GtkWidget *fs_seconds_label;
    GtkWidget *fs_filename_label;
    GSource *fs_source;
    gboolean disable_autohide;

    // Slideshow (sl) variables
    gboolean slideshow;
    guint sl_source_tag;
    gint sl_timeout;
    GtkWidget *sl_timeout_widget;
};

//struct _VnrWindowClass
//{
//    GtkWindowClass parent_class;
//};

GType window_get_type() G_GNUC_CONST;

// Constructor
GtkWindow* window_new();

// Actions
gboolean window_open(VnrWindow *window, gboolean fit_to_screen);
void window_open_from_list(VnrWindow *window, GSList *uri_list);
void window_close(VnrWindow *window);

void window_list_set(VnrWindow *window, GList *list);
VnrFile *window_list_get_current(VnrWindow *window);
gboolean window_next(VnrWindow *window, gboolean rem_timeout);
gboolean window_prev(VnrWindow *window);
gboolean window_first(VnrWindow *window);
gboolean window_last(VnrWindow *window);

void window_preferences_apply(VnrWindow *window);
void window_fullscreen_toggle(VnrWindow *window);
void window_slideshow_deny(VnrWindow *window);

G_END_DECLS

#endif // __VNR_WINDOW_H__


