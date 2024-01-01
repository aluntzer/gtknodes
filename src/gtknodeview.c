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
#include "gtknodeview.h"

#include "gtk/gtkbuilder.h"
#include "glib/gprintf.h"
#include "glib/gstdio.h"

/* gtkprivate.h */
#include "glib-object.h"
#define GTK_NODES_VIEW_PARAM_RW G_PARAM_READWRITE|G_PARAM_STATIC_NAME|G_PARAM_STATIC_NICK|G_PARAM_STATIC_BLURB




#define RESIZE_RECTANGLE 16

/**
 * SECTION:gtknodeview
 * @Short_description: A node viewer
 * @Title: GtkNodesNodeView
 *
 * The #GtkNodesNodeView widget is a viewer and connection manager for
 * #GtkNodesNode widgets.
 */

typedef struct _GtkNodesNodeViewChild        GtkNodesNodeViewChild;
typedef struct _GtkNodesNodeViewConnection   GtkNodesNodeViewConnection;

enum {
  CHILD_PROP_0,
  CHILD_PROP_X,
  CHILD_PROP_Y,
  CHILD_PROP_WIDTH,
  CHILD_PROP_HEIGHT,
};

typedef enum
{
  ACTION_NONE,
  ACTION_DRAG_CHILD,
  ACTION_DRAG_CON,
  ACTION_RESIZE,
  NUM_ACTIONS
} Action;

/* Signals */
enum
{
  NODE_DRAG_BEGIN,
  NODE_DRAG_END,
  LAST_SIGNAL
};


struct _GtkNodesNodeViewPrivate
{
  GList     *children;
  GList     *connections;
  GdkSurface *event_surface;

  GdkCursor *cursor_default;
  GdkCursor *cursor_se_resize;


  Action action;

  guint node_id;                /* node id counter */

  gint x0, y0;                  /* connection drag start coordinates */
  gint x1, y1;                  /* current connection drag cursor coordinates */
};


struct _GtkNodesNodeViewChild
{
  GtkWidget *widget;

  GdkRectangle rectangle;       /* rectangle representing the child */
  GdkRectangle south_east;      /* resize corner */

  gint start_x, start_y;        /* node drag start position */
  gint dx, dy;                  /* node drag deltas */
};


struct _GtkNodesNodeViewConnection
{
  GtkWidget *source;
  GtkWidget *sink;
};


/* widget class basics */


static void     gtk_nodes_node_view_map                 (GtkWidget           *widget);
static void     gtk_nodes_node_view_unmap               (GtkWidget           *widget);
static void     gtk_nodes_node_view_realize             (GtkWidget           *widget);
static void     gtk_nodes_node_view_unrealize           (GtkWidget           *widget);
static void     gtk_nodes_node_view_size_allocate       (GtkWidget           *widget,
                                                         GtkAllocation       *allocation);
static gboolean gtk_nodes_node_view_draw                (GtkWidget           *widget,
                                                         cairo_t             *cr);

/* widget class events */
static gboolean gtk_nodes_node_view_button_press_event  (GtkWidget           *widget,
                                                         GdkEventButton      *event);

static gboolean gtk_nodes_node_view_motion_notify_event (GtkWidget           *widget,
                                                         GdkEventMotion      *event);


/* container class public */
static void     gtk_nodes_node_view_add                 (GtkContainer        *widget,
                                                         GtkWidget           *child);
static void     gtk_nodes_node_view_remove              (GtkContainer        *widget,
                                                         GtkWidget           *child);

static GType    gtk_nodes_node_view_child_type          (GtkContainer        *container);

static void     gtk_nodes_node_view_set_child_property  (GtkContainer         *container,
                                                         GtkWidget            *child,
                                                         guint                 property_id,
                                                         const GValue         *value,
                                                         GParamSpec           *pspec);
static void     gtk_nodes_node_view_get_child_property  (GtkContainer         *container,
                                                         GtkWidget            *child,
                                                         guint                 property_id,
                                                         GValue               *value,
                                                         GParamSpec           *pspec);


/* private methods */
static void     gtk_nodes_node_view_cursor_init         (GtkNodesNodeView    *node_view);
static void     gtk_nodes_node_cursor_set               (GtkNodesNodeView    *node_view,
                                                         Action               action);
static gboolean gtk_nodes_node_view_drag_motion         (GtkWidget           *widget,
                                                         GdkDragContext      *context,
                                                         gint                 x,
                                                         gint                 y,
                                                         guint                time,
                                                         GtkNodesNodeViewPrivate *priv);
static gboolean gtk_nodes_node_view_point_in_rectangle  (GdkRectangle        *rectangle,
                                                         gint                 x,
                                                         gint                 y);

static guint node_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GtkNodesNodeView, gtk_nodes_node_view, GTK_TYPE_CONTAINER)


