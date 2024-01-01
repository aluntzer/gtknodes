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

#include "gtknode.h"
#include "gtknodesocket.h"

#include "gtk/gtkgestureclick.h"
#include "gtk/gtkrender.h"
#include "gtk/gtkicontheme.h"
#include "gtk/gtkstylecontext.h"
#include "gtk/gtkbox.h"
#include "gtk/gtkexpander.h"
#include "gtk/gtkbutton.h"
#include "gtk/gtkwindow.h"
#include "gtk/gtkbuildable.h"

/* gtkprivate.h */
#include "glib-object.h"
#define GTK_NODES_VIEW_PARAM_RW G_PARAM_READWRITE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB


#define CLICKED_TIMEOUT 250

/**
 * SECTION:gtknode
 * @Short_description: A node container
 * @Title: GtkNodesNode
 *
 * # Overview #
 *
 * The #GtkNodesNode widget is a widget container drived from #GtkBox.
 * Widgets added to the node are assigned a #GtkNodesNodeSocket. The user must
 * configure the type of socket and connect to the socket-incoming signal
 * in order to be able to receive the data passed through a connection.
 *
 * The Node can be collapsed by clicking the #GtkExpander which will hide the
 * node items, and show a compact representation of the node with just the
 * GtkExpander and the sockets visible.
 *
 *
 * # Notes #
 *
 * While possible, using a #GtkNodesNode outside of a #GtkNodesNodeView does
 * not make much sense.
 *
 * The placement of sockets is currently only properly supported for the
 * GTK_ORIENTATION_VERTICAL orientation
 *
 *
 * Custom GtkNodesNode widgets can save and restore their internal child
 * widget states and other properties and special tags by by implementing
 * the proper #GtkBuildable interfaces. To export the configuration,
 * node_export_properties() must be implemented and is expected to return
 * valid xml output which is integrated into the xml output produced
 * by #GtkNodesNodeView.
 *
 * For example, to restore the value of an internal spin button widget,
 * the function would return an allocated string containing:
 *
 * <child internal-child="spinbutton">
 *	<object class="GtkSpinButton">
 *		<property name="value">5</property>
 *	</object>
 * </child>
 *
 */

typedef struct _GtkNodesNodeChild        GtkNodesNodeChild;

struct _GtkNodesNodePrivate
{
  GdkSurface	*event_surface;
  GList		  *children;

  guint id;                     /* our numeric id */

  GtkWidget *expander;
  GtkWidget *button;

  gint     expander_signal;
  gboolean expander_blocked;
  gboolean last_expanded;

  /* meh ... */
  guint width;
  guint height;

  guint socket_id;              /* socket id counter, only increments */


  GtkAllocation allocation;

  GdkRectangle rectangle_func;  /* functional button ("close") */

  gchar *icon_name;             /* the name of the icon to display */

  GtkBorder padding;
  GtkBorder margin;

  guint activate_id;

  gdouble socket_radius;
};


struct _GtkNodesNodeChild
{
  GtkWidget *item;
  GtkWidget *socket;

  guint input_id;                 /* input gtk node identifier */

  gint drag_begin_signal;
  gint drag_end_signal;
  gint socket_connect_signal;
  gint socket_disconnect_signal;
  gint socket_destroyed_signal;
};

enum {
  PROP_0,
  PROP_X,
  PROP_Y,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_ID,              /* node id */
  PROP_MARGIN_TOP,
  PROP_MARGIN_BOTTOM,
  PROP_MARGIN_LEFT,
  PROP_MARGIN_RIGHT,
  NUM_PROPERTIES
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_SOCKET_ID,        /* id of item  ( == ID of socket) */
  CHILD_PROP_INPUT_ID,        /* id of item  ( == ID of socket) */
  CHILD_PROP_IO_MODE,
  CHILD_NUM_PROPERTIES
};

/* Signals */
enum
{
	NODE_SOCKET_DRAG_BEGIN,
	NODE_SOCKET_DRAG_END,
	NODE_FUNC_CLICKED,
	NODE_SOCKET_CONNECT,
	NODE_SOCKET_DISCONNECT,
	NODE_SOCKET_DESTROYED,
	LAST_SIGNAL
};


/* gobject overridable methods */
static void       gtk_nodes_node_set_property                        (GObject              *object,
                                                                      guint                 param_id,
                                                                      const GValue         *value,
                                                                      GParamSpec           *pspec);
static void       gtk_nodes_node_get_property                        (GObject              *object,
                                                                      guint                 param_id,
                                                                      GValue               *value,
                                                                      GParamSpec           *pspec);
/* widget class basics */
static void       gtk_nodes_node_map                                 (GtkWidget            *widget);
static void       gtk_nodes_node_unmap                               (GtkWidget            *widget);
static void       gtk_nodes_node_realize                             (GtkWidget            *widget);
static void       gtk_nodes_node_unrealize                           (GtkWidget            *widget);

static void       gtk_nodes_node_size_allocate                       (GtkWidget            *widget,
                                                                      GtkAllocation        *allocation);
static gboolean   gtk_nodes_node_draw                                (GtkWidget            *widget,
                                                                      cairo_t              *cr);
/* widget class events */
static gboolean   gtk_nodes_node_gesture_press                       (GtkGestureClick* self,
                                                                      gint n_press,
                                                                      gdouble x,
                                                                      gdouble y,
                                                                      gpointer user_data);
static gboolean   gtk_nodes_node_gesture_release                     (GtkGestureClick* self,
                                                                      gint n_press,
                                                                      gdouble x,
                                                                      gdouble y,
                                                                      gpointer user_data);

/* widget class accessibility support */
static void       gtk_nodes_node_adjust_size_request                 (GtkWidget            *widget,
                                                                      GtkOrientation        orientation,
                                                                      gint                  *minimum_size,
                                                                      gint                  *natural_size);

/* container class methods */
static void       gtk_nodes_node_add                                 (GtkWidget            *widget,
                                                                      GtkWidget            *child);
static void       gtk_nodes_node_remove                              (GtkWidget            *widget,
                                                                      GtkWidget            *child);
static void       gtk_nodes_node_set_child_property                  (GtkContainer         *container,
                                                                      GtkWidget            *child,
                                                                      guint                 property_id,
                                                                      const GValue         *value,
                                                                      GParamSpec           *pspec);
static void       gtk_nodes_node_get_child_property                  (GtkContainer         *container,
                                                                      GtkWidget            *child,
                                                                      guint                 property_id,
                                                                      GValue               *value,
                                                                      GParamSpec           *pspec);
/* internal */
static void       gtk_nodes_node_size_allocate_visible_sockets       (GtkNodesNode         *node,
                                                                      GtkAllocation        *allocation);
