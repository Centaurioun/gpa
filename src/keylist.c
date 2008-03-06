/* keylist.c - The GNU Privacy Assistant keylist.
   Copyright (C) 2003 Miguel Coca.
   Copyright (C) 2005, 2008 g10 Code GmbH.

   This file is part of GPA

   GPA is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   GPA is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
 
#include <config.h>

#include <glib/gstdio.h>

#include "gpa.h"
#include "keylist.h"
#include "gpapastrings.h"
#include "gpawidgets.h"
#include "keytable.h"
#include "icons.h"


/* Properties */
enum
{
  PROP_0,
  PROP_WINDOW,
  PROP_PUBLIC_ONLY
};

/* GObject */
static GObjectClass *parent_class = NULL;


/* Symbols to access the columns.  */
typedef enum
{
  /* These are the displayed columns */
  GPA_KEYLIST_COLUMN_IMAGE, 
  GPA_KEYLIST_COLUMN_KEYID,
  GPA_KEYLIST_COLUMN_EXPIRY,
  GPA_KEYLIST_COLUMN_OWNERTRUST,
  GPA_KEYLIST_COLUMN_VALIDITY,
  GPA_KEYLIST_COLUMN_USERID,
  /* This column contains the gpgme_key_t */
  GPA_KEYLIST_COLUMN_KEY,
  /* These columns are used only internally for sorting */
  GPA_KEYLIST_COLUMN_HAS_SECRET,
  GPA_KEYLIST_COLUMN_EXPIRY_TS,
  GPA_KEYLIST_COLUMN_OWNERTRUST_VALUE,
  GPA_KEYLIST_COLUMN_VALIDITY_VALUE,
  GPA_KEYLIST_N_COLUMNS
} GpaKeyListColumn;



static void add_trustdb_dialog (GpaKeyList * keylist);
static void gpa_keylist_next (gpgme_key_t key, gpointer data);
static void gpa_keylist_end (gpointer data);



/************************************************************ 
 ******************  Object Management  *********************
 ************************************************************/

