/**
 * @file    examples/nodes/node_show_number.c
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
 * @brief a GtkNode which shows numerical data types as types are identified by
 *        their input key, if a key is unknown, the type will default to int8
 *	  only the first element will be used per incoming socket data call
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_show_number.h>

typedef struct _NodeShowNumberOutput		NodeShowNumberOutput;

enum conv_type {INT8, INT16, INT32, DOUBLE};


struct _NodeShowNumberPrivate {

	GtkWidget  *input;

	GtkWidget *lbl;
	GtkWidget *lbl_i;

	GdkRGBA rgba_i;

	enum conv_type conv_inp;
};


G_DEFINE_TYPE_WITH_PRIVATE(NodeShowNumber, node_show_number, GTKNODES_TYPE_NODE)


static void node_show_number_input(GtkWidget         *widget,
				   GByteArray     *payload,
				   NodeShowNumber *show_number)
{

	gchar *buf = NULL;

	NodeShowNumberPrivate *priv;


	priv = show_number->priv;

	if (!payload)
		return;

	if (!payload->len)
		return;

	switch (priv->conv_inp) {
	case INT8:
		if (payload->len < sizeof(gint8))
			break;

		buf = g_strdup_printf("%d\n[0x%x]",
				      ((gint8 *) payload->data)[0] & 0xff,
				      ((gint8 *) payload->data)[0] & 0xff);
		break;
	case INT16:
		if (payload->len < sizeof(gint16))
			break;

		buf = g_strdup_printf("%d\n[0x%x]",
				      ((gint16 *) payload->data)[0] & 0xffff,
				      ((gint16 *) payload->data)[0] & 0xffff);
		break;
	case INT32:
		if (payload->len < sizeof(gint32))
			break;

		buf = g_strdup_printf("%d\n[0x%x]",
				      ((gint32 *) payload->data)[0],
				      ((gint32 *) payload->data)[0]);
		break;
	case DOUBLE:
		if (payload->len < sizeof(gdouble))
			break;

		buf = g_strdup_printf("%g\n", ((gdouble *) payload->data)[0]);
		break;
	default:
		/* treat as int8 */
		buf = g_strdup_printf("%d\n[0x%x]",
				      ((gint8 *) payload->data)[0] & 0xff,
				      ((gint8 *) payload->data)[0] & 0xff);
		break;
	}


	gtk_label_set_text(GTK_LABEL(priv->lbl), buf);

	g_free(buf);
}


static void node_show_number_input_disconnected(GtkWidget *socket,
						GtkWidget *source,
						NodeShowNumber *show_number)
{
	NodeShowNumberPrivate *priv;


	priv = show_number->priv;


	priv->conv_inp = INT8;
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
				       &priv->rgba_i);
	gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input), 0);
	gtk_label_set_text(GTK_LABEL(priv->lbl_i), "Input");
}


static void node_show_number_input_connected(GtkWidget *socket,
					     GtkWidget *source,
					     NodeShowNumber *show_number)
{
	guint key;
	NodeShowNumberPrivate *priv;


	priv = show_number->priv;

	key = gtk_nodes_node_socket_get_remote_key(GTKNODES_NODE_SOCKET(socket));

	switch (key) {
	case KEY_INT8:
		priv->conv_inp = INT8;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
					       &COL_INT8);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input),
					      KEY_INT8);
		gtk_label_set_text(GTK_LABEL(priv->lbl_i), "int8");
		break;
	case KEY_INT16:
		priv->conv_inp = INT16;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
					       &COL_INT16);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input),
					      KEY_INT16);
		gtk_label_set_text(GTK_LABEL(priv->lbl_i), "int16");
		break;
	case KEY_INT32:
		priv->conv_inp = INT32;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
					       &COL_INT32);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input),
					      KEY_INT32);
		gtk_label_set_text(GTK_LABEL(priv->lbl_i), "int32");
		break;
	case KEY_DOUBLE:
		priv->conv_inp = DOUBLE;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
					       &COL_DOUBLE);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input),
					      KEY_DOUBLE);
		gtk_label_set_text(GTK_LABEL(priv->lbl_i), "double");
		break;
	default:
		/* treat as int8, but set key to 0 (accept all) */
		priv->conv_inp = INT8;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
					       &priv->rgba_i);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input),
					      0);
		gtk_label_set_text(GTK_LABEL(priv->lbl_i), "Input");
		break;
	}

}


static void node_show_number_remove(GtkWidget *show_number,
				    gpointer user_data)
{
	gtk_widget_destroy(show_number);
}


static void node_show_number_class_init(NodeShowNumberClass *klass)
{
	__attribute__((unused))
		GtkWidgetClass *widget_class;


	widget_class = GTK_WIDGET_CLASS(klass);

	/* override widget methods go here if needed */
}


static void node_show_number_init(NodeShowNumber *show_number)
{
	GtkWidget *w;
	NodeShowNumberPrivate *priv;


	priv = show_number->priv = node_show_number_get_instance_private (show_number);


	g_signal_connect(G_OBJECT(show_number), "node-func-clicked",
			 G_CALLBACK(node_show_number_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (show_number),
				 "Number Converter");


	/* input socket */
	w = gtk_label_new("Input");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->input = gtk_nodes_node_item_add(GTKNODES_NODE(show_number), w,
					      GTKNODES_NODE_SOCKET_SINK);
	priv->lbl_i = w;

	g_signal_connect(G_OBJECT(priv->input), "socket-connect",
			 G_CALLBACK(node_show_number_input_connected),
			 show_number);

	g_signal_connect(G_OBJECT(priv->input), "socket-disconnect",
			 G_CALLBACK(node_show_number_input_disconnected),
			 show_number);

	g_signal_connect(G_OBJECT(priv->input), "socket-incoming",
			 G_CALLBACK(node_show_number_input), show_number);

	/* get original RGBA */
	gtk_nodes_node_socket_get_rgba(GTKNODES_NODE_SOCKET(priv->input),
				       &priv->rgba_i);



	priv->lbl = gtk_label_new("");
	gtk_label_set_justify(GTK_LABEL(priv->lbl), GTK_JUSTIFY_CENTER);
	gtk_nodes_node_item_add(GTKNODES_NODE(show_number), priv->lbl,
				GTKNODES_NODE_SOCKET_DISABLE);

}


GtkWidget *node_show_number_new(void)
{
	NodeShowNumber *show_number;


	show_number = g_object_new(TYPE_NODE_SHOW_NUMBER, NULL);

	return GTK_WIDGET(show_number);
}
