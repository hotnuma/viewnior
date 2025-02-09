
#if 0

//static void _action_about(GtkAction *action, VnrWindow *window);

static void _action_about(GtkAction *action, VnrWindow *window)
{
    static const char *authors[] =
    {
        "Programming &amp; icon design",
        "\tSiyan Panayotov",
        "\nRefer to source code from GtkImageView",
        NULL
    };

    char *license =
        ("Viewnior is free software: you can redistribute it and/or modify "
         "it under the terms of the GNU General Public License as published by "
         "the Free Software Foundation, either version 3 of the License, or "
         "(at your option) any later version.\n\n"
         "Viewnior is distributed in the hope that it will be useful, "
         "but WITHOUT ANY WARRANTY; without even the implied warranty of "
         "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
         "GNU General Public License for more details.\n\n"
         "You should have received a copy of the GNU General Public License "
         "along with Viewnior.  If not, see <http://www.gnu.org/licenses/>.\n");

    gtk_show_about_dialog(
                GTK_WINDOW(window),
                "program-name", "Viewnior",
                "version", VERSION,
                "copyright", "Copyright \xc2\xa9 2009-2018 Siyan Panayotov",
                "comments", _("Elegant Image Viewer"),
                "authors", authors,
                "logo-icon-name", "viewnior",
                "wrap-license", TRUE,
                "license", license,
                "website", "https://github.com/hotnuma/viewnior",
                "translator-credits", _("translator-credits"),
                NULL);
}

static void _action_fullscreen(GtkAction *action, VnrWindow *window);
static void _action_fullscreen(GtkAction *action, VnrWindow *window)
{
    gboolean fullscreen = gtk_toggle_action_get_active(
                GTK_TOGGLE_ACTION(action));

    if (fullscreen)
        _window_fullscreen(window);
    else
        _window_unfullscreen(window);
}

//static void _action_first(GtkAction *action, gpointer user_data);
//static void _action_last(GtkAction *action, gpointer user_data);
//static void _action_reload(GtkAction *action, VnrWindow *window);
//static void _action_set_wallpaper(GtkAction *action, VnrWindow *win);
//static void _action_scrollbar(GtkAction *action, VnrWindow *window);
//static void _action_fit(GtkAction *action, gpointer user_data);
//static void _action_zoom_in(GtkAction *action, gpointer user_data);
//static void _action_zoom_out(GtkAction *action, gpointer user_data);
//static void _action_normal_size(GtkAction *action, gpointer user_data);

static void _action_reload(GtkAction *action, VnrWindow *window)
{
    window_open(window, FALSE);
}

static void _action_zoom_in(GtkAction *action, gpointer user_data)
{
    uni_image_view_zoom_in(UNI_IMAGE_VIEW(VNR_WINDOW(user_data)->view));
}

static void _action_zoom_out(GtkAction *action, gpointer user_data)
{
    uni_image_view_zoom_out(UNI_IMAGE_VIEW(VNR_WINDOW(user_data)->view));
}

static void _action_normal_size(GtkAction *action, gpointer user_data)
{
    uni_image_view_set_zoom(UNI_IMAGE_VIEW(VNR_WINDOW(user_data)->view), 1);
    uni_image_view_set_fitting(UNI_IMAGE_VIEW(VNR_WINDOW(user_data)->view), UNI_FITTING_NONE);
}

static void _action_fit(GtkAction *action, gpointer user_data)
{
    uni_image_view_set_fitting(UNI_IMAGE_VIEW(VNR_WINDOW(user_data)->view), UNI_FITTING_FULL);
}

static void _action_first(GtkAction *action, gpointer user_data)
{
    window_first(VNR_WINDOW(user_data));
}

static void _action_last(GtkAction *action, gpointer user_data)
{
    window_last(VNR_WINDOW(user_data));
}

static void _action_set_wallpaper(GtkAction *action, VnrWindow *win)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        gchar *tmp;

        VnrPrefsDesktop desktop_environment = win->prefs->desktop;

        if (desktop_environment == VNR_PREFS_DESKTOP_AUTO)
        {
            desktop_environment = uni_detect_desktop_environment();
        }

        VnrFile *current = window_list_get_current(win);

        switch (desktop_environment)
        {

        case VNR_PREFS_DESKTOP_GNOME2:
            execlp("gconftool-2", "gconftool-2",
                   "--set", "/desktop/gnome/background/picture_filename",
                   "--type", "string",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_MATE:
            execlp("gsettings", "gsettings",
                   "set", "org.mate.background",
                   "picture-filename", current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_GNOME3:
            tmp = g_strdup_printf("file://%s", current->path);
            execlp("gsettings", "gsettings",
                   "set", "org.gnome.desktop.background",
                   "picture-uri", tmp,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_XFCE:
            tmp = g_strdup_printf(
                "/backdrop/screen%d/monitor0/workspace0/last-image",
                gdk_screen_get_number(
                    gtk_widget_get_screen(GTK_WIDGET(win))));
            execlp("xfconf-query", "xfconf-query",
                   "-c", "xfce4-desktop",
                   "-p", tmp,
                   "--type", "string",
                   "--set",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_LXDE:
            execlp("pcmanfm", "pcmanfm",
                   "--set-wallpaper",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_PUPPY:
            execlp("set_bg", "set_bg",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_FLUXBOX:
            execlp("fbsetbg", "fbsetbg",
                   "-f", current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_NITROGEN:
            execlp("nitrogen", "nitrogen",
                   "--set-zoom-fill", "--save",
                   current->path,
                   NULL);
            break;

        case VNR_PREFS_DESKTOP_CINNAMON:
            tmp = g_strdup_printf("file://%s",
                                  current->path);
            execlp("gsettings", "gsettings",
                   "set", "org.cinnamon.desktop.background",
                   "picture-uri", tmp,
                   NULL);
            break;

        default:
            _exit(0);
        }
    }
    else
    {
        wait(NULL);
    }
}

static void _action_scrollbar(GtkAction *action, VnrWindow *window)
{
    gboolean show = gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action));
    vnr_prefs_set_show_scrollbar(window->prefs, show);
    uni_scroll_win_set_show_scrollbar(UNI_SCROLL_WIN(window->scroll_view), show);
}

#endif

