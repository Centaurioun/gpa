/* gpasubkeylist.h  -  The GNU Privacy Assistant
 *	Copyright (C) 2003, Miguel Coca.
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

#ifndef GPA_SUBKEYLIST_H
#define GPA_SUBKEYLIST_H

#include <gtk/gtk.h>

/* Create a new subkey list.
 */
GtkWidget * gpa_subkey_list_new (void);

/* Set the key whose subkeys should be displayed.
 */
void gpa_subkey_list_set_key (GtkWidget * list, gpgme_key_t key);

#endif /* GPA_SUBKEYLIST_H */
