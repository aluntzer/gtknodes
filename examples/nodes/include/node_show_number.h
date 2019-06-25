/**
 * @file    examples/nodes/node_show_number.h
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

#ifndef _EXAMPLES_NODES_NODE_SHOW_NUMBER_H_
#define _EXAMPLES_NODES_NODE_SHOW_NUMBER_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_SHOW_NUMBER		(node_show_number_get_type())
#define NODE_SHOW_NUMBER(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_SHOW_NUMBER, NodeShowNumber))
#define NODE_SHOW_NUMBER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_SHOW_NUMBER, NOdeShowNumberClass))
#define IS_NODE_SHOW_NUMBER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_SHOW_NUMBER))
#define IS_NODE_SHOW_NUMBER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_SHOW_NUMBER))
#define NODE_SHOW_NUMBER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_SHOW_NUMBER, NodeShowNumberClass))

typedef struct _NodeShowNumber		NodeShowNumber;
typedef struct _NodeShowNumberPrivate	NodeShowNumberPrivate;
typedef struct _NodeShowNumberClass		NodeShowNumberClass;

struct _NodeShowNumber {
	GtkNodesNode parent;
	NodeShowNumberPrivate *priv;
};

struct _NodeShowNumberClass {
	GtkNodesNodeClass parent_class;
};

GType      node_show_number_get_type (void) G_GNUC_CONST;
GtkWidget* node_show_number_new      (void);

#endif /* _EXAMPLES_NODES_NODE_SHOW_NUMBER_H_ */
