/**
 * @file    examples/nodes/node_bitwise_and.h
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

#ifndef _EXAMPLES_NODES_NODE_LOGICAL_AND_H_
#define _EXAMPLES_NODES_NODE_LOGICAL_AND_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_LOGICAL_AND			(node_bitwise_and_get_type())
#define NODE_LOGICAL_AND(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_LOGICAL_AND, NodeBitwiseAnd))
#define NODE_LOGICAL_AND_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_LOGICAL_AND, NOdeBitwiseAndClass))
#define IS_NODE_LOGICAL_AND(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_LOGICAL_AND))
#define IS_NODE_LOGICAL_AND_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_LOGICAL_AND))
#define NODE_LOGICAL_AND_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_LOGICAL_AND, NodeBitwiseAndClass))

typedef struct _NodeBitwiseAnd	NodeBitwiseAnd;
typedef struct _NodeBitwiseAndPrivate	NodeBitwiseAndPrivate;
typedef struct _NodeBitwiseAndClass	NodeBitwiseAndClass;

struct _NodeBitwiseAnd {
	GtkNodesNode parent;
	NodeBitwiseAndPrivate *priv;
};

struct _NodeBitwiseAndClass {
	GtkNodesNodeClass parent_class;
};

GType      node_bitwise_and_get_type (void) G_GNUC_CONST;
GtkWidget* node_bitwise_and_new      (void);

#endif /* _EXAMPLES_NODES_NODE_LOGICAL_AND_H_ */

