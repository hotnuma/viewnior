#include "list.h"
#include "config.h"


static GList* _parse_directory(gchar *path, gboolean sort,
                               gboolean include_hidden);
static gint _list_compare_func(gconstpointer a,
                                   gconstpointer b,
                                   gpointer);
static gint _file_compare_func(VnrFile *file, char *uri);
static GList* _vnr_list_delete_link(GList *list);


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

GList* vnr_list_new_for_path(gchar *filepath, gboolean include_hidden,
                             GError **error)
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

static GList* _parse_directory(gchar *path, gboolean sort,
                               gboolean include_hidden)
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

    GList *list = NULL;

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

        if (mime_type_is_supported(mimetype)
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

            list = g_list_prepend(list, vnrfile);
        }

        g_object_unref(fileinfo);
        fileinfo = g_file_enumerator_next_file(_file_enum, NULL, NULL);
    }

    g_object_unref(gfile);
    g_file_enumerator_close(_file_enum, NULL, NULL);
    g_object_unref(_file_enum);

    if (sort)
        list = vnr_list_sort(list);

    return list;
}

GList* vnr_list_insert(GList *list, VnrFile *newfile)
{
    GList *first = g_list_first(list);
    GList *l = first;

    while (l)
    {
        VnrFile *file = VNR_FILE(l->data);

        if (g_strcmp0(file->path, newfile->path) == 0)
            return NULL;

        l = l->next;
    }

    return g_list_insert_sorted_with_data(
                                    first,
                                    newfile,
                                    _list_compare_func,
                                    NULL);
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

GList* vnr_list_new_multiple(GSList *uri_list,
                             gboolean include_hidden,
                             GError **error)
{
    GList *file_list = NULL;

    while (uri_list != NULL)
    {
        gchar *filepath = uri_list->data;

        GFile *file = g_file_new_for_path(filepath);

        GFileInfo *fileinfo = g_file_query_info(
                                file,
                                G_FILE_ATTRIBUTE_STANDARD_TYPE ","
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

        GFileType filetype = g_file_info_get_file_type(fileinfo);

        if (filetype != G_FILE_TYPE_DIRECTORY)
        {
            VnrFile *vnrfile = vnr_file_new();
            const char *mimetype = g_file_info_get_content_type(fileinfo);

            if (mimetype == NULL)
            {
                mimetype = g_file_info_get_attribute_string(
                            fileinfo,
                            G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
            }

            if (mime_type_is_supported(mimetype)
                && (include_hidden || !g_file_info_get_is_hidden(fileinfo)))
            {
                vnr_file_set_display_name(
                                    vnrfile,
                                    (char*) g_file_info_get_display_name(fileinfo));

                vnrfile->mtime = g_file_info_get_attribute_uint64(
                                    fileinfo,
                                    G_FILE_ATTRIBUTE_TIME_MODIFIED);

                vnrfile->path = g_strdup(filepath);

                file_list = g_list_prepend(file_list, vnrfile);
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

GList* vnr_list_delete_item(GList *list)
{
    GList *first = g_list_first(list);

    // empty list
    if (first == NULL)
        return NULL;

    // only one item in the list
    if (g_list_length(first) == 1)
        return vnr_list_free(first);

    GList *next = g_list_next(list);
    if (next == NULL)
        next = first;

    g_assert(next != list);

    _vnr_list_delete_link(list);

    return next;
}

static GList* _vnr_list_delete_link(GList *list)
{
    if (list == NULL)
        return NULL;

    VnrFile *file = VNR_FILE(list->data);
    if (file)
        g_object_unref(file);

    // the doc says it must point to the top of the list

    return g_list_delete_link(list, list);
}

GList* vnr_list_free(GList *list)
{
    if (list == NULL)
        return NULL;

    GList *first = g_list_first(list);
    g_list_free_full(first, g_object_unref);

    return NULL;
}


