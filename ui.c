/*\ 
 *  pgtc - Pete's Gnome Time Card is a gnome panel applet for keeping
 *  track of hours
 *
 *  $Id: ui.c,v 1.4 2000/02/27 22:22:18 prijks Exp $
 *
 *  Copyright (C) 2000 Pete Rijks
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
 *
 *  $Log: ui.c,v $
 *  Revision 1.4  2000/02/27 22:22:18  prijks
 *  added properties dialog box and support functions
 *
 *  Revision 1.3  2000/02/27 04:00:54  prijks
 *  added remove_job function to delete a job entry from the job selection
 *  entry
 *
 *  Revision 1.2  2000/02/27 01:23:56  prijks
 *  added a delete job item to the menu
\*/

#include <gnome.h>
#include <applet-widget.h>
#include "pgtc.h"
#include "red.xpm"
#include "green.xpm"

static GtkWidget *ap;
static GtkWidget *startbutton;
static GtkWidget *panelbox;
static GtkWidget *jobsel;
static GList *jobs = NULL;
static GnomeCanvasItem *light;
static GtkWidget *canvas;
static GdkImlibImage *green;
static GdkImlibImage *red;
static GtkTooltips *lighttips;
static lc lightcolor = g;


gchar*
get_active_job()
{
  return gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(jobsel)->entry));
}

gint
setup_ui(GtkWidget *applet)
{
  ap = applet;

  /* box to hold it all together */
  panelbox = gtk_hbox_new(FALSE,2);
  gtk_widget_show(panelbox);
  applet_widget_add(APPLET_WIDGET(applet), panelbox);

  /* about box menu item */
  applet_widget_register_stock_callback(APPLET_WIDGET(applet), "about",
					GNOME_STOCK_MENU_ABOUT,
					_("About"), 
					GTK_SIGNAL_FUNC(about_pgtc), NULL);

  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"properties",
					GNOME_STOCK_MENU_PROP,
					_("Properties..."),
					prop_dialog,
					NULL);


  /* new job menu item */
  applet_widget_register_callback(APPLET_WIDGET(applet),
				  "new-job", _("New Job..."),
				  GTK_SIGNAL_FUNC(new_job), NULL);

  /* delete job menu item */
  applet_widget_register_callback(APPLET_WIDGET(applet),
				  "del-job", _("Delete Job..."),
				  GTK_SIGNAL_FUNC(del_job), NULL);
  
  /* calculate time card menu item */
  applet_widget_register_callback(APPLET_WIDGET(applet),
				  "calculate", _("Calculate Timecard"),
				  GTK_SIGNAL_FUNC(calc_card), NULL);

  /* clear time card menu item */
  applet_widget_register_callback(APPLET_WIDGET(applet),
				  "clear", _("Clear Timecard"),
				  GTK_SIGNAL_FUNC(clear_card), NULL);

  /* button to punch in or out */
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  canvas = gnome_canvas_new();
  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();
  gnome_canvas_set_pixels_per_unit(GNOME_CANVAS(canvas),1);

  gtk_widget_set_usize (canvas, 25, 32);

  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), 0.0, 0.0, 
				  25, 32);

  green = gdk_imlib_create_image_from_xpm_data(green_xpm);
  red = gdk_imlib_create_image_from_xpm_data(red_xpm);
  light = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(canvas)),
			       gnome_canvas_image_get_type(),
			       "image", red,
			       "x", 12.5,
			       "y", 16.0,
			       "width", 25.0,
			       "height", 32.0,
			       NULL);

  gtk_widget_show(canvas);
  startbutton = gtk_button_new();
  gtk_container_add (GTK_CONTAINER (startbutton), canvas); 
  gtk_signal_connect(GTK_OBJECT(startbutton),
		     "clicked",
		     GTK_SIGNAL_FUNC(punch_callback),
		     NULL);
  gtk_widget_show(startbutton);
  gtk_box_pack_start(GTK_BOX(panelbox), startbutton, TRUE, TRUE, 1);
  
  /* tooltips */
  lighttips = gtk_tooltips_new();
  gtk_tooltips_set_tip(GTK_TOOLTIPS(lighttips), startbutton, 
		       "Punch In", NULL);

  /* job selector combo */
  jobsel = gtk_combo_new();
  gtk_widget_show(jobsel);
  gtk_box_pack_start(GTK_BOX(panelbox), jobsel, TRUE, TRUE, 1);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(jobsel)->list),
		     "select_child",
		     GTK_SIGNAL_FUNC(jobchange_callback),
		     NULL);
  gtk_combo_set_value_in_list(GTK_COMBO(jobsel), TRUE, FALSE);

  return FALSE;
}

static void
pd_punchout_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  if (GTK_TOGGLE_BUTTON (togglebutton)->active) {
    new_logout_pref(PUNCHOUT_ALL);
  }
}

static void
pd_prompt_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  if (GTK_TOGGLE_BUTTON(togglebutton)->active) {
    new_logout_pref(PROMPT);
  }
}

static void
pd_leave_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  if (GTK_TOGGLE_BUTTON(togglebutton)->active) {
    new_logout_pref(LEAVE_ALL);
  }
}

static void
pd_apply_signal()
{
  set_logout_pref();
}

