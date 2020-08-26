/*\
 *  pgtc - Pete's Gnome Time Card is a gnome panel applet for keeping
 *  track of hours
 *
 *  $Id: timing.c,v 1.3 2000/02/27 22:21:31 prijks Exp $
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
 *
 *  $Log: timing.c,v $
 *  Revision 1.3  2000/02/27 22:21:31  prijks
 *  added a delete event handler so that upon quitting, jobs can be saved
 *  or punched out based on the user's preferences. made clearing the
 *  timecard prompt before doing so.
 *
 *  Revision 1.2  2000/02/27 04:00:17  prijks
 *  added del_job function to allow users to delete unwanted jobs.
 *
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
    new->name = g_strdup(jb->name); /* PWR: fix memory leaks */
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

void
del_job()
{
  GtkWidget *dj_dialog;
  GtkWidget *dj_label;
  gchar dj_label_text[81];
  gint dj_button_pressed;
  gchar *dj_jobname;
  jobdesc tmp;
  GList *found;

  dj_dialog = gnome_dialog_new("Delete Job", GNOME_STOCK_BUTTON_OK,
			       GNOME_STOCK_BUTTON_CANCEL, NULL);

  dj_jobname = get_active_job();
  g_snprintf(dj_label_text, 80, "Are you sure you want to\n"
	     "delete the job \"%s\"?",dj_jobname);
  dj_label = gtk_label_new(dj_label_text);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dj_dialog)->vbox),
		     dj_label, TRUE, TRUE, 0);
  gtk_widget_show(dj_label);

  dj_button_pressed = gnome_dialog_run(GNOME_DIALOG(dj_dialog));

  if (dj_button_pressed == 0) {
    /* remove any hours logged for the job to be deleted */
    clear_card();

    /* find the job in the job list and remove it*/
    tmp.name = dj_jobname;
    found = g_list_find_custom(joblist, (gpointer)&tmp, job_compare_func);
    if (found) {
      joblist = g_list_remove_link(joblist, found);
      g_list_free_1(found);
    }

    /* remove the entry for the job from the job pull-down menu */
    remove_job(dj_jobname);

    g_free(dj_jobname);

    /* save */
    save_signal();
  }

  if (dj_button_pressed >= 0) 
    gnome_dialog_close(GNOME_DIALOG(dj_dialog));
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
  time_t now;

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
    if (t == NULL) {
      break;
    }
    if (g_strcasecmp(cc_jobname, t->name))
      continue;
    if (stat == t->what)
      continue; /* PWR: come up with a better thing to do here... */
    if (i == num_hours() - 1) {
      if (t->what == PUNCHIN) {
	lastin = t->timestamp;
	dates[0] = g_strdup(ctime(&(t->timestamp)));
	/* if we're currently punched in, we'll calculate the hours
	 * worked up to the minute ... */
	now = time(NULL);
	total += (now - lastin);
	dates[1] = g_strdup(ctime(&now));
	gtk_clist_append(GTK_CLIST(cc_clist), dates);
      }
    }
    if (g_strcasecmp(cc_jobname, t->name))
      continue;
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
  GtkWidget *cc_dialog;
  GtkWidget *cc_label;
  gchar cc_label_text[81];
  gint cc_button_pressed;
  gchar *cc_jobname;
  gchar *j;
  hours tmp;
  GList *found;
  jobdesc tmp2;
  gint p;
  jobdesc *jb;

  j = get_active_job();
  tmp.name = j;

  cc_dialog = gnome_dialog_new("Clear Hours", GNOME_STOCK_BUTTON_OK,
			       GNOME_STOCK_BUTTON_CANCEL, NULL);

  cc_jobname = get_active_job();
  g_snprintf(cc_label_text, 80, "Are you sure you want to\n"
	     "clear the hours for the job \"%s\"?",cc_jobname);
  cc_label = gtk_label_new(cc_label_text);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(cc_dialog)->vbox),
		     cc_label, TRUE, TRUE, 0);
  gtk_widget_show(cc_label);

  cc_button_pressed = gnome_dialog_run(GNOME_DIALOG(cc_dialog));

  if (cc_button_pressed == 0) {

    while ((found = g_list_find_custom(hourslist,
				       (gpointer)&tmp, hour_compare_func))) {
      hourslist = g_list_remove_link(hourslist, found);
      g_list_free_1(found);
    }

    tmp2.name = j;
    found = g_list_find_custom(joblist, (gpointer)&tmp2,job_compare_func);
    if (found) {
      p = g_list_position(joblist, found);
      jb = (jobdesc*) g_list_nth_data(joblist,p);
      if (jb->active) {
	jb->active = 0;
	red_light();
      }
    }
    save_signal();
  }
  if (cc_button_pressed >= 0) 
    gnome_dialog_close(GNOME_DIALOG(cc_dialog));

}