static void gtk_nodes_node_view_class_init(GtkNodesNodeViewClass *class)
{
  GObjectClass      *gobject_class;
  GtkWidgetClass    *widget_class;
  GtkContainerClass *container_class;


  widget_class    = GTK_WIDGET_CLASS(class);
  gobject_class   = G_OBJECT_CLASS (class);

  /* widget basics */
  widget_class->map           = gtk_nodes_node_view_map;
  widget_class->unmap         = gtk_nodes_node_view_unmap;
  widget_class->realize       = gtk_nodes_node_view_realize;
  widget_class->unrealize     = gtk_nodes_node_view_unrealize;
  widget_class->size_allocate = gtk_nodes_node_view_size_allocate;
  widget_class->draw          = gtk_nodes_node_view_draw;

  /* widget events */
  widget_class->button_press_event  = gtk_nodes_node_view_button_press_event;
  widget_class->motion_notify_event = gtk_nodes_node_view_motion_notify_event;

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_X,
                                              g_param_spec_int ("x",
                                                                "X position",
                                                                "X position of Node",
                                                                G_MININT, G_MAXINT, 0,
                                                                GTK_NODES_VIEW_PARAM_RW));

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_Y,
                                              g_param_spec_int ("y",
                                                                "Y position",
                                                                "Y position of Node",
                                                                G_MININT, G_MAXINT, 0,
                                                                GTK_NODES_VIEW_PARAM_RW));


  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_WIDTH,
                                              g_param_spec_int ("width",
                                                                "requested width",
                                                                "requested width of Node",
                                                                G_MININT, G_MAXINT, 0,
                                                                GTK_NODES_VIEW_PARAM_RW));

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_HEIGHT,
                                              g_param_spec_int ("height",
                                                                "reqested height",
                                                                "requested height of Node",
                                                                G_MININT, G_MAXINT, 0,
                                                                GTK_NODES_VIEW_PARAM_RW));


    /**
   * GtkNodesNodeSocket::node-drag-begin:
   * @widget: the object which received the signal.
   *
   * The ::node-drag-begin signal is emitted when the user begins a drag
   * on the socket handle.
   *
   */

  node_view_signals[NODE_DRAG_BEGIN] =
    g_signal_new ("node-drag-begin",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeViewClass, node_drag_begin),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, GTKNODES_TYPE_NODE);

  /**
   * GtkNodesNodeSocket::node-drag-end:
   * @widget: the object which received the signal.
   *
   * The ::node-drag-begin signal is emitted when the user ends a drag
   * on the socket handle.
   *
   */

  node_view_signals[NODE_DRAG_END] =
    g_signal_new ("node-drag-end",
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GtkNodesNodeViewClass, node_drag_begin),
                  NULL, NULL,
                  NULL,
                  G_TYPE_NONE,
                  1, GTKNODES_TYPE_NODE);


}

static void
gtk_nodes_node_view_init (GtkNodesNodeView *node_view)
{
  GtkNodesNodeViewPrivate *priv;

  node_view->priv = gtk_nodes_node_view_get_instance_private (node_view);
  priv = node_view->priv;

  gtk_nodes_node_view_cursor_init (node_view);

  gtk_widget_set_has_window (GTK_WIDGET (node_view), FALSE);
  gtk_widget_set_size_request(GTK_WIDGET (node_view), 100, 100);

  gtk_drag_dest_set (GTK_WIDGET (node_view),
                     GTK_DEST_DEFAULT_MOTION,
                     NULL, 0, GDK_ACTION_PRIVATE);
  gtk_drag_dest_set_track_motion (GTK_WIDGET (node_view), TRUE);
  g_signal_connect (node_view, "drag-motion",
                    G_CALLBACK (gtk_nodes_node_view_drag_motion), priv);
}

/* Widget Methods */

static void
gtk_nodes_node_view_map (GtkWidget *widget)
{
  GtkNodesNodeViewPrivate *priv;
  GList *l;


  g_return_if_fail (GTKNODES_IS_NODE_VIEW (widget));

  priv = gtk_nodes_node_view_get_instance_private (GTKNODES_NODE_VIEW (widget));

  gtk_widget_set_mapped (widget, TRUE);

  l = priv->children;

  while (l)
    {
      GtkNodesNodeViewChild *child = l->data;

      l = l->next;

      if (!gtk_widget_get_visible (child->widget))
        continue;

      if (!gtk_widget_get_mapped (child->widget))
        gtk_widget_map (child->widget);
    }

  if (priv->event_surface)
    gdk_window_show (priv->event_surface);

}

static void
gtk_nodes_node_view_unmap (GtkWidget *widget)
{
  GtkNodesNodeViewPrivate *priv;


  priv = gtk_nodes_node_view_get_instance_private (GTKNODES_NODE_VIEW (widget));

  if (priv->event_surface)
    gdk_window_hide (priv->event_surface);

  GTK_WIDGET_CLASS (gtk_nodes_node_view_parent_class)->unmap (widget);
}


static void
gtk_nodes_node_view_realize (GtkWidget *widget)
{
  GtkNodesNodeViewPrivate *priv;
  GdkSurface          *surface;
  GtkAllocation       allocation;
  GdkWindowAttr       attributes;
  gint                attributes_mask;
  GList              *l;


  priv = gtk_nodes_node_view_get_instance_private (GTKNODES_NODE_VIEW (widget));

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x           = allocation.x;
  attributes.y           = allocation.y;
  attributes.width       = allocation.width;
  attributes.height      = allocation.height;
  attributes.wclass      = GDK_INPUT_OUTPUT;
  attributes.event_mask  = gtk_widget_get_events (widget);
  attributes.event_mask |= (GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_TOUCH_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_LEAVE_NOTIFY_MASK);

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  window = gtk_widget_get_parent_window (widget);
  g_object_ref (window);
  gtk_widget_set_window (widget, window);

  priv->event_surface = gdk_window_new (window, &attributes, attributes_mask);
  gtk_widget_register_window (widget, priv->event_surface);


  l = priv->children;

  while (l)
    {
      GtkNodesNodeViewChild *child = l->data;

      l = l->next;

      gtk_widget_set_parent_window (child->widget, priv->event_surface);
    }
}

static void
gtk_nodes_node_view_unrealize (GtkWidget *widget)
{
  GtkNodesNodeViewPrivate *priv;


  priv = gtk_nodes_node_view_get_instance_private (GTKNODES_NODE_VIEW (widget));

  if (priv->event_surface)
    {
      gtk_widget_unregister_window (widget, priv->event_surface);
      gdk_window_destroy (priv->event_surface);
      priv->event_surface = NULL;
    }

  GTK_WIDGET_CLASS (gtk_nodes_node_view_parent_class)->unrealize (widget);
}

