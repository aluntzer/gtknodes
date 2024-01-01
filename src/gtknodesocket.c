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


#include "gtknodesocket.h"

#include "gtk/gtkdragsource.h"


/* gtkprivate.h */
#include "glib-object.h"
#define GTK_NODES_VIEW_PARAM_RW G_PARAM_READWRITE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB


/**
 * SECTION:gtknodesocket
 * @Short_description: A node socket
 * @Title: GtkNodesNodeSocket
 *
 * The #GtkNodesNodeSocket is a widget, serving as a IO transporter to other
 * GtkNodeSockets. The user can set one of three IO modes: SINK, SOURCE and
 * DISABLE (see #GtkNodesNodeSocketIO).
 * A socket in sink mode will accept only a single input source, a socket in
 * source mode will provide output to any connectd sink.
 * Connections are established by drag-and-drop action of a source to
 * a sink by the user. A sink socket will emit the :socket-connect signal on
 * when a connection is established. If the user initiates a drag on a sink
 * which is already connected to a source, the sink will disconnect from the
 * source and the drag event will be redirected to the source socket.
 *
 * In order to identify compatible sockets, a key can be provided by the user.
 * The source key is transported to the sink in the drag data exchange and the
 * sink will reject the connection if the key does not match. This mechanism
 * ensures that only interpretable data is received on the sink input.
 *
 * A key value of 0 is special in that any input will be accepted.
 *
 * If the user changes the key, the socket will emit the :socket-disconnect
 * signal to notify any connected sinks or sources, so the can initiate a
 * disconnect, if their key does not match or is different from 0.
 *
 * # Connections and data transport #
 *
 * Connections on sinks are established by connecting to the :socket-outgoing
 * signal of the source. This means that the source is not aware of the number
 * of connected sinks, as all data is transported by the GType signal system.
 *
 * The user can push output from a source by calling
 * @gtk_nodes_node_socket_write() on the socket. To get data received by a sink,
 * the user must connect to the ::socket-incoming signal.
 *
 * # Cleanup #
 *
 * If a socket is destroyed or disconnects from a source, it will emit the
 * :socket-destroyed and :socket-disconnect signals respectively.
 *
 * Any sockets attached to the destroyed socket will initiate a disconnect.
 *
 *
 */

struct _GtkNodesNodeSocketPrivate
{
  GdkSurface	           *event_surface;

  GtkNodesNodeSocketIO  io;              /* the socket IO mode */
  guint                 id;              /* the numeric identifier of the socket */
  guint                 key;             /* the compatibility key of the socket */
  GdkRGBA               rgba;            /* the socket colour */
  gdouble               radius;          /* the socket radius */

  GtkNodesNodeSocket   *input;           /* the connected input source for SINK IO */
  gulong                input_handler;
  gulong                disconnect_handler;
  gulong                key_change_handler;
  gulong                destroyed_handler;

  guint                 in_node_socket:1;
};

/* Properties */
enum
{
  PROP_0,
  PROP_RGBA,
  PROP_RADIUS,
  PROP_IO,
  PROP_KEY,
  PROP_ID,
  PROP_INPUT_ID,
  NUM_PROPERTIES
};

/* Signals */
enum
{
  SOCKET_DRAG_BEGIN,
  SOCKET_DRAG_END,
  SOCKET_CONNECT,
  SOCKET_DISCONNECT,
  SOCKET_KEY_CHANGE,
  SOCKET_INCOMING,
  SOCKET_OUTGOING,
  SOCKET_DESTROYED,
  LAST_SIGNAL
};


/* gobject overridable methods */
static void     gtk_nodes_node_socket_set_property         (GObject           *object,
                                                            guint              param_id,
                                                            const GValue      *value,
                                                            GParamSpec        *pspec);
static void     gtk_nodes_node_socket_get_property         (GObject           *object,
                                                            guint              param_id,
                                                            GValue            *value,
                                                            GParamSpec        *pspec);

/* widget class basics */
static void     gtk_nodes_node_socket_destroy              (GtkWidget         *widget);
static void     gtk_nodes_node_socket_map                  (GtkWidget         *widget);
static void     gtk_nodes_node_socket_unmap                (GtkWidget         *widget);
static void     gtk_nodes_node_socket_realize              (GtkWidget         *widget);
static void     gtk_nodes_node_socket_unrealize            (GtkWidget         *widget);
static void     gtk_nodes_node_socket_size_allocate        (GtkWidget         *widget,
                                                            GtkAllocation     *allocation);
static gboolean gtk_nodes_node_socket_draw                 (GtkWidget         *widget,
                                                            cairo_t           *cr);

/* widget class size requests */
static void     gtk_nodes_node_socket_get_preferred_width  (GtkWidget         *widget,
                                                            gint              *minimum,
                                                            gint              *natural);
static void     gtk_nodes_node_socket_get_preferred_height (GtkWidget         *widget,
                                                            gint              *minimum,
                                                            gint              *natural);
/* widget class events */
static gboolean gtk_nodes_node_socket_button_press         (GtkWidget         *widget,
                                                            GdkEventButton    *event);
static gboolean gtk_nodes_node_socket_motion_notify        (GtkWidget         *widget,
                                                            GdkEventMotion    *event);


/* internals */
static void     gtk_nodes_node_socket_drag_begin          (GtkWidget          *widget,
                                                           GdkDragContext     *context,
                                                           gpointer            user_data);
static gboolean gtk_nodes_node_socket_drag_motion         (GtkWidget          *widget,
                                                           GdkDragContext     *context,
                                                           gint                x,
                                                           gint                y,
                                                           guint               time);
static void     gtk_nodes_node_socket_drag_data_received  (GtkWidget          *widget,
                                                           GdkDragContext     *context,
                                                           gint                x,
                                                           gint                y,
                                                           GtkSelectionData   *selection_data,
                                                           guint               info,
                                                           guint32             time,
                                                           GtkNodesNodeSocket *socket);
