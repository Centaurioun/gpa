/* gpgmeparsers.h - The GNU Privacy Assistant
 *      Copyright (C) 2002, Miguel Coca.
 *
 * This file is part of GPA
 *
 * GPA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* Parsers for the bits of data GPGME provides in XML */

#ifndef GPGMEPARSERS_H
#define GPGMEPARSERS_H

#include <glib.h>
#include "gpa.h"

/* Retrieve and parse the result of gpgme_get_engine_info () */
typedef struct
{
  gchar *version;
  gchar *path;
} GpaEngineInfo;

void gpa_parse_engine_info (GpaEngineInfo *info);

/* Retrieve and parse the detailed results of an import operation */
typedef struct
{
  GList *keyids;
  gint count, no_user_id, imported, imported_rsa, unchanged, n_uids, n_subk,
    n_sigs, s_sigs, n_revoc, sec_read, sec_imported, sec_dups, skipped_new;
} GpaImportInfo;

void gpa_parse_import_info (GpaImportInfo *info);

#endif
