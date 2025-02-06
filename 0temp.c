
#if 0

/*

FileOpen
FileOpenDir
FileOpenWith
FileSelectDirectory
FileRename
FileMove
FileDelete
FileProperties
EditPreferences

HelpAbout


*/

static const GtkToggleActionEntry _toggle_entries_collection[] =
{
    {"ViewSlideshow", "gtk-network",
     N_("Sli_deshow"), "F6",
     N_("Show in slideshow mode"),
     G_CALLBACK(_action_slideshow)},
};

static const GtkActionEntry _action_entries_collection[] =
{
    {"GoPrevious", "gtk-go-back",
     N_("_Previous Image"), NULL,
     N_("Go to the previous image of the collection"),
     G_CALLBACK(_action_prev)},

    {"GoNext", "gtk-go-forward",
     N_("_Next Image"), NULL,
     N_("Go to the next image of the collection"),
     G_CALLBACK(_action_next)},

    {"GoFirst", "gtk-go-first",
     N_("_First Image"), NULL,
     N_("Go to the first image of the collection"),
     G_CALLBACK(_action_first)},

    {"GoLast", "gtk-goto-last",
     N_("_Last Image"), NULL,
     N_("Go to the last image of the collection"),
     G_CALLBACK(_action_last)},
};

static const GtkActionEntry _action_entry_save[] =
{
    {"FileSave", "gtk-save",
     N_("_Save"), "<control>S",
     N_("Save changes"),
     G_CALLBACK(_action_save_image)},
};

static const GtkActionEntry _action_entry_wallpaper[] =
{
    {"SetAsWallpaper", NULL,
     N_("Set as _Wallpaper"), NULL,
     N_("Set the selected image as the desktop background"),
     G_CALLBACK(_action_set_wallpaper)},
};

static const GtkToggleActionEntry _toggle_entries_image[] =
{
    {"ViewFullscreen", "gtk-fullscreen",
     N_("Full _Screen"), "F11",
     N_("Show in fullscreen mode"),
     G_CALLBACK(_action_fullscreen)},

    {"ViewResizeWindow", NULL,
     N_("_Adjust window size"), NULL,
     N_("Adjust window size to fit the image"),
     G_CALLBACK(_action_resize)},
};


static const GtkToggleActionEntry _toggle_entries_window[] =
{
    {"ViewScrollbar", NULL,
     N_("Scrollbar"), NULL,
     N_("Show Scrollbar"),
     G_CALLBACK(_action_scrollbar)},
};

    {"FileReload", "gtk-refresh",
     N_("_Reload"), NULL,
     N_("Reload the current file"),
     G_CALLBACK(_action_reload)},

    {"ViewZoomIn", "gtk-zoom-in",
     N_("_Zoom In"), "<control>plus",
     N_("Enlarge the image"),
     G_CALLBACK(_action_zoom_in)},

    {"ViewZoomOut", "gtk-zoom-out",
     N_("Zoom _Out"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_out)},

    {"ViewZoomNormal", "gtk-zoom-100",
     N_("_Normal Size"), "<control>0",
     N_("Show the image at its normal size"),
     G_CALLBACK(_action_normal_size)},

    {"ViewZoomFit", "gtk-zoom-fit",
     N_("Best _Fit"), NULL,
     N_("Fit the image to the window"),
     G_CALLBACK(_action_fit)},

    {"ControlEqual", "gtk-zoom-in",
     N_("_Zoom In"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_in)},

    {"ControlKpAdd", "gtk-zoom-in",
     N_("_Zoom In"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_in)},

    {"ControlKpSub", "gtk-zoom-out",
     N_("Zoom _Out"), NULL,
     N_("Shrink the image"),
     G_CALLBACK(_action_zoom_out)},

#endif