static void     gtk_nodes_node_socket_drag_data_get       (GtkWidget          *widget,
                                                           GdkDragContext     *context,
                                                           GtkSelectionData   *selection_data,
                                                           guint               info,
                                                           guint               time,
                                                           GtkNodesNodeSocket *socket);
static gboolean gtk_nodes_node_socket_drag_failed         (GtkWidget          *widget,
                                                           GdkDragContext     *context,
                                                           GtkDragResult       result,
                                                           gpointer            user_data);
static void     gtk_nodes_node_socket_drag_end            (GtkWidget          *widget,
                                                           GdkDragContext     *context,
                                                           gpointer            user_data);
static void     gtk_nodes_node_socket_input_incoming      (GtkWidget          *widget,
                                                           GByteArray         *data,
                                                           GtkNodesNodeSocket *socket);
static void     gtk_nodes_node_socket_disconnect_signal   (GtkWidget          *widget,
                                                           GtkNodesNodeSocket *socket,
                                                           GtkNodesNodeSocket *sink);
static void     gtk_nodes_node_socket_key_change_signal   (GtkWidget          *widget,
                                                           GtkNodesNodeSocket *source,
                                                           GtkNodesNodeSocket *sink);
static void     gtk_nodes_node_socket_destroyed_signal    (GtkWidget          *widget,
                                                           GtkNodesNodeSocket *sink);
static void     gtk_nodes_node_socket_set_drag_icon       (GdkDragContext     *context,
                                                           GtkNodesNodeSocket *node);
static void     gtk_nodes_node_socket_drag_src_redirect   (GtkWidget          *widget);


static guint node_socket_signals[LAST_SIGNAL] = { 0 };

static const GtkTargetEntry drop_types[] = { { "gtk_nodesocket", GTK_TARGET_SAME_APP, 0 } };

G_DEFINE_TYPE_WITH_PRIVATE(GtkNodesNodeSocket, gtk_nodes_node_socket, GTK_TYPE_WIDGET)


static void
gtk_nodes_node_socket_class_init (GtkNodesNodeSocketClass *class)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *widget_class;


  gobject_class = G_OBJECT_CLASS (class);
  widget_class  = GTK_WIDGET_CLASS (class);

  /* gobject methods */
  gobject_class->get_property = gtk_nodes_node_socket_get_property;
  gobject_class->set_property = gtk_nodes_node_socket_set_property;

  /* widget basics */
  widget_class->destroy       = gtk_nodes_node_socket_destroy;
  widget_class->map           = gtk_nodes_node_socket_map;
  widget_class->unmap         = gtk_nodes_node_socket_unmap;
  widget_class->realize       = gtk_nodes_node_socket_realize;
  widget_class->unrealize     = gtk_nodes_node_socket_unrealize;
  widget_class->size_allocate = gtk_nodes_node_socket_size_allocate;
  widget_class->draw          = gtk_nodes_node_socket_draw;

  /* widget size requests */
  widget_class->get_preferred_width  = gtk_nodes_node_socket_get_preferred_width;
  widget_class->get_preferred_height = gtk_nodes_node_socket_get_preferred_height;

  /* widget events */
  widget_class->button_press_event  = gtk_nodes_node_socket_button_press;
  widget_class->motion_notify_event = gtk_nodes_node_socket_motion_notify;


 /**
   * GtkNodesNodeSocket:rgba:
   *
   * The rgba colour of the socket
   */

  g_object_class_install_property (gobject_class,
                                   PROP_RGBA,
                                   g_param_spec_boxed ("rgba",
                                                       "Current RGBA Color",
                                                       "The RGBA color of the socket",
                                                       GDK_TYPE_RGBA,
                                                       GTK_NODES_VIEW_PARAM_RW));
  /**
   * GtkNodesNodeSocket:radius:
   *
   * The radius of the socket
   */

  g_object_class_install_property (gobject_class,
                                   PROP_RADIUS,
                                   g_param_spec_double ("radius",
                                                        "Current Socket Radius",
                                                        "The radius of the socket",
                                                         1.0,
                                                        30.0,
                                                         8.0,
                                                        GTK_NODES_VIEW_PARAM_RW));
  /**
   * GtkNodesNodeSocket:io:
   *
   * The type of socket (input or output)
   */

  g_object_class_install_property (gobject_class,
                                   PROP_IO,
                                   g_param_spec_enum ("io",
                                                      "Socket I/O Type",
                                                      "The configured socket type, either input or output",
                                                      GTKNODES_TYPE_NODE_SOCKET_IO,
                                                      GTKNODES_NODE_SOCKET_DISABLE,
                                                      GTK_NODES_VIEW_PARAM_RW));

  /**
   * GtkNodesNodeSocket:key:
   *
   * The compatibility key of the socket
   */

  g_object_class_install_property (gobject_class,
                                   PROP_KEY,
                                   g_param_spec_uint ("key",
                                                      "Socket Compatibility Key",
                                                      "The socket compatibility key",
                                                      0,
                                                      G_MAXUINT,
                                                      0,
                                                      GTK_NODES_VIEW_PARAM_RW));
  /**
   * GtkNodesNodeSocket:id:
   *
   * The numeric identifier of the socket
   */

  g_object_class_install_property (gobject_class,
                                   PROP_ID,
                                   g_param_spec_uint ("id",
                                                      "Socket Numeric Identifier",
                                                      "The socket numeric identifier",
                                                      0,
                                                      G_MAXUINT,
                                                      0,
                                                      GTK_NODES_VIEW_PARAM_RW));

  /**
   * GtkNodesNodeSocket::socket-drag-begin:
   * @widget: the object which received the signal.
   *
   * The ::socket-drag-begin signal is emitted when the user begins a drag
   * on the socket handle.
   *
   */

  node_socket_signals[SOCKET_DRAG_BEGIN] =
    g_signal_new ("socket-drag-begin",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeSocketClass, socket_drag_begin),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE, 0);

  /**
   * GtkNodesNodeSocket::socket-drag-end:
   * @widget: the object which received the signal.
   *
   * The ::socket-drag-begin signal is emitted when the user ends a drag
   * on the socket handle.
   *
   */

  node_socket_signals[SOCKET_DRAG_END] =
    g_signal_new ("socket-drag-end",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeSocketClass, socket_drag_begin),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE, 0);

  /**
   * GtkNodesNodeSocket::socket-connect:
   * @widget: the object which received the signal.
   * @source: the #GtkNodesNodeSocket this socket connected to
   *
   * The ::socket-connect signal is emitted when a sink socket connects
   * to a source socket
   *
   */

  node_socket_signals[SOCKET_CONNECT] =
    g_signal_new ("socket-connect",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeSocketClass, socket_connect),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, GTKNODES_TYPE_NODE_SOCKET);

  /**
   * GtkNodesNodeSocket::socket-disconnect:
   * @widget: the object which received the signal.
   * @source: the #GtkNodesNodeSocket this socket disconnected from, may be NULL
   *
   * The ::socket-disconnect signal is emitted when a sink socket disconnects
   * from a source socket or notifies sinks to disconnect because of a change
   * in configuration (in this case @source is NULL)
   *
   */

  node_socket_signals[SOCKET_DISCONNECT] =
    g_signal_new ("socket-disconnect",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeSocketClass, socket_disconnect),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, GTKNODES_TYPE_NODE_SOCKET);

  /**
   * GtkNodesNodeSocket::socket-key-change:
   * @widget: the object which received the signal.
   * @source: the #GtkNodesNodeSocket which changed the key, may be NULL
   *
   * The ::socket-key-change signal is emitted when a sink socket changes its
   * key
   *
   */

  node_socket_signals[SOCKET_KEY_CHANGE] =
    g_signal_new ("socket-key-change",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeSocketClass, socket_key_change),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, GTKNODES_TYPE_NODE_SOCKET);

  /**
   * GtkNodesNodeSocket::socket-incoming:
   * @widget: the object which received the signal.
   * @data:   the data pointer
   * @size:   the size of the data in bytes
   *
   * The ::socket-incoming signal is emitted when data is incoming on the socket
   * The data pointer reference and size are transported via the callback.
   */

  node_socket_signals[SOCKET_INCOMING] =
    g_signal_new ("socket-incoming",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeSocketClass, socket_incoming),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, G_TYPE_BYTE_ARRAY);

  /**
   * GtkNodesNodeSocket::socket-outgoing:
   * @widget: the object which received the signal.
   * @data:   the data pointer
   * @size:   the size of the data in bytes
   *
   * The ::socket-outgoing signal is emitted when data is outgoing on the socket
   * The data pointer reference and size are transported via the callback.
   */

  node_socket_signals[SOCKET_OUTGOING] =
    g_signal_new ("socket-outgoing",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, G_TYPE_BYTE_ARRAY);

  /**
   * GtkNodesNodeSocket::socket-destroyed:
   * @widget: the object which received the signal.
   *
   * The ::socket-drag-begin signal is emitted when the user ends a drag
   * on the socket handle.
   *
   */

  node_socket_signals[SOCKET_DESTROYED] =
    g_signal_new ("socket-destroyed",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeSocketClass, socket_destroyed),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE, 0);



}