static void
gtk_nodes_node_view_size_allocate (GtkWidget     *widget,
                                   GtkAllocation *allocation)
{
  GtkNodesNodeViewPrivate *priv;
  GList *l;


  priv = gtk_nodes_node_view_get_instance_private (GTKNODES_NODE_VIEW (widget));


  l = priv->children;

  while (l)
    {
      GtkAllocation allocation_child;
      GtkRequisition requisition;
      gint w, h, socket_radius;

      GtkNodesNodeViewChild *child = l->data;

      l = l->next;

      gtk_widget_get_preferred_size (child->widget, &requisition, NULL);


      g_object_get (G_OBJECT(child->widget), "x", &child->rectangle.x, NULL);
      g_object_get (G_OBJECT(child->widget), "y", &child->rectangle.y, NULL);

      allocation_child.x      = child->rectangle.x;
      allocation_child.y      = child->rectangle.y;
      allocation_child.width  = MAX(requisition.width, child->rectangle.width);
      allocation_child.height = MAX(requisition.height, child->rectangle.height);

      gtk_widget_size_allocate (child->widget, &allocation_child);

      gtk_widget_get_allocation (child->widget, &allocation_child);

      if (GTKNODES_IS_NODE (child->widget))
        socket_radius = (gint) gtk_nodes_node_get_socket_radius (GTKNODES_NODE (child->widget));
      else
        socket_radius = 0;

      child->south_east.x = allocation_child.width  - socket_radius - RESIZE_RECTANGLE;
      child->south_east.y = allocation_child.height - socket_radius - RESIZE_RECTANGLE;

      w = allocation_child.x + allocation_child.width;
      h = allocation_child.y + allocation_child.height;

      if (w > allocation->width)
        allocation->width = w;

      if (h > allocation->height)
        allocation->height = h;
    }

  gtk_widget_set_allocation (widget, allocation);
  gtk_widget_set_size_request (widget, allocation->width, allocation->height);

  if (!gtk_widget_get_realized (widget))
    return;

  if (!priv->event_surface)
    return;

  gdk_window_move_resize (priv->event_surface,
                          allocation->x,
                          allocation->y,
                          allocation->width,
                          allocation->height);
}

static void
gtk_nodes_node_connecting_curve (GtkWidget *widget,
                                 cairo_t   *cr,
                                 gint       x0,
                                 gint       y0,
                                 gint       x1,
                                 gint       y1)
{
  gint x1m, y1m;
  gint x2m, y2m;
  gint d;


  cairo_move_to (cr, x0, y0);

  d = abs(x1 - x0) / 2;

  x1m = x0 + d;
  y1m = y0;

  x2m = x1 - d;
  y2m = y1;

  cairo_curve_to (cr, x1m, y1m, x2m, y2m, x1 , y1);
}

static void
gtk_nodes_node_draw_socket_connection (GtkWidget                  *widget,
                                       cairo_t                    *cr,
                                       GtkNodesNodeViewConnection *c)
{
  cairo_pattern_t *pat;
  GdkRGBA col_src, col_sink;

  GtkAllocation allocation;
  GtkAllocation alloc_parent;
  gint x0, x1, y0, y1;


  gtk_widget_get_allocation (gtk_widget_get_parent (c->source), &alloc_parent);
  gtk_widget_get_allocation (c->source, &allocation);
  x0 = allocation.x + allocation.width  / 2 + alloc_parent.x;
  y0 = allocation.y + allocation.height / 2 + alloc_parent.y;

  gtk_widget_get_allocation (gtk_widget_get_parent (c->sink), &alloc_parent);
  gtk_widget_get_allocation (c->sink, &allocation);
  x1 = allocation.x + allocation.width  / 2 + alloc_parent.x;
  y1 = allocation.y + allocation.height / 2 + alloc_parent.y;



  pat = cairo_pattern_create_linear (x0, y0,  x1, y1);

  gtk_nodes_node_socket_get_rgba (GTKNODES_NODE_SOCKET (c->source), &col_src);
  gtk_nodes_node_socket_get_rgba (GTKNODES_NODE_SOCKET (c->sink),   &col_sink);

  cairo_pattern_add_color_stop_rgba (pat, 0, col_src.red,
                                     col_src.green,
                                     col_src.blue,
                                     col_src.alpha);

  cairo_pattern_add_color_stop_rgba (pat, 1, col_sink.red,
                                     col_sink.green,
                                     col_sink.blue,
                                     col_sink.alpha);

  cairo_save(cr);

  gtk_nodes_node_connecting_curve(widget, cr, x0, y0, x1, y1);

  cairo_set_source (cr, pat);
  cairo_stroke (cr);
  cairo_pattern_destroy (pat);

  cairo_restore(cr);
}

static gboolean
gtk_nodes_node_view_draw (GtkWidget *widget,
                          cairo_t   *cr)
{
  GtkNodesNodeViewPrivate *priv;
  GList *l;


  priv = gtk_nodes_node_view_get_instance_private (GTKNODES_NODE_VIEW (widget));

  if (priv->action == ACTION_DRAG_CON)
    {
      cairo_save(cr);
      cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);

      gtk_nodes_node_connecting_curve(widget, cr,
                                      priv->x0, priv->y0,
                                      priv->x1, priv->y1);

      cairo_stroke(cr);
      cairo_restore(cr);
    }


  l = priv->connections;

  while (l)
    {
      GtkNodesNodeViewConnection *c = l->data;
      l = l->next;

      gtk_nodes_node_draw_socket_connection (widget, cr, c);

    }

  if (gtk_cairo_should_draw_window (cr, priv->event_surface))
    GTK_WIDGET_CLASS (gtk_nodes_node_view_parent_class)->draw (widget, cr);

  return GDK_EVENT_PROPAGATE;
}

