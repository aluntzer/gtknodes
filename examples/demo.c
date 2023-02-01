#include <gtk/gtk.h>

#include <gtknode.h>
#include <gtknodeview.h>

#include <node_pulse.h>
#include <node_step.h>
#include <node_binary_decode.h>
#include <node_binary_encode.h>
#include <node_convert_number.h>
#include <node_show_number.h>
#include <node_bitwise_and.h>
#include <node_bitwise_or.h>
#include <node_bitwise_and.h>
#include <node_bitwise_xor.h>
#include <node_bitwise_not.h>


static void node_view_create_pulse_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_pulse_new());
}

static void node_view_create_step_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_step_new());
}

static void node_view_create_node_binary_decode_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_binary_decode_new());
}

static void node_view_create_node_binary_encode_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_binary_encode_new());
}

static void node_view_create_node_convert_number_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_convert_number_new());
}

static void node_view_create_node_show_number_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_show_number_new());
}

static void node_view_create_node_bitwise_and_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_bitwise_and_new());
}

static void node_view_create_node_bitwise_or_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_bitwise_or_new());
}

static void node_view_create_node_bitwise_xor_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_bitwise_xor_new());
}

static void node_view_create_node_bitwise_not_cb(GtkWidget *menu, GtkWidget *node_view)
{
	gtk_container_add(GTK_CONTAINER(node_view), node_bitwise_not_new());
}

static void node_view_save_cb(GtkWidget *widget, GtkWidget *node_view)
{
	GtkWidget *dia;
	GtkFileChooser *chooser;
	gint res;

	GtkWidget *win;



	win = gtk_widget_get_toplevel(GTK_WIDGET(widget));

	if (!GTK_IS_WINDOW(win)) {
		g_warning("%s: toplevel widget is not a window", __func__);
		return;
	}

	dia = gtk_file_chooser_dialog_new("Export Graph",
					  GTK_WINDOW(win),
					  GTK_FILE_CHOOSER_ACTION_SAVE,
					  "_Cancel",
					  GTK_RESPONSE_CANCEL,
					  "_Save",
					  GTK_RESPONSE_ACCEPT,
					  NULL);

	chooser = GTK_FILE_CHOOSER(dia);


	gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

	gtk_file_chooser_set_current_name(chooser, "graph.xml");

	gtk_file_chooser_set_current_folder(chooser, g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS));

	res = gtk_dialog_run(GTK_DIALOG(dia));

	if (res == GTK_RESPONSE_ACCEPT) {

		gboolean ret;
		gchar *fname;

		fname = gtk_file_chooser_get_filename(chooser);

		ret = gtk_nodes_node_view_save (GTKNODES_NODE_VIEW(node_view),
						fname);

		if (!ret) {

			GtkWidget *d;

			d = gtk_message_dialog_new(GTK_WINDOW(win),
						   GTK_DIALOG_MODAL,
						   GTK_MESSAGE_ERROR,
						   GTK_BUTTONS_CLOSE,
						   "Could not open file %s",
						   fname);

			gtk_dialog_run(GTK_DIALOG(d));
			gtk_widget_destroy(d);
		}

		g_free(fname);
	}

	gtk_widget_destroy(dia);
}

static void node_view_load_cb(GtkWidget *widget,  GtkWidget *node_view)
{
	GtkWidget *dia;
	GtkFileChooser *chooser;
	gint res;

	GtkWidget *win;

	gchar *fname = NULL;


	win = gtk_widget_get_toplevel(GTK_WIDGET(widget));

	if (!GTK_IS_WINDOW(win)) {
		g_warning("%s: toplevel widget is not a window", __func__);
		return;
	}

	dia = gtk_file_chooser_dialog_new("Import Graph",
					  GTK_WINDOW(win),
					  GTK_FILE_CHOOSER_ACTION_OPEN,
					  "_Cancel",
					  GTK_RESPONSE_CANCEL,
					  "_Open",
					  GTK_RESPONSE_ACCEPT,
					  NULL);

	chooser = GTK_FILE_CHOOSER(dia);

	gtk_file_chooser_set_current_folder(chooser, g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS));

	res = gtk_dialog_run(GTK_DIALOG(dia));


	if (res == GTK_RESPONSE_ACCEPT) {

		gboolean ret;

		fname = gtk_file_chooser_get_filename(chooser);

		ret = gtk_nodes_node_view_load(GTKNODES_NODE_VIEW (node_view),
					       fname);

		if (!ret) {

			GtkWidget *dia;
			GtkWindow * win;

			win = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(widget)));
			dia = gtk_message_dialog_new(win,
						     GTK_DIALOG_MODAL,
						     GTK_MESSAGE_ERROR,
						     GTK_BUTTONS_CLOSE,
						     "Error loading file %s",
						     fname);

			gtk_dialog_run(GTK_DIALOG(dia));
			gtk_widget_destroy(dia);


		}

		g_free(fname);
	}

	gtk_widget_destroy(dia);
}

static void node_view_popup_menu(GtkWidget *node_view)
{
	static GtkWidget *menu;
	GtkWidget *menuitem;


	if (menu) {
		gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
		return;
	}

	menu = gtk_menu_new();

	menuitem = gtk_menu_item_new_with_label("LOAD");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_load_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("SAVE");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_save_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Pulse");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_pulse_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Step");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_step_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Binary Decoder");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_binary_decode_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Binary Encoder");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_binary_encode_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Number Converter");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_convert_number_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Number Display");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_show_number_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Bitwise AND");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_bitwise_and_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Bitwise OR");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_bitwise_or_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Bitwise XOR");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_bitwise_xor_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	menuitem = gtk_menu_item_new_with_label("Bitwise NOT");
	g_signal_connect(menuitem, "activate",
			 G_CALLBACK(node_view_create_node_bitwise_not_cb), node_view);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all(menu);

	gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}



static gboolean node_view_button_press_cb(GtkWidget      *node_view,
					  GdkEventButton *event,
					  gpointer        user_data)
{
	if (event->type == GDK_BUTTON_PRESS)
		if (event->button == 3)
			node_view_popup_menu(node_view);

	return TRUE;
}


static void activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *sw;
	GtkWidget *frame;
	GtkWidget *node_view;


	/* create our application window */
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "GtkNodes Demo");

	/* create a frame for decoration and add it to the window */
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 10);
	gtk_container_add(GTK_CONTAINER(window), frame);


        sw = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(frame), sw);

	/* create a node view and add it to the frame */
	node_view = gtk_nodes_node_view_new();
	gtk_container_add(GTK_CONTAINER(sw), node_view);


	g_signal_connect (G_OBJECT(node_view), "button-press-event",
			  G_CALLBACK(node_view_button_press_cb), NULL);


	/* show everything */
	gtk_widget_set_size_request(window, 400, 400);
	gtk_widget_show_all(window);
}


int main(int argc, char **argv)
{
	int status;
	GtkApplication *app;


	app = gtk_application_new("org.uvie.nodes", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
