#!/usr/bin/python3
# need gtk, gtknodes, numpy, scipy, pillow, matplotlib
# Note: this was quickly hacked together in an hour or so.
#        As a consequence, stuff may not always work perfectly (i.e. ImageOp
#        on differently shaped input images etc.
#

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('GtkNodes', '0.1')

from gi.repository import GLib
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GtkNodes

import struct
import sys


import numpy as np
from scipy import interpolate
from matplotlib.backends.backend_gtk3agg import (
        FigureCanvasGTK3Agg as FigureCanvas)
from matplotlib.figure import Figure

from scipy import misc
from scipy import ndimage
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import pickle



def get_rgba(name):
    # I recommend using only valid color names ;)
    rgba = Gdk.RGBA()
    rgba.parse(mcolors.to_hex(name))
    return rgba



class ImgSrcNode(GtkNodes.Node):
    __gtype_name__ = 'SrcNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Source")
        self.connect("node_func_clicked", self.remove)


        self.img = misc.face()

        f = Figure(figsize=(5, 4), dpi=100)
        self.a = f.add_subplot(111)

        sw = Gtk.ScrolledWindow()
        sw.set_border_width(10)

        self.canvas = FigureCanvas(f)
        sw.add(self.canvas)
        sw.set_size_request(200, 200)

        self.item_add(sw, GtkNodes.NodeSocketIO.DISABLE)
        self.set_child_packing(sw, True, True, 0, Gtk.PackType.START)

        self.draw_image()

        # create local node settings
        sources = ["face", "ascent"]
        self.combobox = Gtk.ComboBoxText()
        self.combobox.set_entry_text_column(0)
        for src in sources:
            self.combobox.append_text(src)
        self.combobox.set_active(0)
        self.combobox.connect("changed", self.change_image)

        # internal node items are type DISABLE
        self.item_add(self.combobox, GtkNodes.NodeSocketIO.DISABLE)

        # create node output socket
        lbl = Gtk.Label.new("Image")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        # the compatibility key
        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))

    def draw_image(self):

        self.a.imshow(self.img)
        self.canvas.draw()

    def change_image(self, combobox):
        src = self.combobox.get_active_text()

        if src == "face":
            self.img = misc.face()
        elif src == "ascent":
            self.img = misc.ascent()

        self.draw_image()
        self.node_socket_output.write(pickle.dumps(self.img))


    def node_socket_connect(self, sink, source):
        self.node_socket_output.write(pickle.dumps(self.img))

    def remove(self, node):
        self.destroy()


class ImgPltNode(GtkNodes.Node):
    __gtype_name__ = 'PltNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Plot")
        self.connect("node_func_clicked", self.remove)

        # create input socket
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming)

        # the compatibility key
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        f = Figure(figsize=(5, 4), dpi=100)
        self.a = f.add_subplot(111)

        sw = Gtk.ScrolledWindow()
        sw.set_border_width(10)

        self.canvas = FigureCanvas(f)
        sw.add(self.canvas)
        sw.set_size_request(200, 200)

        self.item_add(sw, GtkNodes.NodeSocketIO.DISABLE)
        self.set_child_packing(sw, True, True, 0, Gtk.PackType.END)


    def node_socket_incoming(self, socket, payload):

        img = pickle.loads(payload)

        self.a.imshow(img)
        self.canvas.draw()

    def remove(self, node):
        self.destroy()


class ImgMaskNode(GtkNodes.Node):
    __gtype_name__ = 'MaskNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Mask")
        self.connect("node_func_clicked", self.remove)

        # create input socket
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming)

        # the compatibility key
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        # radius control
        adjustment = Gtk.Adjustment(value = 500, lower = 0, upper = 1000,
                step_increment = 10, page_increment = 50,
                page_size = 0)

        self.spinbutton = Gtk.SpinButton()
        self.spinbutton.set_adjustment(adjustment)
        self.spinbutton.set_size_request(50, 20)
        self.spinbutton.connect("value_changed", self.do_value_changed)

        self.item_add(self.spinbutton, GtkNodes.NodeSocketIO.DISABLE)

        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))


    def circular_mask(self, w, h, r):
        x, y = np.ogrid[:w, :h]
        center = [int(w / 2), int(h / 2)]
        d = np.sqrt((x - center[0])**2 + (y - center[1])**2)
        mask = d <= r
        return mask


    def do_value_changed(self, widget):
        r = self.spinbutton.get_value()
        w, h = self.img.shape[:2]
        mask = self.circular_mask(w, h, r)
        self.masked_img = self.img.copy()
        self.masked_img[~mask] = 0
        self.node_socket_output.write(pickle.dumps(self.masked_img))

    def node_socket_connect(self, sink, source):
        self.node_socket_output.write(pickle.dumps(self.masked_img))

    def node_socket_incoming(self, socket, payload):
        self.img = pickle.loads(payload)
        self.do_value_changed(None)

    def remove(self, node):
        self.destroy()


