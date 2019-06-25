/**
 * @file    examples/nodes/nodes_common.h
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

#ifndef _EXAMPLES_NODES_NODES_COMMON_H_
#define _EXAMPLES_NODES_NODES_COMMON_H_

#include <gtk/gtk.h>

static const GdkRGBA node_red         = {0.635, 0.078, 0.184, 1.0};
static const GdkRGBA node_orange      = {0.851, 0.325, 0.098, 1.0};
static const GdkRGBA node_yellow      = {0.920, 0.670, 0.000, 1.0};
static const GdkRGBA node_green       = {0.467, 0.675, 0.188, 1.0};
static const GdkRGBA node_blue        = {0.000, 0.380, 0.650, 1.0};
static const GdkRGBA node_purple      = {0.494, 0.184, 0.557, 1.0};
static const GdkRGBA node_light_blue  = {0.302, 0.745, 0.933, 1.0};
static const GdkRGBA node_lime_green  = {0.667, 0.863, 0.196, 1.0};
static const GdkRGBA node_dark_purple = {0.267, 0.400, 0.329, 1.0};
static const GdkRGBA node_red_magenta = {0.659, 0.212, 0.333, 1.0};
static const GdkRGBA node_light_red   = {0.984, 0.604, 0.600, 1.0};


struct nodes_point {
	gdouble p0;
	gdouble p1;
};

#define KEY_INT		0x00000001
#define KEY_DOUBLE	0x00000002
#define KEY_POINTS	0x00000003
#define KEY_INT8	0x00000004
#define KEY_INT16	0x00000005
#define KEY_INT32	0x00000006


#define COL_BLINK	node_green
#define COL_DOUBLE	node_light_blue
#define COL_POINTS	node_orange
#define COL_INT8	node_lime_green
#define COL_INT16	node_dark_purple
#define COL_INT32	node_red_magenta


#endif /* _EXAMPLES_NODES_NODES_COMMON_H_ */
