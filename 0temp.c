
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


#endif


