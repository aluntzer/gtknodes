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

#ifndef __GTK_NODE_VIEW__H__
#define __GTK_NODE_VIEW__H__

#define GTK_COMPILATION

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtk/gtkcontainer.h>


G_BEGIN_DECLS


#define GTKNODES_TYPE_NODE_VIEW            (gtk_nodes_node_view_get_type ())
#define GTKNODES_NODE_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTKNODES_TYPE_NODE_VIEW, GtkNodesNodeView))
#define GTKNODES_NODE_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTKNODES_TYPE_NODE_VIEW, GtkNodesNodeViewClass))
#define GTKNODES_IS_NODE_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTKNODES_TYPE_NODE_VIEW))
#define GTKNODES_IS_NODE_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTKNODES_TYPE_NODE_VIEW))
#define GTKNODES_NODE_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTKNODES_TYPE_NODE_VIEW, GtkNodesNodeViewClass))

typedef struct _GtkNodesNodeView            GtkNodesNodeView;
typedef struct _GtkNodesNodeViewPrivate     GtkNodesNodeViewPrivate;
typedef struct _GtkNodesNodeViewClass       GtkNodesNodeViewClass;

struct _GtkNodesNodeView
{
  GtkContainer widget;

  GtkNodesNodeViewPrivate *priv;
};

struct _GtkNodesNodeViewClass
{
  GtkContainerClass parent_class;

  /* padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


GDK_AVAILABLE_IN_ALL
GType gtk_nodes_node_view_get_type          (void) G_GNUC_CONST;

GDK_AVAILABLE_IN_ALL
GtkWidget*     gtk_nodes_node_view_new      (void);

GDK_AVAILABLE_IN_ALL
gboolean       gtk_nodes_node_view_save (GtkNodesNodeView *node_view,
                                         const gchar      *filename);

GDK_AVAILABLE_IN_ALL
gboolean       gtk_nodes_node_view_load (GtkNodesNodeView *node_view,
                                         const gchar      *filename);

G_END_DECLS


#endif /* __GTK_NODE_VIEW_H__ */
