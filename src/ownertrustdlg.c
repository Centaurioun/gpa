/* keyring.c  -	 The GNU Privacy Assistant
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>

#include <gpgme.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gpa.h"
#include "gpawidgets.h"
#include "gtktools.h"
#include "gpgmeedit.h"

/*
 *	Owner Trust dialog
 */

static void
init_radio_buttons (gpgme_validity_t trust, GtkWidget *unknown_radio, 
                    GtkWidget *never_radio, GtkWidget *marginal_radio, 
                    GtkWidget *full_radio, GtkWidget *ultimate_radio)
{  
  switch (trust)
    {
    case GPGME_VALIDITY_UNKNOWN:
    case GPGME_VALIDITY_UNDEFINED:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (unknown_radio), TRUE);
      break;
    case GPGME_VALIDITY_NEVER:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (never_radio), TRUE);
      break;
    case GPGME_VALIDITY_MARGINAL:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (marginal_radio), TRUE);
      break;
    case GPGME_VALIDITY_FULL:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (full_radio), TRUE);
      break;
    case GPGME_VALIDITY_ULTIMATE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ultimate_radio), TRUE);
      break;
    }
}

static gpgme_validity_t
get_selected_validity (GtkWidget *unknown_radio, GtkWidget *never_radio,
                       GtkWidget *marginal_radio, GtkWidget *full_radio,
                       GtkWidget *ultimate_radio)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (unknown_radio)))
    {
      return GPGME_VALIDITY_UNKNOWN;
    }
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (never_radio)))
    {
      return GPGME_VALIDITY_NEVER;
    }
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (marginal_radio)))
    {
      return GPGME_VALIDITY_MARGINAL;
    }
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (full_radio)))
    {
      return GPGME_VALIDITY_FULL;
    }
  else
    {
      return GPGME_VALIDITY_ULTIMATE;
    }
}

