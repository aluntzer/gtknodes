/**
 * @file    examples/nodes/node_convert_number.c
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
 * @brief a GtkNode which converts between numerical data types
 *	  input types are identified by their key, if a key is unknown, the
 *	  type will default to int8
 *	  only the first element will be used per incoming socket data call,
 *	  this is not an array converter
 */

#include <gtk/gtk.h>
#include <gtknode.h>
#include <gtknodesocket.h>

#include <nodes_common.h>
#include <node_convert_number.h>

typedef struct _NodeConvertNumberOutput		NodeConvertNumberOutput;

enum conv_type {INT8, INT16, INT32, DOUBLE};


struct _NodeConvertNumberPrivate {

	GtkWidget  *input;
	GtkWidget *output;

	GtkWidget *lbl_i;
	GtkWidget *lbl_o;

	GdkRGBA rgba_i;

	enum conv_type conv_inp;
	enum conv_type conv_out;

	GByteArray *payload;
};


G_DEFINE_TYPE_WITH_PRIVATE(NodeConvertNumber, node_convert_number, GTKNODES_TYPE_NODE)


static void node_convert_number_output(NodeConvertNumber *convert_number)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;

	gtk_nodes_node_socket_write(GTKNODES_NODE_SOCKET(priv->output),
				    priv->payload);
}

static void node_convert_number_output_connected(GtkWidget         *socket,
						 GtkWidget         *source,
						 NodeConvertNumber *convert_number)
{

	node_convert_number_output(convert_number);
}

static void node_convert_int8_to_output (NodeConvertNumber *convert_number,
					 const guint8      *ptr)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;


	priv->payload->len = sizeof(gint8);

	switch (priv->conv_out) {
	case INT8:
		((gint8 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT16:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT32:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case DOUBLE:
		((double *) priv->payload->data)[0] = ptr[0];
		break;
	default:
		break;

	}
}


static void node_convert_int16_to_output (NodeConvertNumber *convert_number,
					  const guint16      *ptr)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;


	priv->payload->len = sizeof(gint16);

	switch (priv->conv_out) {
	case INT8:
		((gint8 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT16:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT32:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case DOUBLE:
		((double *) priv->payload->data)[0] = ptr[0];
		break;
	default:
		break;

	}
}


static void node_convert_int32_to_output (NodeConvertNumber *convert_number,
					  const guint32      *ptr)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;


	priv->payload->len = sizeof(gint32);

	switch (priv->conv_out) {
	case INT8:
		((gint8 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT16:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT32:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case DOUBLE:
		((double *) priv->payload->data)[0] = ptr[0];
		break;
	default:
		break;

	}
}


static void node_convert_double_to_output (NodeConvertNumber *convert_number,
					   const gdouble      *ptr)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;


	priv->payload->len = sizeof(gdouble);

	switch (priv->conv_out) {
	case INT8:
		((gint8 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT16:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case INT32:
		((gint16 *) priv->payload->data)[0] = ptr[0];
		break;
	case DOUBLE:
		((double *) priv->payload->data)[0] = ptr[0];
		break;
	default:
		break;

	}
}

static void node_convert_number_input(GtkWidget         *widget,
				      GByteArray        *payload,
				      NodeConvertNumber *convert_number)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;

	if (!payload)
		return;

	if (!payload->len)
		return;

	switch (priv->conv_inp) {
	case INT8:
		if (payload->len < sizeof(gint8))
			break;
		node_convert_int8_to_output(convert_number,
					    (guint8 *) payload->data);
		break;
	case INT16:
		if (payload->len < sizeof(gint16))
			break;
		node_convert_int16_to_output(convert_number,
					     (guint16 *) payload->data);
		break;
	case INT32:
		if (payload->len < sizeof(gint32))
			break;
		node_convert_int32_to_output(convert_number,
					     (guint32 *) payload->data);
		break;
	case DOUBLE:
		if (payload->len < sizeof(gdouble))
			break;
		node_convert_double_to_output(convert_number,
					      (gdouble *) payload->data);
		break;
	default:
		break;
	}


	node_convert_number_output(convert_number);
}


static void node_convert_number_input_disconnected(GtkWidget *socket,
						   GtkWidget *source,
						   NodeConvertNumber *convert_number)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;


	priv->conv_inp = INT8;
	gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->input),
				       &priv->rgba_i);
	gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->input), 0);
	gtk_label_set_text(GTK_LABEL(priv->lbl_i), "Input");
}