void
create_jobs()
{
  gint i;
  jobdesc *jb;
  gchar *j;
  jobdesc tmp;
  GList *found;
  gint p;

  for (i = 0; i < num_jobs(); i++) {
    jb = (jobdesc*)g_list_nth_data(joblist,i);
    add_job(jb->name);
  }
  j = get_active_job();
  tmp.name = j;
  found = g_list_find_custom(joblist, (gpointer)&tmp,job_compare_func);
  if (found) {
    p = g_list_position(joblist, found);
    jb = (jobdesc*) g_list_nth_data(joblist,p);
    if (jb->active) {
      green_light();
    }
  }
}

static void
prompt_quit()
{
  jobdesc* jb;
  hours *new;
  gint numjobs;
  gint i;
  GtkWidget *pq_dialog;
  GtkWidget *pq_label;
  gchar pq_label_text[81];
  gint pq_button_pressed;

  numjobs = num_jobs();
  for (i = 0; i < numjobs; i++) {
    jb = (jobdesc*)g_list_nth_data(joblist,i);
    if (jb->active) {

      pq_dialog = gnome_dialog_new("Question", "Punch Out",
				   "Leave", NULL);

      g_snprintf(pq_label_text,80,"You are still logged in to job \"%s\",\n"
		 "Please choose an action for this job.\n",jb->name);
      pq_label = gtk_label_new(pq_label_text);
      gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(pq_dialog)->vbox),
			 pq_label, TRUE, TRUE, 0);
      gtk_widget_show(pq_label);

      pq_button_pressed = gnome_dialog_run(GNOME_DIALOG(pq_dialog));

      if (pq_button_pressed == 0) {
	red_light();
	jb->active = 0;
	new = g_new(hours,1);
	new->name = g_strdup(jb->name); /* PWR: fix memory leaks */
	new->what = PUNCHOUT;
	new->timestamp = time(NULL);
	hourslist = g_list_append(hourslist, new);
	save_session(NULL,NULL,NULL);
      }

      if (pq_button_pressed >= 0) 
	gnome_dialog_close(GNOME_DIALOG(pq_dialog));
    }
  }
}

gboolean
delevent_handler(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  jobdesc* jb;
  hours *new;
  gint what;
  gint numjobs;
  gint i;

  /* first we check what they want us to do upon quitting */
  switch (logout_pref()) {
  case PUNCHOUT_ALL:
    numjobs = num_jobs();
    for (i = 0; i < numjobs; i++) {
      jb = (jobdesc*)g_list_nth_data(joblist,i);
      if (jb->active) {
	red_light();
	jb->active = 0;
	what = PUNCHOUT;
	new = g_new(hours,1);
	new->name = g_strdup(jb->name); /* PWR: fix memory leaks */
	new->what = what;
	new->timestamp = time(NULL);
	hourslist = g_list_append(hourslist, new);
	/* apparently save_signal no longer works once we've
	 * been asked to remove ourselves from the panel ... 
	 * oh well, we'll just circumvent their abstractions */
	save_session(NULL,NULL,NULL);
      }
    }
    break;
  case PROMPT:
    prompt_quit();
    break;
  }

  return FALSE;
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
