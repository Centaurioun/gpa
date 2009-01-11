/* gpacardreloadop.h - The GpaCardReloadOperation object.
 *	Copyright (C) 2003 Miguel Coca.
 *	Copyright (C) 2008 g10 Code GmbH.
 *
 * This file is part of GPA.
 *
 * GPA is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GPA is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GPA_CARD_RELOAD_OP_H
#define GPA_CARD_RELOAD_OP_H

/* Definitions to define the object.  */
#define GPA_CARD_RELOAD_OPERATION_TYPE \
          (gpa_card_reload_operation_get_type ())

#define GPA_CARD_RELOAD_OPERATION(obj) \
          (G_TYPE_CHECK_INSTANCE_CAST \
            ((obj), GPA_CARD_RELOAD_OPERATION_TYPE,\
              GpaCardReloadOperation))

#define GPA_CARD_RELOAD_OPERATION_CLASS(klass) \
          (G_TYPE_CHECK_CLASS_CAST \
            ((klass), GPA_CARD_RELOAD_OPERATION_TYPE,\
              GpaCardReloadOperationClass))

#define GPA_IS_CARD_RELOAD_OPERATION(obj) \
          (G_TYPE_CHECK_INSTANCE_TYPE \
            ((obj), GPA_CARD_RELOAD_OPERATION_TYPE))

#define GPA_IS_CARD_RELOAD_OPERATION_CLASS(klass) \
          (G_TYPE_CHECK_CLASS_TYPE \
            ((klass), GPA_CARD_RELOAD_OPERATION_TYPE))

#define GPA_CARD_RELOAD_OPERATION_GET_CLASS(obj) \
          (G_TYPE_INSTANCE_GET_CLASS \
            ((obj), GPA_CARD_RELOAD_OPERATION_TYPE,\
              GpaCardReloadOperationClass))


typedef struct _GpaCardReloadOperation GpaCardReloadOperation;
typedef struct _GpaCardReloadOperationClass GpaCardReloadOperationClass;

GType gpa_card_reload_operation_get_type (void) G_GNUC_CONST;


/************************************
 ************ Public API ************
 ************************************/

/* Type for the callback passed to GpaCardReloadOp object creation,
   which is used for passing card data items to the caller. */
typedef void (*gpa_card_reload_cb_t) 
     (void *opaque, const char *identifier, int idx, const void *value);

/* Create a new GpaCardReloadOperation object. Use CB for passing card
   data items to the caller. OPAQUE is the opaque argument for CB.  */
GpaCardReloadOperation *gpa_card_reload_operation_new (GtkWidget *window,
						       gpa_card_reload_cb_t cb,
						       void *opaque);

#endif /*GPA_CARD_RELOAD_OP_H*/