static void node_convert_number_input_connected(GtkWidget *socket,
						GtkWidget *source,
						NodeConvertNumber *convert_number)
{
	guint key;
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;

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


static void node_convert_number_output_changed(GtkComboBox       *cb,
					       NodeConvertNumber *convert_number)
{
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv;

	switch (gtk_combo_box_get_active(cb)) {
	case 0:
		gtk_label_set_text(GTK_LABEL(priv->lbl_o), "int8");
		priv->conv_out = INT8;

		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->output),
					       &COL_INT8);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->output),
					      KEY_INT8);
		break;
	case 1:
		gtk_label_set_text(GTK_LABEL(priv->lbl_o), "int16");
		priv->conv_out = INT16;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->output),
					       &COL_INT16);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->output),
					      KEY_INT16);
		break;
	case 2:
		gtk_label_set_text(GTK_LABEL(priv->lbl_o), "int32");
		priv->conv_out = INT16;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->output),
					       &COL_INT32);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->output),
					      KEY_INT32);
		break;
	case 3:
		gtk_label_set_text(GTK_LABEL(priv->lbl_o), "double");
		priv->conv_out = DOUBLE;
		gtk_nodes_node_socket_set_rgba(GTKNODES_NODE_SOCKET(priv->output),
					       &COL_DOUBLE);
		gtk_nodes_node_socket_set_key(GTKNODES_NODE_SOCKET(priv->output),
					      KEY_DOUBLE);
		break;

	default:
		break;
	}
}


static void node_convert_number_remove(GtkWidget *convert_number,
				       gpointer user_data)
{
	NodeConvertNumberPrivate *priv;


	priv = NODE_CONVERT_NUMBER(convert_number)->priv;

	gtk_widget_destroy(convert_number);

	if (priv->payload) {
		g_byte_array_free (priv->payload, TRUE);
		priv->payload = NULL;
	}
}


static void node_convert_number_class_init(NodeConvertNumberClass *klass)
{
	__attribute__((unused))
		GtkWidgetClass *widget_class;


	widget_class = GTK_WIDGET_CLASS(klass);

	/* override widget methods go here if needed */
}


static void node_convert_number_init(NodeConvertNumber *convert_number)
{
	GtkWidget *w;
	GtkWidget *cb;
	NodeConvertNumberPrivate *priv;


	priv = convert_number->priv = node_convert_number_get_instance_private (convert_number);


	/* we do no reallocation, as we don't consider arrays and hence
	 * reserve the required space only once, blindly assuming that the
	 * double type can hold all others
	 */
	priv->payload = g_byte_array_sized_new(sizeof(double));

	g_signal_connect(G_OBJECT(convert_number), "node-func-clicked",
			 G_CALLBACK(node_convert_number_remove), NULL);

	gtk_nodes_node_set_label(GTKNODES_NODE (convert_number),
				 "Number Converter");


	/* input socket */
	w = gtk_label_new("Input");
	gtk_label_set_xalign(GTK_LABEL(w), 0.0);
	priv->input = gtk_nodes_node_item_add(GTKNODES_NODE(convert_number), w,
					      GTKNODES_NODE_SOCKET_SINK);
	priv->lbl_i = w;

	g_signal_connect(G_OBJECT(priv->input), "socket-connect",
			 G_CALLBACK(node_convert_number_input_connected),
			 convert_number);

	g_signal_connect(G_OBJECT(priv->input), "socket-disconnect",
			 G_CALLBACK(node_convert_number_input_disconnected),
			 convert_number);

	g_signal_connect(G_OBJECT(priv->input), "socket-incoming",
			 G_CALLBACK(node_convert_number_input), convert_number);

	/* get original RGBA */
	gtk_nodes_node_socket_get_rgba(GTKNODES_NODE_SOCKET(priv->input),
				       &priv->rgba_i);


	/* style selector */
	cb = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cb), NULL, "INT8");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cb), NULL, "INT16");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cb), NULL, "INT32");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cb), NULL, "DOUBLE");
	gtk_nodes_node_item_add(GTKNODES_NODE(convert_number), cb,
				GTKNODES_NODE_SOCKET_DISABLE);


	/* output socket */
	w = gtk_label_new("Output");
	gtk_label_set_xalign(GTK_LABEL(w), 1.0);
	priv->output = gtk_nodes_node_item_add(GTKNODES_NODE(convert_number), w,
					       GTKNODES_NODE_SOCKET_SOURCE);
	priv->lbl_o = w;

	g_signal_connect(G_OBJECT(priv->output), "socket-connect",
			 G_CALLBACK(node_convert_number_output_connected),
			 convert_number);


	/* the output must have be created before we can set anything */
	g_signal_connect(GTK_COMBO_BOX(cb), "changed",
			 G_CALLBACK(node_convert_number_output_changed),
			 convert_number);
	gtk_combo_box_set_active(GTK_COMBO_BOX(cb), 0);	/* default int8 */
}


GtkWidget *node_convert_number_new(void)
{
	NodeConvertNumber *convert_number;


	convert_number = g_object_new(TYPE_NODE_CONVERT_NUMBER, NULL);

	return GTK_WIDGET(convert_number);
}
