/**
 * @file    examples/nodes/node_bitwise_xor.h
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

#ifndef _EXAMPLES_NODES_NODE_LOGICAL_XOR_H_
#define _EXAMPLES_NODES_NODE_LOGICAL_XOR_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_LOGICAL_XOR			(node_bitwise_xor_get_type())
#define NODE_LOGICAL_XOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_LOGICAL_XOR, NodeBitwiseXor))
#define NODE_LOGICAL_XOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_LOGICAL_XOR, NOdeBitwiseXorClass))
#define IS_NODE_LOGICAL_XOR(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_LOGICAL_XOR))
#define IS_NODE_LOGICAL_XOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_LOGICAL_XOR))
#define NODE_LOGICAL_XOR_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_LOGICAL_XOR, NodeBitwiseXorClass))

typedef struct _NodeBitwiseXor	NodeBitwiseXor;
typedef struct _NodeBitwiseXorPrivate	NodeBitwiseXorPrivate;
typedef struct _NodeBitwiseXorClass	NodeBitwiseXorClass;

struct _NodeBitwiseXor {
	GtkNodesNode parent;
	NodeBitwiseXorPrivate *priv;
};

struct _NodeBitwiseXorClass {
	GtkNodesNodeClass parent_class;
};

GType      node_bitwise_xor_get_type (void) G_GNUC_CONST;
GtkWidget* node_bitwise_xor_new      (void);

#endif /* _EXAMPLES_NODES_NODE_LOGICAL_XOR_H_ */