static void
gpa_keylist_get_property (GObject     *object,
			  guint        prop_id,
			  GValue      *value,
			  GParamSpec  *pspec)
{
  GpaKeyList *list = GPA_KEYLIST (object);

  switch (prop_id)
    {
    case PROP_WINDOW:
      g_value_set_object (value, list->window);
      break;
    case PROP_PUBLIC_ONLY:
      g_value_set_boolean (value, list->public_only);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gpa_keylist_set_property (GObject     *object,
			  guint        prop_id,
			  const GValue *value,
			  GParamSpec  *pspec)
{
  GpaKeyList *list = GPA_KEYLIST (object);

  switch (prop_id)
    {
    case PROP_WINDOW:
      list->window = (GtkWidget*) g_value_get_object (value);
      break;
    case PROP_PUBLIC_ONLY:
      list->public_only = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
gpa_keylist_finalize (GObject *object)
{  
  GpaKeyList *list = GPA_KEYLIST (object);

  /* Dereference all keys in the list */
  g_list_foreach (list->keys, (GFunc) gpgme_key_unref, NULL);
  g_list_free (list->keys);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static void
gpa_keylist_init (GpaKeyList *list)
{

}


static GObject*
gpa_keylist_constructor (GType type,
                         guint n_construct_properties,
                         GObjectConstructParam *construct_properties)
{
  GObject *object;
  GpaKeyList *list;
  GtkListStore *store;
  GtkTreeSelection *selection; 

  object = parent_class->constructor (type,
				      n_construct_properties,
				      construct_properties);
  list = GPA_KEYLIST (object);

   /* Init the model */
  store = gtk_list_store_new (GPA_KEYLIST_N_COLUMNS,
			      GDK_TYPE_PIXBUF,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_POINTER,
			      G_TYPE_INT,
			      G_TYPE_ULONG,
			      G_TYPE_ULONG,
			      G_TYPE_LONG);

  /* The view */
  gtk_tree_view_set_model (GTK_TREE_VIEW (list), GTK_TREE_MODEL (store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (list), TRUE);
  gpa_keylist_set_brief (list);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (list));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  /* Load the keyring */
  add_trustdb_dialog (list);
  gpa_keytable_list_keys (gpa_keytable_get_public_instance(),
			  gpa_keylist_next, gpa_keylist_end, list);

  return object;
}


static void
gpa_keylist_class_init (GpaKeyListClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  
  parent_class = g_type_class_peek_parent (klass);

  object_class->constructor = gpa_keylist_constructor;
  object_class->finalize = gpa_keylist_finalize;
  object_class->set_property = gpa_keylist_set_property;
  object_class->get_property = gpa_keylist_get_property;

  g_object_class_install_property 
    (object_class, PROP_PUBLIC_ONLY,
     g_param_spec_boolean
     ("public-only", "Public-only",
      "A flag indicating that we are only interested in public keys.",
      FALSE,
      G_PARAM_WRITABLE|G_PARAM_CONSTRUCT_ONLY));

}



GType
gpa_keylist_get_type (void)
{
  static GType keylist_type = 0;
  
  if (!keylist_type)
    {
      static const GTypeInfo keylist_info =
      {
        sizeof (GpaKeyListClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gpa_keylist_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GpaKeyList),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gpa_keylist_init,
      };
      
      keylist_type = g_type_register_static (GTK_TYPE_TREE_VIEW,
						  "GpaKeyList",
						  &keylist_info, 0);
    }
  
  return keylist_type;
}



/************************************************************ 
 ******************  Internal Functions  ********************
 ************************************************************/

static gboolean
display_dialog (GpaKeyList * keylist)
{
  gtk_widget_show_all (keylist->dialog);

  keylist->timeout_id = 0;

  return FALSE;
}


static void
add_trustdb_dialog (GpaKeyList * keylist)
{
  /* Display this warning until the first key is received.  It may be
     shown at times when it's not needed. But it shouldn't appear for
     long those times.  */
  keylist->dialog = gtk_message_dialog_new
    (GTK_WINDOW (keylist->window), GTK_DIALOG_MODAL, 
     GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
     _("GnuPG is rebuilding the trust database.\n"
       "This might take a few seconds."));

  /* Wait a second before displaying the dialog. This avoids most
     "false alarms".  That is the message will not be shown if GnuPG
     does not run a long check trustdb.  */
  keylist->timeout_id = g_timeout_add (1000, (GSourceFunc) display_dialog, 
				       keylist);
}


static void
remove_trustdb_dialog (GpaKeyList * keylist)
{
  if (keylist->timeout_id)
    {
      g_source_remove (keylist->timeout_id);
      keylist->timeout_id = 0;
    }
  if (keylist->dialog)
    {
      gtk_widget_destroy (keylist->dialog);
      keylist->dialog = NULL;
    }
}


static GdkPixbuf*
get_key_pixbuf (gpgme_key_t key)
{
  static gboolean pixmaps_created = FALSE;
  static GdkPixbuf *secret_pixbuf = NULL;
  static GdkPixbuf *public_pixbuf = NULL;

  if (!pixmaps_created)
    {
      secret_pixbuf = gpa_create_icon_pixbuf ("blue_yellow_key");
      public_pixbuf = gpa_create_icon_pixbuf ("blue_key");
      pixmaps_created = TRUE;
    }

  if (gpa_keytable_lookup_key 
      (gpa_keytable_get_secret_instance(), 
       key->subkeys->fpr) != NULL)
    {
      return secret_pixbuf;
    }
  else
    {
      return public_pixbuf;
    }
}



/* For keys, gpg can't cope with, the fingerprint is set to all
   zero. This helper function returns true for such a FPR. */
static int
is_zero_fpr (const char *fpr)
{
  for (; *fpr; fpr++)
    if (*fpr != '0')
      return 0;
  return 1;
}


static void 
gpa_keylist_next (gpgme_key_t key, gpointer data)
{
  GpaKeyList *list = data;
  GtkListStore *store;
  GtkTreeIter iter;
  const gchar *keyid, *ownertrust, *validity;
  gchar *userid, *expiry;
  gboolean has_secret;
  long int val_value;

  /* Remove the dialog if it is being displayed */
  remove_trustdb_dialog (list);
  
  list->keys = g_list_append (list->keys, key);
  store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (list)));
  /* Get the column values */
  keyid = gpa_gpgme_key_get_short_keyid (key);
  expiry = gpa_expiry_date_string (key->subkeys->expires);
  ownertrust = gpa_key_ownertrust_string (key);
  validity = gpa_key_validity_string (key);
  userid = gpa_gpgme_key_get_userid (key->uids);
  if (list->public_only)
    has_secret = 0;
  else
    has_secret = (!is_zero_fpr (key->subkeys->fpr)
                  && gpa_keytable_lookup_key 
                  (gpa_keytable_get_secret_instance(), key->subkeys->fpr));

  /* Append the key to the list */
  gtk_list_store_append (store, &iter);

  /* Set an appropiate value for sorting revoked and expired keys. This 
   * includes a hack for forcing a value to a range outside the
   * usual validity values */
  if (key->subkeys->revoked)
      val_value = GPGME_VALIDITY_UNKNOWN-2;
  else if (key->subkeys->expired)
      val_value = GPGME_VALIDITY_UNKNOWN-1;
  else if (key->uids)
      val_value = key->uids->validity;
  else
      val_value = GPGME_VALIDITY_UNKNOWN;

  gtk_list_store_set (store, &iter,
		      GPA_KEYLIST_COLUMN_KEYID, keyid, 
		      GPA_KEYLIST_COLUMN_EXPIRY, expiry,
		      GPA_KEYLIST_COLUMN_OWNERTRUST, ownertrust,
		      GPA_KEYLIST_COLUMN_VALIDITY, validity,
		      GPA_KEYLIST_COLUMN_USERID, userid,
		      GPA_KEYLIST_COLUMN_KEY, key, 
		      GPA_KEYLIST_COLUMN_HAS_SECRET, has_secret,
		      /* Set "no expiration" to a large value for sorting */
		      GPA_KEYLIST_COLUMN_EXPIRY_TS, 
		      key->subkeys->expires ? 
		      key->subkeys->expires : G_MAXULONG,
		      GPA_KEYLIST_COLUMN_OWNERTRUST_VALUE, 
		      key->owner_trust,
		      /* Set revoked and expired keys to "never trust" for 
		       * sorting */
		      GPA_KEYLIST_COLUMN_VALIDITY_VALUE, val_value,
                      /* Store the image only if enabled.  */
		      list->public_only? -1: GPA_KEYLIST_COLUMN_IMAGE,
                      list->public_only? NULL:get_key_pixbuf (key),
		      -1);
  /* Clean up */
  g_free (userid);
  g_free (expiry);
}


static void 
gpa_keylist_end (gpointer data)
{
  GpaKeyList *list = data;
  
  remove_trustdb_dialog (list);
}


static void 
gpa_keylist_clear_columns (GpaKeyList *keylist)
{
  GList *columns, *i;

  columns = gtk_tree_view_get_columns (GTK_TREE_VIEW (keylist));
  for (i = columns; i; i = g_list_next (i))
    {
      gtk_tree_view_remove_column (GTK_TREE_VIEW (keylist),
                                   (GtkTreeViewColumn*) i->data);
    }
}



/************************************************************ 
 **********************  Public API  ************************
 ************************************************************/

/* Create a new key list widget.  */
GtkWidget *
gpa_keylist_new (GtkWidget *window)
{
  GtkWidget *list = (GtkWidget*) g_object_new (GPA_KEYLIST_TYPE, NULL);

  return list;
}


/* Create a new key list widget in public_only mode. */
GpaKeyList *
gpa_keylist_new_public_only (GtkWidget *window)
{
  GpaKeyList *list;

  list = g_object_new (GPA_KEYLIST_TYPE,
                       "public-only", TRUE,
                       NULL);

  return list;
}



/* Set the key list in "brief" mode.  */
void 
gpa_keylist_set_brief (GpaKeyList *keylist)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  gpa_keylist_clear_columns (keylist);

  if (!keylist->public_only)
    {
      renderer = gtk_cell_renderer_pixbuf_new ();
      column = gtk_tree_view_column_new_with_attributes 
        ("", renderer, "pixbuf", GPA_KEYLIST_COLUMN_IMAGE, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
      gtk_tree_view_column_set_sort_column_id (column, 
                                               GPA_KEYLIST_COLUMN_HAS_SECRET);
      gtk_tree_view_column_set_sort_indicator (column, TRUE);
    }

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Key ID"), renderer,
						     "text",
						     GPA_KEYLIST_COLUMN_KEYID,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
  gtk_tree_view_column_set_sort_column_id (column, GPA_KEYLIST_COLUMN_KEYID);
  gtk_tree_view_column_set_sort_indicator (column, TRUE);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("User Name"), renderer,
						     "text",
						     GPA_KEYLIST_COLUMN_USERID,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
  gtk_tree_view_column_set_sort_column_id (column, GPA_KEYLIST_COLUMN_USERID);
  gtk_tree_view_column_set_sort_indicator (column, TRUE);
}


/* Set the key list in "detailed" mode.  */
void 
gpa_keylist_set_detailed (GpaKeyList * keylist)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  gpa_keylist_clear_columns (keylist);

  if (!keylist->public_only)
    {
      renderer = gtk_cell_renderer_pixbuf_new ();
      column = gtk_tree_view_column_new_with_attributes 
        ("", renderer,
         "pixbuf",
         GPA_KEYLIST_COLUMN_IMAGE,
         NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
      gtk_tree_view_column_set_sort_column_id 
        (column, GPA_KEYLIST_COLUMN_HAS_SECRET);
      gtk_tree_view_column_set_sort_indicator (column, TRUE);
    }

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Key ID"), renderer,
						     "text",
						     GPA_KEYLIST_COLUMN_KEYID,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
  gtk_tree_view_column_set_sort_column_id (column, GPA_KEYLIST_COLUMN_KEYID);
  gtk_tree_view_column_set_sort_indicator (column, TRUE);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Expiry Date"), 
						     renderer,
						     "text",
						     GPA_KEYLIST_COLUMN_EXPIRY,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
  gtk_tree_view_column_set_sort_column_id (column, GPA_KEYLIST_COLUMN_EXPIRY_TS);
  gtk_tree_view_column_set_sort_indicator (column, TRUE);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Owner Trust"), 
						     renderer,
						     "text",
						     GPA_KEYLIST_COLUMN_OWNERTRUST,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
  gtk_tree_view_column_set_sort_column_id (column, GPA_KEYLIST_COLUMN_OWNERTRUST_VALUE);
  gtk_tree_view_column_set_sort_indicator (column, TRUE);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Key Validity"),
						     renderer,
						     "text",
						     GPA_KEYLIST_COLUMN_VALIDITY,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);
  gtk_tree_view_column_set_sort_column_id (column, GPA_KEYLIST_COLUMN_VALIDITY_VALUE);
  gtk_tree_view_column_set_sort_indicator (column, TRUE);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("User Name"), renderer,
						     "text",
						     GPA_KEYLIST_COLUMN_USERID,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (keylist), column);  
  gtk_tree_view_column_set_sort_column_id (column, GPA_KEYLIST_COLUMN_USERID);
  gtk_tree_view_column_set_sort_indicator (column, TRUE);
}


/* Return true if any key is selected in the list.  */
gboolean 
gpa_keylist_has_selection (GpaKeyList * keylist)
{
  GtkTreeSelection *selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (keylist));
  return gtk_tree_selection_count_selected_rows (selection) > 0;
}


