
#if 0

const gchar* _ui_definition_wallpaper =
    "<ui>"
        "<popup name=\"ButtonMenu\">"
            "<menu action=\"Image\">"
                "<placeholder name=\"WallpaperEntry\">"
                    "<separator/>"
                    "<menuitem name=\"Wallpaper\" action=\"SetAsWallpaper\"/>"
                "</placeholder>"
            "</menu>"
        "</popup>"

        "<popup name=\"PopupMenu\">"
            "<placeholder name=\"WallpaperEntry\">"
                "<separator/>"
                "<menuitem action=\"SetAsWallpaper\"/>"
            "</placeholder>"
        "</popup>"
    "</ui>";
 clang-format on

 if (!gtk_ui_manager_add_ui_from_string(window->ui_manager,
                                        _ui_definition_wallpaper, -1,
                                        &error))
 {
     g_error("building menus failed: %s\n", error->message);
     g_error_free(error);
 }

#endif