static void       gtk_nodes_node_size_allocate_visible_child_sockets (GtkNodesNode         *node);


static void       gtk_nodes_node_draw_frame                          (GtkNodesNode         *node,
                                                                      cairo_t              *cr,
                                                                      const GtkAllocation  *allocation);
static GtkWidget* gtk_nodes_node_item_add_real                       (GtkNodesNode         *node,
                                                                      GtkWidget            *child,
                                                                      GtkNodesNodeSocketIO  mode);
static void       gtk_nodes_node_socket_drag_begin                   (GtkWidget            *widget,
                                                                      GtkNodesNode         *node);
static void       gtk_nodes_node_socket_drag_end                     (GtkWidget            *widget,
                                                                      GtkNodesNode         *node);
static void       gtk_nodes_node_socket_connect_cb                   (GtkWidget            *sink,
                                                                      GtkWidget            *source,
                                                                      GtkNodesNode         *node);
static void       gtk_nodes_node_socket_disconnect_cb                (GtkWidget            *sink,
                                                                      GtkWidget            *source,
                                                                      GtkNodesNode         *node);
static void       gtk_nodes_node_socket_destroyed                    (GtkWidget            *socket,
                                                                      GtkNodesNode         *node);
static void       gtk_nodes_node_expander_cb                         (GtkExpander          *expander,
                                                                      GParamSpec           *param_spec,
                                                                      GtkNodesNode         *node);
static gboolean   gtk_nodes_node_clicked_timeout                     (gpointer              data);


static guint node_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GtkNodesNode, gtk_nodes_node, GTK_TYPE_BOX)


static void
gtk_nodes_node_class_init (GtkNodesNodeClass *class)
{
  GObjectClass      *gobject_class;
  GtkWidgetClass    *widget_class;


  gobject_class   = G_OBJECT_CLASS (class);
  widget_class    = GTK_WIDGET_CLASS (class);

  /* gobject methods */
  gobject_class->get_property = gtk_nodes_node_get_property;
  gobject_class->set_property = gtk_nodes_node_set_property;

  /* widget basics */
  widget_class->map           = gtk_nodes_node_map;
  widget_class->unmap         = gtk_nodes_node_unmap;
  widget_class->realize       = gtk_nodes_node_realize;
  widget_class->unrealize     = gtk_nodes_node_unrealize;
  widget_class->size_allocate = gtk_nodes_node_size_allocate;
  widget_class->draw          = gtk_nodes_node_draw;

  /* widget events */

  /* widget accessibility support */
  widget_class->adjust_size_request = gtk_nodes_node_adjust_size_request;

  /* nodes class function for internal property export */
  class->export_properties = NULL;



  /**
   * GtkNodesNode:x:
   *
   * The x position of the node
   */

  g_object_class_install_property (gobject_class,
                                   PROP_X,
                                   g_param_spec_int ("x",
                                                     "X position",
                                                     "X position of Node",
                                                     G_MININT, G_MAXINT, 0,
                                                     GTK_NODES_VIEW_PARAM_RW));
  /**
   * GtkNodesNode:y:
   *
   * The y position of the node
   */

  g_object_class_install_property (gobject_class,
                                   PROP_Y,
                                   g_param_spec_int ("y",
                                                     "Y position",
                                                     "Y position of Node",
                                                     G_MININT, G_MAXINT, 0,
                                                     GTK_NODES_VIEW_PARAM_RW));
  /**
   * GtkNodesNode:width:
   *
   * The width of the node
   */

  g_object_class_install_property (gobject_class,
                                   PROP_WIDTH,
                                   g_param_spec_int ("width",
                                                     "requested width",
                                                     "requested width of Node",
                                                     G_MININT, G_MAXINT, 0,
                                                     GTK_NODES_VIEW_PARAM_RW));
  /**
   * GtkNodesNode:height:
   *
   * The height of the node
   */

  g_object_class_install_property (gobject_class,
                                   PROP_HEIGHT,
                                   g_param_spec_int ("height",
                                                     "reqested eight",
                                                     "requested height of Node",
                                                     G_MININT, G_MAXINT, 0,
                                                     GTK_NODES_VIEW_PARAM_RW));

  /**
   * GtkNodesNode:id:
   *
   * The numeric identifier of the node
   */

  g_object_class_install_property (gobject_class,
                                   PROP_ID,
                                   g_param_spec_uint ("id",
                                                     "numeric node identifier",
                                                     "numeric node identifier",
                                                     0, G_MAXUINT, 0,
                                                     GTK_NODES_VIEW_PARAM_RW));




  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_SOCKET_ID,
                                              g_param_spec_uint ("socketid",
                                                                 "numeric socket identifier",
                                                                 "numeric socket identifier",
                                                                 0, G_MAXUINT, 0,
                                                                 GTK_NODES_VIEW_PARAM_RW));
  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_INPUT_ID,
                                              g_param_spec_uint ("inputid",
                                                                 "numeric input identifier",
                                                                 "numeric input identifier",
                                                                 0, G_MAXUINT, 0,
                                                                 GTK_NODES_VIEW_PARAM_RW));
  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_IO_MODE,
                                              g_param_spec_enum ("Mode",
                                                                 "Item I/O Mode",
                                                                 "The configured items mode, either none, sink or source",
                                                                 GTKNODES_TYPE_NODE_SOCKET_IO,
                                                                 GTKNODES_NODE_SOCKET_DISABLE,
                                                                 GTK_NODES_VIEW_PARAM_RW));

  /**
   * GtkNodesNode::node-socket-drag-begin:
   * @widget: the object which received the signal.
   * @x:      the x coordinate of the event
   * @y:      the y coordinate of the event
   *
   * The ::node-socket-drag-begin signal is emitted when the user initiates a drag on
   * a node socket. Coordinates are given with respect to the parent.
   */

	node_signals[NODE_SOCKET_DRAG_BEGIN] =
    g_signal_new ("node-socket-drag-begin",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  2, G_TYPE_INT, G_TYPE_INT);

  /**
   * GtkNodesNode::node-socket-drag-end:
   * @widget: the object which received the signal.
   *
   * The ::node-socket-drag-end signal is emitted when the user ends a drag operation
   * on a node socket.
   */

	node_signals[NODE_SOCKET_DRAG_END] =
    g_signal_new ("node-socket-drag-end",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  0);

  /**
   * GtkNodesNode::node-func-clicked
   * @widget: the object which received the signal.
   *
   * The ::node-func-clicked signal is emitted when the user clicks the
   * functional node button
   * on a node socket.
   */

	node_signals[NODE_FUNC_CLICKED] =
    g_signal_new ("node-func-clicked",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  0);

  /**
   * GtkNodesNode::node-socket-connect:
   * @widget: the object which received the signal.
   * @sink: the socket which emitted the signal.
   * @source: the source which connected to the sink
   *
   * The ::node-socket-connect signal is emitted when a drag on a node socket
   * results in a successful connection
   */

	node_signals[NODE_SOCKET_CONNECT] =
    g_signal_new ("node-socket-connect",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  2, GTK_TYPE_WIDGET, GTK_TYPE_WIDGET);

  /**
   * GtkNodesNode::node-socket-disconnect:
   * @widget: the object which received the signal.
   * @sink: the socket which emitted the signal.
   * @source: the source which was connected to the sink
   *
   * The ::node-socket-disconnect signal is emitted when a node socket disconnects
   * from a source socket
   */

	node_signals[NODE_SOCKET_DISCONNECT] =
    g_signal_new ("node-socket-disconnect",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  2, GTK_TYPE_WIDGET, GTK_TYPE_WIDGET);

  /**
   * GtkNodesNode::node-socket-destroyed:
   * @widget: the object which received the signal.
   * @socket: the socket which emitted the signal.
   *
   * The ::node-socket-destroyed signal is emitted when a node socket disconnects
   * from a source socket
   */

	node_signals[NODE_SOCKET_DESTROYED] =
    g_signal_new ("node-socket-destroyed",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, GTK_TYPE_WIDGET);

}