class ImgRotateNode(GtkNodes.Node):
    __gtype_name__ = 'RotateNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Rotate")
        self.connect("node_func_clicked", self.remove)

        # create input socket
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming)

        # the compatibility key
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        # rotation control
        adjustment = Gtk.Adjustment(value = 45, lower = 0, upper = 360,
                step_increment = 5, page_increment = 45,
                page_size = 0)

        self.spinbutton = Gtk.SpinButton()
        self.spinbutton.set_adjustment(adjustment)
        self.spinbutton.set_size_request(50, 20)
        self.spinbutton.connect("value_changed", self.do_value_changed)

        self.item_add(self.spinbutton, GtkNodes.NodeSocketIO.DISABLE)

        self.reshapebutton = Gtk.CheckButton.new_with_label("Reshape")
        self.reshapebutton.connect("toggled", self.do_value_changed)
        self.item_add(self.reshapebutton, GtkNodes.NodeSocketIO.DISABLE)


        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))

    def do_value_changed(self, widget):
        deg = self.spinbutton.get_value()
        self.rot = ndimage.rotate(self.img, deg, reshape=self.reshapebutton.get_active())
        self.node_socket_output.write(pickle.dumps(self.rot))

    def node_socket_connect(self, sink, source):
        self.node_socket_output.write(pickle.dumps(self.rot))

    def node_socket_incoming(self, socket, payload):
        self.img = pickle.loads(payload)
        self.do_value_changed(None)

    def remove(self, node):
        self.destroy()




class ImgCropNode(GtkNodes.Node):
    __gtype_name__ = 'CropNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Crop")
        self.connect("node_func_clicked", self.remove)

        # create input socket
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming)

        # the compatibility key
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        # cropy controls
        adjustment = Gtk.Adjustment(value = 0, lower = 1, upper = 1000,
                step_increment = 10, page_increment = 50,
                page_size = 0)

        self.spinbuttonx = Gtk.SpinButton()
        self.spinbuttonx.set_adjustment(adjustment)
        self.spinbuttonx.set_size_request(50, 20)
        self.spinbuttonx.connect("value_changed", self.do_value_changed)

        self.item_add(self.spinbuttonx, GtkNodes.NodeSocketIO.DISABLE)


        adjustment = Gtk.Adjustment(value = 0, lower = 1, upper = 500,
                step_increment = 10, page_increment = 50,
                page_size = 0)

        self.spinbuttony = Gtk.SpinButton()
        self.spinbuttony.set_adjustment(adjustment)
        self.spinbuttony.set_size_request(50, 20)
        self.spinbuttony.connect("value_changed", self.do_value_changed)

        self.item_add(self.spinbuttony, GtkNodes.NodeSocketIO.DISABLE)



        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))


    def do_value_changed(self, widget):
        x = int(self.spinbuttonx.get_value())
        y = int(self.spinbuttony.get_value())
        self.crop = self.img[x: -x, y: -y]
        self.node_socket_output.write(pickle.dumps(self.crop))

    def node_socket_connect(self, sink, source):

        self.node_socket_output.write(pickle.dumps(self.crop))

    def node_socket_incoming(self, socket, payload):
        self.img = pickle.loads(payload)
        w, h = self.img.shape[:2]
        self.spinbuttonx.set_range(1, w / 2 - 1)
        self.spinbuttony.set_range(1, h / 2 - 1)
        self.do_value_changed(None)

    def remove(self, node):
        self.destroy()


class ImgFlipNode(GtkNodes.Node):
    __gtype_name__ = 'FlipNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Flip")
        self.connect("node_func_clicked", self.remove)

        # create input socket
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming)

        # the compatibility key
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        # output socket
        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))


    def do_output(self, widget):
        self.node_socket_output.write(pickle.dumps(self.flip))

    def node_socket_connect(self, sink, source):
        self.do_output(self)

    def node_socket_incoming(self, socket, payload):
        self.flip = np.flipud(pickle.loads(payload))
        self.do_output(None)

    def remove(self, node):
        self.destroy()



