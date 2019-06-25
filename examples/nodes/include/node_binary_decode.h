/**
 * @file    examples/nodes/node_binary_decode.h
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

#ifndef _EXAMPLES_NODES_NODE_BINARY_DECODE_H_
#define _EXAMPLES_NODES_NODE_BINARY_DECODE_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_BINARY_DECODE			(node_binary_decode_get_type())
#define NODE_BINARY_DECODE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_BINARY_DECODE, NodeBinaryDecode))
#define NODE_BINARY_DECODE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_BINARY_DECODE, NOdeBinaryDecodeClass))
#define IS_NODE_BINARY_DECODE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_BINARY_DECODE))
#define IS_NODE_BINARY_DECODE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_BINARY_DECODE))
#define NODE_BINARY_DECODE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_BINARY_DECODE, NodeBinaryDecodeClass))

typedef struct _NodeBinaryDecode	NodeBinaryDecode;
typedef struct _NodeBinaryDecodePrivate	NodeBinaryDecodePrivate;
typedef struct _NodeBinaryDecodeClass	NodeBinaryDecodeClass;

struct _NodeBinaryDecode {
	GtkNodesNode parent;
	NodeBinaryDecodePrivate *priv;
};

struct _NodeBinaryDecodeClass {
	GtkNodesNodeClass parent_class;
};

GType      node_binary_decode_get_type (void) G_GNUC_CONST;
GtkWidget* node_binary_decode_new      (void);

#endif /* _EXAMPLES_NODES_NODE_BINARY_DECODE_H_ */