static void
gtk_nodes_node_init (GtkNodesNode *node)
{
  GtkNodesNodePrivate *priv;
  GtkGestureClick *gesture;

  node->priv = gtk_nodes_node_get_instance_private (node);

  priv = node->priv;



  /* XXX set some defaults here, some are not properly configurable yet */
  priv->rectangle_func.width  = 20;
  priv->rectangle_func.height = 20;

  priv->width  = 100;
  priv->height = 100;

  priv->socket_radius = 8.0;

  priv->padding.top    = 10;
  priv->padding.bottom = 10;
  priv->padding.left   = 10;
  priv->padding.right  = 10;

  priv->margin.top    = priv->socket_radius;
  priv->margin.bottom = priv->socket_radius;
  priv->margin.left   = priv->socket_radius;
  priv->margin.right  = priv->socket_radius;

  priv->icon_name = g_strdup_printf("edit-delete-symbolic");


  gtk_box_set_homogeneous(GTK_BOX(node), FALSE);

  /* XXX ensure this once on init, if the user decides to change this later,
   *     it's not our problem
   *
   * TDOO: make whole thing orientable (maybe)
   **/
  g_object_set(node, "orientation", GTK_ORIENTATION_VERTICAL, NULL);

  /* add an expander for minimisation of the node, and a descriptive label */
  priv->expander = gtk_expander_new ("Node");
  gtk_expander_set_expanded (GTK_EXPANDER (priv->expander), TRUE);
  gtk_box_pack_start (GTK_BOX (node), priv->expander, FALSE, FALSE, 0);

  priv->expander_signal =
    g_signal_connect (priv->expander, "notify::expanded",
                      G_CALLBACK (gtk_nodes_node_expander_cb), node);

  priv->expander_blocked = FALSE;
  priv->last_expanded    = TRUE;


  gtk_widget_set_has_window (GTK_WIDGET (node), FALSE);

  gesture = gtk_gesture_click_new ();

  g_signal_connect (gesture, "pressed",
                    G_CALLBACK(gtk_nodes_node_gesture_press), node);
  g_signal_connect (gesture, "released",
                    G_CALLBACK(gtk_nodes_node_gesture_release), node);

  gtk_widget_add_controller (node->parent, GTK_EVENT_CONTROLLER (gesture));
}


/* GObject Methods */

static void
gtk_nodes_node_set_property (GObject      *object,
                             guint         param_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GtkNodesNode *node;
  GtkNodesNodePrivate *priv;


  node = GTKNODES_NODE (object);
  priv = gtk_nodes_node_get_instance_private (node);

  switch (param_id)
    {
      case PROP_X:
        priv->allocation.x = g_value_get_int (value);
        gtk_widget_queue_allocate(GTK_WIDGET (node));
        break;
      case PROP_Y:
        priv->allocation.y = g_value_get_int (value);
        gtk_widget_queue_allocate(GTK_WIDGET (node));
        break;
      case PROP_WIDTH:
        priv->width = g_value_get_int (value);
        gtk_widget_queue_allocate(GTK_WIDGET (node));
        break;
      case PROP_HEIGHT:
        priv->height = g_value_get_int (value);
        gtk_widget_queue_allocate(GTK_WIDGET (node));
        break;
      case PROP_ID:
        priv->id = g_value_get_uint (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
gtk_nodes_node_get_property (GObject    *object,
                             guint       param_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GtkNodesNode *node;
  GtkNodesNodePrivate *priv;


  node = GTKNODES_NODE (object);
  priv = gtk_nodes_node_get_instance_private (node);

  switch (param_id)
    {
      case PROP_X:
        g_value_set_int (value, priv->allocation.x);
        break;
      case PROP_Y:
        g_value_set_int (value, priv->allocation.y);
        break;
      case PROP_WIDTH:
        g_value_set_int (value, priv->width);
        break;
      case PROP_HEIGHT:
        g_value_set_int (value, priv->height);
        break;
      case PROP_ID:
        g_value_set_uint (value, priv->id);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}


/* Widget Methods */

static void
gtk_nodes_node_map (GtkWidget *widget)
{
  GtkNodesNodePrivate *priv;


  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));


  GTK_WIDGET_CLASS (gtk_nodes_node_parent_class)->map (widget);

  if (priv->event_surface)
      gdk_window_show(priv->event_surface);
}

static void
gtk_nodes_node_unmap (GtkWidget *widget)
{
  GtkNodesNodePrivate *priv;


  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));

  if (priv->event_surface)
    gdk_window_hide (priv->event_surface);

  GTK_WIDGET_CLASS (gtk_nodes_node_parent_class)->unmap (widget);
}

static void
gtk_nodes_node_realize (GtkWidget *widget)
{
  GtkNodesNodePrivate *priv;
  GdkSurface     *surface;
  GtkAllocation   allocation;
  GdkWindowAttr   attributes;
  gint            attributes_mask;
  GList          *l;


  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));

  GTK_WIDGET_CLASS (gtk_nodes_node_parent_class)->realize (widget);

  gtk_widget_set_realized (widget, TRUE);

  window = gtk_widget_get_parent_window (widget);
  g_object_ref (window);
  gtk_widget_set_window (widget, window);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x           = allocation.x;
  attributes.y           = allocation.y;
  attributes.width       = allocation.width;
  attributes.height      = allocation.height;
  attributes.visual      = gtk_widget_get_visual (widget);
  attributes.wclass      = GDK_INPUT_OUTPUT;
  attributes.event_mask  = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_TOUCH_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_LEAVE_NOTIFY_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y;


  priv->event_surface = gdk_window_new (window, &attributes, attributes_mask);
  gtk_widget_register_window (widget, priv->event_surface);

  l = priv->children;

  while (l)
  {
    GtkNodesNodeChild *child = l->data;

    l = l->next;

    gtk_widget_set_parent_window (child->item,   priv->event_surface);
    gtk_widget_set_parent_window (child->socket, priv->event_surface);
  }

  gtk_widget_set_parent_window (priv->expander, priv->event_surface);

}

