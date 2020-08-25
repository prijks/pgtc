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

/* ahh, the vanity of it all... */
void
about_pgtc(AppletWidget *applet, gpointer data)
{
  GtkWidget *about;
  const gchar *authors[2];
  gchar version[32];

  g_snprintf (version, sizeof (version), "%d.%d.%d",
	      APPLET_VERSION_MAJ,
	      APPLET_VERSION_MIN,
	      APPLET_VERSION_REV);

  authors[0] = "Pete Rijks <prijks@esgeroth.org>";
  authors[1] = NULL;

  about = gnome_about_new (_ ("Pete's Gnome Time Card"), version,
			   "(C) 2000",
			   authors,
		       _ ("Released under the GNU general public license.\n\n"
			  "A simple time tracking utility\n"
			  "http://www.nd.edu/~prijks/puters/petesoft/pgtc/"),
			   NULL);
  gtk_widget_show (about);
}