static void
gtk_nodes_node_view_move_child (GtkNodesNodeView      *node_view,
                                GtkNodesNodeViewChild *child,
                                gint                   x,
                                gint                   y)
{
  gint xmax, ymax;

  GtkAllocation allocation_view;
  GtkAllocation allocation_child;
  GtkNodesNodeViewPrivate *priv;


  priv = gtk_nodes_node_view_get_instance_private (node_view);

  g_assert (child);

  gtk_widget_get_allocation (GTK_WIDGET (node_view), &allocation_view);
  gtk_widget_get_allocation (GTK_WIDGET (child->widget), &allocation_child);

  x = child->rectangle.x + x;
  y = child->rectangle.y + y;

  xmax = allocation_view.width  - allocation_child.width;
  ymax = allocation_view.height - allocation_child.height;

  /* keep child within node view allocation */
  if ((x > 0) && (x < xmax))
    child->rectangle.x = x;
  else if (x < 0)
    child->rectangle.x = 0;
  else if (x > xmax)
    child->rectangle.x = xmax;

  if ((y > 0) && (y < ymax))
    child->rectangle.y = y;
  else if (y < 0)
    child->rectangle.y = 0;
  else if (y > ymax)
    child->rectangle.y = ymax;

  g_object_set (G_OBJECT(child->widget), "x", child->rectangle.x, NULL);
  g_object_set (G_OBJECT(child->widget), "y", child->rectangle.y, NULL);

  if (gtk_widget_get_visible (child->widget))
    gtk_widget_queue_resize (child->widget);

  /* "raise" window, drawing occurs from start -> end of list */
  priv->children = g_list_append( g_list_remove (priv->children, child), child);

  /* queue draw for smooth refresh while the drag action is going on */
  gtk_widget_queue_draw (GTK_WIDGET (node_view));
}

static gboolean
gtk_nodes_node_view_point_in_rectangle (GdkRectangle *rectangle,
                                        gint          x,
                                        gint          y)
{
  GdkRectangle point = {x, y, 1, 1};

  return gdk_rectangle_intersect (rectangle, &point, NULL);
}

static gboolean
gtk_nodes_node_view_child_motion_notify_event (GtkWidget         *widget,
                                               GdkEventMotion    *event,
                                               GtkNodesNodeViewChild *child)
{

  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;

  node_view = GTKNODES_NODE_VIEW (gtk_widget_get_parent (widget));

  priv = gtk_nodes_node_view_get_instance_private (node_view);


  if (priv->action == ACTION_NONE)
    {
      gboolean inside;

      inside = gtk_nodes_node_view_point_in_rectangle (&child->south_east,
                                                       (gint) event->x,
                                                       (gint) event->y);
      if (inside)
        gtk_nodes_node_cursor_set(node_view, ACTION_RESIZE);
      else
        gtk_nodes_node_cursor_set(node_view, ACTION_NONE);
    }

  if (event->state & GDK_BUTTON1_MASK)
    {

      if (priv->action == ACTION_DRAG_CHILD)
        {
          gtk_nodes_node_block_expander (GTKNODES_NODE (child->widget));
          gtk_nodes_node_view_move_child(node_view, child,
                                         (gint) event->x - child->start_x,
                                         (gint) event->y - child->start_y);
        }

      if (priv->action == ACTION_RESIZE)
        {
          gint w, h;

          w = (gint) event->x - child->rectangle.x - child->dx;
          h = (gint) event->y - child->rectangle.y - child->dy;

          child->rectangle.width  = MAX (0, w);
          child->rectangle.height = MAX (0, h);

          g_object_set (G_OBJECT(child->widget), "width",
                        child->rectangle.width, NULL);

          g_object_set (G_OBJECT(child->widget), "height",
                        child->rectangle.height, NULL);

          gtk_widget_queue_resize(child->widget);
          gtk_widget_queue_draw (GTK_WIDGET (node_view));
        }
    }

  return GDK_EVENT_LAST;
}