static void
gtk_nodes_node_socket_init (GtkNodesNodeSocket *socket)
{
  GtkNodesNodeSocketPrivate *priv;

  /* Create the widgets */
  priv = socket->priv = gtk_nodes_node_socket_get_instance_private (socket);

  gtk_widget_set_can_focus (GTK_WIDGET (socket), TRUE);
  gtk_widget_set_receives_default (GTK_WIDGET (socket), TRUE);
  gtk_widget_set_has_window (GTK_WIDGET (socket), FALSE);


  priv->rgba.red   = 1.0;
  priv->rgba.green = 1.0;
  priv->rgba.blue  = 1.0;
  priv->rgba.alpha = 1.0;

  priv->radius = 8.0;

  priv->key = 0; /* accept any connection by default */

  priv->in_node_socket = FALSE;

  priv->input = NULL;


  g_signal_connect (socket, "drag-begin",
                    G_CALLBACK (gtk_nodes_node_socket_drag_begin), socket);
  g_signal_connect (socket, "drag-motion",
                    G_CALLBACK (gtk_nodes_node_socket_drag_motion), socket);
  g_signal_connect (socket, "drag-failed",
                    G_CALLBACK (gtk_nodes_node_socket_drag_failed), socket);
  g_signal_connect (socket, "drag-end",
                    G_CALLBACK (gtk_nodes_node_socket_drag_end), socket);
  g_signal_connect (socket, "drag-data-received",
                    G_CALLBACK (gtk_nodes_node_socket_drag_data_received),
                    socket);
  g_signal_connect (socket, "drag-data-get",
                    G_CALLBACK (gtk_nodes_node_socket_drag_data_get), socket);

}


/* GObject Methods */