/* Return true if one, and only one, key is selected in the list.  */
gboolean 
gpa_keylist_has_single_selection (GpaKeyList * keylist)
{
  GtkTreeSelection *selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (keylist));
  return gtk_tree_selection_count_selected_rows (selection) == 1;
}


/* Return true if one, and only one, secret key is selected in the list.  */
gboolean 
gpa_keylist_has_single_secret_selection (GpaKeyList *keylist)
{
  GtkTreeSelection *selection;

  if (keylist->public_only)
    return FALSE;
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (keylist));
  if (gtk_tree_selection_count_selected_rows (selection) == 1)
    {
      GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (keylist));
      GList *list = gtk_tree_selection_get_selected_rows (selection, &model);
      gpgme_key_t key;
      GtkTreeIter iter;
      GtkTreePath *path = list->data;
      GValue value = {0,};

      gtk_tree_model_get_iter (model, &iter, path);
      gtk_tree_model_get_value (model, &iter, GPA_KEYLIST_COLUMN_KEY,
				&value);
      key = g_value_get_pointer (&value);
      g_value_unset(&value);      

      g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (list);
      return (gpa_keytable_lookup_key (gpa_keytable_get_secret_instance(), 
				       key->subkeys->fpr) != NULL);
    }
  else
    {
      return FALSE;
    }
}