static gboolean
gtk_nodes_node_view_child_pointer_crossing_event (GtkWidget        *widget,
                                                  GdkEventCrossing *event)
{
  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;

  node_view = GTKNODES_NODE_VIEW (gtk_widget_get_parent (widget));

  priv = gtk_nodes_node_view_get_instance_private (node_view);

  if (priv->action == ACTION_RESIZE)
    return GDK_EVENT_PROPAGATE;

  switch (event->type)
    {
    case GDK_LEAVE_NOTIFY:
    default:
      gtk_nodes_node_cursor_set(node_view, ACTION_NONE);
      break;
    }

	return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_view_child_button_press_event (GtkWidget        *widget,
                                              GdkEventButton   *event,
                                              GtkNodesNodeViewChild *child)
{
  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;

  node_view = GTKNODES_NODE_VIEW (gtk_widget_get_parent (widget));

  priv = gtk_nodes_node_view_get_instance_private (node_view);


  if (event->button == GDK_BUTTON_PRIMARY)
    {
      gboolean inside;
      GtkAllocation child_alloc;

      inside = gtk_nodes_node_view_point_in_rectangle (&child->south_east,
                                                       (gint) event->x,
                                                       (gint) event->y);
      if (inside)
        {
        priv->action = ACTION_RESIZE;
        }
      else
        {
        priv->action = ACTION_DRAG_CHILD;
        g_signal_emit (node_view, node_view_signals[NODE_DRAG_BEGIN], 0, child->widget);
        }

      child->start_x = (gint) event->x;
      child->start_y = (gint) event->y;

      gtk_widget_get_allocation(child->widget, &child_alloc);
      child->dx = (gint) event->x - (child_alloc.x + child_alloc.width);
      child->dy = (gint) event->y - (child_alloc.y + child_alloc.height);
    }


  return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_view_child_button_release_event (GtkWidget         *widget,
                                                GdkEventButton    *event,
                                                GtkNodesNodeViewChild *child)
{
  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;


  node_view = GTKNODES_NODE_VIEW (gtk_widget_get_parent (widget));

  priv = gtk_nodes_node_view_get_instance_private (node_view);

  if (event->button == GDK_BUTTON_PRIMARY)
    gtk_nodes_node_unblock_expander (GTKNODES_NODE (child->widget));

  if (priv->action == ACTION_DRAG_CHILD)
    g_signal_emit (node_view, node_view_signals[NODE_DRAG_END], 0, child->widget);

  priv->action = ACTION_NONE;

  /* "raise" last clicked window, drawing occurs from start -> end of list */
  priv->children = g_list_append( g_list_remove (priv->children, child), child);

  gtk_widget_queue_draw (GTK_WIDGET (node_view));

  return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_view_socket_drag_begin_event (GtkWidget           *widget,
                                             gint                 x0,
                                             gint                 y0,
                                             GtkNodesNodeViewPrivate *priv)
{
  priv->action = ACTION_DRAG_CON;
  priv->x0 = x0;
  priv->y0 = y0;

  return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_view_socket_drag_end_event (GtkWidget           *widget,
                                           GtkNodesNodeViewPrivate *priv)
{
  priv->action = ACTION_NONE;

  gtk_widget_queue_draw (gtk_widget_get_parent (widget));
  return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_view_socket_connect_event (GtkWidget *node,
                                          GtkWidget *sink,
                                          GtkWidget *source,
                                          gpointer   user_data)
{
  GtkNodesNodeViewPrivate *priv;
  GtkNodesNodeViewConnection *con;


  priv = gtk_nodes_node_view_get_instance_private (GTKNODES_NODE_VIEW (user_data) );

  con = g_slice_new (GtkNodesNodeViewConnection);

  con->source = source;
  con->sink   = sink;

  priv->connections = g_list_append (priv->connections, con);

  gtk_widget_queue_draw (GTK_WIDGET (user_data));

  return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_view_socket_disconnect_event (GtkWidget *node,
                                             GtkWidget *sink,
                                             GtkWidget *source,
                                             gpointer   user_data)
{
  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;
  GList *l;

  node_view = GTKNODES_NODE_VIEW (user_data);
  priv = gtk_nodes_node_view_get_instance_private (node_view);

  l = priv->connections;
  while (l)
    {
      GtkNodesNodeViewConnection *con = l->data;

      if ((con->source == source) && (con->sink == sink)) {

        priv->connections = g_list_remove_link (priv->connections, l);
        g_slice_free (GtkNodesNodeViewConnection, con);
        g_list_free_1 (l);

        break; /* there should only ever be one */
      }

      l = l->next;
    }

  gtk_widget_queue_draw (GTK_WIDGET (node_view));

  return GDK_EVENT_PROPAGATE;
}

static gboolean
gtk_nodes_node_view_socket_destroyed_event (GtkWidget *node,
                                            GtkWidget *socket,
                                            gpointer  user_data)
{
  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;
  GList *l;
  GList *tmp;


  node_view = GTKNODES_NODE_VIEW (user_data);
  priv = gtk_nodes_node_view_get_instance_private (node_view);

  l = priv->connections;

  while (l)
    {
      GtkNodesNodeViewConnection *con = l->data;

      tmp = l;
      l = l->next;

      if ((con->source == socket) || (con->sink == socket)) {

        priv->connections = g_list_remove_link (priv->connections, tmp);
        g_slice_free (GtkNodesNodeViewConnection, con);
        g_list_free_1 (tmp);
      }
    }

  gtk_widget_queue_draw (GTK_WIDGET (node_view));

  return GDK_EVENT_PROPAGATE;
}


/* Container Methods */

static void
gtk_nodes_node_view_add (GtkContainer *container,
                         GtkWidget    *widget)
{
  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;
  GtkNodesNodeViewChild *child;


  g_return_if_fail (GTKNODES_IS_NODE_VIEW (container));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  node_view = GTKNODES_NODE_VIEW (container);
  priv = gtk_nodes_node_view_get_instance_private (node_view);

  child = g_slice_new (GtkNodesNodeViewChild);

  child->widget = widget;
  child->rectangle.x      = 100;
  child->rectangle.y      = 100;
  child->rectangle.width  = 100;
  child->rectangle.height = 100;

  child->south_east.x      = child->rectangle.width  - RESIZE_RECTANGLE;
  child->south_east.y      = child->rectangle.height - RESIZE_RECTANGLE;
  child->south_east.width  = RESIZE_RECTANGLE;
  child->south_east.height = RESIZE_RECTANGLE;


  gtk_widget_add_events(widget, GDK_BUTTON_PRESS_MASK |
                        GDK_BUTTON_RELEASE_MASK |
                        GDK_BUTTON1_MOTION_MASK);

  g_signal_connect(G_OBJECT (widget),
                   "button-press-event",
                   G_CALLBACK (gtk_nodes_node_view_child_button_press_event),
                   child);

  g_signal_connect(G_OBJECT (widget),
                   "button-release-event",
                   G_CALLBACK (gtk_nodes_node_view_child_button_release_event),
                   child);

  g_signal_connect(G_OBJECT (widget),
                   "motion-notify-event",
                   G_CALLBACK (gtk_nodes_node_view_child_motion_notify_event),
                   child);
#if 0
  g_signal_connect (G_OBJECT (widget),
                    "enter-notify-event",
                    G_CALLBACK (gtk_nodes_node_view_child_pointer_crossing_event),
                    NULL);
#endif
  g_signal_connect (G_OBJECT (widget),
                    "leave-notify-event",
                    G_CALLBACK (gtk_nodes_node_view_child_pointer_crossing_event),
                    NULL);



  /* the things we do for glade... */
  if (GTKNODES_IS_NODE (child->widget))
    {
      g_signal_connect(G_OBJECT (widget),
                       "node-socket-drag-begin",
                       G_CALLBACK (gtk_nodes_node_view_socket_drag_begin_event),
                       priv);

      g_signal_connect(G_OBJECT (widget),
                       "node-socket-drag-end",
                       G_CALLBACK (gtk_nodes_node_view_socket_drag_end_event),
                       priv);

      g_signal_connect(G_OBJECT (widget),
                       "node-socket-connect",
                       G_CALLBACK (gtk_nodes_node_view_socket_connect_event),
                       node_view);

      g_signal_connect(G_OBJECT (widget),
                       "node-socket-disconnect",
                       G_CALLBACK (gtk_nodes_node_view_socket_disconnect_event),
                       node_view);

      g_signal_connect(G_OBJECT (widget),
                       "node-socket-destroyed",
                       G_CALLBACK (gtk_nodes_node_view_socket_destroyed_event),
                       node_view);

      g_object_set (G_OBJECT (child->widget), "id", priv->node_id++, NULL);
    }

  priv->children = g_list_append (priv->children, child);

  if (gtk_widget_get_realized (GTK_WIDGET (node_view)))
    gtk_widget_set_parent_window (child->widget, priv->event_surface);

  gtk_widget_set_parent (widget, GTK_WIDGET (container));
}

static GtkNodesNodeViewChild*
gtk_nodes_node_view_get_child (GtkNodesNodeView  *node_view,
                               GtkWidget *widget)
{
  GtkNodesNodeViewPrivate *priv;
  GList *l;

  priv = gtk_nodes_node_view_get_instance_private (node_view);

  l = priv->children;

  while (l)
    {
      GtkNodesNodeViewChild *child = l->data;

      l = l->next;

      if (child->widget == widget)
        return child;
    }

  g_warning ("GtkWidget %p is not a child of GtkNodeView %p",
             (void *) widget, (void *) node_view);

  return NULL;
}

static void
gtk_nodes_node_view_cursor_init (GtkNodesNodeView *node_view)
{
	GdkDisplay *display;
  GtkNodesNodeViewPrivate *priv;

  priv = gtk_nodes_node_view_get_instance_private (node_view);

	display = gtk_widget_get_display (GTK_WIDGET (node_view));

  priv->cursor_default   = gdk_cursor_new_from_name (display, "default");
  priv->cursor_se_resize = gdk_cursor_new_from_name (display, "se-resize");
}

static void
gtk_nodes_node_cursor_set (GtkNodesNodeView *node_view, Action action)
{
  GdkCursor *cursor;
	GdkSurface *surface;
  GtkNodesNodeViewPrivate *priv;


  priv = gtk_nodes_node_view_get_instance_private (node_view);

	window  = gtk_widget_get_window (GTK_WIDGET (node_view));

  switch (action)
    {
      case ACTION_RESIZE:
        cursor = priv->cursor_se_resize;
        break;
      case ACTION_NONE:
      default:
        cursor = priv->cursor_default;
        break;
    }

  gdk_window_set_cursor(window, cursor);
}

static gboolean
gtk_nodes_node_view_drag_motion (GtkWidget      *widget,
                                 GdkDragContext *context,
                                 gint            x,
                                 gint            y,
                                 guint           time,
                                 GtkNodesNodeViewPrivate *priv)
{
  priv->x1 = x;
  priv->y1 = y;

  gtk_widget_queue_draw(widget);

  return GDK_EVENT_PROPAGATE;

}

static gboolean
gtk_nodes_node_view_button_press_event (GtkWidget      *widget,
                                        GdkEventButton *event)
{
  GTK_WIDGET_CLASS (gtk_nodes_node_view_parent_class)->button_press_event (widget, event);

  return FALSE;
}

static gboolean
gtk_nodes_node_view_motion_notify_event (GtkWidget      *widget,
                                   GdkEventMotion *event)
{
  GTK_WIDGET_CLASS (gtk_nodes_node_view_parent_class)->motion_notify_event (widget, event);

  return FALSE;
}

static void
gtk_nodes_node_view_remove (GtkContainer *container,
                            GtkWidget    *widget)
{
  GtkNodesNodeView *node_view;
  GtkNodesNodeViewPrivate *priv;
  GtkNodesNodeViewChild *child = NULL;
  GList *l;


  node_view = GTKNODES_NODE_VIEW (container);
  priv = gtk_nodes_node_view_get_instance_private (node_view);

  l = priv->children;

  while (l)
    {
      child = l->data;

      if (child->widget == widget)
        break;

      l = l->next;
    }

  if (l == NULL)
    return;


  priv->children = g_list_remove_link (priv->children, l);

  gtk_widget_unparent (widget);

  g_slice_free (GtkNodesNodeViewChild, l->data);
  g_list_free_1 (l);
}

static GType
gtk_nodes_node_view_child_type (GtkContainer *container)
{
  return GTKNODES_TYPE_NODE;
}

static void
gtk_nodes_node_view_set_child_property (GtkContainer *container,
                                        GtkWidget    *child,
                                        guint         property_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  GtkNodesNodeViewChild *node_child;
  GtkNodesNodeView *node_view = GTKNODES_NODE_VIEW (container);

  node_child = gtk_nodes_node_view_get_child (node_view, child);

  if (node_child == NULL)
    return;

  /* this is kinda stupid, but at the moment I don't see how I can save an
   * XML description of the node view's contents and restore a (partial)
   * configuration without tracking the child properties within the
   * node children as well */

  switch (property_id)
  {
  case CHILD_PROP_X:
    node_child->rectangle.x = g_value_get_int (value);
    if (GTKNODES_IS_NODE (node_child->widget))
      g_object_set (G_OBJECT(node_child->widget),
                    "x", node_child->rectangle.x, NULL);
    gtk_widget_queue_allocate(child);
    break;
  case CHILD_PROP_Y:
    node_child->rectangle.y = g_value_get_int (value);
    if (GTKNODES_IS_NODE (node_child->widget))
      g_object_set (G_OBJECT(node_child->widget),
                    "y", node_child->rectangle.x, NULL);
    gtk_widget_queue_allocate(child);
    break;
  case CHILD_PROP_WIDTH:
    node_child->rectangle.width = g_value_get_int (value);
    if (GTKNODES_IS_NODE (node_child->widget))
      g_object_set (G_OBJECT(node_child->widget),
                    "width", node_child->rectangle.width, NULL);
    gtk_widget_queue_allocate(child);
    break;
  case CHILD_PROP_HEIGHT:
    node_child->rectangle.height = g_value_get_int (value);
    if (GTKNODES_IS_NODE (node_child->widget))
      g_object_set (G_OBJECT(node_child->widget),
                    "height", node_child->rectangle.height, NULL);
    gtk_widget_queue_allocate(child);
    break;
  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
      break;
    }
}

static void
gtk_nodes_node_view_get_child_property (GtkContainer *container,
                                        GtkWidget    *child,
                                        guint         property_id,
                                        GValue       *value,
                                        GParamSpec   *pspec)
{
  GtkNodesNodeViewChild *node_child;
  GtkNodesNodeView *node_view = GTKNODES_NODE_VIEW (container);

  node_child = gtk_nodes_node_view_get_child (node_view, child);

  if (node_child == NULL)
    return;

  switch (property_id)
    {
    case CHILD_PROP_X:
      if (GTKNODES_IS_NODE (node_child->widget))
        g_object_get (G_OBJECT(node_child->widget),
                      "x", &node_child->rectangle.x, NULL);
      g_value_set_int (value, node_child->rectangle.x);
      break;
    case CHILD_PROP_Y:
      if (GTKNODES_IS_NODE (node_child->widget))
        g_object_get (G_OBJECT(node_child->widget),
                      "x", &node_child->rectangle.y, NULL);
      g_value_set_int (value, node_child->rectangle.y);
      break;
    case CHILD_PROP_WIDTH:
      if (GTKNODES_IS_NODE (node_child->widget))
        g_object_get (G_OBJECT(node_child->widget),
                      "width", &node_child->rectangle.width, NULL);
      g_value_set_int (value, node_child->rectangle.width);
      break;
    case CHILD_PROP_HEIGHT:
      if (GTKNODES_IS_NODE (node_child->widget))
        g_object_get (G_OBJECT(node_child->widget),
                      "height", &node_child->rectangle.height, NULL);
      g_value_set_int (value, node_child->rectangle.height);
      break;
    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
      break;
    }
}

static void
gtk_nodes_node_view_connection_mapper (GtkBuilder    *builder,
                                       GObject       *object,
                                       const gchar   *signal_name,
                                       const gchar   *handler_name,
                                       GObject       *connect_object,
                                       GConnectFlags  flags,
                                       gpointer       user_data)
{
  GtkNodesNodeSocket *sink = NULL;
  GtkNodesNodeSocket *source = NULL;
  GList *l;
  GList *sockets;

  guint id_source;
  guint id_sink;

  /* retrieve sink and source ids from handler name */
  sscanf(handler_name, "%u_%u", &id_source, &id_sink);

  sockets = gtk_nodes_node_get_sources (GTKNODES_NODE (connect_object));

  l = sockets;

  while (l)
    {
      guint id;

      g_object_get(GTKNODES_NODE_SOCKET (l->data), "id", &id, NULL);

      if (id == id_source)
        {
          source = GTKNODES_NODE_SOCKET (l->data);
          break;
        }

      l = l->next;
    }


  sockets = gtk_nodes_node_get_sinks (GTKNODES_NODE (object));
  l = sockets;

  while (l)
    {
      guint id;

      g_object_get(GTKNODES_NODE_SOCKET (l->data), "id", &id, NULL);

      if (id == id_sink)
        {
          sink = GTKNODES_NODE_SOCKET (l->data);
          break;
        }

      l = l->next;
    }

  if (source == NULL || sink == NULL)
    return;

  gtk_nodes_node_socket_connect_sockets (sink, source);
}






/**
 * gtk_nodes_node_view_save:
 * @node_view: a GtkNodesNodeView
 * @filename: the name of the file to save, if the file exists, it will be overwritten
 *
 * Saves a representation of the current node view setup as XML so
 * it can be recreated with gtkbuilder
 * This only works properly for nodes which are their own widget types, as we
 * don't (and can't) in-depth clone the nodes
 *
 * Returns: 0 on error
 */

gboolean
gtk_nodes_node_view_save (GtkNodesNodeView *node_view,
                          const gchar      *filename)
{
  GtkNodesNodeViewPrivate *priv;
  GList *l;
  GList *s;
  GList *sockets;

  priv = gtk_nodes_node_view_get_instance_private (node_view);

	FILE *f;


  g_return_val_if_fail (GTKNODES_IS_NODE_VIEW (node_view), FALSE);

  if (filename == NULL)
    {
      g_warning ("No filename specified");
      return FALSE;
    }

	f = g_fopen(filename, "w+");

  if (f == NULL)
    {
      g_warning ("Error opening file %s for writing", filename);
      return FALSE;
    }


  /* lead in */
  g_fprintf (f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  g_fprintf (f, "<interface>\n");



  /* fixup the IDs so we can properly load, add and save again
   * XXX I really need to think of a better method for unique IDs
   */
  priv->node_id = 0;
  l = priv->children;
  while (l)
    {
      GtkNodesNodeViewChild *child = l->data;

      l = l->next;

      if (!GTKNODES_IS_NODE (child->widget))
        continue;

      g_object_set(child->widget, "id", priv->node_id, NULL);
      priv->node_id++;
    }


  l = priv->children;

  while (l)
    {
      gchar *internal_cfg;
      GtkNodesNodeSocket *input;
      guint x, y, width, height, id;

      GtkNodesNodeViewChild *child = l->data;

      l = l->next;

      if (!GTKNODES_IS_NODE (child->widget))
        continue;

      g_object_get(child->widget, "x", &x, "y", &y,
                   "width", &width, "height", &height,
                   "id", &id, NULL);

      g_fprintf (f, "<object class=\"%s\" id=\"%d\">\n",
                 G_OBJECT_TYPE_NAME(child->widget), id);

      g_fprintf (f, "<property name=\"x\">%d</property>\n", x);
      g_fprintf (f, "<property name=\"y\">%d</property>\n", y);
      g_fprintf (f, "<property name=\"width\">%d</property>\n", width);
      g_fprintf (f, "<property name=\"height\">%d</property>\n", height);
      g_fprintf (f, "<property name=\"id\">%d</property>\n", id);



      sockets = gtk_nodes_node_get_sinks (GTKNODES_NODE (child->widget));

      if (sockets == NULL)
        {
          /* meh...*/
          internal_cfg = gtk_nodes_node_export_properties(GTKNODES_NODE (child->widget));

          if (internal_cfg != NULL)
          {
            g_fprintf (f, "%s", internal_cfg);
            g_free (internal_cfg);
          }


          g_fprintf (f, "</object>\n");
          continue;
        }

      s = sockets;

      while (s)
        {
          GtkWidget *node;
          guint id_source;
          guint id_sink;

          input = gtk_nodes_node_socket_get_input (GTKNODES_NODE_SOCKET (s->data));

          if (input == NULL)
            {
              s = s->next;
              continue;
            }

          g_object_get(GTK_WIDGET (input), "id", &id_source, NULL);
          g_object_get(GTK_WIDGET (s->data), "id", &id_sink, NULL);
          node = gtk_widget_get_ancestor (GTK_WIDGET (input), GTKNODES_TYPE_NODE);
          g_object_get(node, "id", &id, NULL);

          /* we'll save the socket ids in the name of the handler and
           * reconstruct them in gtk_nodes_node_view_connection_mapper()
           * this way we can (ab)use GtkBuilder to do most of the work for us
           */
          g_fprintf (f, "<signal name=\"node-socket-connect\" handler=\"%d_%d\" "
                        "object=\"%d\"/>\n", id_source, id_sink, id);

          s = s->next;
        }

          /* meh...*/
      internal_cfg = gtk_nodes_node_export_properties(GTKNODES_NODE (child->widget));

      if (internal_cfg != NULL)
        {
         g_fprintf (f, "%s", internal_cfg);
         g_free (internal_cfg);
        }

      g_fprintf (f, "</object>\n");

      g_list_free (sockets);
    }


  /*lead out */
  g_fprintf (f, "</interface>\n");

  fclose(f);

  return TRUE;
}

/**
 * gtk_nodes_node_view_load:
 * @node_view: a GtkNodesNodeView
 * @filename: the name of the file to load
 *
 * Loads an XML description parseable by #GtkBuilder and reconstructs
 * a node configuration. Note that this will not restore the internal
 * state of any nodes, but only their placement and the connections
 * between sockets.
 *
 * This only works properly for nodes which are their own widget types.
 *
 * Returns: 0 if and error occured
 */

gboolean
gtk_nodes_node_view_load (GtkNodesNodeView *node_view,
                          const gchar      *filename)
{
	GtkBuilder* builder;
  GError *error = NULL;
  GSList *l;


  g_return_val_if_fail (GTKNODES_IS_NODE_VIEW (node_view), FALSE);

  if (filename == NULL)
    {
      g_warning ("No filename specified");
      return FALSE;
    }

  builder = gtk_builder_new();

  if (!gtk_builder_add_from_file(builder, filename, &error))
    {
      g_warning ("Error occured loading nodes from file: %s", error->message);
      g_clear_error(&error);

      return FALSE;
    }

  l = gtk_builder_get_objects(builder);

	while (l)
    {
 	 	 GObject *n = l->data;

	   l = l->next;

     if (gtk_widget_get_parent(GTK_WIDGET(n)) == NULL)
       gtk_container_add(GTK_CONTAINER(node_view), GTK_WIDGET(n));
 	 }

	gtk_builder_connect_signals_full (builder,
                                    gtk_nodes_node_view_connection_mapper,
                                    node_view);

	gtk_widget_show_all(GTK_WIDGET (node_view));

  return TRUE;
}

/**
 * gtk_nodes_node_view_new:
 *
 * Creates a new node view.
 *
 * Returns: a new #GtkNodesNodeView.
 */

GtkWidget*
gtk_nodes_node_view_new (void)
{
  return g_object_new (GTKNODES_TYPE_NODE_VIEW, NULL);
}