static void
gtk_nodes_node_socket_get_property (GObject    *object,
                                    guint       param_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GtkNodesNodeSocket *socket;


  socket = GTKNODES_NODE_SOCKET (object);

  switch (param_id)
    {
    case PROP_RGBA:
      {
        GdkRGBA rgba;

        gtk_nodes_node_socket_get_rgba (socket, &rgba);
        g_value_set_boxed (value, &rgba);
      }
      break;
    case PROP_RADIUS:
      {
        gdouble radius;

        gtk_nodes_node_socket_get_radius(socket, &radius);
        g_value_set_double (value, radius);
      }
      break;
    case PROP_IO:
      g_value_set_enum (value, gtk_nodes_node_socket_get_io(socket));
      break;
    case PROP_KEY:
      g_value_set_uint (value, gtk_nodes_node_socket_get_key(socket));
      break;
    case PROP_ID:
      g_value_set_uint (value, gtk_nodes_node_socket_get_id(socket));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
gtk_nodes_node_socket_set_property (GObject      *object,
                                    guint         param_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  GtkNodesNodeSocket *socket;


  socket = GTKNODES_NODE_SOCKET (object);

  switch (param_id)
    {
    case PROP_RGBA:
      gtk_nodes_node_socket_set_rgba (socket, g_value_get_boxed (value));
      break;
    case PROP_RADIUS:
      gtk_nodes_node_socket_set_radius (socket, g_value_get_double (value));
      break;
    case PROP_IO:
      gtk_nodes_node_socket_set_io (socket, g_value_get_enum (value));
      break;
    case PROP_KEY:
      gtk_nodes_node_socket_set_key (socket, g_value_get_uint (value));
      break;
    case PROP_ID:
      gtk_nodes_node_socket_set_id (socket, g_value_get_uint (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

/* Widget Methods */

static void
gtk_nodes_node_socket_destroy (GtkWidget *widget)
{
  g_signal_emit (widget, node_socket_signals[SOCKET_DESTROYED], 0);

  /* disconnect any inputs */
  gtk_nodes_node_socket_disconnect (GTKNODES_NODE_SOCKET (widget));

  GTK_WIDGET_CLASS (gtk_nodes_node_socket_parent_class)->destroy (widget);
}

static void
gtk_nodes_node_socket_map (GtkWidget *widget)
{
  GtkNodesNodeSocket *socket;
  GtkNodesNodeSocketPrivate *priv;


  socket = GTKNODES_NODE_SOCKET (widget);
  priv = gtk_nodes_node_socket_get_instance_private (socket);


  GTK_WIDGET_CLASS (gtk_nodes_node_socket_parent_class)->map (widget);

  if (priv->event_surface)
    gdk_window_show(priv->event_surface);

}

static void
gtk_nodes_node_socket_unmap (GtkWidget *widget)
{
  GtkNodesNodeSocket *socket;
  GtkNodesNodeSocketPrivate *priv;


  socket = GTKNODES_NODE_SOCKET (widget);
  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (priv->event_surface)
    gdk_window_hide (priv->event_surface);

  GTK_WIDGET_CLASS (gtk_nodes_node_socket_parent_class)->unmap (widget);

}

static void gtk_nodes_node_socket_realize (GtkWidget *widget)
{

  GtkNodesNodeSocket *socket;
  GtkNodesNodeSocketPrivate *priv;
  GtkAllocation allocation;
  GdkSurface *surface;
  GdkWindowAttr attributes;
  gint attributes_mask;


  socket = GTKNODES_NODE_SOCKET (widget);
  priv = gtk_nodes_node_socket_get_instance_private (socket);

  gtk_widget_set_realized (widget, TRUE);

  window = gtk_widget_get_parent_window (widget);
  g_object_ref (window);
  gtk_widget_set_window (widget, window);

  gtk_widget_get_allocation (widget, &allocation);

  /* event window of size of circle */
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x           = allocation.x;
  attributes.y           = allocation.y;
  attributes.width       = 2.0 * priv->radius;
  attributes.height      = 2.0 * priv->radius;
  attributes.wclass      = GDK_INPUT_ONLY;
  attributes.event_mask  = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK   |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_TOUCH_MASK          |
                            GDK_ENTER_NOTIFY_MASK   |
                            GDK_LEAVE_NOTIFY_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  priv->event_surface = gdk_window_new (window, &attributes, attributes_mask);

  gtk_widget_register_window (widget, priv->event_surface);
}

static void
gtk_nodes_node_socket_unrealize (GtkWidget *widget)
{
  GtkNodesNodeSocket *socket;
  GtkNodesNodeSocketPrivate *priv;


  socket = GTKNODES_NODE_SOCKET (widget);
  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (priv->event_surface)
  {
    gtk_widget_unregister_window (widget, priv->event_surface);
    gdk_window_destroy (priv->event_surface);
    priv->event_surface = NULL;
  }

  g_signal_emit (widget, node_socket_signals[SOCKET_DESTROYED], 0);

  GTK_WIDGET_CLASS (gtk_nodes_node_socket_parent_class)->unrealize (widget);
}

static void
gtk_nodes_node_socket_size_allocate (GtkWidget     *widget,
                                     GtkAllocation *allocation)
{
  GtkNodesNodeSocketPrivate *priv;


  priv = gtk_nodes_node_socket_get_instance_private (GTKNODES_NODE_SOCKET (widget));

  gtk_widget_set_allocation (widget, allocation);


  if (!gtk_widget_get_realized (widget))
    return;

  if (!priv->event_surface)
    return;

  /* move and resize to circle representation */
  gdk_window_move_resize (priv->event_surface,
                          (gint) allocation->x,
                          (gint) allocation->y,
                          (gint) (2.0 * priv->radius),
                          (gint) (2.0 * priv->radius));
}

static gboolean
gtk_nodes_node_socket_draw (GtkWidget *widget,
                            cairo_t   *cr)
{
  GtkNodesNodeSocketPrivate *priv;


  priv = gtk_nodes_node_socket_get_instance_private (GTKNODES_NODE_SOCKET (widget));

  if (priv->io == GTKNODES_NODE_SOCKET_DISABLE)
    return GDK_EVENT_PROPAGATE;


  cairo_save(cr);

  gdk_cairo_set_source_rgba (cr, &priv->rgba);

  cairo_arc (cr, priv->radius, priv->radius,
             priv->radius, 0.0, 2.0 * G_PI);


  cairo_fill(cr);
  cairo_restore(cr);


  return GDK_EVENT_PROPAGATE;
}

static void
gtk_nodes_node_socket_get_preferred_width (GtkWidget *widget,
                                           gint      *minimum,
                                           gint      *natural)
{
  GtkNodesNodeSocketPrivate *priv;

  priv = gtk_nodes_node_socket_get_instance_private (GTKNODES_NODE_SOCKET (widget));

  /* always be a as wide as the circle diameter */
  (*minimum ) = (gint) (2.0 * priv->radius);
  (*natural ) = (*minimum);
}

static void
gtk_nodes_node_socket_get_preferred_height (GtkWidget *widget,
                                            gint      *minimum,
                                            gint      *natural)
{
  GtkNodesNodeSocket        *socket;
  GtkNodesNodeSocketPrivate *priv;

  socket = GTKNODES_NODE_SOCKET (widget);
  priv = gtk_nodes_node_socket_get_instance_private (socket);

  /* always be a as high as the circle diameter */
  (*minimum ) = (gint) (2.0 * priv->radius);
  (*natural ) = (*minimum);
}

static gboolean
gtk_nodes_node_socket_button_press (GtkWidget      *widget,
                                    GdkEventButton *event)
{
  return TRUE;
}

static gboolean
gtk_nodes_node_socket_motion_notify (GtkWidget      *widget,
                                     GdkEventMotion *event)
{
  return TRUE;
}


/* Internal Methods */

static void
gtk_nodes_node_socket_set_drag_icon (GdkDragContext     *context,
                                     GtkNodesNodeSocket *node)
{
  GtkNodesNodeSocketPrivate *priv;

  cairo_t *cr;
  cairo_surface_t *surface;


  priv = gtk_nodes_node_socket_get_instance_private (node);


  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                        (int) (2.0 * priv->radius),
                                        (int) (2.0 * priv->radius));
  cr = cairo_create (surface);

  gdk_cairo_set_source_rgba (cr, &priv->rgba);

  cairo_arc (cr, priv->radius, priv->radius,
             priv->radius, 0.0, 2.0 * G_PI);

  cairo_fill (cr);

  gtk_drag_set_icon_surface (context, surface);

  cairo_destroy (cr);
  cairo_surface_destroy (surface);
}

static void
gtk_nodes_node_socket_drag_src_redirect (GtkWidget *widget)
{
  GtkWidget *source;

  GtkNodesNodeSocket        *socket;
  GtkNodesNodeSocketPrivate *priv;

  socket = GTKNODES_NODE_SOCKET (widget);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (!priv->input)
    return;


  source = GTK_WIDGET (priv->input);

  g_signal_handler_disconnect (source, priv->input_handler);
  g_signal_handler_disconnect (source, priv->disconnect_handler);
  g_signal_handler_disconnect (source, priv->key_change_handler);
  g_signal_handler_disconnect (source, priv->destroyed_handler);
  priv->input_handler      = 0;
  priv->disconnect_handler = 0;
  priv->key_change_handler = 0;
  priv->destroyed_handler  = 0;
  g_signal_emit (widget, node_socket_signals[SOCKET_DISCONNECT], 0, source);
  priv->input = NULL;

  /* remove as drag source */
  if (priv->io == GTKNODES_NODE_SOCKET_SINK)
    gtk_drag_source_unset (GTK_WIDGET (widget));

  /* begin drag on previous source, so user can redirect connection */
  gtk_drag_begin_with_coordinates (source,
                                   gtk_target_list_new (drop_types, 1),
                                   GDK_ACTION_COPY,
                                   GDK_BUTTON1_MASK|GDK_BUTTON3_MASK,
                                   NULL, -1, -1);

}

static void
gtk_nodes_node_socket_drag_begin (GtkWidget      *widget,
                                  GdkDragContext *context,
                                  gpointer        user_data)
{
  /* we apparently have to set it... */
  gtk_nodes_node_socket_set_drag_icon (context, GTKNODES_NODE_SOCKET (widget));

  /* ...to hide it */
  gdk_window_hide (gdk_drag_context_get_drag_window (context));

  g_signal_emit (widget, node_socket_signals[SOCKET_DRAG_BEGIN], 0);

  /* if we're connected, disconnect here and abort the drag and reroute
   * it to our original source socket */
  gtk_nodes_node_socket_drag_src_redirect(widget);
}

static gboolean
gtk_nodes_node_socket_drag_motion (GtkWidget      *widget,
                                   GdkDragContext *context,
                                   gint            x,
                                   gint            y,
                                   guint           time)
{
  return GDK_EVENT_PROPAGATE;
}

static void
gtk_nodes_node_socket_connect_sockets_internal (GtkNodesNodeSocket *sink,
                                                GtkNodesNodeSocket *source)
{
  GtkNodesNodeSocketPrivate *priv_sink;
  GtkNodesNodeSocketPrivate *priv_source;


  priv_sink = gtk_nodes_node_socket_get_instance_private (sink);

  priv_source = gtk_nodes_node_socket_get_instance_private (source);


  if (priv_source->io != GTKNODES_NODE_SOCKET_SOURCE)
    {
      g_warning("Node Socket %p not in source mode.", (void *) priv_source);
      return;
    }

  if (priv_sink->io != GTKNODES_NODE_SOCKET_SINK)
    {
      g_warning("Node Socket %p not in sink mode.", (void *) priv_sink);
      return;
    }



  if (priv_sink->key && (priv_source->key != priv_sink->key))
    {
      g_message("Node Socket keys incompatible, source rejected");
      return;
    }

  /* if there is an input source, disconnect it */
  gtk_nodes_node_socket_disconnect (sink);

  priv_sink->input = source;

  priv_sink->input_handler =
    g_signal_connect (G_OBJECT (priv_sink->input), "socket-outgoing",
                      G_CALLBACK (gtk_nodes_node_socket_input_incoming),
                      sink);


  priv_sink->disconnect_handler =
    g_signal_connect (G_OBJECT (priv_sink->input), "socket-disconnect",
                      G_CALLBACK (gtk_nodes_node_socket_disconnect_signal),
                      sink);

  priv_sink->key_change_handler =
    g_signal_connect (G_OBJECT (priv_sink->input), "socket-key-change",
                      G_CALLBACK (gtk_nodes_node_socket_key_change_signal),
                      sink);


  priv_sink->destroyed_handler =
    g_signal_connect (G_OBJECT (priv_sink->input), "socket-destroyed",
                      G_CALLBACK (gtk_nodes_node_socket_destroyed_signal),
                      sink);



  /* become a drag source, so the user can disconnect from the sink */
  gtk_drag_source_set (GTK_WIDGET (sink),
                       GDK_BUTTON1_MASK|GDK_BUTTON3_MASK,
                       drop_types, 1,
                       GDK_ACTION_COPY);

  /* emit notification of connection from sink */
  g_signal_emit (sink, node_socket_signals[SOCKET_CONNECT], 0, source);

  /* emit notification of connection from source */
  g_signal_emit (source, node_socket_signals[SOCKET_CONNECT], 0, source);

}


static void
gtk_nodes_node_socket_drag_data_received (GtkWidget         *widget,
                                          GdkDragContext    *context,
                                          gint               x,
                                          gint               y,
                                          GtkSelectionData  *selection_data,
                                          guint              info,
                                          guint32            time,
                                          GtkNodesNodeSocket *socket)
{
  void *ref;
  GtkNodesNodeSocket *source;


  /* we want the source socket so we can connect to our input */
  ref = (* ((gpointer *) gtk_selection_data_get_data (selection_data)));
  source = GTKNODES_NODE_SOCKET (ref);

  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (source));

  g_return_if_fail (((gpointer) widget) == ((gpointer) socket));

  gtk_nodes_node_socket_connect_sockets_internal (socket, source);
}

static void
gtk_nodes_node_socket_drag_data_get (GtkWidget          *widget,
                                     GdkDragContext     *context,
                                     GtkSelectionData   *selection_data,
                                     guint               info,
                                     guint               time,
                                     GtkNodesNodeSocket *socket)
{
  /* provide the socket reference so the other side can connect */
  gtk_selection_data_set (selection_data,
                          gtk_selection_data_get_target (selection_data),
                          32, (const guchar *) &socket, sizeof(gpointer));
}

static gboolean
gtk_nodes_node_socket_drag_failed (GtkWidget      *widget,
                                   GdkDragContext *context,
                                   GtkDragResult   result,
                                   gpointer        user_data)
{
  /* do not show drag cancel animation */
  return TRUE;
}

static void
gtk_nodes_node_socket_drag_end (GtkWidget      *widget,
                                GdkDragContext *context,
                                gpointer        user_data)
{
  GtkNodesNodeSocket *socket;
  GtkNodesNodeSocketPrivate *priv;


  socket = GTKNODES_NODE_SOCKET (user_data);
  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (priv->io != GTKNODES_NODE_SOCKET_SINK)
    g_signal_emit (widget, node_socket_signals[SOCKET_DRAG_END], 0);
}

static void
gtk_nodes_node_socket_input_incoming (GtkWidget          *widget,
                                      GByteArray         *payload,
                                      GtkNodesNodeSocket *socket)
{
  gtk_nodes_node_socket_write(socket, payload);
}

static void
gtk_nodes_node_socket_disconnect_signal (GtkWidget          *widget,
                                         GtkNodesNodeSocket *source,
                                         GtkNodesNodeSocket *sink)
{
  gtk_nodes_node_socket_disconnect (sink);
}

static void
gtk_nodes_node_socket_key_change_signal (GtkWidget          *widget,
                                         GtkNodesNodeSocket *source,
                                         GtkNodesNodeSocket *sink)
{
  GtkNodesNodeSocketPrivate *priv;

  priv = gtk_nodes_node_socket_get_instance_private (sink);

  /* we disconnect if our key does not match */
  if (priv->key != gtk_nodes_node_socket_get_remote_key (sink))
    gtk_nodes_node_socket_disconnect (sink);
}

static void
gtk_nodes_node_socket_destroyed_signal (GtkWidget          *widget,
                                        GtkNodesNodeSocket *sink)
{
  /* technically, this is not necessary right now, because our signal
   * callbacks will be disconnect on destruction of a source anyways, but
   * we may want to do other things in the future, so we'll handle that signa
   * alyways
   */
  gtk_nodes_node_socket_disconnect (sink);
}


/* public methods */


/**
 * gtk_nodes_node_socket_set_rgba:
 * @socket: a #GtkNodesNodeSocket
 * @rgba: the #GdkRGBA colour to set
 *
 * Sets the GdkRGBA colour of the socket
 */

void
gtk_nodes_node_socket_set_rgba (GtkNodesNodeSocket *socket,
                                const GdkRGBA      *rgba)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));
  g_return_if_fail (rgba != NULL);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  priv->rgba = (* rgba);

  g_object_notify (G_OBJECT (socket), "rgba");

  gtk_widget_queue_draw (GTK_WIDGET (socket));
}

/**
 * gtk_nodes_node_socket_get_rgba:
 * @socket: a #GtkNodesNodeSocket
 * @rgba: the #GdkRGBA colour to get
 *
 * Sets the GdkRGBA colour of the socket
 */

void
gtk_nodes_node_socket_get_rgba (GtkNodesNodeSocket *socket,
                                GdkRGBA            *rgba)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));
  g_return_if_fail (rgba != NULL);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  (* rgba) = priv->rgba;
}

