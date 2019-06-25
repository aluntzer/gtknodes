/**
 * @file    examples/nodes/node_step.c
 * @author  Armin Luntzer (armin.luntzer@univie.ac.at)
 *
 * @copyright GPLv2
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * @brief a GtkNode stepping through a configurable interval
 *
 * @note This example also demonstrates the use of GtkBuilder to restore
 *	 internal child properties.
 *
 * XXX The weird thing is that GtkBuilder apparently wants to add the internal
 * children to the GtkNode widget, even though the "internal-child" property is
 * set. This obviously fails, resulting in a warning message to be printed,
 * however the restoration of the child's property succeeds. I have not
 * yet figured out, why this is the case, maybe this requires another builder
 * class function to be set.
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_step.h>

/* configurable pulse range */
#define NODE_STEP_INTERVAL_MIN	-1000.0
#define NODE_STEP_INTERVAL_MAX	 1000.0
#define NODE_STEP_INTERVAL_STP	 0.01

#define NODE_STEP_BLINK_TIMEOUT_MS	100


struct _NodeStepPrivate {
	GtkWidget *rst;
	GtkWidget *trg_i;
	GtkWidget *trg_o;
	GtkWidget *dat_o;
	GtkWidget *bar;

	GtkWidget *sb_start;
	GtkWidget *sb_stop;
	GtkWidget *sb_step;

	GByteArray *payload;

	gdouble *cur;
	gdouble min;
	gdouble max;
	gdouble stp;

	GdkRGBA rgba_trg_o;

	guint id_trg;
	guint id_out;
	guint id_inp;
};


static GtkBuildableIface *parent_buildable_iface;


static void node_step_buildable_interface_init(GtkBuildableIface *iface);

static GObject *node_step_buildable_get_internal_child(GtkBuildable *buildable,
						       GtkBuilder   *builder,
						       const gchar  *childname);

G_DEFINE_TYPE_WITH_CODE (NodeStep, node_step, GTKNODES_TYPE_NODE,
                         G_ADD_PRIVATE (NodeStep)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                                                node_step_buildable_interface_init))


static void node_step_buildable_interface_init(GtkBuildableIface *iface)
{
	parent_buildable_iface = g_type_interface_peek_parent(iface);
	iface->get_internal_child = node_step_buildable_get_internal_child;
}

/* needed to restore an internal state */
static GObject *node_step_buildable_get_internal_child (GtkBuildable *buildable,
							GtkBuilder   *builder,
							const gchar  *childname)
{
	NodeStep *step;
	NodeStepPrivate *priv;


	step = NODE_STEP(buildable);

	priv = node_step_get_instance_private(step);

	if (g_strcmp0 (childname, "sb_start") == 0)
		return G_OBJECT (priv->sb_start);

	if (g_strcmp0 (childname, "sb_stop") == 0)
		return G_OBJECT (priv->sb_stop);

	if (g_strcmp0 (childname, "sb_step") == 0)
		return G_OBJECT (priv->sb_step);

	return parent_buildable_iface->get_internal_child(buildable,
							  builder,
							  childname);
}


static gboolean node_step_deactivate_timeout_trg_out (gpointer data)
{
	NodeStepPrivate *priv;


	priv = NODE_STEP(data)->priv;

	priv->id_trg = 0;

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->trg_o),
				       &priv->rgba_trg_o);

	return G_SOURCE_REMOVE;
}


static gboolean node_step_deactivate_timeout_dat_out (gpointer data)
{
	NodeStepPrivate *priv;


	priv = NODE_STEP(data)->priv;

	priv->id_out = 0;

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->dat_o),
				       &COL_DOUBLE);

	return G_SOURCE_REMOVE;
}


static void node_step_blink_data_out(NodeStep *step)
{
	NodeStepPrivate *priv;


	priv = step->priv;

	if (priv->id_out)
		return;

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->dat_o),
				       &COL_BLINK);

	priv->id_out = g_timeout_add(NODE_STEP_BLINK_TIMEOUT_MS,
				     node_step_deactivate_timeout_dat_out,
				     step);
}


static void node_step_blink_trg_out(NodeStep *step)
{
	NodeStepPrivate *priv;


	priv = step->priv;

	if (priv->id_trg)
		return;

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->trg_o),
				       &COL_BLINK);

	priv->id_trg = g_timeout_add(NODE_STEP_BLINK_TIMEOUT_MS,
				     node_step_deactivate_timeout_trg_out,
				     step);
}


static void node_step_field_changed(GtkWidget *w, gdouble *val)
{
	(*val) = gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
}


static void node_progress_bar_update(NodeStep *step)
{
	gdouble frac;
	gchar *buf;
	NodeStepPrivate *priv;


	priv = step->priv;

	frac = ((* priv->cur) - priv->min) / (priv->max - priv->min);

	buf = g_strdup_printf("%6.2f of [%g : %g]", (* priv->cur),
			      priv->min, priv->max);

	if (frac >= 0.0) {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->bar), frac);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(priv->bar), buf);
	}

	g_free(buf);
}


