/**
 * @file    examples/nodes/node_pulse.c
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
 * @brief a GtkNode emitting a configurable periodic or one-shot pulse
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_pulse.h>

/* configurable pulse range */
#define NODE_PULSE_INTERVAL_MIN_MS	0
#define NODE_PULSE_INTERVAL_MAX_MS	100000
#define NODE_PULSE_INTERVAL_STP_MS	1

/* colour toggle and continuous ON limit */
#define NODE_PULSE_BLINK_TIMEOUT_MS	50
#define NODE_PULSE_BLINK_LIMIT_MS	(NODE_PULSE_BLINK_TIMEOUT_MS * 2)


struct _NodePulsePrivate {

	GtkWidget  *output;
	GByteArray *payload;

	guint interval_ms;

	GdkRGBA rgba;

	gboolean pulse;
	guint id_to;
	guint id_col;
};


G_DEFINE_TYPE_WITH_PRIVATE(NodePulse, node_pulse, GTKNODES_TYPE_NODE)


static gboolean node_pulse_deactivate_timeout (gpointer data)
{
	NodePulsePrivate *priv;


	priv = NODE_PULSE(data)->priv;

	priv->id_col = 0;
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->output),
				       &priv->rgba);

	return G_SOURCE_REMOVE;
}


static void node_pulse_emit(NodePulse *pulse)
{
	NodePulsePrivate *priv;


	priv = pulse->priv;

	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(priv->output),
				    priv->payload);

	/* do not set if interval is faster than our reset timeout, or periodic
	 * pulses; in this case, the colour has been set by the installer of the
	 * timeout
	 */
	if (priv->id_to)
		if (priv->interval_ms <= NODE_PULSE_BLINK_LIMIT_MS)
			return;

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->output),
				       &COL_BLINK);

	priv->id_col = g_timeout_add(NODE_PULSE_BLINK_TIMEOUT_MS,
				    node_pulse_deactivate_timeout, pulse);
}


static gboolean node_pulse_timeout_cb(void *data)
{
	NodePulsePrivate *priv;


	priv = NODE_PULSE(data)->priv;

	node_pulse_emit(NODE_PULSE(data));

	return priv->pulse;
}


void node_pulse_clicked(GtkWidget *button,  NodePulse *pulse)
{
	node_pulse_emit (pulse);
}


static gboolean node_pulse_toggle_periodic(GtkWidget *w,
					   gboolean state,
					   NodePulse *pulse)
{
	NodePulsePrivate *priv;
	GtkNodesNodeSocket *socket;

	const GSourceFunc sf = node_pulse_timeout_cb;


	priv = pulse->priv;

	socket = GTKNODES_NODE_SOCKET(priv->output);

	if (gtk_switch_get_active(GTK_SWITCH(w)) && !priv->id_to) {

		if (priv->pulse)
			return TRUE;

		priv->pulse = G_SOURCE_CONTINUE;
		priv->id_to = g_timeout_add(priv->interval_ms, sf, pulse);

		/* set permanently "active" for very fast timeout */
		if (priv->interval_ms <= NODE_PULSE_BLINK_LIMIT_MS)
			gtk_nodes_node_socket_set_rgba(socket, &COL_BLINK);
	} else {
		if (priv->id_to) {
			g_source_remove(priv->id_to);
			priv->id_to = 0;
			gtk_nodes_node_socket_set_rgba(socket, &priv->rgba);
		}
		priv->pulse = G_SOURCE_REMOVE;
	}

	return FALSE;
}


static void node_pulse_timeout_changed(GtkWidget *w, NodePulse *pulse)
{
	NodePulsePrivate *priv;
	GtkNodesNodeSocket *socket;

	const GSourceFunc sf = node_pulse_timeout_cb;


	priv = pulse->priv;

	socket = GTKNODES_NODE_SOCKET(priv->output);

	priv->interval_ms = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));

	if (!priv->id_to)
		return;

	g_source_remove(priv->id_to);
	priv->id_to = g_timeout_add(priv->interval_ms, sf, pulse);

	/* set permanently "active" for very fast timeout */
	if (priv->interval_ms <= NODE_PULSE_BLINK_LIMIT_MS)
		gtk_nodes_node_socket_set_rgba(socket, &COL_BLINK);
}