/**
 * gtk_nodes_node_socket_set_radius:
 * @socket: a #GtkNodesNodeSocket
 * @radius: the socket radius to set
 *
 * Sets the radius of the socket
 */

void
gtk_nodes_node_socket_set_radius (GtkNodesNodeSocket *socket,
                                  const gdouble       radius)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  priv->radius = radius;

  g_object_notify (G_OBJECT (socket), "radius");

  gtk_widget_queue_resize (GTK_WIDGET (socket));
}

/**
 * gtk_nodes_node_socket_get_radius:
 * @socket: a #GtkNodesNodeSocket
 * @radius: the socket radius to get
 *
 * Gets the radius of the socket
 */

void
gtk_nodes_node_socket_get_radius (GtkNodesNodeSocket *socket,
                                  gdouble            *radius)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));
  g_return_if_fail (radius != NULL);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  (* radius) = priv->radius;
}

/**
 * gtk_nodes_node_socket_set_io
 * @socket: a #GtkNodesNodeSocket
 * @io: a #GtkNodesNodeSocketIO
 *
 * Sets the IO mode of the socket. Changing the mode will disconnect
 * existing source inputs, as these are tracked internally.
 * Attached sinks and sources will be notified by the ::socket-disconnect
 * signal. It is the responsibility of the user to disconnect their
 * ::socket-incoming signal callbacks. If the user attempts to set the
 * current socket mode, no action will be taken.
 */