static void
gtk_nodes_node_unrealize (GtkWidget *widget)
{
  GtkNodesNodePrivate *priv;


  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));

  if (priv->event_surface)
  {
    gtk_widget_unregister_window (widget, priv->event_surface);
    gdk_window_destroy (priv->event_surface);
    priv->event_surface = NULL;
  }

  if (priv->activate_id) {
          g_source_remove (priv->activate_id);
          priv->activate_id = 0;
  }

  GTK_WIDGET_CLASS (gtk_nodes_node_parent_class)->unrealize (widget);
}

static void
gtk_nodes_node_adjust_size_request (GtkWidget      *widget,
                                     GtkOrientation  orientation,
                                     gint           *minimum_size,
                                     gint           *natural_size)
{
  gint h, v;

  GtkNodesNodePrivate *priv;

  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));

  h = priv->padding.left + priv->padding.right
    + priv->margin.left  + priv->margin.right;

  v = priv->padding.top + priv->padding.bottom
    + priv->margin.top  + priv->margin.bottom;


  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      /* ajust extra pixel size of "func" button as well */
      *minimum_size += h + 25; /* XXX */
      *natural_size += h + 25;
    }
  else
    {
      *minimum_size += v;
      *natural_size += v;
    }
}

static void
gtk_nodes_node_size_allocate (GtkWidget        *widget,
                              int               width,
                              int               height,
                              int               baseline)
{
  GtkNodesNode *node;
  GtkNodesNodePrivate *priv;
  GtkAllocation alloc;

  gint top, bottom, left, right;


  node = GTKNODES_NODE (widget);
  priv = gtk_nodes_node_get_instance_private (node);

  priv->allocation.x      = allocation->x;
  priv->allocation.y      = allocation->y;
  priv->allocation.width  = width;
  priv->allocation.height = height;

  top    = priv->padding.top    + priv->margin.top;
  left   = priv->padding.left   + priv->margin.left;
  right  = priv->padding.right  + priv->margin.right;
  bottom = priv->padding.bottom + priv->margin.bottom;

  allocation->x       = left;
  allocation->y       = top;
  allocation->width  -= (left + right);
  allocation->height -= (top  + bottom);

  /* chain up to allocate the node items */
  GTK_WIDGET_CLASS (gtk_nodes_node_parent_class)->size_allocate (widget, allocation);



  if (!gtk_expander_get_expanded (GTK_EXPANDER (priv->expander)))
    {
      gint minimum, natural;

      /* adjust for expander/label first, it may have gotten too much space
       * allocated, we want to go as compact as possible
       */
      gtk_widget_get_allocation (priv->expander, &alloc);
      gtk_widget_get_preferred_width (priv->expander, &minimum, &natural);
      alloc.width = MIN(minimum, natural);
      gtk_widget_set_allocation (priv->expander, &alloc);

      /* now adapt the allocation */
      priv->allocation.width = alloc.width + left + right + 25; /* XXX */

      alloc.y = 0;
      gtk_nodes_node_size_allocate_visible_sockets (node, &alloc);

      /* update height from socket placement */
      priv->allocation.height = alloc.height;
    }
  else
    {
      gtk_nodes_node_size_allocate_visible_child_sockets (node);
    }


  gtk_widget_set_allocation (widget, &priv->allocation);

  if (!gtk_widget_get_realized (widget))
    return;

  if (!priv->event_surface)
    return;

  gdk_window_move_resize (priv->event_surface,
                          priv->allocation.x,
                          priv->allocation.y,
                          priv->allocation.width,
                          priv->allocation.height);

}

static gboolean
gtk_nodes_node_draw (GtkWidget *widget,
                     cairo_t   *cr)
{
  GtkNodesNodePrivate *priv;
  GtkAllocation allocation;


  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));

  gtk_widget_get_allocation (widget, &allocation);

  allocation.x       = priv->margin.left;
  allocation.y       = priv->margin.top;
  allocation.width  -= priv->margin.left + priv->margin.right;
  allocation.height -= priv->margin.top  + priv->margin.bottom;

  gtk_nodes_node_draw_frame (GTKNODES_NODE (widget), cr, &allocation);

  GTK_WIDGET_CLASS (gtk_nodes_node_parent_class)->draw (widget, cr);

  return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_gesture_press (GtkGestureClick* self,
                              gint n_press,
                              gdouble x,
                              gdouble y,
                              gpointer user_data)
{
  GdkRectangle point = {x, y, 1, 1};
  GtkWidget *widget = user_data;
  GtkNodesNodePrivate *priv;

  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));

  if (!gdk_rectangle_intersect (&priv->rectangle_func, &point, NULL))
    return FALSE;

  priv->activate_id = gdk_threads_add_timeout (CLICKED_TIMEOUT,
                                               gtk_nodes_node_clicked_timeout,
                                               widget);
  return TRUE;
}

static gboolean
gtk_nodes_node_gesture_release (GtkGestureClick* self,
                                gint n_press,
                                gdouble x,
                                gdouble y,
                                gpointer user_data)
{
  GtkNodesNode *widget = user_data;
  GtkNodesNodePrivate *priv;

  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (widget));

  if (priv->activate_id) {
	  g_signal_emit (widget, node_signals[NODE_FUNC_CLICKED], 0);
  }

  if (GDK_IS_SURFACE(priv->event_surface)) {
    gdk_window_raise(priv->event_surface);
  }

  return TRUE;
}

/* Container Methods */

static void
gtk_nodes_node_add (GtkWidget    *widget,
                    GtkWidget    *child)
{
  GtkNodesNode *node = GTKNODES_NODE (widget);

  gtk_nodes_node_item_add_real (node, child, GTKNODES_NODE_SOCKET_DISABLE);
}

static void
gtk_nodes_node_remove (GtkWidget    *container,
                       GtkWidget    *widget)
{
  GtkNodesNodePrivate *priv;
  GList *l;

  g_return_if_fail (GTKNODES_IS_NODE (container));

  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (container));

  l = priv->children;

  while (l)
    {
      GtkNodesNodeChild *child = l->data;

      if (child->item != widget)
        {
          l = l->next;
          continue;
        }

      gtk_widget_unparent (GTK_WIDGET (child->socket));
      GTK_CONTAINER_CLASS (gtk_nodes_node_parent_class)->remove (container,
                                                                 widget);

      priv->children = g_list_remove_link (priv->children, l);
      g_list_free_1 (l);
      g_free (child);

      return;
    }
}

