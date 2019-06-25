/**
 * @file    examples/nodes/node_pulse.h
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

#ifndef _EXAMPLES_NODES_NODE_PULSE_H_
#define _EXAMPLES_NODES_NODE_PULSE_H_

#include <gtk/gtk.h>
#include <gtknode.h>

#define TYPE_NODE_PULSE			(node_pulse_get_type())
#define NODE_PULSE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_NODE_PULSE, NodePulse))
#define NODE_PULSE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_NODE_PULSE, NodePulseClass))
#define IS_NODE_PULSE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_NODE_PULSE))
#define IS_NODE_PULSE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_NODE_PULSE))
#define NODE_PULSE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_NODE_PULSE, NodePulseClass))

typedef struct _NodePulse		NodePulse;
typedef struct _NodePulsePrivate	NodePulsePrivate;
typedef struct _NodePulseClass		NodePulseClass;

struct _NodePulse {
	GtkNodesNode parent;
	NodePulsePrivate *priv;
};

struct _NodePulseClass {
	GtkNodesNodeClass parent_class;
};

struct _StepClass {
	GtkNodesNodeClass parent_class;
};

GType      node_pulse_get_type (void) G_GNUC_CONST;
GtkWidget* node_pulse_new      (void);


#endif /* _EXAMPLES_NODES_NODES_COMMON_H_ */
