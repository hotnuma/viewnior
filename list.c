#include "list.h"
#include "config.h"

#include "file.h"

GList *_supported_mime_types;

static GList* _parse_directory(gchar *path, gboolean sort,
                               gboolean include_hidden);
static gint _list_compare_func(gconstpointer a,
                                   gconstpointer b,
                                   gpointer);
static gint _file_compare_func(VnrFile *file, char *uri);
static gboolean _mime_type_is_supported(const char *mime_type);
static GList* _mime_types_get_supported();
static gint compare_quarks(gconstpointer a, gconstpointer b);


GList* vnr_list_new(gchar *filepath, gboolean include_hidden, GError **error)
{
    gchar *dir = g_path_get_dirname(filepath);
    GList *result = _parse_directory(dir, TRUE, include_hidden);
    g_free(dir);

    if (!result)
        return NULL;

    result = g_list_find_custom(result, filepath,
                                (GCompareFunc) _file_compare_func);

    if (!result)
    {
        *error = g_error_new(1, 0,
                             _("Couldn't recognise the image file\n"
                               "format for file '%s'"),
                             filepath);
    }

    return result;
}

GList* vnr_list_new_for_path(gchar *filepath, gboolean include_hidden, GError **error)
{
    GFile *file = g_file_new_for_path(filepath);

    GFileInfo *fileinfo = g_file_query_info(
        file,
        G_FILE_ATTRIBUTE_STANDARD_TYPE ","
        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
        0, NULL, error);

    if (fileinfo == NULL)
        return NULL;

    GFileType filetype = g_file_info_get_file_type(fileinfo);

    GList *filelist = NULL;

    if (filetype == G_FILE_TYPE_DIRECTORY)
    {
        filelist = _parse_directory(filepath, TRUE, include_hidden);
    }
    else
    {
        filelist = vnr_list_new(filepath, include_hidden, error);
    }

    g_object_unref(fileinfo);
    g_object_unref(file);

    return filelist;
}

