<link href="style.css" rel="stylesheet"></link>

#### Notes

* vnr_file_load_single_uri
    
    Open a single image.
    
* vnr_file_dir_content_to_list
    
    Parse a directory and return a sorted list of files.

* menu_bar
    
    ```
    File
        Open file
        Open dir
        Open with
        Save
        Reload
        Properties
        Close
    Edit
        Delete
        Preferences
    Show
    Image
    Goto
    Help
    ```


#### Deprecations

* uni-scroll-win
    
    gtk_hscrollbar_new
    GTK_TABLE
    gtk_container_set_resize_mode

* preferences.c
    
    GtkTable

* uni-anim-view
    
    GTimeVal time;
    g_time_val_add(&aview->time, delay_us);  
    g_get_current_time(&aview->time);  
    