void
gtk_nodes_node_socket_set_io (GtkNodesNodeSocket   *socket,
                              GtkNodesNodeSocketIO  io)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (priv->io == io)
    return;

  priv->io = io;

  /* if there is an input source, disconnect it */
  gtk_nodes_node_socket_disconnect (socket);

  /* notify any sinks so they can disconnect */
  g_signal_emit (GTK_WIDGET (socket),
                 node_socket_signals[SOCKET_DISCONNECT], 0, NULL);

  if (io == GTKNODES_NODE_SOCKET_SOURCE)
    {
      gtk_drag_source_set (GTK_WIDGET (socket),
                           GDK_BUTTON1_MASK|GDK_BUTTON3_MASK,
                           drop_types, 1,
                           GDK_ACTION_COPY);


      gtk_drag_dest_unset (GTK_WIDGET (socket));
    }


  if (io == GTKNODES_NODE_SOCKET_SINK)
    {
      gtk_drag_source_unset (GTK_WIDGET (socket));

      gtk_drag_dest_set (GTK_WIDGET (socket),
                         GTK_DEST_DEFAULT_MOTION |
                         GTK_DEST_DEFAULT_HIGHLIGHT |
                         GTK_DEST_DEFAULT_DROP,
                         drop_types, 1, GDK_ACTION_COPY);
      gtk_drag_dest_set_track_motion(GTK_WIDGET(socket), TRUE);
    }

  gtk_widget_queue_allocate(GTK_WIDGET (socket));
}