/* Return a GList of selected keys. The caller must not dereference
   the keys as they belong to the caller.  */
GList *
gpa_keylist_get_selected_keys (GpaKeyList * keylist)
{
  GtkTreeSelection *selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (keylist));
  GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (keylist));
  GList *list = gtk_tree_selection_get_selected_rows (selection, &model);
  GList *keys = NULL;
  GList *cur;

  for (cur = list; cur; cur = g_list_next (cur))
    {
      gpgme_key_t key;
      GtkTreeIter iter;
      GtkTreePath *path = cur->data;
      GValue value = {0,};

      gtk_tree_model_get_iter (model, &iter, path);
      gtk_tree_model_get_value (model, &iter, GPA_KEYLIST_COLUMN_KEY,
				&value);
      key = g_value_get_pointer (&value);
      g_value_unset(&value);      

      keys = g_list_append (keys, key);
    }

  g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (list);
  
  return keys;
}


/* Return the selected key.  This function returns NULL if no or more
   than one key has been selected.  */
gpgme_key_t
gpa_keylist_get_selected_key (GpaKeyList *keylist)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GList *list;
  GtkTreePath *path;
  GtkTreeIter iter;
  GValue value = {0};
  gpgme_key_t key;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (keylist));
  if (gtk_tree_selection_count_selected_rows (selection) != 1)
    return NULL;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (keylist));
  list = gtk_tree_selection_get_selected_rows (selection, &model);
  if (!list)
    return NULL;
  path = list->data;

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get_value (model, &iter, GPA_KEYLIST_COLUMN_KEY, &value);
  key = g_value_get_pointer (&value);
  g_value_unset (&value);      
  gpgme_key_ref (key);

  g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
  g_list_free (list);
  
  return key;
}