/* Run the owner trust dialog modally. */
gboolean gpa_ownertrust_run_dialog (gpgme_key_t key, GtkWidget *parent,
				    gpgme_validity_t *return_trust)
{
  GtkWidget *dialog;
  GtkWidget *key_info;
  GtkWidget *grid;
  GtkWidget *frame;
  GtkWidget *unknown_radio, *never_radio, *marginal_radio, *full_radio,
    *ultimate_radio;
  GtkWidget *label;
  GtkResponseType response;
  gpgme_validity_t trust = key->owner_trust;
  gboolean result;

  /* Create the dialog */

  dialog = gtk_dialog_new_with_buttons
    (_("Change key ownertrust"), GTK_WINDOW(parent), GTK_DIALOG_MODAL,
     _("_Cancel"), GTK_RESPONSE_CANCEL,
     _("_OK"), GTK_RESPONSE_OK,
     NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);

  key_info = gpa_key_info_new (key);

  GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  gtk_box_pack_start(GTK_BOX (box), key_info, FALSE, FALSE, 0);

  /* Create the "Owner Trust" frame */

  frame = gtk_frame_new (_("Owner Trust"));
  gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);
  grid = gtk_grid_new();

  unknown_radio = gtk_radio_button_new (NULL);
  gtk_grid_attach(GTK_GRID(grid), unknown_radio, 0, 0, 1, 1);
  label = gtk_label_new_with_mnemonic (_("_Unknown"));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), unknown_radio);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);
  gtk_grid_attach(GTK_GRID (grid), label, 1, 0, 1, 1);

  label = gtk_label_new (_("You don't know how much to trust this user to "
                           "verify other people's keys.\n"));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);
  gtk_grid_attach(GTK_GRID (grid), label, 1, 1, 1, 1);

  never_radio = gtk_radio_button_new_from_widget 
    (GTK_RADIO_BUTTON (unknown_radio));
  gtk_grid_attach(GTK_GRID (grid), never_radio, 0, 2, 1, 1);
  label = gtk_label_new_with_mnemonic (_("_Never"));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), never_radio);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);
  gtk_grid_attach(GTK_GRID (grid), label, 1, 2, 1, 1);

  label = gtk_label_new (_("You don't trust this user at all to verify the "
                           "validity of other people's keys at all.\n"));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);
  gtk_grid_attach(GTK_GRID (grid), label, 1, 3, 1, 1);

  marginal_radio = gtk_radio_button_new_from_widget 
    (GTK_RADIO_BUTTON (unknown_radio));
  gtk_grid_attach(GTK_GRID (grid), marginal_radio, 0, 4, 1, 1);
  label = gtk_label_new_with_mnemonic (_("_Marginal"));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), marginal_radio);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);
  gtk_grid_attach(GTK_GRID (grid), label, 1, 4, 1, 1);

  label = gtk_label_new (_("You don't trust this user's ability to "
                           "verify the validity of other people's keys "
                           "enough to consider keys valid based on his/her "
                           "sole word.\n"
                           "However, provided this user's key is "
                           "valid, you will consider a key signed by this "
                           "user valid if it is also signed by at least "
                           "other two marginally trusted users with "
                           "valid keys\n"));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);

  gtk_grid_attach(GTK_GRID (grid), label, 1, 5, 1, 1);

  full_radio = gtk_radio_button_new_from_widget 
    (GTK_RADIO_BUTTON (unknown_radio));
  gtk_grid_attach(GTK_GRID (grid), full_radio, 0, 6, 1, 1);
  label = gtk_label_new_with_mnemonic (_("_Full"));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), full_radio);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);

  gtk_grid_attach(GTK_GRID (grid), label, 1, 6, 1, 1);

  label = gtk_label_new (_("You trust this user's ability to "
                           "verify the validity of other people's keys "
                           "so much, that you'll consider valid any key "
                           "signed by him/her, provided this user's key "
                           "is valid.\n"));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);

  gtk_grid_attach(GTK_GRID (grid), label, 1, 7, 1, 1);

  ultimate_radio = gtk_radio_button_new_from_widget 
    (GTK_RADIO_BUTTON (unknown_radio));
  gtk_grid_attach(GTK_GRID (grid), ultimate_radio, 0, 8, 1, 1);
  label = gtk_label_new_with_mnemonic (_("U_ltimate"));
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), ultimate_radio);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);
  gtk_grid_attach(GTK_GRID (grid), label, 1, 8, 1, 1);

  label = gtk_label_new (_("You consider this key valid, and trust the user "
                           "so much that you will consider any key signed "
                           "by him/her fully valid.\n\n"
                           "(Warning: This is intended to be used for keys "
                           "you own. Don't use it with other people's keys "
                           "unless you really know what you are doing)\n"));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_widget_set_valign(label, GTK_ALIGN_START);
  gtk_grid_attach(GTK_GRID (grid), label, 1, 9, 1, 1);

  /* Initialize */
  init_radio_buttons (trust, unknown_radio, never_radio, marginal_radio, 
                      full_radio, ultimate_radio);

  gtk_container_add (GTK_CONTAINER (frame), grid);
  gtk_window_resize(GTK_WINDOW(dialog), 400, 300);

  /* Run */
  gtk_widget_show_all (dialog);
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  /* Return the ownertrust */
  if (response == GTK_RESPONSE_OK) 
    {
      gpgme_validity_t new_trust = get_selected_validity (unknown_radio,
							  never_radio, 
							  marginal_radio,
							  full_radio,
							  ultimate_radio);
      /* If the user didn't change the trust, don't edit the key */
      if (trust == new_trust ||
          (trust == GPGME_VALIDITY_UNDEFINED && 
           new_trust == GPGME_VALIDITY_UNKNOWN))
        {
          result = FALSE;
        }
      else
        {
	  *return_trust = new_trust;
          result = TRUE;
        }
    }
  else
    {
      result = FALSE;
    }

  gtk_widget_destroy (dialog);
  return result;
}
