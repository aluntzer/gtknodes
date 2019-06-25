/**
 * @file    examples/nodes/node_binary_encode.c
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
 * @brief a GtkNode which encodes individual bit inputs to a 8-bit composite
 *	  integer output
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_binary_encode.h>

typedef struct _NodeBinaryEncodeInput		NodeBinaryEncodeInput;

struct _NodeBinaryEncodePrivate {

	GtkWidget  *output;

	gint8      *byte;
	GByteArray *payload;

	GList *inputs;
};

struct _NodeBinaryEncodeInput
{
  NodeBinaryEncode *parent;
  GtkWidget *socket;
  gint idx;		/* the bit index represented */
};


G_DEFINE_TYPE_WITH_PRIVATE(NodeBinaryEncode, node_binary_encode, GTKNODES_TYPE_NODE)



static void node_binary_encode_output(NodeBinaryEncode *binary_encode)
{
	NodeBinaryEncodePrivate *priv;


	priv = binary_encode->priv;

	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(priv->output),
				    priv->payload);
}


static void node_binary_encode_output_connected(GtkWidget *socket,
						GtkWidget *source,
						NodeBinaryEncode *binary_encode)
{
	node_binary_encode_output(binary_encode);
}


static void node_binary_encode_input(GtkWidget             *widget,
				     GByteArray            *payload,
				     NodeBinaryEncodeInput *ip)
{
	guint8 ibyte;
	NodeBinaryEncodePrivate *priv;


	if (!payload)
		return;

	if (!payload->len)
		return;

	priv = ip->parent->priv;

	ibyte = ((guint8 *) payload->data)[0];

	(*priv->byte) &= ~(1 << ip->idx);
	(*priv->byte) |= (ibyte & 0x1) << ip->idx;


	if (ibyte & 0x1)
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(ip->socket),
					       &node_red);
	else
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(ip->socket),
					       &node_yellow);

	node_binary_encode_output(ip->parent);
}


static void node_binary_encode_remove(GtkWidget *binary_encode,
				      gpointer user_data)
{
	NodeBinaryEncodePrivate *priv;


	priv = NODE_BINARY_ENCODE(binary_encode)->priv;

	g_list_free (priv->inputs);
	gtk_widget_destroy(binary_encode);

	if (priv->payload) {
		g_byte_array_free (priv->payload, TRUE);
		priv->payload = NULL;
	}
}


static void node_binary_encode_class_init(NodeBinaryEncodeClass *klass)
{
	__attribute__((unused))
	GtkWidgetClass *widget_class;


	widget_class = GTK_WIDGET_CLASS(klass);

	/* override widget methods go here if needed */
}


static void node_binary_encode_init(NodeBinaryEncode *binary_encode)
{
	int i;
	GtkWidget *w;
	NodeBinaryEncodePrivate *priv;


	priv = binary_encode->priv = node_binary_encode_get_instance_private (binary_encode);

	/* our output payload is constant allocate it here */
	priv->byte = g_malloc0(sizeof(gint8));
	priv->payload = g_byte_array_new_take((void *) priv->byte, sizeof(gint8));


	g_signal_connect(G_OBJECT(binary_encode), "node-func-clicked",
			 G_CALLBACK(node_binary_encode_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (binary_encode),
				 "Binary Encoder");



	/* input sockets */
	for (i = 0; i < 8; i++) {
		gchar *buf;
		NodeBinaryEncodeInput *ip;

		ip = g_new0 (NodeBinaryEncodeInput, 1);

		/* we can transport only one userdata, so we will keep a
		 * reference to our parent in the NodeBinaryEncodeInputs, which
		 * we will pass to the callback. This way we don't have to
		 * iterate the list to find the current socket if we'd pass
		 * the NodeBinaryEncode instead.
		 */
		ip->parent = binary_encode;

		ip->idx = i;
		buf = g_strdup_printf("Bit %d", ip->idx);
		w = gtk_label_new(buf);
		gtk_label_set_xalign(GTK_LABEL(w), 0.0);
		ip->socket = gtk_nodes_node_item_add(GTKNODES_NODE(binary_encode), w,
						     GTKNODES_NODE_SOCKET_SINK);
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(ip->socket),
					       &node_yellow);
		g_signal_connect(G_OBJECT(ip->socket), "socket-incoming",
			 G_CALLBACK(node_binary_encode_input), ip);

		priv->inputs = g_list_append (priv->inputs, ip);
		g_free(buf);
	}


	/* output socket */
	w = gtk_label_new("Output");
	gtk_label_set_xalign(GTK_LABEL(w), 1.0);
	priv->output = gtk_nodes_node_item_add(GTKNODES_NODE(binary_encode), w,
					      GTKNODES_NODE_SOCKET_SOURCE);

	gtk_box_set_child_packing(GTK_BOX(binary_encode), w, FALSE, FALSE, 0,
				  GTK_PACK_END);

	g_signal_connect(G_OBJECT(priv->output), "socket-connect",
			 G_CALLBACK(node_binary_encode_output_connected),
			 binary_encode);


	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->output),
				       &COL_INT8);
	gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->output),
				      KEY_INT8);
}


GtkWidget *node_binary_encode_new(void)
{
	NodeBinaryEncode *binary_encode;


	binary_encode = g_object_new(TYPE_NODE_BINARY_ENCODE, NULL);

	return GTK_WIDGET(binary_encode);
}
