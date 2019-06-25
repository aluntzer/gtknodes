/**
 * @file    examples/nodes/node_step.h
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

#ifndef _EXAMPLES_NODES_NODE_STEP_H_
#define _EXAMPLES_NODES_NODE_STEP_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_STEP			(node_step_get_type())
#define NODE_STEP(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_STEP, NodeStep))
#define NODE_STEP_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_STEP, NOdeStepClass))
#define IS_NODE_STEP(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_STEP))
#define IS_NODE_STEP_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_STEP))
#define NODE_STEP_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_STEP, NodeStepClass))

typedef struct _NodeStep	NodeStep;
typedef struct _NodeStepPrivate	NodeStepPrivate;
typedef struct _NodeStepClass	NodeStepClass;

struct _NodeStep {
	GtkNodesNode parent;
	NodeStepPrivate *priv;
};

struct _NodeStepClass {
	GtkNodesNodeClass parent_class;
};

GType      node_step_get_type (void) G_GNUC_CONST;
GtkWidget* node_step_new      (void);

#endif /* _EXAMPLES_NODES_NODE_STEP_H_ */
