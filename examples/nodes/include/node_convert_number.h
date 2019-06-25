/**
 * @file    examples/nodes/node_convert_number.h
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

#ifndef _EXAMPLES_NODES_NODE_CONVERT_NUMBER_H_
#define _EXAMPLES_NODES_NODE_CONVERT_NUMBER_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_CONVERT_NUMBER		(node_convert_number_get_type())
#define NODE_CONVERT_NUMBER(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_CONVERT_NUMBER, NodeConvertNumber))
#define NODE_CONVERT_NUMBER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_CONVERT_NUMBER, NOdeConvertNumberClass))
#define IS_NODE_CONVERT_NUMBER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_CONVERT_NUMBER))
#define IS_NODE_CONVERT_NUMBER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_CONVERT_NUMBER))
#define NODE_CONVERT_NUMBER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_CONVERT_NUMBER, NodeConvertNumberClass))

typedef struct _NodeConvertNumber		NodeConvertNumber;
typedef struct _NodeConvertNumberPrivate	NodeConvertNumberPrivate;
typedef struct _NodeConvertNumberClass		NodeConvertNumberClass;

struct _NodeConvertNumber {
	GtkNodesNode parent;
	NodeConvertNumberPrivate *priv;
};

struct _NodeConvertNumberClass {
	GtkNodesNodeClass parent_class;
};

GType      node_convert_number_get_type (void) G_GNUC_CONST;
GtkWidget* node_convert_number_new      (void);

#endif /* _EXAMPLES_NODES_NODE_CONVERT_NUMBER_H_ */
