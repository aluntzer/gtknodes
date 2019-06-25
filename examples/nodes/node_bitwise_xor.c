/**
 * @file    examples/nodes/node_bitwise_xor.c
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
 * @brief a GtkNode which outputs at C the result of a bitwise XOR to
 *	  the values at gates A and B. Note that the considered data width
 *	  is restricted the first 8 bits of the input data
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_bitwise_xor.h>

typedef struct _NodeBitwiseXorInput	NodeBitwiseXorInput;

struct _NodeBitwiseXorPrivate {

	GtkWidget  *GateA;
	GtkWidget  *GateB;
	GtkWidget  *GateC;

	gint8 A;
	gint8 B;
	gint8 *C;

	GByteArray *payload;

	GList *inputs;
};



G_DEFINE_TYPE_WITH_PRIVATE(NodeBitwiseXor, node_bitwise_xor, GTKNODES_TYPE_NODE)



static void node_bitwise_xor_output(NodeBitwiseXor *bitwise_xor)
{
	NodeBitwiseXorPrivate *priv;


	priv = bitwise_xor->priv;

	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(priv->GateC),
				    priv->payload);
}


static void node_bitwise_xor_output_connected(GtkWidget      *socket,
					      GtkWidget      *source,
					      NodeBitwiseXor *bitwise_xor)
{
	node_bitwise_xor_output(bitwise_xor);
}


static void node_bitwise_xor_input(GtkWidget      *widget,
				   GByteArray     *payload,
				   NodeBitwiseXor *bitwise_xor)
{
	guint8 ibyte;
	NodeBitwiseXorPrivate *priv;


	if (!payload)
		return;

	if (!payload->len)
		return;


	priv = bitwise_xor->priv;

	ibyte = ((guint8 *) payload->data)[0];

	if (widget == priv->GateA)
		priv->A = ibyte;

	if (widget == priv->GateB)
		priv->B = ibyte;

	(*priv->C) = priv->A ^ priv->B;

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


	node_bitwise_xor_output(bitwise_xor);
}


static void node_bitwise_xor_remove(GtkWidget *bitwise_xor,
				    gpointer user_data)
{
	NodeBitwiseXorPrivate *priv;


	priv = NODE_LOGICAL_XOR(bitwise_xor)->priv;

	gtk_widget_destroy(bitwise_xor);

	if (priv->payload) {
		g_byte_array_free (priv->payload, TRUE);
		priv->payload = NULL;
	}
}


static void node_bitwise_xor_class_init(NodeBitwiseXorClass *klass)
{
	__attribute__((unused))
		GtkWidgetClass *widget_class;


	widget_class = GTK_WIDGET_CLASS(klass);

	/* override widget methods go here if needed */
}


static void node_bitwise_xor_init(NodeBitwiseXor *bitwise_xor)
{
	GtkWidget *w;
	NodeBitwiseXorPrivate *priv;


	priv = bitwise_xor->priv = node_bitwise_xor_get_instance_private (bitwise_xor);

	/* our output payload is constant allocate it here */
	priv->C = g_malloc0(sizeof(gint8));
	priv->payload = g_byte_array_new_take((void *) priv->C, sizeof(gint8));


	g_signal_connect(G_OBJECT(bitwise_xor), "node-func-clicked",
			 G_CALLBACK(node_bitwise_xor_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (bitwise_xor),
				 "Bitwise XOR");



	/* input gate A */
	w = gtk_label_new("Gate A");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->GateA = gtk_nodes_node_item_add(GTKNODES_NODE(bitwise_xor), w,
					      GTKNODES_NODE_SOCKET_SINK);
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->GateA),
				       &node_yellow);
	g_signal_connect(G_OBJECT(priv->GateA), "socket-incoming",
			 G_CALLBACK(node_bitwise_xor_input), bitwise_xor);



	/* output gate C */
	w = gtk_label_new("Gate C");
	gtk_label_set_xalign(GTK_LABEL(w), 1.0);
	priv->GateC = gtk_nodes_node_item_add(GTKNODES_NODE(bitwise_xor), w,
					      GTKNODES_NODE_SOCKET_SOURCE);

	g_signal_connect(G_OBJECT(priv->GateC), "socket-connect",
			 G_CALLBACK(node_bitwise_xor_output_connected),
			 bitwise_xor);

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->GateC),
				       &node_blue);


	/* input gate B */
	w = gtk_label_new("Gate B");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->GateB = gtk_nodes_node_item_add(GTKNODES_NODE(bitwise_xor), w,
					      GTKNODES_NODE_SOCKET_SINK);
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->GateB),
				       &node_yellow);
	gtk_box_set_child_packing(GTK_BOX(bitwise_xor), w, FALSE, FALSE, 0,
				  GTK_PACK_END);
	g_signal_connect(G_OBJECT(priv->GateB), "socket-incoming",
			 G_CALLBACK(node_bitwise_xor_input), bitwise_xor);
}


GtkWidget *node_bitwise_xor_new(void)
{
	NodeBitwiseXor *bitwise_xor;


	bitwise_xor = g_object_new(TYPE_NODE_LOGICAL_XOR, NULL);

	return GTK_WIDGET(bitwise_xor);
}