/**
 * gtk_nodes_node_socket_get_io
 * @socket: a #GtkNodesNodeSocket
 *
 * Returns: the #GtkNodesNodeSocketIO mode of the socket
 */

GtkNodesNodeSocketIO
gtk_nodes_node_socket_get_io (GtkNodesNodeSocket *socket)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_val_if_fail (GTKNODES_IS_NODE_SOCKET (socket),
                        GTKNODES_NODE_SOCKET_DISABLE);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  return priv->io;
}


/**
 * gtk_nodes_node_socket_set_key
 * @socket: a #GtkNodesNodeSocket
 * @key: a compatibility key (0 == any)
 *
 * Sets the compatibility key of the socket. Changing the key will disconnect
 * any existing source inputs, unless the key is set to 0 or their source
 * key matches.
 * Attached sinks will not be notified, it is the responsibility of the user
 * to disconnect socket-incoming signals. If the user attempts to set the
 * current key, no action will be taken.
 */

void
gtk_nodes_node_socket_set_key (GtkNodesNodeSocket *socket,
                               guint               key)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (priv->key == key)
    return;

  priv->key = key;

  if (!key)
    return;


  /* the key is not 0; if there is a non-matching input source, disconnect it */
  if (priv->key != gtk_nodes_node_socket_get_remote_key (socket))
    gtk_nodes_node_socket_disconnect (socket);

  /* notify any sinks so they can disconnect */
  g_signal_emit (GTK_WIDGET (socket),
                 node_socket_signals[SOCKET_KEY_CHANGE], 0, socket);
}

/**
 * gtk_nodes_node_socket_get_key
 * @socket: a #GtkNodesNodeSocket
 *
 * @returns the compatibility key of the socket (always 0 on error)
 */

GtkNodesNodeSocketIO
gtk_nodes_node_socket_get_key (GtkNodesNodeSocket *socket)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_val_if_fail (GTKNODES_IS_NODE_SOCKET (socket), 0);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  return priv->key;
}

/**
 * gtk_nodes_node_socket_get_remote_key
 * @socket: a #GtkNodesNodeSocket
 *
 * @returns the compatibility key of the input socket (always 0 if key cannot be determined)
 */

GtkNodesNodeSocketIO
gtk_nodes_node_socket_get_remote_key (GtkNodesNodeSocket *socket)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_val_if_fail (GTKNODES_IS_NODE_SOCKET (socket), 0);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (priv->input)
    return gtk_nodes_node_socket_get_key (priv->input);

  return 0;
}

/**
 * gtk_nodes_node_socket_set_id
 * @socket: a #GtkNodesNodeSocket
 * @id: the socket identifier to set
 *
 * Sets the numeric identifier of the socket
 * Note: can not be changed once set != 0
 */

void
gtk_nodes_node_socket_set_id (GtkNodesNodeSocket *socket,
                              guint               id)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  if (!priv->id)
          priv->id = id;
}


/**
 * gtk_nodes_node_socket_get_id
 * @socket: a #GtkNodesNodeSocket
 *
 * Returns: the numeric identifier of the socket (always 0 on error)
 */

