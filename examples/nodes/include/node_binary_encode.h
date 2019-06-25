/**
 * @file    examples/nodes/node_binary_encode.h
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
 */

#ifndef _EXAMPLES_NODES_NODE_BINARY_ENCODE_H_
#define _EXAMPLES_NODES_NODE_BINARY_ENCODE_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_BINARY_ENCODE			(node_binary_encode_get_type())
#define NODE_BINARY_ENCODE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_BINARY_ENCODE, NodeBinaryEncode))
#define NODE_BINARY_ENCODE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_BINARY_ENCODE, NOdeBinaryEncodeClass))
#define IS_NODE_BINARY_ENCODE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_BINARY_ENCODE))
#define IS_NODE_BINARY_ENCODE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_BINARY_ENCODE))
#define NODE_BINARY_ENCODE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_BINARY_ENCODE, NodeBinaryEncodeClass))

typedef struct _NodeBinaryEncode	NodeBinaryEncode;
typedef struct _NodeBinaryEncodePrivate	NodeBinaryEncodePrivate;
typedef struct _NodeBinaryEncodeClass	NodeBinaryEncodeClass;

struct _NodeBinaryEncode {
	GtkNodesNode parent;
	NodeBinaryEncodePrivate *priv;
};

struct _NodeBinaryEncodeClass {
	GtkNodesNodeClass parent_class;
};

GType      node_binary_encode_get_type (void) G_GNUC_CONST;
GtkWidget* node_binary_encode_new      (void);

#endif /* _EXAMPLES_NODES_NODE_BINARY_ENCODE_H_ */