static GtkNodesNodeChild*
gtk_nodes_node_get_child (GtkNodesNode  *node,
                          GtkWidget *widget)
{
  GtkNodesNodePrivate *priv;
  GList *l;

  priv = gtk_nodes_node_get_instance_private (node);

  l = priv->children;

  while (l) {
    GtkNodesNodeChild *child = l->data;
    l = l->next;

    if (child->item == widget)
      return child;
  }

  g_warning ("GtkWidget %p is not a child of GtkNode %p",
             (void *) widget, (void *) node);

  return NULL;
}

static void
gtk_nodes_node_set_child_property (GtkWidget    *widget,
                                   GtkWidget    *child,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  GtkNodesNodeChild *node_child;
  GtkNodesNode *node = GTKNODES_NODE (widget);

  node_child = gtk_nodes_node_get_child (node, child);

  if (node_child == NULL)
    return;

  /* this is kinda stupid, but at the moment I don't see how I can save an
   * XML description of the node view's contents and restore a (partial)
   * configuration with gtkbuilder all without tracking the child properties
   * within the node children as well */

  switch (property_id)
    {
    case CHILD_PROP_SOCKET_ID:
      {
        guint id = g_value_get_uint (value);
        g_object_set (G_OBJECT(node_child->socket), "id", id, NULL);
      }
      break;

    case CHILD_PROP_INPUT_ID:
      node_child->input_id = g_value_get_uint (value);
      break;

    case CHILD_PROP_IO_MODE:
      {
        GtkNodesNodeSocket *socket;
        socket = GTKNODES_NODE_SOCKET(node_child->socket);

        gtk_nodes_node_socket_set_io (socket, g_value_get_enum (value));
      }
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (widget, property_id,
                                                    pspec);
      break;
    }
}

static void
gtk_nodes_node_get_child_property (GtkWidget    *widget,
                                   GtkWidget    *child,
                                   guint         property_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
  GtkNodesNodeChild *node_child;
  GtkNodesNode *node = GTKNODES_NODE (widget);

  node_child = gtk_nodes_node_get_child (node, child);

  if (node_child == NULL)
    return;


  switch (property_id)
    {
    case CHILD_PROP_SOCKET_ID:
      {
        guint id;
        g_object_get (G_OBJECT(node_child->socket), "id", &id, NULL);
        g_value_set_uint (value, id);
      }
      break;

    case CHILD_PROP_INPUT_ID:
      g_value_set_uint (value, node_child->input_id);
      break;

    case CHILD_PROP_IO_MODE:
      {
        GtkNodesNodeSocket *socket;
        socket = GTKNODES_NODE_SOCKET(node_child->socket);
        g_value_set_enum (value, gtk_nodes_node_socket_get_io (socket));
      }
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (widget, property_id,
                                                    pspec);
      break;
    }
}

/* interals */

static void
gtk_nodes_node_socket_drag_begin (GtkWidget    *widget,
                                  GtkNodesNode *node)
{
  GtkAllocation alloc_node;
  GtkAllocation alloc_socket;


  gtk_widget_get_allocation (widget, &alloc_socket);
  gtk_widget_get_allocation (GTK_WIDGET (node), &alloc_node);

	g_signal_emit (node, node_signals[NODE_SOCKET_DRAG_BEGIN], 0,
                 alloc_node.x + alloc_socket.x + alloc_socket.width / 2,
                 alloc_node.y + alloc_socket.y + alloc_socket.height / 2);
}

static void
gtk_nodes_node_socket_drag_end (GtkWidget    *widget,
                                GtkNodesNode *node)
{
	g_signal_emit (node, node_signals[NODE_SOCKET_DRAG_END], 0);
}


static void
gtk_nodes_node_socket_connect_cb (GtkWidget    *sink,
                                  GtkWidget    *source,
                                  GtkNodesNode *node)
{
  g_signal_emit (node, node_signals[NODE_SOCKET_CONNECT], 0, sink, source);
}

static void
gtk_nodes_node_socket_disconnect_cb (GtkWidget    *sink,
                                     GtkWidget    *source,
                                     GtkNodesNode *node)
{
  g_signal_emit (node, node_signals[NODE_SOCKET_DISCONNECT], 0, sink, source);
}

static void
gtk_nodes_node_socket_destroyed (GtkWidget    *socket,
                                 GtkNodesNode *node)
{
	g_signal_emit (node, node_signals[NODE_SOCKET_DESTROYED], 0, socket);
}

static void
gtk_nodes_node_expander_cb (GtkExpander   *expander,
                            GParamSpec    *param_spec,
                            GtkNodesNode  *node)
{
  GtkWidget *center;
  GtkNodesNodePrivate *priv;
  GList *l;

  gboolean exp;


  priv = gtk_nodes_node_get_instance_private (node);

  exp = gtk_expander_get_expanded (expander);


  l = priv->children;

  while (l)
    {
      GtkNodesNodeChild *child = l->data;
      l = l->next;

        gtk_widget_set_visible (GTK_WIDGET (child->item), exp);
    }

  /* If the user set a center widget, make sure to hide it too, as
   * we don't track it as one of the items in the node.
   * We could actually use the center widget to display a persistent
   * widget which is shown even when the expander is collapsed, but this
   * could open another can of worms of allocation manangement.
   * Let's keep it simple for now.
   */
  center = gtk_box_get_center_widget (GTK_BOX (node));
  if (center != NULL)
        gtk_widget_set_visible (center, exp);


  gtk_widget_queue_draw (gtk_widget_get_parent (GTK_WIDGET (node)));
}


