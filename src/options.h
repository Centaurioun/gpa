/* options.h - global option declarations
 *	Copyright (C) 2000, 2001 G-N-U GmbH.
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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <glib.h>
#include <glib-object.h>

/* GObject stuff */
#define GPA_OPTIONS_TYPE	  (gpa_options_get_type ())
#define GPA_OPTIONS(obj)	  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPA_OPTIONS_TYPE, GpaOptions))
#define GPA_OPTIONS_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), GPA_OPTIONS_TYPE, GpaOptionsClass))
#define GPA_IS_OPTIONS(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPA_OPTIONS_TYPE))
#define GPA_IS_OPTIONS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPA_OPTIONS_TYPE))
#define GPA_OPTIONS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GPA_OPTIONS_TYPE, GpaOptionsClass))

typedef struct _GpaOptions GpaOptions;
typedef struct _GpaOptionsClass GpaOptionsClass;

struct _GpaOptions {
  GObject parent;

  gchar *options_file;

  gboolean simplified_ui;
  gboolean backup_generated;

  gchar *default_key;
  gchar *default_keyserver;
};

struct _GpaOptionsClass {
  GObjectClass parent_class;

  /* Signal handlers */
  void (*changed_ui_mode) (GpaOptions *options);
  void (*changed_default_key) (GpaOptions *options);
  void (*changed_default_keyserver) (GpaOptions *options);
  void (*changed_backup_generated) (GpaOptions *options);
};

GType gpa_options_get_type (void) G_GNUC_CONST;

/* API */

/* Create a new GpaOptions object, reading the options, reading the options
 * from the file given */
GpaOptions *gpa_options_new (const gchar *filename);

/* Set whether the ui should be in simplified mode */
void gpa_options_set_simplified_ui (GpaOptions *options, gboolean value);
gboolean gpa_options_get_simplified_ui (GpaOptions *options);

/* Choose the default key */
void gpa_options_set_default_key (GpaOptions *options, const gchar *fpr);
const gchar *gpa_options_get_default_key (GpaOptions *options);
/* Try to find a reasonable value for the default key if there wasn't one */
void gpa_options_update_default_key (GpaOptions *options);

/* Specify the default keyserver */
void gpa_options_set_default_keyserver (GpaOptions *options,
                                        const gchar *keyserver);
const gchar *gpa_options_get_default_keyserver (GpaOptions *options);

/* Remember whether the default key has already been backed up */
void gpa_options_set_backup_generated (GpaOptions *options, gboolean value);
gboolean gpa_options_get_backup_generated (GpaOptions *options);

/* Destroy the GpaOptions object */
void gpa_options_destroy (GpaOptions *options);

#endif /*OPTIONS_H*/