static GList* _parse_directory(gchar *path, gboolean sort, gboolean include_hidden)
{
    GFile *gfile = g_file_new_for_path(path);

    GFileEnumerator *_file_enum = g_file_enumerate_children(
        gfile,
        G_FILE_ATTRIBUTE_STANDARD_NAME ","
        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
        G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
        G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
        G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
        G_FILE_ATTRIBUTE_TIME_MODIFIED,
        G_FILE_QUERY_INFO_NONE,
        NULL, NULL);

    GFileInfo *fileinfo = g_file_enumerator_next_file(_file_enum, NULL, NULL);

    GList *filelist = NULL;

    while (fileinfo != NULL)
    {
        VnrFile *vnrfile = vnr_file_new();

        const char *mimetype = g_file_info_get_content_type(fileinfo);
        if (mimetype == NULL)
        {
            mimetype = g_file_info_get_attribute_string(
                                fileinfo,
                                G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
        }

        if (_mime_type_is_supported(mimetype)
            && (include_hidden || !g_file_info_get_is_hidden(fileinfo)))
        {
            vnr_file_set_display_name(
                vnrfile,
                (char*) g_file_info_get_display_name(fileinfo));

            vnrfile->mtime = g_file_info_get_attribute_uint64(
                                        fileinfo,
                                        G_FILE_ATTRIBUTE_TIME_MODIFIED);

            vnrfile->path = g_strjoin(G_DIR_SEPARATOR_S, path,
                                       vnrfile->display_name, NULL);

            filelist = g_list_prepend(filelist, vnrfile);
        }

        g_object_unref(fileinfo);
        fileinfo = g_file_enumerator_next_file(_file_enum, NULL, NULL);
    }

    g_object_unref(gfile);
    g_file_enumerator_close(_file_enum, NULL, NULL);
    g_object_unref(_file_enum);

    if (sort)
        filelist = vnr_list_sort(filelist);

        //filelist = g_list_sort_with_data(filelist,
        //                                 _list_compare_func, NULL);

    return filelist;
}

GList* vnr_list_sort(GList *list)
{
    return g_list_sort_with_data(list, _list_compare_func, NULL);
}

static gint _list_compare_func(gconstpointer a, gconstpointer b, gpointer)
{
    return g_strcmp0(VNR_FILE((void *) a)->display_name_collate,
                     VNR_FILE((void *) b)->display_name_collate);
}

static gint _file_compare_func(VnrFile *file, char *uri)
{
    if (g_strcmp0(uri, file->path) == 0)
        return 0;
    else
        return 1;
}

GList* vnr_list_new_multiple(GSList *uri_list, gboolean include_hidden, GError **error)
{
    GFileType filetype;
    gchar *p_path;

    GList *file_list = NULL;

    while (uri_list != NULL)
    {
        p_path = uri_list->data;
        GFile *file = g_file_new_for_path(p_path);
        GFileInfo *fileinfo = g_file_query_info(
                    file, G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                    G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME ","
                    G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                    G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
                    G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN ","
                    G_FILE_ATTRIBUTE_TIME_MODIFIED,
                                                0, NULL, error);

        if (fileinfo == NULL)
        {
            g_clear_error(error);
            g_object_unref(file);

            uri_list = g_slist_next(uri_list);
            continue;
        }

        filetype = g_file_info_get_file_type(fileinfo);

        if (filetype != G_FILE_TYPE_DIRECTORY)
        {
            VnrFile *new_vnrfile = vnr_file_new();
            const char *mimetype = g_file_info_get_content_type(fileinfo);

            if (mimetype == NULL)
            {
                mimetype = g_file_info_get_attribute_string(
                            fileinfo,
                            G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
            }

            if (_mime_type_is_supported(mimetype)
                && (include_hidden || !g_file_info_get_is_hidden(fileinfo)))
            {
                vnr_file_set_display_name(
                                    new_vnrfile,
                                    (char*) g_file_info_get_display_name(fileinfo));

                new_vnrfile->mtime = g_file_info_get_attribute_uint64(
                                    fileinfo,
                                    G_FILE_ATTRIBUTE_TIME_MODIFIED);

                new_vnrfile->path = g_strdup(p_path);

                file_list = g_list_prepend(file_list, new_vnrfile);
            }
        }

        g_object_unref(file);
        g_object_unref(fileinfo);

        uri_list = g_slist_next(uri_list);
    }

    //file_list = g_list_sort_with_data(file_list, _list_compare_func, NULL);
    file_list = vnr_list_sort(file_list);

    return file_list;
}

static gboolean _mime_type_is_supported(const char *mime_type)
{
    GList *result;
    GQuark quark;

    if (mime_type == NULL)
    {
        return FALSE;
    }

    _supported_mime_types = _mime_types_get_supported();

    quark = g_quark_from_string(mime_type);

    result = g_list_find_custom(_supported_mime_types,
                                GINT_TO_POINTER(quark),
                                (GCompareFunc) compare_quarks);

    return (result != NULL);
}

gint vnr_list_get_position(GList *list, gint *total)
{
    gint after = 0;
    gint before = 0;

    for (GList *it = list; it != NULL; it = it->next)
        ++after;

    for (GList *it = list; it != NULL; it = it->prev)
        ++before;

    if (total)
        *total = before + after - 1;

    return before;
}

// Private functions ---------------------------------------------------------

static GList* _mime_types_get_supported()
{
    // Modified version of eog's eog_image_get_supported_mime_types

    GSList *format_list;
    GSList *it;

    if (!_supported_mime_types)
    {
        format_list = gdk_pixbuf_get_formats();

        for (it = format_list; it != NULL; it = it->next)
        {
            gchar **mime_types =
                gdk_pixbuf_format_get_mime_types((GdkPixbufFormat *)it->data);

            for (int i = 0; mime_types[i] != NULL; ++i)
            {
                _supported_mime_types =
                    g_list_prepend(_supported_mime_types,
                                   g_strdup(mime_types[i]));
            }

            g_strfreev(mime_types);
        }

        _supported_mime_types = g_list_prepend(_supported_mime_types,
                                              "image/vnd.microsoft.icon");

        _supported_mime_types = g_list_sort(_supported_mime_types,
                                           (GCompareFunc) compare_quarks);

        g_slist_free(format_list);
    }

    return _supported_mime_types;
}

static gint compare_quarks(gconstpointer a, gconstpointer b)
{
    GQuark quark;

    quark = g_quark_from_string((const gchar *)a);

    return quark - GPOINTER_TO_INT(b);
}

GList* vnr_list_free(GList *list)
{
    if (list == NULL)
        return NULL;

    GList *first = g_list_first(list);
    g_list_free_full(first, g_object_unref);

    return NULL;
}


