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

#ifndef __GTK_NODE_SOCKET__H__
#define __GTK_NODE_SOCKET__H__

#define GTK_COMPILATION

#if !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
#error "Only <gtk/gtk.h> can be included directly."
#endif

#include <gtk/gtkwidget.h>


G_BEGIN_DECLS


#define GTKNODES_TYPE_NODE_SOCKET            (gtk_nodes_node_socket_get_type ())
#define GTKNODES_TYPE_NODE_SOCKET_IO         (gtk_nodes_node_socket_io_get_type ())
#define GTKNODES_NODE_SOCKET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTKNODES_TYPE_NODE_SOCKET, GtkNodesNodeSocket))
#define GTKNODES_NODE_SOCKET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTKNODES_TYPE_NODE_SOCKET, GtkNodesNodeSocketClass))
#define GTKNODES_IS_NODE_SOCKET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTKNODES_TYPE_NODE_SOCKET))
#define GTKNODES_IS_NODE_SOCKET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTKNODES_TYPE_NODE_SOCKET))
#define GTKNODES_NODE_SOCKET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTKNODES_TYPE_NODE_SOCKET, GtkNodesNodeSocketClass))


/**
 * GtkNodesNodeSocketIO:
 * @GTKNODES_NODE_SOCKET_DISABLE: the socket IO is deactivated
 * @GTKNODES_NODE_SOCKET_SINK:    the socket is in sink mode
 * @GTKNODES_NODE_SOCKET_SOURCE:  the socket is in source mode
 *
 * If the socket is in SINK mode, it can accept only one input source,
 * if the socket is in source mode, it will provide output to any connected sink
 */

typedef enum
{
  GTKNODES_NODE_SOCKET_DISABLE,
  GTKNODES_NODE_SOCKET_SINK,
  GTKNODES_NODE_SOCKET_SOURCE,
} GtkNodesNodeSocketIO;


typedef struct _GtkNodesNodeSocket              GtkNodesNodeSocket;
typedef struct _GtkNodesNodeSocketPrivate       GtkNodesNodeSocketPrivate;
typedef struct _GtkNodesNodeSocketClass         GtkNodesNodeSocketClass;

struct _GtkNodesNodeSocket
{
  GtkWidget socket;

  GtkNodesNodeSocketPrivate *priv;
};

struct _GtkNodesNodeSocketClass
{
  GtkWidgetClass parent_class;

  void (* socket_drag_begin) (GtkWidget            *widget);
  void (* socket_drag_end)   (GtkWidget            *widget);
  void (* socket_connect)    (GtkWidget            *widget,
                              GtkNodesNodeSocket   *source);
  void (* socket_disconnect) (GtkWidget            *widget,
                              GtkNodesNodeSocket   *source);
  void (* socket_key_change) (GtkWidget            *widget,
                              GtkNodesNodeSocket   *source);
  void (* socket_incoming)   (GtkWidget            *widget,
                              GByteArray            data);
  void (* socket_outgoing)   (GtkWidget            *widget,
                              GByteArray            data);
  void (* socket_destroyed)  (GtkWidget            *widget);


};


GDK_AVAILABLE_IN_ALL
GType               gtk_nodes_node_socket_get_type            (void) G_GNUC_CONST;

GDK_AVAILABLE_IN_ALL
GType               gtk_nodes_node_socket_io_get_type         (void) G_GNUC_CONST;

GDK_AVAILABLE_IN_ALL
GtkWidget*          gtk_nodes_node_socket_new                 (void);

GDK_AVAILABLE_IN_ALL
GtkWidget*          gtk_nodes_node_socket_new_with_io         (const GtkNodesNodeSocketIO  io);

GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_set_rgba            (GtkNodesNodeSocket         *socket,
					                                                     const GdkRGBA              *rgba);
GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_get_rgba            (GtkNodesNodeSocket         *socket,
						                                                   GdkRGBA                    *rgba);
GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_set_radius          (GtkNodesNodeSocket         *socket,
					                                                     const gdouble               radius);
GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_get_radius          (GtkNodesNodeSocket         *socket,
						                                                   gdouble                    *radius);
GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_set_io              (GtkNodesNodeSocket         *socket,
						                                                   GtkNodesNodeSocketIO        io);
GDK_AVAILABLE_IN_ALL
GtkNodesNodeSocketIO gtk_nodes_node_socket_get_io             (GtkNodesNodeSocket         *socket);

GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_set_key             (GtkNodesNodeSocket         *socket,
						                                                   guint                       key);
GDK_AVAILABLE_IN_ALL
guint               gtk_nodes_node_socket_get_key             (GtkNodesNodeSocket         *socket);

GDK_AVAILABLE_IN_ALL
guint               gtk_nodes_node_socket_get_remote_key      (GtkNodesNodeSocket         *socket);

GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_set_id              (GtkNodesNodeSocket         *socket,
						                                                   guint                       id);
GDK_AVAILABLE_IN_ALL
guint               gtk_nodes_node_socket_get_id              (GtkNodesNodeSocket         *socket);

GDK_AVAILABLE_IN_ALL
gboolean            gtk_nodes_node_socket_write               (GtkNodesNodeSocket         *socket,
                                                               GByteArray                 *payload);
GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_disconnect          (GtkNodesNodeSocket         *socket);

GDK_AVAILABLE_IN_ALL
GtkNodesNodeSocket* gtk_nodes_node_socket_get_input           (GtkNodesNodeSocket         *socket);

GDK_AVAILABLE_IN_ALL
void                gtk_nodes_node_socket_connect_sockets     (GtkNodesNodeSocket         *sink,
                                                               GtkNodesNodeSocket         *source);
G_END_DECLS


#endif /* __GTK_NODE_SOCKET_H__ */
