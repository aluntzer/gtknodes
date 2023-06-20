/**
 * I don't know vala, so this is as much as I can give you for an example.
 *
 * This demo allows you to connect the output socket to the input and receive
 * an event which will change the label of the input node.
 *
 * The cast of to GtkNodes.NodeSocket seems necessary, because vala does not
 * consider the socket_connect signal to be part of the socket's signals if it
 * is returned as a GtkWidget on item_add() (which is done intentionally).
 * 
 * Compile with 'valac --pkg gtknodes example.vala' with the library installed.
 * There is no makefile integration for now, sorry.
 */


using Gtk;
using GtkNodes;


int main (string[] args)
{
	Gtk.init (ref args);

	var window = new Window ();
	window.title = "Nodes Demo";
	window.border_width = 10;
	window.window_position = WindowPosition.CENTER;
	window.set_default_size (300, 300);
	window.destroy.connect (Gtk.main_quit);

	var node_view = new GtkNodes.NodeView();
	var node = new GtkNodes.Node();

	node.set_label("Demo");

	var ilbl = new Gtk.Label("Input");
	ilbl.set_xalign(0.0f);

	unowned var input = (GtkNodes.NodeSocket) node.item_add(ilbl, GtkNodes.NodeSocketIO.SINK);


    	input.socket_connect.connect (() => {
		ilbl.label = "connected";
	});

	input.socket_disconnect.connect (() => {
		ilbl.label = "disconnected";
	});


	var olbl = new Gtk.Label("Output");
	olbl.set_xalign(1.0f);
	node.item_add(olbl, GtkNodes.NodeSocketIO.SOURCE);


	node_view.add(node);
	node.show();
	window.add(node_view);
	window.show_all();

	Gtk.main();
	return 0;
}