class ImgBlurNode(GtkNodes.Node):
    __gtype_name__ = 'BlurNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Blur")
        self.connect("node_func_clicked", self.remove)

        # create input socket
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming)

        # the compatibility key
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        # sigma control
        adjustment = Gtk.Adjustment(value = 3, lower = 0, upper = 50,
                step_increment = 1, page_increment = 10,
                page_size = 0)

        self.spinbutton = Gtk.SpinButton()
        self.spinbutton.set_adjustment(adjustment)
        self.spinbutton.set_size_request(50, 20)
        self.spinbutton.connect("value_changed", self.do_value_changed)

        self.item_add(self.spinbutton, GtkNodes.NodeSocketIO.DISABLE)

        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))

    def do_value_changed(self, widget):
        sig = self.spinbutton.get_value()
        self.filt = ndimage.gaussian_filter(self.img, sigma=(sig, sig, 0))
        self.node_socket_output.write(pickle.dumps(self.filt))

    def node_socket_connect(self, sink, source):
        self.node_socket_output.write(pickle.dumps(self.filt))

    def node_socket_incoming(self, socket, payload):
        self.img = pickle.loads(payload)
        self.do_value_changed(None)

    def remove(self, node):
        self.destroy()


class ImgSobelNode(GtkNodes.Node):
    __gtype_name__ = 'SobelNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Sobel")
        self.connect("node_func_clicked", self.remove)

        # create input socket
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming)

        # the compatibility key
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))

    def node_socket_connect(self, sink, source):
        self.node_socket_output.write(pickle.dumps(self.sob))

    def node_socket_incoming(self, socket, payload):
        img = pickle.loads(payload)
        self.sob = ndimage.sobel(img)
        self.node_socket_output.write(pickle.dumps(self.sob))

    def remove(self, node):
        self.destroy()


class ImgOpNode(GtkNodes.Node):
    __gtype_name__ = 'OpNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.set_label("Image Arithmetic")
        self.connect("node_func_clicked", self.remove)

        self.img_1 = None
        self.img_2 = None

        # create input socket 1
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming, 1)
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))

        # create input socket 2
        lbl = Gtk.Label.new("Input")
        lbl.set_xalign(0.0)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming, 2)
        node_socket_input.set_key(1234)
        node_socket_input.set_rgba(get_rgba('darkturquoise'))


        # create local node settings
        operations = ["+", "-", "*"]
        self.combobox = Gtk.ComboBoxText()

        self.combobox.set_entry_text_column(0)
        for op in operations:
            self.combobox.append_text(op)
        self.combobox.set_active(1)
        self.combobox.connect("changed", self.calculate)

        # internal node items are type DISABLE
        self.item_add(self.combobox, GtkNodes.NodeSocketIO.DISABLE)


        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(1.0)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

        self.node_socket_output.set_key(1234)
        self.node_socket_output.set_rgba(get_rgba('darkturquoise'))

    def node_socket_connect(self, sink, source):
        self.node_socket_output.write(pickle.dumps(self.res))


    def calculate(self, combobox):

        if self.img_1 is None:
            return

        if self.img_2 is None:
            return

        im1 = self.img_1
        im2 = self.img_2

        op = self.combobox.get_active_text();
        if op == "+":
            res = im1 + im2;
        elif op == "-":
            res = im1 - im2;
        elif op == "*":
            res = im1 * im2;
        else:
            return


        self.res = res

        self.node_socket_output.write(pickle.dumps(self.res))



    def node_socket_incoming(self, socket, payload, datasource):
        img = pickle.loads(payload)

        if datasource == 1:
            self.img_1 = img;
        elif datasource == 2:
            self.img_2 = img;

        self.calculate(self);

    def remove(self, node):
        self.destroy()