static GtkWidget *
gtk_nodes_node_item_add_real (GtkNodesNode         *node,
                              GtkWidget            *child,
                              GtkNodesNodeSocketIO  mode)
{
  GtkNodesNodePrivate *priv;
  GtkNodesNodeChild *child_info;


  g_return_val_if_fail (GTKNODES_IS_NODE (node), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  priv = gtk_nodes_node_get_instance_private (node);

  child_info = g_new0 (GtkNodesNodeChild, 1);

  child_info->item = child;


  switch (mode) {
    case GTKNODES_NODE_SOCKET_SOURCE:
      {
        GdkRGBA rgba = {0.0, 0.38, 0.65, 1.0};

        child_info->socket =
          gtk_nodes_node_socket_new_with_io(GTKNODES_NODE_SOCKET_SOURCE);

        gtk_nodes_node_socket_set_rgba (GTKNODES_NODE_SOCKET (child_info->socket),
                                        &rgba);
      }
      break;
    case GTKNODES_NODE_SOCKET_SINK:
      {
        GdkRGBA rgba = {0.92, 0.67, 0.0, 1.0};

        child_info->socket =
          gtk_nodes_node_socket_new_with_io(GTKNODES_NODE_SOCKET_SINK);

        gtk_nodes_node_socket_set_rgba (GTKNODES_NODE_SOCKET (child_info->socket),
                                        &rgba);
      }
      break;

    default:
        child_info->socket = gtk_nodes_node_socket_new();
  }


  if (priv->event_surface)
  {
    gtk_widget_set_parent_window (child_info->item,   priv->event_surface);
    gtk_widget_set_parent_window (child_info->socket, priv->event_surface);
  }


  /* we set an incremental socket id here, so a node item can later be
   * identified for restoring socket connections when loading from
   * XML via gtknodeview
   */
  gtk_nodes_node_socket_set_id (GTKNODES_NODE_SOCKET (child_info->socket),
                                priv->socket_id++);

  gtk_nodes_node_socket_set_io (GTKNODES_NODE_SOCKET (child_info->socket),
                                mode);

  gtk_nodes_node_socket_set_radius (GTKNODES_NODE_SOCKET (child_info->socket),
                                    priv->socket_radius);

  /* we need to collect all the signals emitted by the sockets associated with
   * our node items, so we can pass them on to the next layer
   */
  child_info->drag_begin_signal =
    g_signal_connect(G_OBJECT(child_info->socket),
                     "socket-drag-begin",
                     G_CALLBACK(gtk_nodes_node_socket_drag_begin),
                     node);

  child_info->drag_end_signal =
    g_signal_connect(G_OBJECT(child_info->socket),
                     "socket-drag-end",
                     G_CALLBACK(gtk_nodes_node_socket_drag_end),
                     node);

  child_info->socket_connect_signal =
    g_signal_connect(G_OBJECT(child_info->socket),
                     "socket-connect",
                     G_CALLBACK(gtk_nodes_node_socket_connect_cb),
                     node);

  child_info->socket_disconnect_signal =
    g_signal_connect(G_OBJECT(child_info->socket),
                     "socket-disconnect",
                     G_CALLBACK(gtk_nodes_node_socket_disconnect_cb),
                     node);

  child_info->socket_destroyed_signal =
    g_signal_connect(G_OBJECT(child_info->socket),
                     "socket-destroyed",
                     G_CALLBACK(gtk_nodes_node_socket_destroyed),
                     node);


  priv->children = g_list_append (priv->children, child_info);

  gtk_box_pack_start (GTK_BOX (node), child_info->item, FALSE, FALSE, 0);

  gtk_widget_set_parent(child_info->socket, GTK_WIDGET (node));
  gtk_widget_set_visible(child_info->socket, TRUE);

  return child_info->socket;
}

static GtkStyleContext *
get_style_node(void)
{
  GtkWidget *b;
  GtkStyleContext *c;

  /* all we really want is to draw a frame, so we'll take our
   * style context from a button
   */

  b = gtk_button_new();

  c = gtk_widget_get_style_context(b);

  g_object_ref(c);
  g_object_ref_sink(b);

  return c;
}

static void
gtk_nodes_node_draw_frame(GtkNodesNode        *node,
                          cairo_t             *cr,
                          const GtkAllocation *allocation)
{
  GtkStyleContext *c;
  GdkPixbuf *pb;
  GtkIconTheme *it;

  GtkNodesNodePrivate *priv;

  priv = gtk_nodes_node_get_instance_private (node);

  c = get_style_node ();

  /* draw a representation of the node */
  gtk_style_context_save (c);
  gtk_render_background (c, cr, allocation->x,     allocation->y,
                                allocation->width, allocation->height);

  gtk_render_frame (c, cr, allocation->x,     allocation->y,
                           allocation->width, allocation->height);
  gtk_style_context_restore (c);

  /* set functional icon allocation for clicks */
  priv->rectangle_func.x = allocation->x + allocation->width - 25; /* XXX */
  priv->rectangle_func.y = allocation->y + priv->padding.top;

  if (priv->icon_name)
    {
      it = gtk_icon_theme_get_default ();
      pb = gtk_icon_theme_load_icon (it, priv->icon_name,
                                     priv->rectangle_func.height, 0, NULL);

      cairo_save(cr);
      gdk_cairo_set_source_pixbuf (cr,
                                   pb,
                                   priv->rectangle_func.x,
                                   priv->rectangle_func.y);

      cairo_paint (cr);
      cairo_restore (cr);
    }

  g_clear_object (&c);
}

static void
gtk_nodes_node_socket_allocate_socket (GtkNodesNode        *node,
                                       GtkNodesNodeChild   *child,
                                       const GtkAllocation *allocation)
{
  GtkNodesNodePrivate *priv;
  GtkNodesNodeSocketIO mode;
  GtkAllocation alloc_socket;
  gint minimum, natural;


  priv = gtk_nodes_node_get_instance_private (node);


  mode = gtk_nodes_node_socket_get_io (GTKNODES_NODE_SOCKET (child->socket));

  if ((mode != GTKNODES_NODE_SOCKET_SOURCE) &&
      (mode != GTKNODES_NODE_SOCKET_SINK))
    return;



  gtk_widget_get_preferred_height (child->socket, &minimum, &natural);
  alloc_socket.height = MIN (minimum, natural);

  alloc_socket.y = allocation->y;

  if (gtk_expander_get_expanded (GTK_EXPANDER (priv->expander)))
    alloc_socket.y += ((allocation->height - alloc_socket.height) / 2
                            - priv->allocation.y);

  gtk_widget_get_preferred_width (child->socket, &minimum, &natural);
  alloc_socket.width = MIN (minimum, natural);

  alloc_socket.x = (- alloc_socket.width / 2 + priv->margin.left);


  if (mode == GTKNODES_NODE_SOCKET_SOURCE)
    alloc_socket.x += (priv->allocation.width - priv->margin.right
                       - priv->margin.left);

  gtk_widget_size_allocate (child->socket, &alloc_socket);
}

static void
gtk_nodes_node_size_allocate_visible_child_sockets (GtkNodesNode  *node)
{
  GtkNodesNodePrivate *priv;
  GtkAllocation alloc;
  GList *l;


  priv = gtk_nodes_node_get_instance_private (node);

  l = priv->children;

  while (l)
    {
      GtkNodesNodeChild *child = l->data;
      l = l->next;

      if (!gtk_widget_is_visible (child->item))
        continue;

      gtk_widget_get_allocation (child->item, &alloc);

      /* sockets are not drawn within the context of the box, so their
       * origin is relative to the parent of the node
       */
      alloc.y += priv->allocation.y;

      gtk_nodes_node_socket_allocate_socket (node, child, &alloc);
    }
}

static void
gtk_nodes_node_get_visible_socket_stack (GtkNodesNode         *node,
                                         GtkNodesNodeSocketIO  mode,
                                         gint                 *sockets,
                                         gint                 *height)
{
  GtkNodesNodePrivate *priv;
  GList *l;

  gint minimum, natural;

  gint h = 0;
  gint n = 0;


  priv = gtk_nodes_node_get_instance_private (node);

  l = priv->children;

  while (l)
    {
      GtkNodesNodeSocketIO socket_mode;
      GtkNodesNodeChild *child = l->data;

      socket_mode = gtk_nodes_node_socket_get_io (GTKNODES_NODE_SOCKET (child->socket));

      l = l->next;

      if (socket_mode != mode)
        continue;

      if (!gtk_widget_is_visible (child->socket))
        continue;

      gtk_widget_get_preferred_height (child->socket, &minimum, &natural);

      h += MIN (minimum, natural);
      n++;
    }

  (* sockets) = n;
  (* height)  = h;
}

static void
gtk_nodes_node_distribute_visible_socket_stack (GtkNodesNode         *node,
                                                GtkAllocation         allocation,
                                                GtkNodesNodeSocketIO  mode,
                                                gint                  sockets,
                                                gint                  height)
{
  GtkNodesNodePrivate *priv;
  GList *l;

  gdouble y;
  gdouble step = 0.0;


  priv = gtk_nodes_node_get_instance_private (node);


  /* prevent div/0 */
  sockets = MIN (height, sockets);

  if (sockets > 1)
    step = (gdouble) height / (sockets - 1);
  else
    allocation.y = (gdouble) height / 2;

  /* not doing this in a floating point representation can cause noticable
   * rounding errors depnding on the socket radius and number of sockets
   */
  y = (gdouble) allocation.y;

  l = priv->children;
  while (l)
    {
      GtkNodesNodeSocketIO socket_mode;
      GtkNodesNodeChild *child = l->data;

      socket_mode = gtk_nodes_node_socket_get_io (GTKNODES_NODE_SOCKET (child->socket));

      l = l->next;

      if (socket_mode != mode)
        continue;

      if (!gtk_widget_is_visible (child->socket))
        continue;

      gtk_nodes_node_socket_allocate_socket (node, child, &allocation);

      y += step;

      allocation.y = (gint) y;
    }
}

static void
gtk_nodes_node_size_allocate_visible_sockets (GtkNodesNode  *node,
                                              GtkAllocation *allocation)
{
  GtkNodesNodePrivate *priv;

  GtkAllocation alloc_exp;

  gint height;
  gint sinks, sources;
  gint sink_height, source_height;


  priv = gtk_nodes_node_get_instance_private (node);


  gtk_nodes_node_get_visible_socket_stack (node, GTKNODES_NODE_SOCKET_SOURCE,
                                           &sources, &source_height);
  gtk_nodes_node_get_visible_socket_stack (node, GTKNODES_NODE_SOCKET_SINK,
                                           &sinks, &sink_height);

  height = MAX (source_height, sink_height);


  /* adjust for expander/label */
  gtk_widget_get_allocation (priv->expander, &alloc_exp);

  if (height < alloc_exp.height)
    height += (priv->padding.top + priv->padding.bottom);



  gtk_nodes_node_distribute_visible_socket_stack (node, (* allocation),
                                                  GTKNODES_NODE_SOCKET_SOURCE,
                                                  sources, height);

  gtk_nodes_node_distribute_visible_socket_stack (node, (* allocation),
                                                  GTKNODES_NODE_SOCKET_SINK,
                                                  sinks, height);

  /* our sockets are round and stick out of the top and bottom of the frame
   * by one radius
   */
  allocation->height = height + 2 * priv->socket_radius;
}

static gboolean
gtk_nodes_node_clicked_timeout (gpointer data)
{
  GtkNodesNodePrivate *priv;

  /* just in case we were destroyed while timeout was running */
  if (!GTKNODES_IS_NODE (data))
        return G_SOURCE_REMOVE;

  priv = gtk_nodes_node_get_instance_private (GTKNODES_NODE (data));

  if (priv->activate_id)
    g_source_remove (priv->activate_id);

  priv->activate_id = 0;

  return G_SOURCE_REMOVE;
}


/* Public Interface */

/**
 * gtk_nodes_node_set_label:
 * @node: a GtkNodesNode
 * @label: the label text to set
 *
 */

void
gtk_nodes_node_set_label (GtkNodesNode *node,
                          const gchar  *label)
{
  GtkNodesNodePrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE (node));

  priv = gtk_nodes_node_get_instance_private (node);

  gtk_expander_set_label (GTK_EXPANDER (priv->expander), label);
}

