/*\
 *  pgtc - Pete's Gnome Time Card is a gnome panel applet for keeping
 *  track of hours
 *
 *  $Id: pgtc.c,v 1.2 2000/02/27 21:28:27 prijks Exp $
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
 *  $Log: pgtc.c,v $
 *  Revision 1.2  2000/02/27 21:28:27  prijks
 *  made load_session call before connecting save_session signal, so that
 *  we don't accidentally overwrite anything during load_session if we
 *  receive a save_session signal.
 *
 *
\*/

#include <gnome.h>
#include <applet-widget.h>
#include "pgtc.h"

int
main(int argc, char **argv)
{
  GtkWidget *applet;
  
  /* intialize, this will basically set up the applet, corba and
     call gnome_init */
  applet_widget_init("pgtc", NULL, argc, argv, NULL, 0, NULL);

  /* create a new applet_widget */
  applet = applet_widget_new("pgtc");
  /* in the rare case that the communication with the panel
     failed, error out */
  if (!applet) {
    g_error("Can't create applet!\n");
    exit(1);
  }

  setup_ui(applet);

  /* session stuff */
  if (load_session(applet)) {
    exit(1);
  }
  gtk_signal_connect(GTK_OBJECT(applet), "save_session",
		     GTK_SIGNAL_FUNC(save_session), NULL);
  gtk_signal_connect(GTK_OBJECT(applet), "delete_event",
		     GTK_SIGNAL_FUNC(delevent_handler), NULL);

  gtk_widget_show(applet);

  /* special corba main loop */
  applet_widget_gtk_main();

  return 0;
}