class OperationNode(GtkNodes.Node):
    __gtype_name__ = 'OperationNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.val_1 = 0;
        self.val_2 = 0;

        self.set_label("Operation")

        self.connect("node_func_clicked", self.remove)

        # create node input socket 1
        lbl = Gtk.Label.new("Input 1")
        lbl.set_xalign(0.2)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming, 1)

        # create nodeinput socket 2
        lbl = Gtk.Label.new("Input 2")
        lbl.set_xalign(0.2)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming, 2)


        # create local node settings
        operations = ["+", "-", "*", "/"]
        self.combobox = Gtk.ComboBoxText()
        self.combobox.set_entry_text_column(0)
        for op in operations:
            self.combobox.append_text(op)
        self.combobox.set_active(0)
        self.combobox.connect("changed", self.calculate)

        # internal node items are type DISABLE
        self.item_add(self.combobox, GtkNodes.NodeSocketIO.DISABLE)


        self.result = Gtk.Label()
        self.item_add(self.result, GtkNodes.NodeSocketIO.DISABLE)

        self.set_child_packing(self.result, True, True, 0, Gtk.PackType.START);

        # create node output socket
        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(0.8)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)


    def calculate(self, combobox):

        op = self.combobox.get_active_text();
        if op == "+":
            res = self.val_1 + self.val_2;
        elif op == "-":
            res = self.val_1 - self.val_2;
        elif op == "*":
            res = self.val_1 * self.val_2;
        elif op == "/":
            if (self.val_2 != 0):
                res = self.val_1 / self.val_2;
            else:
                res = None
        else:
            res = None;

        self.result.set_text(str(res));

        if res != None:
            x = struct.Struct("d")
            self.node_socket_output.write(x.pack(res))


    def node_socket_incoming(self, socket, payload, datasource):

        x = struct.Struct("d")

        val = x.unpack(payload)[0]

        if datasource == 1:
            self.val_1 = val;
        elif datasource == 2:
            self.val_2 = val;

        self.calculate(self);

    def node_socket_connect(self, sink, source):
        self.calculate(None)

    def remove(self, node):
        self.destroy()




class NumberNode(GtkNodes.Node, Gtk.Buildable):
    __gtype_name__ = 'NumberNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)


        self.set_label("Number Generator")

        self.connect("node_func_clicked", self.remove)

        adjustment = Gtk.Adjustment(value = 0, lower = 0, upper = 10000,
                step_increment = 1, page_increment = 10,
                page_size = 0)

        self.spinbutton = Gtk.SpinButton()
        self.spinbutton.set_adjustment(adjustment)
        self.spinbutton.set_size_request(50, 20)
        self.spinbutton.connect("value_changed", self.do_value_changed)

        self.item_add(self.spinbutton, GtkNodes.NodeSocketIO.DISABLE)

        lbl = Gtk.Label.new("Output")
        lbl.set_xalign(0.8)
        self.node_socket_output = self.item_add(lbl, GtkNodes.NodeSocketIO.SOURCE)
        self.node_socket_output.connect("socket_connect", self.node_socket_connect)

    # GtkBuildable interface
    def do_get_internal_child(self, builder, childname):

        if childname == "spinbutton":
            return self.spinbutton;
        else:
            return None

    # implementation of virtual export method
    def do_export_properties(self):
        return str("<child internal-child=\"spinbutton\">\n"
	           " <object class=\"GtkSpinButton\">\n"
		   "   <property name=\"value\">"
                   + str(self.spinbutton.get_value()) + "</property>\n"
                   " </object>\n"
                   "</child>\n")

    def do_value_changed(self, widget):
        val = float(self.spinbutton.get_value())

        x = struct.Struct("d")
        self.node_socket_output.write(x.pack(val))

    def node_socket_connect(self, sink, source):
        self.do_value_changed(None)

    def remove(self, node):
        self.destroy()


class PlotNode(GtkNodes.Node):
    __gtype_name__ = 'PlotNode'

    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)

        self.val_1 = 0;
        self.val_2 = 4;
        self.val_3 = 12;

        self.set_label("Plot")

        self.connect("node_func_clicked", self.remove)

        # create node input socket 1
        lbl = Gtk.Label.new("Input 1")
        lbl.set_xalign(0.1)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming, 1)

        # create nodeinput socket 2
        lbl = Gtk.Label.new("Input 2")
        lbl.set_xalign(0.1)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming, 2)

        # create nodeinput socket 3
        lbl = Gtk.Label.new("Input 3")
        lbl.set_xalign(0.1)
        node_socket_input = self.item_add(lbl, GtkNodes.NodeSocketIO.SINK)
        node_socket_input.connect("socket_incoming", self.node_socket_incoming, 3)


        f = Figure(figsize=(5, 4), dpi=100)
        self.a = f.add_subplot(111)

        sw = Gtk.ScrolledWindow()
        sw.set_border_width(10)

        self.canvas = FigureCanvas(f)
        sw.add(self.canvas)
        sw.set_size_request(200, 200)

        self.item_add(sw, GtkNodes.NodeSocketIO.DISABLE)
        self.set_child_packing(sw, True, True, 0, Gtk.PackType.START);


    def plot(self, combobox):

        x = np.linspace(self.val_1, self.val_2, self.val_3)
        y = np.cos(x**2/3 + 4)

        tck,u = interpolate.splprep(np.array((x,y)), s = 0)
        unew = np.arange(0, 1.01, 0.01)
        out = interpolate.splev(unew, tck)

        self.a.clear()

        self.a.plot(out[0], out[1], color='orange')
        self.a.plot(x, y, 'o')
        self.canvas.draw()


    def node_socket_incoming(self, socket, payload, datasource):

        x = struct.Struct("d")

        val = x.unpack(payload)[0]

        if datasource == 1:
            self.val_1 = int(val);
        elif datasource == 2:
            self.val_2 = int(val);
        elif datasource == 3:
            if (val > 10):
                self.val_3 = int(val);

        self.plot(self);

    def remove(self, node):
        self.destroy()