void
prop_dialog()
{
  static GnomeHelpMenuEntry help_entry = { NULL,  "properties" };
  GtkWidget *pd_pbox;
  GtkWidget *radio_punchout;
  GtkWidget *radio_prompt;
  GtkWidget *radio_leave;
  GtkWidget *pd_label;
  GtkWidget *behavior_vbox;
  GtkWidget *logout_frame;
  GtkWidget *logout_vbox;

  help_entry.name = "pgtc"; /* PWR: alright, gotta figure this stuff out */

  pd_pbox = gnome_property_box_new();
  gtk_window_set_title(GTK_WINDOW(pd_pbox), "pgtc Preferences");

  behavior_vbox = gtk_vbox_new(FALSE, GNOME_PAD_BIG);
  gtk_container_set_border_width(GTK_CONTAINER(behavior_vbox),
				 GNOME_PAD_SMALL);

  logout_frame = gtk_frame_new("Logout Behavior");
  gtk_box_pack_start(GTK_BOX(behavior_vbox), logout_frame, FALSE, FALSE, 0);

  logout_vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
  gtk_container_set_border_width(GTK_CONTAINER(logout_vbox), GNOME_PAD_SMALL);
  gtk_container_add(GTK_CONTAINER(logout_frame), logout_vbox);

  pd_label = gtk_label_new("Action to perform upon logout:");
  gtk_box_pack_start(GTK_BOX(logout_vbox), pd_label, TRUE, TRUE, 0);
  gtk_widget_show(pd_label);

  radio_punchout = gtk_radio_button_new_with_label(NULL,
		    "Punch out all active jobs");
  gtk_signal_connect_object(GTK_OBJECT(radio_punchout), "toggled",
			    GTK_SIGNAL_FUNC(pd_punchout_toggle),
			    GTK_OBJECT(radio_punchout));  
  gtk_signal_connect_object(GTK_OBJECT(radio_punchout), "toggled",
			    GTK_SIGNAL_FUNC(gnome_property_box_changed),
			    GTK_OBJECT(pd_pbox));
  if (logout_pref() == PUNCHOUT_ALL)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_punchout), TRUE);

  radio_prompt = gtk_radio_button_new_with_label_from_widget(
		    GTK_RADIO_BUTTON(radio_punchout),
		    "Prompt for action on active jobs");
  gtk_signal_connect_object(GTK_OBJECT(radio_prompt), "toggled",
			    GTK_SIGNAL_FUNC(pd_prompt_toggle),
			    GTK_OBJECT(radio_prompt));  
  gtk_signal_connect_object(GTK_OBJECT(radio_prompt), "toggled",
			    GTK_SIGNAL_FUNC(gnome_property_box_changed),
			    GTK_OBJECT(pd_pbox));
  if (logout_pref() == PROMPT)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_prompt), TRUE);

  radio_leave = gtk_radio_button_new_with_label_from_widget(
		    GTK_RADIO_BUTTON(radio_prompt),
		    "Leave all jobs unchanged");
  gtk_signal_connect_object(GTK_OBJECT(radio_leave), "toggled",
			    GTK_SIGNAL_FUNC(pd_leave_toggle),
			    GTK_OBJECT(radio_leave));  
  gtk_signal_connect_object(GTK_OBJECT(radio_leave), "toggled",
			    GTK_SIGNAL_FUNC(gnome_property_box_changed),
			    GTK_OBJECT(pd_pbox));
  if (logout_pref() == LEAVE_ALL)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_leave), TRUE);

  gtk_box_pack_start(GTK_BOX(logout_vbox), radio_punchout, TRUE, TRUE, 0);
  gtk_widget_show(radio_punchout);
  gtk_box_pack_start(GTK_BOX(logout_vbox), radio_prompt, TRUE, TRUE, 0);
  gtk_widget_show(radio_prompt);
  gtk_box_pack_start(GTK_BOX(logout_vbox), radio_leave, TRUE, TRUE, 0);
  gtk_widget_show(radio_leave);

  gnome_property_box_append_page(GNOME_PROPERTY_BOX(pd_pbox),
				 behavior_vbox, gtk_label_new("Behavior"));

  gtk_signal_connect(GTK_OBJECT(pd_pbox), "apply",
		       GTK_SIGNAL_FUNC(pd_apply_signal),
		       NULL);

  gtk_signal_connect(GTK_OBJECT(pd_pbox), "help",
		     GTK_SIGNAL_FUNC(gnome_help_pbox_display), &help_entry);

  gtk_widget_show_all(pd_pbox);

}

lc
light_status()
{
  return lightcolor;
}

void
green_light()
{
  lightcolor = g;
  gnome_canvas_item_set (light, "image", green, NULL);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(lighttips), startbutton, 
		       "Punch Out", NULL);
}

void
red_light()
{
  lightcolor = r;
  gnome_canvas_item_set (light, "image", red, NULL);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(lighttips), startbutton, 
		       "Punch In", NULL);
}

void
add_job(gchar *j)
{
  jobs = g_list_append(jobs, j);
  gtk_combo_set_popdown_strings(GTK_COMBO(jobsel), jobs);
}

/* since glist's default find doesn't know how to compare strings... */
static gint
j_compare_func(gconstpointer a, gconstpointer b)
{
  return (g_strcasecmp((gchar*)a, (gchar*)b));
}

void
remove_job(gchar *j)
{
  GList *found;
  found = g_list_find_custom(jobs, (gpointer)j, j_compare_func);
  if (found) {
    jobs = g_list_remove_link(jobs, found);
    g_list_free_1(found);
  }
  gtk_combo_set_popdown_strings(GTK_COMBO(jobsel), jobs);
}

void
save_signal()
{
  applet_widget_sync_config(APPLET_WIDGET(ap));
}