static void node_step_output(NodeStep *step)
{
	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(step->priv->dat_o),
				    step->priv->payload);

	node_step_blink_data_out(step);
}


static void node_step_trigger(GtkWidget  *widget,
			      GByteArray *payload,
			      NodeStep   *step)
{
	NodeStepPrivate *priv;


	priv = step->priv;

	if (priv->min < priv->max) {
		if ((* priv->cur) < priv->max)
			(* priv->cur) += priv->stp;
	} else {
		if ((* priv->cur) > priv->max)
			(* priv->cur) += priv->stp;
	}

	node_progress_bar_update(step);
	node_step_output(step);

	if (priv->min < priv->max) {
		if ((* priv->cur) < priv->max)
			return;
	} else {
		if ((* priv->cur) > priv->max)
			return;
	}

	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(priv->trg_o),
				    priv->payload);
	node_step_blink_trg_out(step);
}

static void node_step_reset(GtkWidget *widget,
			    GByteArray *payload,
			    NodeStep *step)
{
	(* step->priv->cur) = step->priv->min;

	node_progress_bar_update(step);
	node_step_output(step);
}

static void node_step_data_connected(GtkWidget *widget,
				     GtkWidget *source,
				     NodeStep  *step)
{
	/* push last value */
	node_step_output(step);
}


void node_step_clicked(GtkWidget *button, NodeStep *step)
{
	node_step_trigger(NULL, NULL, step);
}

void node_reset_clicked(GtkWidget *button, NodeStep *step)
{
	node_step_reset(NULL, NULL, step);
}

static void node_step_remove(GtkWidget *step, gpointer user_data)
{
	NodeStepPrivate *priv;


	priv = NODE_STEP(step)->priv;

	if (priv->id_trg) {
		g_source_remove(priv->id_trg);
		priv->id_trg = 0;
	}

	if (priv->id_out) {
		g_source_remove(priv->id_out);
		priv->id_out = 0;
	}


	gtk_widget_destroy(step);

	if (priv->payload) {
		g_byte_array_free (priv->payload, TRUE);
		priv->payload = NULL;
	}
}

static gchar *node_step_export_properties(GtkNodesNode *node)
{
	NodeStepPrivate *priv;
	gchar *cfg;

	priv = NODE_STEP(node)->priv;

	cfg = g_strdup_printf("<child internal-child=\"sb_start\">\n"
			      " <object class=\"GtkSpinButton\">\n"
			      "   <property name=\"value\">%g</property>\n"
			      " </object>\n"
			      "</child>\n"
			      "<child internal-child=\"sb_stop\">\n"
			      " <object class=\"GtkSpinButton\">\n"
			      "   <property name=\"value\">%g</property>\n"
			      " </object>\n"
			      "</child>\n"
			      "<child internal-child=\"sb_step\">\n"
			      " <object class=\"GtkSpinButton\">\n"
			      "   <property name=\"value\">%g</property>\n"
			      " </object>\n"
			      "</child>\n",
			      gtk_spin_button_get_value(GTK_SPIN_BUTTON(priv->sb_start)),
			      gtk_spin_button_get_value(GTK_SPIN_BUTTON(priv->sb_stop)),
			      gtk_spin_button_get_value(GTK_SPIN_BUTTON(priv->sb_step)));

	return cfg;
}

static void node_step_class_init(NodeStepClass *klass)
{
	__attribute__((unused))
	GtkWidgetClass *widget_class;
	GtkNodesNodeClass *node_class;


	widget_class = GTK_WIDGET_CLASS(klass);
	node_class = GTKNODES_NODE_CLASS(klass);

	node_class->export_properties = node_step_export_properties;

	/* override widget methods go here if needed */
}