/* Begin a reload of the keyring. */
void 
gpa_keylist_start_reload (GpaKeyList * keylist)
{
  GtkTreeSelection *selection = 
    gtk_tree_view_get_selection (GTK_TREE_VIEW (keylist));
  gtk_tree_selection_unselect_all (selection);
  gtk_list_store_clear (GTK_LIST_STORE (gtk_tree_view_get_model 
					(GTK_TREE_VIEW (keylist))));
  g_list_foreach (keylist->keys, (GFunc) gpgme_key_unref, NULL);
  g_list_free (keylist->keys);
  keylist->keys = NULL;
  add_trustdb_dialog (keylist);

  gpa_keytable_force_reload (gpa_keytable_get_public_instance (),
			     gpa_keylist_next, gpa_keylist_end, keylist);
}


/* Let the keylist know that a new key with the given fingerprint is
   available.  */
void 
gpa_keylist_new_key (GpaKeyList * keylist, const char *fpr)
{
  /* FIXME: I don't understand the code.  Investigate this and
     implement public_only.  */
  
  add_trustdb_dialog (keylist);
  gpa_keytable_load_new (gpa_keytable_get_secret_instance (), fpr,
			 NULL, (GpaKeyTableEndFunc) gtk_main_quit, NULL);
  /* Hack. Turn the asynchronous listing into a synchronous one */
  gtk_main ();
  remove_trustdb_dialog (keylist);
  /* The trustdb seems not to be updated for a --list-secret, so we
   * cdisplay the dialog both times, just in case */
  add_trustdb_dialog (keylist);
  gpa_keytable_load_new (gpa_keytable_get_public_instance (), fpr,
			 gpa_keylist_next, gpa_keylist_end, keylist);
}


/* Let the keylist know that a new sceret key has been imported. */
void 
gpa_keylist_imported_secret_key (GpaKeyList *keylist)
{
  /* KEYLIST is currently not used. */

  gpa_keytable_load_new (gpa_keytable_get_secret_instance (), NULL,
			 NULL, (GpaKeyTableEndFunc) gtk_main_quit, NULL);
  /* Hack. Turn the asynchronous listing into a synchronous one */
  /* FIXME:  Does that work well with the server shutdown code? */ 
  gtk_main ();
}


