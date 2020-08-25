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

gint
save_session(GtkWidget *w, const char *privcfgpath, const char *globcfgpath)
{
  gint numjobs, numhours;
  gint i;
  GList *l;
  gchar key[32];
  hours *h;

  gnome_config_push_prefix("/pgtc/");

  /* PWR: ok, this is really kinda bogus. if you can help me figure
   * out what is going on, play around with the below code.  the
   * privcfgpath and globcfgpath passed to this signal_func are
   * different than the ones that are the privcfgpath/globcfgpath
   * members of the applet widget. so since load_session doesn't get
   * passed the values that save_session does, and since the members
   * of the applet widget are called /panel.d/default/Applet_Dummy/,
   * I'm going to stick with hardcoded stuff for now until some kind
   * soul can sort this mess out for me.  

  gnome_config_push_prefix(privcfgpath);
  printf("save: %s\n",privcfgpath);
  printf("save: %s\n",globcfgpath);
  printf("save: %s\n",APPLET_WIDGET(w)->privcfgpath);
  printf("save: %s\n",APPLET_WIDGET(w)->globcfgpath);
   * (end bogosity) */

  /* first of all, we save a version number as a sanity check...
   * I have all sorts of intentions of adding new features that 
   * would require mucking with how stuff is saved...  */
  gnome_config_set_int("main/internal_revision", INTERNAL_REVISION);

  /* save the jobs */
  gnome_config_clean_section("jobs");
  gnome_config_sync();
  numjobs = num_jobs();
  l = get_joblist();
  gnome_config_set_int("jobs/numjobs", numjobs);
  for (i = 0; i < numjobs; i++) {
    g_snprintf(key, 31, "jobs/job_%d",i);
    gnome_config_set_string(key, ((jobdesc*)g_list_nth_data(l,i))->name);
  }

  /* save the hours */
  gnome_config_clean_section("hours");
  gnome_config_sync();
  numhours = num_hours();
  l = get_hourslist();
  gnome_config_set_int("hours/numhours", numhours);
  for (i = 0; i < numhours; i++) {
    h = (hours*)g_list_nth_data(l,i);
    g_snprintf(key, 31, "hours/hours_%d",i);
    gnome_config_set_string(key, h->name);
    g_snprintf(key, 31, "hours/stamp_%d",i);
    gnome_config_set_int(key, h->timestamp);
    g_snprintf(key, 31, "hours/what_%d",i);
    gnome_config_set_int(key, h->what);
  }
  
  /* we're done, let gnome do it's thing... */
  /* have a look at 
   * http://cvs.gnome.org/lxr/source/gnome-core/panel/APPLET_WRITING
   * for info on what all this actually does... */
  gnome_config_sync();
  gnome_config_drop_all();
  gnome_config_pop_prefix();
  return FALSE;
}

gint
load_session(GtkWidget *applet)
{
  gint intrev_check;
  gint numjobs, numhours;
  GList *l = NULL;
  gint i;
  jobdesc *tmp;
  gchar *jobname;
  gchar *hour;
  gint stamp, what;
  hours *tmp2;
  gchar key[32];

  gnome_config_push_prefix("/pgtc/");
  /* see above note for save_session and 
   * why this privcfgpath crap is bogus
  gnome_config_push_prefix(APPLET_WIDGET(applet)->privcfgpath);
  printf("load: %s\n",APPLET_WIDGET(applet)->privcfgpath);
  printf("load: %s\n",APPLET_WIDGET(applet)->globcfgpath); 
   */


  /* sanity check */
  g_snprintf(key,31,"main/internal_revision=%d",INTERNAL_REVISION);
  intrev_check = gnome_config_get_int(key);
  if (intrev_check != INTERNAL_REVISION) {
    printf("sanity check failed, revision mismatch:\n");
    printf("my revision: %d. saved revision: %d\n",
	   INTERNAL_REVISION,intrev_check);
    return TRUE;
  }

  /* load the jobs */
  numjobs = gnome_config_get_int("jobs/numjobs");
  for (i = 0; i < numjobs; i++) {
    g_snprintf(key, 31, "jobs/job_%d",i);
    jobname = gnome_config_get_string(key);
    tmp = g_new(jobdesc, 1);
    tmp->name = jobname;
    tmp->active = 0;
    l = g_list_append(l, tmp);
  }
  set_joblist(l);

  /* load the hours */
  l = NULL;
  numhours = gnome_config_get_int("hours/numhours");
  for (i = 0; i < numhours; i++) {
    g_snprintf(key, 31, "hours/hours_%d",i);
    hour = gnome_config_get_string(key);
    g_snprintf(key, 31, "hours/stamp_%d",i);
    stamp = gnome_config_get_int(key);
    g_snprintf(key, 31, "hours/what_%d",i);
    what = gnome_config_get_int(key);
    tmp2 = g_new(hours, 1);
    tmp2->name = hour;
    tmp2->timestamp = stamp;
    tmp2->what = what;
    l = g_list_append(l, tmp2);
  }
  set_hourslist(l);

  create_jobs();

  gnome_config_pop_prefix();
  return FALSE;
}