/**
 * gtk_nodes_node_get_socket_radius:
 * @node: a GtkNodesNode
 *
 * Returns: the radius of any socket on this node, 0 on error
 */

gdouble
gtk_nodes_node_get_socket_radius (GtkNodesNode *node)
{
  GtkNodesNodePrivate *priv;

  g_return_val_if_fail (GTKNODES_IS_NODE (node), 0.0);

  priv = gtk_nodes_node_get_instance_private (node);

  return priv->socket_radius;
}

/**
 * gtk_nodes_node_set_socket_radius:
 * @node: a GtkNodesNode
 * @radius: the radius of any socket on this node
 *
 */

void
gtk_nodes_node_set_socket_radius (GtkNodesNode  *node,
                                  const gdouble  radius)
{
  GtkNodesNodePrivate *priv;
  GList *l;

  g_return_if_fail (GTKNODES_IS_NODE (node));

  priv = gtk_nodes_node_get_instance_private (node);

  priv->socket_radius = radius;

  /* XXX we take the socket radius as the required (minimum) margin,
   * update it here for now
   */
  priv->margin.top    = priv->socket_radius;
  priv->margin.bottom = priv->socket_radius;
  priv->margin.left   = priv->socket_radius;
  priv->margin.right  = priv->socket_radius;

  l = priv->children;

  while (l)
  {
    GtkNodesNodeChild *child = l->data;
    l = l->next;

    gtk_nodes_node_socket_set_radius (GTKNODES_NODE_SOCKET (child->socket),
                                      radius);
  }

  /* the socket radius changes our visbile size */
  gtk_widget_queue_allocate (GTK_WIDGET (node));
}

/**
 * gtk_nodes_node_get_expanded
 * @node: a GtkNodesNode
 *
 * Returns: the state of expansion
 */

gboolean
gtk_nodes_node_get_expanded (GtkNodesNode *node)
{
  GtkNodesNodePrivate *priv;


  g_return_val_if_fail (GTKNODES_IS_NODE (node), FALSE);

  priv = gtk_nodes_node_get_instance_private (node);

  return gtk_expander_get_expanded (GTK_EXPANDER (priv->expander));
}

