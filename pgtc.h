/*\
 *  pgtc - Pete's Gnome Time Card is a gnome panel applet for keeping
 *  track of hours
 *
 *  $Id: pgtc.h,v 1.4 2000/02/27 21:29:53 prijks Exp $
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
 *  $Log: pgtc.h,v $
 *  Revision 1.4  2000/02/27 21:29:53  prijks
 *  added function prototypes for a variety of new functions, changed
 *  version to 0.2.0 and bumped internal_revision up to 2.
 *
 *  Revision 1.3  2000/02/27 04:01:17  prijks
 *  added prototype for remove_job
 *
 *  Revision 1.2  2000/02/27 01:23:56  prijks
 *  added del_job function prototype
 *
 *
\*/

#ifndef PGTC_H_
#define PGTC_H_

#define APPLET_VERSION_MAJ 0
#define APPLET_VERSION_MIN 2
#define APPLET_VERSION_REV 0

#define INTERNAL_REVISION 2 /* changes only when infrastuctrure
			     * changes break compatibility */

#define PUNCHIN  1
#define PUNCHOUT 2

#define PUNCHOUT_ALL 17
#define PROMPT 18
#define LEAVE_ALL 19

/* about.c */
void about_pgtc(AppletWidget *applet, gpointer data);


/* session.c */
gint save_session(GtkWidget *w, const char *privcfgpath,
		  const char *globcfgpath);
gint load_session(GtkWidget *applet);
gint logout_pref(void);
void set_logout_pref(void);
void new_logout_pref(gint newlp);

/* timing.c */
typedef struct {
  gint active;
  gchar *name;
} jobdesc;

typedef struct {
  gchar *name;
  glong timestamp;
  gint what; /* in or out? */
} hours;

gint punch_callback(void);
gint jobchange_callback(void);
gint num_jobs(void);
gint num_hours(void);
GList* get_joblist(void);
GList* get_hourslist(void);
void set_joblist(GList *l);
void set_hourslist(GList *l);
void create_jobs(void);
void new_job(void);
void del_job(void);
void calc_card(void);
void clear_card(void);
gboolean delevent_handler(GtkWidget *widget, GdkEvent *event, 
			  gpointer user_data);


/* ui.c */
typedef enum {r, g} lc;

gint setup_ui(GtkWidget *applet);
void prop_dialog();
lc light_status(void);
void green_light(void);
void red_light(void);
gchar* get_active_job(void);
void add_job(gchar *j);
void remove_job(gchar *j);
void save_signal(void);


#endif /* PGTC_H_ */

