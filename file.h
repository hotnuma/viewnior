/*
 * Copyright © 2009-2018 Siyan Panayotov <contact@siyanpanayotov.com>
 *
 * This file is part of Viewnior.
 *
 * Viewnior is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Viewnior is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Viewnior.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __VNR_FILE_H__
#define __VNR_FILE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VNR_TYPE_FILE (vnr_file_get_type())
G_DECLARE_FINAL_TYPE(VnrFile, vnr_file, VNR, FILE, GObject)

typedef struct _VnrFile VnrFile;

struct _VnrFile
{
    GObject __parent__;

    gchar *display_name;
    gchar *display_name_collate;
    gchar *path;
    time_t mtime;
};

GType vnr_file_get_type() G_GNUC_CONST;

VnrFile* vnr_file_new();
gboolean vnr_file_rename(VnrFile *file, const gchar *newname);

void vnr_load_single_uri(GList **file_list, gchar *filepath, gboolean include_hidden,
                         GError **error);
void vnr_load_uri_list(GList **file_list, GSList *uri_list, gboolean include_hidden,
                       GError **error);

G_END_DECLS

#endif // __VNR_FILE_H__