static void node_pulse_remove(GtkWidget *pulse, gpointer user_data)
{
	NodePulsePrivate *priv;


	priv = NODE_PULSE(pulse)->priv;

	if (priv->id_to) {
		g_source_remove(priv->id_to);
		priv->id_to = 0;
	}

	if (priv->id_col) {
		g_source_remove(priv->id_col);
		priv->id_col = 0;
	}

	gtk_widget_destroy(pulse);

	if (priv->payload) {
		g_byte_array_free (priv->payload, TRUE);
		priv->payload = NULL;
	}
}


static void node_pulse_class_init(NodePulseClass *klass)
{
	__attribute__((unused))
	GtkWidgetClass *widget_class;


	widget_class = GTK_WIDGET_CLASS(klass);

	/* override widget methods go here if needed */
}


static void node_pulse_init(NodePulse *pulse)
{
	GtkWidget *w;
	GtkWidget *grid;
	NodePulsePrivate *priv;

	const gchar *msg = "NODE_PULSE";



	priv = pulse->priv = node_pulse_get_instance_private (pulse);

	/* our output payload is constant, allocate it here */
	priv->payload = g_byte_array_new();
	g_byte_array_append (priv->payload, (guchar *) msg, strlen(msg) + 1);

	g_signal_connect(G_OBJECT(pulse), "node-func-clicked",
			 G_CALLBACK(node_pulse_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (pulse), "Pulse Generator");


	/* grid containing user controls */
	grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
	gtk_grid_set_row_spacing(GTK_GRID(grid), 6);

	gtk_nodes_node_item_add(GTKNODES_NODE(pulse), grid,
				GTKNODES_NODE_SOCKET_DISABLE);

	/* output socket */
	w = gtk_label_new("Output");
	gtk_label_set_xalign(GTK_LABEL(w), 1.0);
	priv->output = gtk_nodes_node_item_add(GTKNODES_NODE(pulse), w,
					      GTKNODES_NODE_SOCKET_SOURCE);
	gtk_box_set_child_packing(GTK_BOX(pulse), w, FALSE, FALSE, 0,
				  GTK_PACK_END);
	/* get original colour */
	gtk_nodes_node_socket_get_rgba(GTKNODES_NODE_SOCKET(priv->output),
				       &priv->rgba);


	/* create main controls, they need the socket reference */
	w = gtk_label_new("Continuous");
	gtk_grid_attach(GTK_GRID(grid), w, 0, 0, 1, 1);

	w = gtk_switch_new();
	gtk_widget_set_tooltip_text(w, "Enable/Disable continuous output\n");
	g_signal_connect(G_OBJECT(w), "state-set",
			 G_CALLBACK(node_pulse_toggle_periodic), pulse);
	gtk_grid_attach(GTK_GRID(grid), w, 1, 0, 1, 1);
	gtk_widget_set_hexpand(w, TRUE);
	gtk_widget_set_halign(w, GTK_ALIGN_END);


	w = gtk_label_new("Interval [ms]");
	gtk_grid_attach(GTK_GRID(grid), w, 0, 1, 1, 1);
	w = gtk_spin_button_new_with_range(NODE_PULSE_INTERVAL_MIN_MS,
					   NODE_PULSE_INTERVAL_MAX_MS,
					   NODE_PULSE_INTERVAL_STP_MS);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(w), TRUE);
	gtk_spin_button_set_snap_to_ticks(GTK_SPIN_BUTTON(w), TRUE);
	g_signal_connect(GTK_SPIN_BUTTON(w), "value-changed",
			 G_CALLBACK(node_pulse_timeout_changed), pulse);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), 500);
	gtk_grid_attach(GTK_GRID(grid), w, 1, 1, 1, 1);

	w = gtk_button_new_with_label("Single");
	g_signal_connect(G_OBJECT(w), "clicked",
			 G_CALLBACK(node_pulse_clicked), pulse);
	gtk_grid_attach(GTK_GRID(grid), w, 0, 2, 1, 1);

	gtk_widget_show_all(grid);
}


GtkWidget *node_pulse_new(void)
{
	NodePulse *pulse;


	pulse = g_object_new(TYPE_NODE_PULSE, NULL);

	return GTK_WIDGET(pulse);
}
