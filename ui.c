/*\ 
 *  pgtc - Pete's Gnome Time Card is a gnome panel applet for keeping
 *  track of hours
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

  /* new job menu item */
  applet_widget_register_callback(APPLET_WIDGET(applet),
				  "new-job", _("New Job..."),
				  GTK_SIGNAL_FUNC(new_job), NULL);
  
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

lc
light_status()
{
  return lightcolor;
}

void
green_light()
{
  if (lightcolor != g) {
    lightcolor = g;
    gnome_canvas_item_set (light, "image", green, NULL);
    gtk_tooltips_set_tip(GTK_TOOLTIPS(lighttips), startbutton, 
			 "Punch Out", NULL);

  }
}

void
red_light()
{
  if (lightcolor != r) {
    lightcolor = r;
    gnome_canvas_item_set (light, "image", red, NULL);
    gtk_tooltips_set_tip(GTK_TOOLTIPS(lighttips), startbutton, 
			 "Punch In", NULL);
  }
}

void
add_job(gchar *j)
{
  jobs = g_list_append(jobs, j);
  gtk_combo_set_popdown_strings(GTK_COMBO(jobsel), jobs);
}

void
save_signal()
{
  applet_widget_sync_config(APPLET_WIDGET(ap));
}
