/**
 * @file    examples/nodes/node_bitwise_not.c
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
 * @brief a GtkNode which outputs at C the result of a bitwise NOT to
 *	  the value at gates A. Note that the considered data width
 *	  is restricted the first 8 bits of the input data
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_bitwise_not.h>

typedef struct _NodeBitwiseNotInput	NodeBitwiseNotInput;

struct _NodeBitwiseNotPrivate {

	GtkWidget  *GateA;
	GtkWidget  *GateC;

	gint8 A;
	gint8 *C;

	GByteArray *payload;

	GList *inputs;
};



G_DEFINE_TYPE_WITH_PRIVATE(NodeBitwiseNot, node_bitwise_not, GTKNODES_TYPE_NODE)



static void node_bitwise_not_output(NodeBitwiseNot *bitwise_not)
{
	NodeBitwiseNotPrivate *priv;


	priv = bitwise_not->priv;

	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(priv->GateC),
				    priv->payload);
}


static void node_bitwise_not_output_connected(GtkWidget      *socket,
					      GtkWidget      *source,
					      NodeBitwiseNot *bitwise_not)
{
	node_bitwise_not_output(bitwise_not);
}


static void node_bitwise_not_input(GtkWidget      *widget,
				   GByteArray     *payload,
				   NodeBitwiseNot *bitwise_not)
{
	guint8 ibyte;
	NodeBitwiseNotPrivate *priv;


	if (!payload)
		return;

	if (!payload->len)
		return;


	priv = bitwise_not->priv;

	ibyte = ((guint8 *) payload->data)[0];

		priv->A = ibyte;

	(*priv->C) = ~priv->A;

	if (ibyte)
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(widget),
					       &node_red);
	else
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(widget),
					       &node_yellow);

	if ((*priv->C))
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->GateC),
					       &node_green);
	else
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->GateC),
					       &node_blue);


	node_bitwise_not_output(bitwise_not);
}


static void node_bitwise_not_remove(GtkWidget *bitwise_not,
				    gpointer user_data)
{
	NodeBitwiseNotPrivate *priv;


	priv = NODE_LOGICAL_NOT(bitwise_not)->priv;

	gtk_widget_destroy(bitwise_not);

	if (priv->payload) {
		g_byte_array_free (priv->payload, TRUE);
		priv->payload = NULL;
	}
}


static void node_bitwise_not_class_init(NodeBitwiseNotClass *klass)
{
	__attribute__((unused))
		GtkWidgetClass *widget_class;


	widget_class = GTK_WIDGET_CLASS(klass);

	/* override widget methods go here if needed */
}


static void node_bitwise_not_init(NodeBitwiseNot *bitwise_not)
{
	GtkWidget *w;
	NodeBitwiseNotPrivate *priv;


	priv = bitwise_not->priv = node_bitwise_not_get_instance_private (bitwise_not);

	/* our output payload is constant allocate it here */
	priv->C = g_malloc0(sizeof(gint8));
	priv->payload = g_byte_array_new_take((void *) priv->C, sizeof(gint8));


	g_signal_connect(G_OBJECT(bitwise_not), "node-func-clicked",
			 G_CALLBACK(node_bitwise_not_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (bitwise_not),
				 "Bitwise AND");



	/* input gate A */
	w = gtk_label_new("Gate A");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->GateA = gtk_nodes_node_item_add(GTKNODES_NODE(bitwise_not), w,
					      GTKNODES_NODE_SOCKET_SINK);
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->GateA),
				       &node_yellow);
	g_signal_connect(G_OBJECT(priv->GateA), "socket-incoming",
			 G_CALLBACK(node_bitwise_not_input), bitwise_not);



	/* output gate C */
	w = gtk_label_new("Gate C");
	gtk_label_set_xalign(GTK_LABEL(w), 1.0);
	priv->GateC = gtk_nodes_node_item_add(GTKNODES_NODE(bitwise_not), w,
					      GTKNODES_NODE_SOCKET_SOURCE);

	g_signal_connect(G_OBJECT(priv->GateC), "socket-connect",
			 G_CALLBACK(node_bitwise_not_output_connected),
			 bitwise_not);

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->GateC),
				       &node_blue);

}


GtkWidget *node_bitwise_not_new(void)
{
	NodeBitwiseNot *bitwise_not;


	bitwise_not = g_object_new(TYPE_NODE_LOGICAL_NOT, NULL);

	return GTK_WIDGET(bitwise_not);
}
