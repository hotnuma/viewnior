/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of Viewnior.
 *
 * Viewnior is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 * Viewnior is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Viewnior.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "file.h"
#include "config.h"
#include "vnr-tools.h"
#include <glib/gstdio.h>

G_DEFINE_TYPE(VnrFile, vnr_file, G_TYPE_OBJECT)

static gboolean _vnr_file_set_path(VnrFile *file, const gchar *filepath);

VnrFile* vnr_file_new()
{
    return VNR_FILE(g_object_new(VNR_TYPE_FILE, NULL));
}

static void vnr_file_class_init(VnrFileClass *klass)
{
}

static void vnr_file_init(VnrFile *file)
{
    file->display_name = NULL;
}

void vnr_file_set_display_name(VnrFile *vnr_file,
                                       const gchar *display_name)
{
    if (vnr_file->display_name)
        g_free(vnr_file->display_name);

    vnr_file->display_name = g_strdup(display_name);

    if (vnr_file->display_name_collate)
        g_free(vnr_file->display_name_collate);

    vnr_file->display_name_collate =
            g_utf8_collate_key_for_filename(display_name, -1);
}

gboolean vnr_file_rename(VnrFile *file, const gchar *fullpath)
{
    if (!file || !fullpath)
        return false;

    //printf("rename %s to %s\n", file->path, fullpath);

    gboolean ret = (g_rename(file->path, fullpath) == 0);

    if (ret)
        _vnr_file_set_path(file, fullpath);

    return ret;
}

static gboolean _vnr_file_set_path(VnrFile *file, const gchar *filepath)
{
    GFile *gfile = g_file_new_for_path(filepath);

    GFileInfo *fileinfo = g_file_query_info(
        gfile,
        G_FILE_ATTRIBUTE_STANDARD_TYPE ","
        G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
        0, NULL, NULL);

    if (fileinfo == NULL)
    {
        g_object_unref(gfile);
        return false;
    }

    if (file->path)
        g_free(file->path);
    file->path = g_strdup(filepath);

    if (file->display_name)
        g_free(file->display_name);
    file->display_name = g_strdup(g_file_info_get_display_name(fileinfo));

    if (file->display_name_collate)
        g_free(file->display_name_collate);
    file->display_name_collate =
            g_utf8_collate_key_for_filename(file->display_name, -1);

    file->mtime = g_file_info_get_attribute_uint64(
                                    fileinfo,
                                    G_FILE_ATTRIBUTE_TIME_MODIFIED);

    g_object_unref(gfile);
    g_object_unref(fileinfo);

    return true;
}