class Demo(object):


    def __init__(self):
        w = Gtk.Window.new(Gtk.WindowType.TOPLEVEL)
        w.set_border_width(20)



        hbox = Gtk.Box.new(Gtk.Orientation.HORIZONTAL, 0)

        b = Gtk.Button(label="Save")
        b.connect("clicked", self.save)
        hbox.add(b)

        b = Gtk.Button(label="Load")
        b.connect("clicked", self.load)
        hbox.add(b)

        b = Gtk.Button(label="NumberGen")
        b.connect("clicked", self.create_numnode)
        hbox.add(b)

        b = Gtk.Button(label="Operation")
        b.connect("clicked", self.create_opernode)
        hbox.add(b)

        b = Gtk.Button(label="Plot")
        b.connect("clicked", self.create_plotnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgSource")
        b.connect("clicked", self.create_imgsrcnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgPlot")
        b.connect("clicked", self.create_imgpltnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgMask")
        b.connect("clicked", self.create_imgmsknode)
        hbox.add(b)

        b = Gtk.Button(label="ImgRotate")
        b.connect("clicked", self.create_imgrotnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgCrop")
        b.connect("clicked", self.create_imgcropnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgFlip")
        b.connect("clicked", self.create_imgflipnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgBlur")
        b.connect("clicked", self.create_imgblurnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgSobel")
        b.connect("clicked", self.create_imgsobelnode)
        hbox.add(b)

        b = Gtk.Button(label="ImgOp")
        b.connect("clicked", self.create_imgopnode)
        hbox.add(b)


        frame = Gtk.Frame.new()

        sw = Gtk.ScrolledWindow(hexpand=True, vexpand=True)
        sw.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        frame.add(sw)

        self.node_view = GtkNodes.NodeView()
        sw.add(self.node_view)

        vbox = Gtk.Box.new(Gtk.Orientation.VERTICAL, 0)
        vbox.pack_start(hbox, False, False, 0)
        vbox.pack_start(frame, True, True, 0)
        w.add(vbox)


        w.show_all()
        w.connect("destroy", self.do_quit)
        Gtk.main()

    def save(self, widget=None):
        self.node_view.save("py_img_proc.xml")

    def load(self, widget=None):
        self.node_view.load("py_img_proc.xml")

    def create_numnode(self, widget=None):
        node = NumberNode()
        self.node_view.add(node)
        node.show_all()

    def create_opernode(self, widget=None):
        node = OperationNode()
        self.node_view.add(node)
        node.show_all()

    def create_plotnode(self, widget=None):
        node = PlotNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgsrcnode(self, widget=None):

        # how to place a node at an arbitrary location:

        pointer = self.node_view.get_display().get_default_seat().get_pointer()
        # x, y are in [1,2]
        pos = self.node_view.get_window().get_device_position(pointer)

        node = ImgSrcNode()

        self.node_view.add(node)
        self.node_view.child_set_property(node, "x", pos[1]);
        # we set x only, we're out of the canvas when selecting from the button bar
        # self.node_view.child_set_property(node, "y", pos[2]);
        node.show_all()

    def create_imgpltnode(self, widget=None):
        node = ImgPltNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgmsknode(self, widget=None):
        node = ImgMaskNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgrotnode(self, widget=None):
        node = ImgRotateNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgcropnode(self, widget=None):
        node = ImgCropNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgflipnode(self, widget=None):
        node = ImgFlipNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgblurnode(self, widget=None):
        node = ImgBlurNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgsobelnode(self, widget=None):
        node = ImgSobelNode()
        self.node_view.add(node)
        node.show_all()

    def create_imgopnode(self, widget=None):
        node = ImgOpNode()
        self.node_view.add(node)
        node.show_all()


    def do_quit(self, widget=None, data=None):
        Gtk.main_quit()
        sys.exit(0)


if __name__ == "__main__":
    Demo();
