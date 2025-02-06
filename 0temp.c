
#if 0

/*

FileOpen\
FileOpenDir\
FileOpenWith\
FileRename\
FileSelectDirectory\
FileMove\
FileDelete\
FileProperties\
ControlEqual\
ControlKpAdd\
ControlKpSub\

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

static const GtkToggleActionEntry _toggle_entries_window[] =
{
    {"ViewScrollbar", NULL,
     N_("Scrollbar"), NULL,
     N_("Show Scrollbar"),
     G_CALLBACK(_action_scrollbar)},
};


#endif