GtkNodesNodeSocketIO
gtk_nodes_node_socket_get_id (GtkNodesNodeSocket *socket)
{
  GtkNodesNodeSocketPrivate *priv;


  g_return_val_if_fail (GTKNODES_IS_NODE_SOCKET (socket), 0);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  return priv->id;
}


/**
 * gtk_nodes_node_socket_write:
 * @socket: a #GtkNodesNodeSocket
 * @payload: the data buffer to write
 *
 * Emits a signal on the #GtkNodesNodeSocket in incoming our outgoing direction.
 *
 * Returns: TRUE on success, FALSE if GTKNODES_NODE_SOCKET_DISABLE is configured
 */

gboolean
gtk_nodes_node_socket_write (GtkNodesNodeSocket *socket,
                             GByteArray     *payload)
{
  GtkNodesNodeSocketPrivate *priv;

  g_return_val_if_fail (GTKNODES_IS_NODE_SOCKET (socket), FALSE);

  priv = gtk_nodes_node_socket_get_instance_private (socket);


  if (priv->io == GTKNODES_NODE_SOCKET_DISABLE)
    return FALSE;

  if (priv->io == GTKNODES_NODE_SOCKET_SINK)
    g_signal_emit (socket, node_socket_signals[SOCKET_INCOMING], 0, payload);

  if (priv->io == GTKNODES_NODE_SOCKET_SOURCE)
    g_signal_emit (socket, node_socket_signals[SOCKET_OUTGOING], 0, payload);

  return TRUE;
}


/**
 * gtk_nodes_node_socket_disconnect:
 * @socket: a #GtkNodesNodeSocket
 *
 * Drops all connections on a given socket. Attached sink or source sockets
 * will be notified by the respective signals
 */

void
gtk_nodes_node_socket_disconnect (GtkNodesNodeSocket *socket)
{
  GtkNodesNodeSocketPrivate *priv;

  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (socket));

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  /* if there is an input source, disconnect it */
  if (priv->input)
    {
      g_signal_handler_disconnect (priv->input, priv->input_handler);
      g_signal_handler_disconnect (priv->input, priv->disconnect_handler);
      g_signal_handler_disconnect (priv->input, priv->key_change_handler);
      g_signal_handler_disconnect (priv->input, priv->destroyed_handler);
      priv->input_handler      = 0;
      priv->disconnect_handler = 0;
      priv->key_change_handler = 0;
      priv->destroyed_handler  = 0;
      g_signal_emit (GTK_WIDGET (socket),
                     node_socket_signals[SOCKET_DISCONNECT], 0, priv->input);
      priv->input = NULL;
    }
}


/**
 * gtk_nodes_node_socket_connect_sockets:
 * @sink: a #GtkNodesNodeSocket in sink mode
 * @source: a #GtkNodesNodeSocket in source mode
 *
 * Explicitly establishes a connection between two sockets. If the sockets
 * are not in the proper mode, the connection will fail. If the sink is already
 * connected to a source, the source will be disconnected from the sink
 * before connecting to the new source. If the compatibility keys of the
 * sockets do not match, the connection will fail as well.
 */

void
gtk_nodes_node_socket_connect_sockets (GtkNodesNodeSocket *sink,
                                       GtkNodesNodeSocket *source)
{
  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (sink));
  g_return_if_fail (GTKNODES_IS_NODE_SOCKET (source));

  gtk_nodes_node_socket_connect_sockets_internal (sink, source);

}

/**
 * gtk_nodes_node_socket_get_input:
 * @socket: a #GtkNodesNodeSocket
 *
 * Returns the reference of the input #GtkNodesNodeSocket for this input, or
 * NULL if no input is connected or the socket is not in sink mode
 *
 * Returns: (transfer none): the reference to the input socket or NULL if invalid
 */

GtkNodesNodeSocket*
gtk_nodes_node_socket_get_input (GtkNodesNodeSocket *socket)
{
  GtkNodesNodeSocketPrivate *priv;

  g_return_val_if_fail (GTKNODES_IS_NODE_SOCKET (socket), NULL);

  priv = gtk_nodes_node_socket_get_instance_private (socket);

  return priv->input;
}

/**
 * gtk_nodes_node_socket_new_with_io:
 * @io: the IO mode to configure
 *
 * Creates a new node socket in the given IO mode
 *
 * Returns: (transfer full): the new #GtkNodesNodeSocket.
 */

GtkWidget *
gtk_nodes_node_socket_new_with_io (const GtkNodesNodeSocketIO io)
{
  return g_object_new (GTKNODES_TYPE_NODE_SOCKET, "io", io, NULL);
}

/**
 * gtk_nodes_node_socket_new:
 *
 * Creates a new node socket.
 *
 * Returns: (transfer full): the new #GtkNodesNodeSocket.
 */

GtkWidget *
gtk_nodes_node_socket_new (void)
{
  return g_object_new (GTKNODES_TYPE_NODE_SOCKET, NULL);
}

/* our enum-based socket type */
GType
gtk_nodes_node_socket_io_get_type (void)
{
  static gsize g_define_type_id__ = 0;

  if (g_once_init_enter (&g_define_type_id__))
    {
      static const GEnumValue values[] = {
        { GTKNODES_NODE_SOCKET_DISABLE, "GTKNODES_NODE_SOCKET_DISABLE", "disabled" },
        { GTKNODES_NODE_SOCKET_SINK,    "GTKNODES_NODE_SOCKET_SINK",    "sink" },
        { GTKNODES_NODE_SOCKET_SOURCE,  "GTKNODES_NODE_SOCKET_SOURCE",  "source" },
        { 0, NULL, NULL }
      };
      GType g_define_type_id =
        g_enum_register_static (g_intern_static_string ("GtkNodesNodeSocketType"), values);
      g_once_init_leave (&g_define_type_id__, g_define_type_id);
    }

  return g_define_type_id__;
}