/**
 * gtk_nodes_node_set_expanded
 * @node: a GtkNodesNode
 * @expanded: the expansion state
 */

void
gtk_nodes_node_set_expanded (GtkNodesNode *node,
                            gboolean expanded)
{
  GtkNodesNodePrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE (node));

  priv = gtk_nodes_node_get_instance_private (node);

  gtk_expander_set_expanded (GTK_EXPANDER (priv->expander), expanded);
}

/**
 * gtk_nodes_node_block_expander
 * @node: a GtkNodesNode
 *
 * The expander is very very nasty in that it responds to an isolated
 * "release" event, which I'm apparently unable to block.
 * If the node is contained in a #GtkNodesNodeView, it will therefore
 * expand/contract if someone clicks the label when executing a node "drag"
 * motion.
 *
 * Note: gtk_nodes_node_block_expander() repeatedly will affect the state only once
 */

void
gtk_nodes_node_block_expander (GtkNodesNode *node)
{
  GtkNodesNodePrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE (node));

  priv = gtk_nodes_node_get_instance_private (node);

  if (priv->expander_blocked)
    return;

  priv->expander_blocked = TRUE;

  priv->last_expanded = gtk_nodes_node_get_expanded (node);

  g_signal_handler_block (G_OBJECT (priv->expander), priv->expander_signal);
}

/**
 * gtk_nodes_node_unblock_expander
 * @node: a GtkNodesNode
 *
 * Unblocks the expander from receiving signals
 */

void
gtk_nodes_node_unblock_expander (GtkNodesNode *node)
{
  GtkNodesNodePrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE (node));

  priv = gtk_nodes_node_get_instance_private (node);

  if (!priv->expander_blocked)
    return;

  priv->expander_blocked = FALSE;

  priv = gtk_nodes_node_get_instance_private (node);

  g_signal_handler_unblock (G_OBJECT (priv->expander), priv->expander_signal);

  gtk_expander_set_expanded (GTK_EXPANDER (priv->expander),
                             priv->last_expanded);
}

/**
 * gtk_nodes_node_get_sinks
 * @node: a GtkNodesNode
 *
 * Returns: (element-type GtkNodesNodeSocket) (transfer container): a list of this node's sockets in sink mode
 */

GList*
gtk_nodes_node_get_sinks (GtkNodesNode *node)
{
  GList *l;
  GList *sockets = NULL;
  GtkNodesNodePrivate *priv;
  GtkNodesNodeSocketIO mode;


  g_return_val_if_fail (GTKNODES_IS_NODE (node), NULL);

  priv = gtk_nodes_node_get_instance_private (node);


  l = priv->children;

  while (l) {
    GtkNodesNodeChild *child = l->data;

    l = l->next;

    if (child->socket == NULL)
      continue;

    mode = gtk_nodes_node_socket_get_io (GTKNODES_NODE_SOCKET (child->socket));
    if (mode == GTKNODES_NODE_SOCKET_SINK)
      sockets = g_list_append (sockets, child->socket);
  }

  return sockets;
}

/**
 * gtk_nodes_node_get_sources
 * @node: a GtkNodesNode
 *
 * Returns: (element-type GtkNodesNodeSocket) (transfer container): a list of this node's sockets in source mode
 */

GList*
gtk_nodes_node_get_sources (GtkNodesNode *node)
{
  GList *l;
  GList *sockets = NULL;
  GtkNodesNodePrivate *priv;
  GtkNodesNodeSocketIO mode;


  g_return_val_if_fail (GTKNODES_IS_NODE (node), NULL);

  priv = gtk_nodes_node_get_instance_private (node);


  l = priv->children;

  while (l) {
    GtkNodesNodeChild *child = l->data;

    l = l->next;

    if (child->socket == NULL)
      continue;

    mode = gtk_nodes_node_socket_get_io (GTKNODES_NODE_SOCKET (child->socket));
    if (mode == GTKNODES_NODE_SOCKET_SOURCE)
      sockets = g_list_append (sockets, child->socket);
  }

  return sockets;
}

/**
 * gtk_nodes_node_set_icon_name
 * @node: a GtkNodesNode
 * @icon_name: the name of the icon to display
 */

void
gtk_nodes_node_set_icon_name (GtkNodesNode *node,
                              const gchar  *icon_name)
{
  GtkNodesNodePrivate *priv;


  g_return_if_fail (GTKNODES_IS_NODE (node));

  priv = gtk_nodes_node_get_instance_private (node);

  g_free (priv->icon_name);

  priv->icon_name = NULL;

  if (icon_name)
    priv->icon_name = g_strdup_printf ("%s", icon_name);
}


/**
 * gtk_nodes_node_export_properties: (virtual export_properties)
 * @node: a #GtkNodesNode
 *
 * This returns an XML description of the internal state configuration,
 * so it can be restored with GtkBuilder. NULL will be returned if the
 * derived GtkNodesNode subclass did not implement this function
 *
 * Returns: (transfer full): an XML string describing the internal configuration; may be NULL
 */
gchar *
gtk_nodes_node_export_properties (GtkNodesNode *node)
{
  GtkNodesNodeClass *class;

  g_return_val_if_fail (GTKNODES_IS_NODE (node), NULL);

  class = GTKNODES_NODE_GET_CLASS (node);

  if (class->export_properties != NULL)
    return class->export_properties (node);

  return NULL;
}


/**
 * gtk_nodes_node_item_add:
 * @node: a GtkNodesNode
 * @widget: a widget to add
 * @type:  the type of the node
 *
 * Returns: (transfer none): the reference to the socket widget
 */

GtkWidget *
gtk_nodes_node_item_add (GtkNodesNode         *node,
                         GtkWidget            *widget,
                         GtkNodesNodeSocketIO  mode)
{
  return gtk_nodes_node_item_add_real (node, widget, mode);
}


/**
 * gtk_nodes_node_item_remove:
 * @node: a GtkNodesNode
 * @widget: a widget to remove
 *
 * This removes an item and its corresponding socket from a GtkNodesNode
 *´
 */

void
gtk_nodes_node_item_remove (GtkNodesNode         *node,
                            GtkWidget            *widget)
{
  gtk_nodes_node_remove (GTK_CONTAINER(node), widget);
}


/**
 * gtk_nodes_node_new:
 *
 * Creates a new node.
 *
 * Returns: (transfer full): the new #GtkNodesNode.
 */

GtkWidget *
gtk_nodes_node_new (void)
{
  return g_object_new (GTKNODES_TYPE_NODE,
                       "orientation", GTK_ORIENTATION_VERTICAL, NULL);
}

