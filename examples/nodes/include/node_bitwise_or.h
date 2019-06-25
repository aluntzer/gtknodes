/**
 * @file    examples/nodes/node_bitwise_or.h
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

#ifndef _EXAMPLES_NODES_NODE_LOGICAL_OR_H_
#define _EXAMPLES_NODES_NODE_LOGICAL_OR_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_LOGICAL_OR			(node_bitwise_or_get_type())
#define NODE_LOGICAL_OR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_LOGICAL_OR, NodeBitwiseOr))
#define NODE_LOGICAL_OR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_LOGICAL_OR, NOdeBitwiseOrClass))
#define IS_NODE_LOGICAL_OR(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_LOGICAL_OR))
#define IS_NODE_LOGICAL_OR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_LOGICAL_OR))
#define NODE_LOGICAL_OR_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_LOGICAL_OR, NodeBitwiseOrClass))

typedef struct _NodeBitwiseOr	NodeBitwiseOr;
typedef struct _NodeBitwiseOrPrivate	NodeBitwiseOrPrivate;
typedef struct _NodeBitwiseOrClass	NodeBitwiseOrClass;

struct _NodeBitwiseOr {
	GtkNodesNode parent;
	NodeBitwiseOrPrivate *priv;
};

struct _NodeBitwiseOrClass {
	GtkNodesNodeClass parent_class;
};

GType      node_bitwise_or_get_type (void) G_GNUC_CONST;
GtkWidget* node_bitwise_or_new      (void);

#endif /* _EXAMPLES_NODES_NODE_LOGICAL_OR_H_ */