static void node_step_init(NodeStep *step)
{
	GtkWidget *w;
	GtkWidget *grid;
	NodeStepPrivate *priv;


	priv = step->priv = node_step_get_instance_private (step);

	/* our output payload is one double, allocate it here */
	priv->cur = g_malloc0(sizeof(double));
	priv->payload = g_byte_array_new_take((void *) priv->cur, sizeof(double));

	g_signal_connect(G_OBJECT(step), "node-func-clicked",
			 G_CALLBACK(node_step_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (step), "Range Stepper");


	/* input sockets */
	w = gtk_label_new("Trigger");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->trg_i = gtk_nodes_node_item_add(GTKNODES_NODE(step), w,
					      GTKNODES_NODE_SOCKET_SINK);
	g_signal_connect(G_OBJECT(priv->trg_i), "socket-incoming",
			 G_CALLBACK(node_step_trigger), step);

	w = gtk_label_new("Reset");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->rst = gtk_nodes_node_item_add(GTKNODES_NODE(step), w,
					    GTKNODES_NODE_SOCKET_SINK);
	g_signal_connect(G_OBJECT(priv->rst), "socket-incoming",
			 G_CALLBACK(node_step_reset), step);



	/* grid containing user controls */

	grid = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(grid), 6);

	gtk_nodes_node_item_add(GTKNODES_NODE(step), grid,
				GTKNODES_NODE_SOCKET_DISABLE);


	/* output sockets */
	w = gtk_label_new("Output");
	gtk_label_set_xalign(GTK_LABEL(w), 1.0);
	priv->dat_o = gtk_nodes_node_item_add(GTKNODES_NODE(step), w,
					     GTKNODES_NODE_SOCKET_SOURCE);
	gtk_box_set_child_packing(GTK_BOX(step), w, FALSE, FALSE, 0,
				  GTK_PACK_END);
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->dat_o),
				       &COL_DOUBLE);
	gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->dat_o),
				      KEY_DOUBLE);
	g_signal_connect(G_OBJECT(priv->dat_o), "socket-connect",
			 G_CALLBACK(node_step_data_connected), step);


	w = gtk_label_new("Last");
	gtk_label_set_xalign(GTK_LABEL(w), 1.0);
	priv->trg_o = gtk_nodes_node_item_add(GTKNODES_NODE(step), w,
					     GTKNODES_NODE_SOCKET_SOURCE);
	gtk_box_set_child_packing(GTK_BOX(step), w, FALSE, FALSE, 0,
				  GTK_PACK_END);
	/* get original colour */
	gtk_nodes_node_socket_get_rgba(GTKNODES_NODE_SOCKET(priv->trg_o),
				       &priv->rgba_trg_o);




	/* create main controls, they need the socket reference */
	w = gtk_label_new("START");
	gtk_grid_attach(GTK_GRID(grid), w, 0, 0, 1, 1);
	w = gtk_spin_button_new_with_range(NODE_STEP_INTERVAL_MIN,
					   NODE_STEP_INTERVAL_MAX,
					   NODE_STEP_INTERVAL_STP);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(w), TRUE);
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(w), TRUE);
	g_signal_connect(GTK_SPIN_BUTTON(w), "value-changed",
			 G_CALLBACK(node_step_field_changed), &priv->min);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), 0.);
	gtk_grid_attach(GTK_GRID(grid), w, 1, 0, 1, 1);
	priv->sb_start = w;


	w = gtk_label_new("STOP");
	gtk_grid_attach(GTK_GRID(grid), w, 0, 1, 1, 1);
	w = gtk_spin_button_new_with_range(NODE_STEP_INTERVAL_MIN,
					   NODE_STEP_INTERVAL_MAX,
					   NODE_STEP_INTERVAL_STP);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(w), TRUE);
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(w), TRUE);
	g_signal_connect(GTK_SPIN_BUTTON(w), "value-changed",
			 G_CALLBACK(node_step_field_changed), &priv->max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), 360.);
	gtk_grid_attach(GTK_GRID(grid), w, 1, 1, 1, 1);
	priv->sb_stop = w;

	w = gtk_label_new("STEP");
	gtk_grid_attach(GTK_GRID(grid), w, 0, 2, 1, 1);
	w = gtk_spin_button_new_with_range(NODE_STEP_INTERVAL_MIN,
					   NODE_STEP_INTERVAL_MAX,
					   NODE_STEP_INTERVAL_STP);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(w), TRUE);
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(w), TRUE);
	g_signal_connect(GTK_SPIN_BUTTON(w), "value-changed",
			 G_CALLBACK(node_step_field_changed), &priv->stp);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), 0.5);
	gtk_grid_attach(GTK_GRID(grid), w, 1, 2, 1, 1);
	priv->sb_step = w;

	priv->bar = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(priv->bar), TRUE);
	gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(priv->bar), TRUE);
	gtk_grid_attach(GTK_GRID(grid), priv->bar, 0, 3, 2, 1);

	w = gtk_button_new_with_label("Step");
	g_signal_connect(G_OBJECT(w), "clicked",
			 G_CALLBACK(node_step_clicked), step);
	gtk_grid_attach(GTK_GRID(grid), w, 0, 4, 1, 1);

	w = gtk_button_new_with_label("Reset");
	g_signal_connect(G_OBJECT(w), "clicked",
			 G_CALLBACK(node_reset_clicked), step);
	gtk_grid_attach(GTK_GRID(grid), w, 0, 5, 1, 1);

	gtk_widget_show_all(grid);
}


GtkWidget *node_step_new(void)
{
	NodeStep *step;


	step = g_object_new(TYPE_NODE_STEP, NULL);

	return GTK_WIDGET(step);
}

