/* # vim: tabstop=2 shiftwidth=2 expandtab
 *
 * Copyright (C) 2019 Armin Luntzer (armin.luntzer@univie.ac.at)
 *               Department of Astrophysics, University of Vienna
 *
 * The initial version of this project was developed as part of the
 * activities under ESA/PRODEX contract number C4000126224.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GTK_NODE__H__
#define __GTK_NODE__H__

#define GTK_COMPILATION

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtknodeview.h>
#include <gtknodesocket.h>
#include <gtk/gtkbox.h>

G_BEGIN_DECLS


#define GTKNODES_TYPE_NODE            (gtk_nodes_node_get_type ())
#define GTKNODES_NODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTKNODES_TYPE_NODE, GtkNodesNode))
#define GTKNODES_NODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  GTKNODES_TYPE_NODE, GtkNodesNodeClass))
#define GTKNODES_IS_NODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTKNODES_TYPE_NODE))
#define GTKNODES_IS_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  GTKNODES_TYPE_NODE))
#define GTKNODES_NODE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  GTKNODES_TYPE_NODE, GtkNodesNodeClass))

typedef struct _GtkNodesNode              GtkNodesNode;
typedef struct _GtkNodesNodePrivate       GtkNodesNodePrivate;
typedef struct _GtkNodesNodeClass         GtkNodesNodeClass;

struct _GtkNodesNode
{
  GtkBox widget;

  GtkNodesNodePrivate *priv;
};

struct _GtkNodesNodeClass
{
  GtkBoxClass parent_class;

  /* signals */
  void   (* node_drag_begin)          (GtkWidget *widget,
                                       gint       x,
                                       gint       y);
  void   (* node_drag_end)            (GtkWidget *widget);
  void   (* node_func_clicked)        (GtkWidget *widget);
  void   (* node_socket_connect)      (GtkWidget *widget,
                                       GtkWidget *sink,
                                       GtkWidget *source);
  void   (* node_socket_disconnect)   (GtkWidget *widget,
                                       GtkWidget *sink,
                                       GtkWidget *source);
  void   (* node_socket_destroyed)    (GtkWidget *widget,
                                       GtkWidget *socket);

  /* vtable */
  gchar* (* export_properties)        (GtkNodesNode *node);

  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


GDK_AVAILABLE_IN_ALL
GType          gtk_nodes_node_get_type          (void) G_GNUC_CONST;
GDK_AVAILABLE_IN_ALL
GtkWidget*     gtk_nodes_node_new               (void);
GDK_AVAILABLE_IN_ALL
GtkWidget*     gtk_nodes_node_item_add          (GtkNodesNode         *node,
                                                 GtkWidget            *widget,
                                                 GtkNodesNodeSocketIO  type);
void           gtk_nodes_node_item_set_expand   (GtkNodesNode         *node,
                                                 GtkWidget            *child,
                                                 gboolean              expand);
GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_item_set_fill     (GtkNodesNode         *node,
                                                 GtkWidget            *child,
                                                 gboolean              fill);
GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_item_set_packing  (GtkNodesNode         *node,
                                                 GtkWidget            *child,
                                                 GtkPackType           pack_type);
GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_set_label         (GtkNodesNode         *node,
                                                 const gchar          *label);
GDK_AVAILABLE_IN_ALL
gdouble        gtk_nodes_node_get_socket_radius (GtkNodesNode         *node);
GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_set_socket_radius (GtkNodesNode         *node,
                                                 const gdouble         radius);
GDK_AVAILABLE_IN_ALL
gboolean       gtk_nodes_node_get_expanded      (GtkNodesNode         *node);
GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_set_expanded      (GtkNodesNode         *node,
                                                 gboolean              expanded);
GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_block_expander    (GtkNodesNode         *node);
GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_unblock_expander  (GtkNodesNode         *node);

GDK_AVAILABLE_IN_ALL
GList*         gtk_nodes_node_get_sinks         (GtkNodesNode         *node);

GDK_AVAILABLE_IN_ALL
GList*         gtk_nodes_node_get_sources       (GtkNodesNode         *node);

GDK_AVAILABLE_IN_ALL
gchar*         gtk_nodes_node_export_properties (GtkNodesNode         *node);

GDK_AVAILABLE_IN_ALL
void           gtk_nodes_node_set_icon_name     (GtkNodesNode         *node,
                                                 const gchar          *icon_name);

G_END_DECLS


#endif /* __GTK_NODE_H__ */
