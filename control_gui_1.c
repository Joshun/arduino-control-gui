#include <gtk/gtk.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>

#include "arduino.h"

#define SIG_BUF 50
#define ENTRY_BUF 50
#define GUI_DATAFILE "control_gui_1.glade"

typedef struct {
	GtkWidget *window1;
	GtkWidget *window2;
	GtkWidget *buttons[9];
	GtkWidget *entry_text;
	GtkWidget *combo_box;
	char entry_buffer[ENTRY_BUF];
	int choice;
	int fd;
	struct threadData *data_set;
} controlData;

void show_settings_menu(GtkWidget *widget, gpointer data);
void close_settings_menu(GtkWidget *widget, GdkEvent *event, gpointer data);
void commit_settings_menu_changes(GtkEditable *editable, gpointer data);
void commit_combo_box_changes(GtkWidget *widget, gpointer data);
void send_message(GtkButton *button, gpointer data);

void print_text(controlData *dataset)
{
	printf("String: %s\n", dataset->entry_buffer);
}

void exit_program(void *param1, void *param2, gpointer data)
{
	controlData *widget_data = (controlData*)data;
	widget_data->data_set->run = FALSE;
	printf("Exiting...\n");
	sleep(1);
	gtk_main_quit();
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	
	const int baud_rates[] = { B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200 };
	
	struct termios config;
	int fd;
	pthread_t poll_thread;
	struct threadData data;
	int thread_err;
	void *thread_result;
	
	GtkBuilder *builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, GUI_DATAFILE, NULL);
	controlData widget_set;
	widget_set.window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
	widget_set.window2 = GTK_WIDGET(gtk_builder_get_object(builder, "window2"));
	widget_set.entry_text = GTK_WIDGET(gtk_builder_get_object(builder, "entry1"));
	widget_set.combo_box = GTK_WIDGET(gtk_builder_get_object(builder, "combobox1"));
	//strcpy(widget_set.entry_buffer, "Hello!");
	//printf("1%s\n", widget_set.entry_buffer);
	//print_text(&widget_set);
	int i = 0;
	char signame[SIG_BUF];

	for(i=0; i<9; i++) {
		memset(signame, '\0', sizeof(signame));
		snprintf(signame, SIG_BUF, "button%d", i+1);
		widget_set.buttons[i] = GTK_WIDGET(gtk_builder_get_object(builder, signame));
	}
	
	memset(widget_set.entry_buffer, '\0', sizeof(widget_set.entry_buffer));
	
	//gtk_button_set_label(GTK_BUTTON(widget_set.buttons[0]), "Hello!");
	gtk_builder_connect_signals(builder, (gpointer)&widget_set);
	
	fd = init_arduino(&config, DEVICE_PATH);
	widget_set.fd = fd;
	data.fd = fd;
	printf("fd: %d\n", fd);
	data.run = TRUE;
	widget_set.data_set = &data;
	thread_err = pthread_create(&poll_thread, NULL, poll_arduino, &data);
	
	gtk_widget_show(widget_set.window1);
	gtk_main();
}

void show_settings_menu(GtkWidget *widget, gpointer data)
{
	controlData *window_widgets = (controlData*) data;
	gtk_widget_show(window_widgets->window2);
	//printf("2%s\n", window_widgets->entry_buffer);
}

void close_settings_menu(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	controlData *window_widgets = (controlData*) data;
	//printf("Entered text:%s\n", window_widgets->entry_buffer);
	//printf("Choice: %d\n", window_widgets->choice);
		
	if( strlen(window_widgets->entry_buffer) > 0 )
		gtk_button_set_label(GTK_BUTTON(window_widgets->buttons[window_widgets->choice]), window_widgets->entry_buffer);
	gtk_widget_hide(widget);
}

void commit_settings_menu_changes(GtkEditable *editable, gpointer data)
{
	controlData *window_widgets = (controlData*) data;
	const char *text_data = gtk_editable_get_chars(editable, 0, -1);
	//const char *entry_text_data = gtk_entry_get_text(GTK_ENTRY(window_widgets->entry_text));
	//printf("Text entered: %s\n", text_data);
	memset(window_widgets->entry_buffer, '\0', strlen(window_widgets->entry_buffer));
	strncpy(window_widgets->entry_buffer, text_data, ENTRY_BUF - 1);
	window_widgets->choice = gtk_combo_box_get_active(GTK_COMBO_BOX(window_widgets->combo_box));
	//printf("Text2: %s\n", window_widgets->entry_buffer);
}

void commit_combo_box_changes(GtkWidget *widget, gpointer data)
{
	controlData *window_widgets = (controlData*) data;
	const char *text_data = gtk_entry_get_text(GTK_ENTRY(window_widgets->entry_text));
	if( strlen(text_data) > 0 ) {
		memset(window_widgets->entry_buffer, '\0', strlen(window_widgets->entry_buffer));
		strncpy(window_widgets->entry_buffer, text_data, ENTRY_BUF - 1);
		gtk_button_set_label(GTK_BUTTON(window_widgets->buttons[window_widgets->choice]), text_data);
	}
	
	window_widgets->choice = gtk_combo_box_get_active(GTK_COMBO_BOX(window_widgets->combo_box));
	const char *button_text = gtk_button_get_label(GTK_BUTTON(window_widgets->buttons[window_widgets->choice]));
	//printf("Text to set: %s\n", button_text);
	gtk_entry_set_text(GTK_ENTRY(window_widgets->entry_text), button_text);
}

void send_message(GtkButton *button, gpointer data)
{
	controlData *widget_data = (controlData*) data;
	const char *label = gtk_button_get_label(button);
	char buffer[ENTRY_BUF];
	INIT_BUF(buffer, sizeof(buffer));
	strncpy(buffer, label, ENTRY_BUF - 2);
	
	printf("Sending %s...\n", label);
	int index = strlen(buffer);
	buffer[index] = '\n';
	buffer[index + 1] = '\0';
	//printf("%s", buffer);
	
	write(widget_data->fd, buffer, strlen(buffer));
}
