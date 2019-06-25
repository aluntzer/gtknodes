/**
 * @file    examples/nodes/node_binary_decode.c
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
 * @brief a GtkNode which decodes an 8-bit composite integer input to
 *        individial bit outputs
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_binary_decode.h>

typedef struct _NodeBinaryDecodeOutput		NodeBinaryDecodeOutput;

struct _NodeBinaryDecodePrivate {

	GtkWidget  *input;

	GByteArray *payload0;
	GByteArray *payload1;

	GList *outputs;
};

struct _NodeBinaryDecodeOutput
{
  GtkWidget *socket;
  gint idx;		/* the bit index represented */

  gboolean state;
};


G_DEFINE_TYPE_WITH_PRIVATE(NodeBinaryDecode, node_binary_decode, GTKNODES_TYPE_NODE)



static void node_binary_decode_output_bit(NodeBinaryDecode       *binary_decode,
					  NodeBinaryDecodeOutput *op)
{
	GByteArray *pl;
	const GdkRGBA *rgba;
	NodeBinaryDecodePrivate *priv;


	priv = binary_decode->priv;

	if (op->state) {
		pl = priv->payload1;
		rgba = &node_green;
	} else {
		pl = priv->payload0;
		rgba = &node_blue;
	}

	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(op->socket), pl);
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(op->socket), rgba);
}


static void node_binary_decode_output_connected(GtkWidget *socket,
						GtkWidget *source,
						NodeBinaryDecode *binary_decode)
{
	GList *l;
	NodeBinaryDecodePrivate *priv;


	priv = binary_decode->priv;

	l = priv->outputs;

	while (l) {
		NodeBinaryDecodeOutput *op = l->data;
		l = l->next;

		if (op->socket != socket)
			continue;

		node_binary_decode_output_bit(binary_decode, op);

		break;
	}
}


static void node_binary_decode_output(NodeBinaryDecode *binary_decode)
{
	GList *l;
	NodeBinaryDecodePrivate *priv;


	priv = binary_decode->priv;


	l = priv->outputs;

	while (l) {
		NodeBinaryDecodeOutput *op = l->data;
		l = l->next;

		node_binary_decode_output_bit(binary_decode, op);
	}
}


static void node_binary_decode_input(GtkWidget        *widget,
				     GByteArray       *payload,
				     NodeBinaryDecode *binary_decode)
{
	GList *l;
	guint8 byte;
	NodeBinaryDecodePrivate *priv;


	priv = binary_decode->priv;

	if (!payload)
		return;

	if (!payload->len)
		return;

	byte = ((guint8 *) payload->data)[0];

	l = priv->outputs;

	while (l) {
		NodeBinaryDecodeOutput *op = l->data;
		l = l->next;

		op->state = (byte >> op->idx) & 0x1;
	}

	node_binary_decode_output(binary_decode);
}


static void node_binary_decode_remove(GtkWidget *binary_decode,
				      gpointer user_data)
{
	NodeBinaryDecodePrivate *priv;


	priv = NODE_BINARY_DECODE(binary_decode)->priv;

	g_list_free (priv->outputs);
	gtk_widget_destroy(binary_decode);

	if (priv->payload0) {
		g_byte_array_free (priv->payload0, TRUE);
		priv->payload0 = NULL;
	}

	if (priv->payload1) {
		g_byte_array_free (priv->payload1, TRUE);
		priv->payload1 = NULL;
	}

}


static void node_binary_decode_class_init(NodeBinaryDecodeClass *klass)
{
	__attribute__((unused))
	GtkWidgetClass *widget_class;


	widget_class = GTK_WIDGET_CLASS(klass);

	/* override widget methods go here if needed */
}


static void node_binary_decode_init(NodeBinaryDecode *binary_decode)
{
	int i;
	GtkWidget *w;
	NodeBinaryDecodePrivate *priv;

	const gchar *out0 = "\0";
	const gchar *out1 = "\1";


	priv = binary_decode->priv = node_binary_decode_get_instance_private (binary_decode);

	/* our output payloads are constant, and 0 or != 0, allocate them here */
	priv->payload0 = g_byte_array_new();
	g_byte_array_append (priv->payload0, (guchar *) out0, strlen(out0) + 1);

	priv->payload1 = g_byte_array_new();
	g_byte_array_append (priv->payload1, (guchar *) out1, strlen(out1) + 1);

	g_signal_connect(G_OBJECT(binary_decode), "node-func-clicked",
			 G_CALLBACK(node_binary_decode_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (binary_decode),
				 "Binary Decoder");


	/* input socket */
	w = gtk_label_new("Input");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->input = gtk_nodes_node_item_add(GTKNODES_NODE(binary_decode), w,
					      GTKNODES_NODE_SOCKET_SINK);
	g_signal_connect(G_OBJECT(priv->input), "socket-incoming",
			 G_CALLBACK(node_binary_decode_input), binary_decode);

	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
				       &COL_INT8);
	gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input),
				      KEY_INT8);


	/* output sockets */

	for (i = 7; i >= 0; i--) {
		gchar *buf;
		NodeBinaryDecodeOutput *op;

		op = g_new0 (NodeBinaryDecodeOutput, 1);

		op->idx = i;
		buf = g_strdup_printf("Bit %d", op->idx);
		w = gtk_label_new(buf);
		gtk_label_set_xalign(GTK_LABEL(w), 1.0);
		op->socket = gtk_nodes_node_item_add(GTKNODES_NODE(binary_decode), w,
						     GTKNODES_NODE_SOCKET_SOURCE);
		gtk_box_set_child_packing(GTK_BOX(binary_decode), w, FALSE, FALSE, 0,
					  GTK_PACK_END);
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(op->socket),
					       &node_blue);

		g_signal_connect(G_OBJECT(op->socket), "socket-connect",
				 G_CALLBACK(node_binary_decode_output_connected),
				 binary_decode);

		priv->outputs = g_list_append (priv->outputs, op);
		g_free(buf);
	}
}


GtkWidget *node_binary_decode_new(void)
{
	NodeBinaryDecode *binary_decode;


	binary_decode = g_object_new(TYPE_NODE_BINARY_DECODE, NULL);

	return GTK_WIDGET(binary_decode);
}
