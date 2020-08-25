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
#include <time.h>
#include "pgtc.h"

static GList *joblist;
static GList *hourslist;

/* comparing one jobdesc or hour to another */
static gint
job_compare_func(gconstpointer a, gconstpointer b)
{
  return (g_strcasecmp(((jobdesc*)a)->name, ((jobdesc*)b)->name));
}

static gint
hour_compare_func(gconstpointer a, gconstpointer b)
{
  return (g_strcasecmp(((hours*)a)->name, ((hours*)b)->name));
}


gint
punch_callback()
{
  gchar *j;
  jobdesc* jb;
  jobdesc tmp;
  hours *new;
  GList *found;
  gint p;
  gint what;
  /* find job in entry from list */
  j = get_active_job();
  tmp.name = j;
  found = g_list_find_custom(joblist, (gpointer)&tmp,job_compare_func);

  if (found == NULL) { /* no such job in list */
    
  } else { /* job found */
    p = g_list_position(joblist, found);
    jb = (jobdesc*) g_list_nth_data(joblist,p);
    if (jb->active) {
      red_light();
      jb->active = 0;
      what = PUNCHOUT;
    } else {
      green_light();
      jb->active = 1;
      what = PUNCHIN;
    }
    new = g_new(hours,1);
    new->name = g_strdup(jb->name); /* PWR: check memory leaks */
    new->what = what;
    new->timestamp = time(NULL);
    hourslist = g_list_append(hourslist, new);
    save_signal();
  }

  return FALSE;
}

gint jobchange_callback()
{
  gchar *j;
  jobdesc* jb;
  jobdesc tmp;
  GList *found;
  gint p;

  /* find job in entry from list */
  j = get_active_job();
  tmp.name = j;
  found = g_list_find_custom(joblist, (gpointer)&tmp,job_compare_func);

  if (found == NULL) { /* no such job in list */
    
  } else { /* job found */
    p = g_list_position(joblist, found);
    jb = (jobdesc*) g_list_nth_data(joblist,p);
    if (jb->active)
      green_light();
    else
      red_light();
  }
  return FALSE;
}


/* pop up a dialog box to prompt for a new job name */
void
new_job()
{
  GtkWidget *nj_dialog;
  GtkWidget *nj_entry;
  GtkWidget *nj_label;
  gint nj_button_pressed;
  gchar *nj_jobname;
  jobdesc *tmp;

  nj_dialog = gnome_dialog_new("New Job", GNOME_STOCK_BUTTON_OK,
			       GNOME_STOCK_BUTTON_CANCEL, NULL);

  nj_label = gtk_label_new("Enter new job name:");
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(nj_dialog)->vbox),
		     nj_label, TRUE, TRUE, 0);
  gtk_widget_show(nj_label);

  nj_entry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(nj_dialog)->vbox), 
		      nj_entry, TRUE, TRUE, 0);
  gtk_widget_show(nj_entry);

  nj_button_pressed = gnome_dialog_run(GNOME_DIALOG(nj_dialog));

  if (nj_button_pressed == 0) {
    nj_jobname = g_strdup(gtk_entry_get_text(GTK_ENTRY(nj_entry)));
    tmp = g_new(jobdesc,1);
    tmp->name = nj_jobname;
    tmp->active = 0;
    joblist = g_list_append(joblist, tmp);
    add_job(nj_jobname);
    save_signal();
  }

  if (nj_button_pressed >= 0) 
    gnome_dialog_close(GNOME_DIALOG(nj_dialog));
}

/* calculate the hours worked on the active job */
void
calc_card()
{
  GtkWidget *cc_dialog;
  GtkWidget *cc_label;
  GtkWidget *cc_totaltime;
  GtkWidget *cc_clist;
  gint cc_button_pressed;
  gchar *cc_jobname;
  gchar cc_labeltext[64];
  gchar cc_totaltimetext[64];
  gchar *titles[] = {"In", "Out"};
  gint i;
  hours *t;
  gint total = 0;
  gint lastin = 0;
  gint stat = PUNCHOUT;
  gchar *dates[2];

  cc_dialog = gnome_dialog_new("Time Card", GNOME_STOCK_BUTTON_OK,
			       _("Clear Hours"), NULL);

  cc_jobname = get_active_job();
  g_snprintf(cc_labeltext, 63, "Time Card for %s:", cc_jobname);
  cc_label = gtk_label_new(cc_labeltext);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(cc_dialog)->vbox), 
		      cc_label, TRUE, TRUE, 0);
  gtk_widget_show(cc_label);

  cc_clist = gtk_clist_new_with_titles(2, titles);
  gtk_clist_set_column_width(GTK_CLIST(cc_clist), 0, 192);
  gtk_clist_set_column_width(GTK_CLIST(cc_clist), 1, 192);

  for (i = 0; i < num_hours(); i++) {
    t = g_list_nth_data(hourslist, i);
    if (t == NULL)
      break;
    if (g_strcasecmp(cc_jobname, t->name))
      continue;
    if (stat == t->what)
      continue; /* PWR: come up with a better thing to do here... */
    if (stat == PUNCHOUT) {
      stat = PUNCHIN;
      lastin = t->timestamp;
      dates[0] = g_strdup(ctime(&(t->timestamp)));
    } else {
      stat = PUNCHOUT;
      total += (t->timestamp - lastin);
      dates[1] = g_strdup(ctime(&(t->timestamp)));
      gtk_clist_append(GTK_CLIST(cc_clist), dates);
    }
  }
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(cc_dialog)->vbox), 
		     cc_clist, TRUE, TRUE, 0);
  gtk_widget_show(cc_clist);

  g_snprintf(cc_totaltimetext, 63, "Total: %.2f hours", 
	     (double) total / 3600.0);
  cc_totaltime = gtk_label_new(cc_totaltimetext);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(cc_dialog)->vbox),
		     cc_totaltime, TRUE, TRUE, 0);
  gtk_widget_show(cc_totaltime);

  cc_button_pressed = gnome_dialog_run(GNOME_DIALOG(cc_dialog));

  if (cc_button_pressed == 1) {
    clear_card();
  }

  if (cc_button_pressed >= 0) 
    gnome_dialog_close(GNOME_DIALOG(cc_dialog));
}

/* clear all the hours for the active job */
void
clear_card()
{
  gchar *j;
  hours tmp;
  GList *found;

  j = get_active_job();
  tmp.name = j;

  while ((found = g_list_find_custom(hourslist,
				     (gpointer)&tmp, hour_compare_func))) {
    hourslist = g_list_remove_link(hourslist, found);
    g_list_free_1(found);
  }
  save_signal();
}

void
create_jobs()
{
  gint i;
  jobdesc *j;
  for (i = 0; i < num_jobs(); i++) {
    j = (jobdesc*)g_list_nth_data(joblist,i);
    add_job(j->name);
  }
}

/* mmm... abstracteriffic */
gint
num_jobs()
{
  return g_list_length(joblist);
}

gint
num_hours()
{
  return g_list_length(hourslist);
}

GList*
get_joblist()
{
  return joblist;
}

GList*
get_hourslist()
{
  return hourslist;
}

void
set_joblist(GList *l)
{
  joblist = l;
}

void
set_hourslist(GList *l)
{
  hourslist = l;
}
