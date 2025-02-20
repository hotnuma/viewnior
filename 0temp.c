
#if 0

vnr-crop.c

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gdk_screen_get_monitor_geometry(
                        screen,
                        gdk_screen_get_monitor_at_window(screen,
                        gtk_widget_get_window(GTK_WIDGET(crop->vnr_win))),
                        &geometry);
        G_GNUC_END_IGNORE_DEPRECATIONS


window.c

        G_GNUC_BEGIN_IGNORE_DEPRECATIONS

        gdk_screen_get_monitor_geometry(
                            screen,
                            gdk_screen_get_monitor_at_window(screen,
                            gtk_widget_get_window(widget)),
                            &geometry);

        G_GNUC_END_IGNORE_DEPRECATIONS


file_copy

g_autoptr(GFile) src = g_file_new_for_path(file->path);
g_autoptr(GFile) dest = g_file_new_for_path(fullpath);

gboolean ret = g_file_copy(src,
                           dest,
                           G_FILE_COPY_ALL_METADATA,
                           NULL, NULL, NULL,
                           NULL);

keycodes

case GDK_KEY_F10:
    _window_action_slideshow(window);
    result = TRUE;
    break;

case GDK_KEY_F11:
    window_fullscreen_toggle(window);
    result = TRUE;
    break;


#endif


