/******************************************************************************/
/*                                 TASK QUEUE                                 */
/*                                                                            */
/* The routines in this file govern the operation of the task queue.  The     */
/* task queue is examined at the start of each iteration of the event loop to */
/* see if there are any pending tasks.  If there are, the tasks are scheduled */
/* to be run in the coming iteration of the event loop (see loop.c).  The     */
/* task queue is implemented as a list store, so that a tree model can be     */
/* used to edit the contents of the task queue interactively.                 */
/*                                                                            */
/* The following keywords have the stated effect:                             */
/*                                                                            */
/* Object			Sets the name of the object to be observed in the         */
/*                  'Object' field of the FITS keywords on the CCD Camera tab */
/*					and also sets the object name as the base filename.       */
/*                                                                            */
/* BeginSequence	Begins a new sequence of commands.  This can be done at   */
/*					any time.  This resets the counter that records the       */
/*					number of elapsed seconds to zero.                        */
/*                                                                            */
/* BeginLoop n      Begins a repeating sequence of commands for n iterations. */
/*                                                                            */
/* EndLoop          Marks the end of a loop of commands                       */
/*                                                                            */
/* IfTrue n         Executes following statements if n is TRUE                */
/*                                                                            */
/* IfFalse n        Executes following statements if n is FALSE               */
/*                                                                            */
/* EndIf            Terminates an 'If' segement of tasks                      */
/*                                                                            */
/* While n          Loops over a sequence of commands while n is TRUE         */
/*                                                                            */
/* EndWhile         Marks the end of a while loop                             */
/*                                                                            */
/* WaitUntil n		Waits until the sequence counter reaches n seconds, before*/
/*					executing the next command.                               */
/*                                                                            */
/* PauseFor n		Pauses for n seconds before executing the next command.   */
/*                                                                            */
/* At hh:mm:ss		Executes the next task starting at hh:mm:ss.              */
/*                                                                            */
/* Expose type filter time htl vtl hbr vbr h_bin v_bin temp                   */
/*					Starts a CCD exposure of time seconds, using a window on  */
/*					the chip defined by the horizontal and vertical top left, */
/*					and horizontal and vertical bottom right coordinates, with*/
/*					horizontal and vertical binning of h_bin and v_bin.       */
/*                                                                            */
/* FocusTo position Moves focuser to given position                           */
/*                                                                            */
/* FocusMove steps  Moves focuser by given number of steps (+/-)              */
/*                                                                            */
/* AugOn			Turn autoguider on                                        */
/*                                                                            */
/* AugOff			Turn autoguider off                                       */
/*                                                                            */
/* GuideStart		Start autoguiding                                         */
/*                                                                            */
/* GuideStop		Stop autoguiding                                          */
/*                                                                            */
/* RecordStart      Start video recording (with Unicap device)                */
/*                                                                            */
/* RecordStop       Stop video recording (with Unicap device)                 */
/*                                                                            */
/* YellowButton     Presses the KIWI-OSD yellow button via the relay box      */
/*                  (Private option - compile with CPPFLAGS="-DYELLOW_BUTTON)"*/
/*                                                                            */
/* GoTo RA Dec		Instructs the telescope to go to RA, Dec                  */
/*                                                                            */
/* Move RA Dec      Instruct telescope to move by RA, Dec arcminutes          */
/*                                                                            */
/* Exec script      Execute the given script, waiting for it to finish        */
/*                                                                            */
/* ExecAsync script Execute the given script, continuing with next task       */
/*                                                                            */
/* SetParam param val                                                         */
/*                  Sets parameter param to value val                         */
/*                                                                            */
/* ParkMount		Parks the telescope in the home position                  */
/*                                                                            */
/* Shutdown         Exits immediately, and schedules system shutdown after    */
/*                  one minute                                                */
/*                                                                            */
/* WarmRestart		Instructs the telescope to warm restart                   */
/*                                                                            */
/* Exit             Immediately exits execution of the task list              */
/*                                                                            */
/* Copyright (C) 2009 - 2013  Edward Simonson                                 */
/*                                                                            */
/* This file is part of GoQat.                                                */
/*                                                                            */
/* GoQat is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 3 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/* GNU General Public License for more details.                               */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program; if not, see <http://www.gnu.org/licenses/> .      */
/*                                                                            */
/******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glib/gstdio.h>

#define GOQAT_TASKS
#include "interface.h"

#define T_NULL     0x0000        /* No task actions required                  */
#define T_START    0x0001        /* Start execution at first task             */
#define T_PAUSE    0x0002        /* Pause execution of tasks                  */
#define T_STOP     0x0004        /* Stop execution of tasks                   */
#define T_NEXT     0x0010        /* Execute next task                         */
#define T_EXEC     0x0020        /* Task is currently executing               */
#define T_TESTONLY 0x0040        /* Task list is in TestOnly mode             */

#define MAX_NEST   10            /* Max. depth of nested loops                */
#define MAX_IF     100           /* Max. no of 'If' statements                */
#define MAX_WHILE  100           /* Max. no of 'While' loops                  */

#define EL         1             /* Markers for loop integrity checking       */
#define EI         2
#define EW         3

#define SCRIPT_RESULTS "script.results" /* File for script results            */
#define SCRIPT_DONE    "script.done"    /* File to be created when script done*/

/* Definition of task queue items.  TAG is a hidden value (not displayed on   */
/* the Tasks tab), for book-keeping of If and While statements in             */
/* tasks_execute_tasks.                                                       */

enum TaskQ {TASK, EXP_TYPE, FILTER, TASK_TIME, H1, V1, H2, V2, 
	        H_BIN, V_BIN, CCDTEMP, NUM, RA, DEC, VALUE, TAG, 
            N_TASKCOLS};

static struct TaskFlags {        /* Task control flags                        */
	guint Task;
	guint Status;
	enum TaskTypes Type;
} Flags;

static struct Tasks {            /* Tasks                                     */
	guint now;                   /* Time at which pause started (milliseconds)*/
	gdouble time;                /* Time for At, Pause and WaitUntil tasks    */
	gchar *type;                 /* Task type                                 */
} task;

static struct Loops {            /* Structure for simple loop control         */
	GtkTreePath *path[MAX_NEST]; /* Path to BeginLoop statement in tree       */
	gushort repeat[MAX_NEST];    /* Number of repeats of loop to make         */
	gushort count[MAX_NEST];     /* Number of current loop iteration          */
	gshort depth;                /* Current nesting depth                     */
} loop;

static FILE *sp = NULL;          /* Script execution file pipe (popen)        */	
static GtkTreeView *trvTasks;    /* Tasks tree view                           */
static GtkListStore *lisTasks;   /* List store - tasks data                   */
static GtkTreePath *path_endif[MAX_IF]; /* Path to 'EndIf' statements         */
static GtkTreePath *path_while[MAX_WHILE]; /* Path to 'While' statements      */
static GtkTreePath *path_endwhile[MAX_WHILE]; /* Path to 'EndWhile' statements*/
static time_t mtime;             /* Modification time for 'watch' file        */
static gushort loop_check[2 * (MAX_IF + MAX_WHILE)]; /* Loop integrity check  */
static guint seq_expno = 1;      /* Exposure number of current sequence       */
static guint seq_start = 0;      /* Start time (ms) of current sequence       */
static gint ip, iep, wp, wep;    /* Counters for 'If', 'While' loops          */
static gint lc, lec;             /* Counter for checking integrity of 'loops' */
static gchar *sr = NULL;         /* Script results file                       */
static gchar *sd = NULL;         /* Script done file                          */
extern gchar *WatchFile;         /* File to watch for incoming tasks          */

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void tasks_init (GtkBuilder *xml);
void tasks_start (void);
void tasks_pause (gboolean pause);
void tasks_stop (void);
void tasks_task_done (enum TaskTypes Task);
void tasks_move_up (void);
void tasks_move_down (void);
void tasks_delete (void);
void tasks_clear (void);
void tasks_add_Object (gchar *str);
void tasks_add_BeginSequence (void);
void tasks_add_WaitUntil (gchar *str);
void tasks_add_PauseFor (gchar *str);
void tasks_add_At (gchar *str);
void tasks_add_Exposure (gchar **stra);
void tasks_add_FocusTo (gchar *str);
void tasks_add_FocusMove (gchar *str);
void tasks_add_BeginLoop (gchar *str);
void tasks_add_EndLoop (void);
void tasks_add_IfTrue (gchar *str);
void tasks_add_IfFalse (gchar *str);
void tasks_add_EndIf (void);
void tasks_add_While (gchar *str);
void tasks_add_EndWhile (void);
void tasks_add_Aug (gboolean on);
void tasks_add_Guide (gboolean start);
void tasks_add_GoTo (gchar *str1, gchar *str2);
void tasks_add_Move (gchar *str1, gchar *str2);
void tasks_add_Exec (gchar *filename, gboolean Exec);
void tasks_add_SetParam (gchar *param, gchar *str);
void tasks_add_WarmRestart (void);
void tasks_add_ParkMount (void);
void tasks_add_Record (gboolean record);
void tasks_add_YellowButton (void);
void tasks_add_Shutdown (void);						 
void tasks_add_Exit (void);						 
void tasks_activate_watch (void);
void tasks_watch_file (void);
gboolean tasks_execute_tasks (gboolean TestOnly, gboolean *error);
guint tasks_get_status (void);
gboolean tasks_task_wait (void);
gboolean tasks_task_pause (void);
gboolean tasks_task_at (void);
gboolean tasks_load_file (gchar *filename);
gboolean tasks_write_file (gchar *filename);
static void tasks_script_execute (gboolean IsExec, gchar *script);
gboolean tasks_script_done (void);
static GtkTreeIter *get_selected_iter (GtkTreeIter *iter);
static void select_current_row (GtkTreeIter *iter);
static gchar *next_token (gchar **tokens, gboolean Init);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void tasks_init (GtkBuilder *xml)
{
	/* Initialise the list store and tree view */
	
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	
	/* Set up the list store for holding the task list */
	
	lisTasks = gtk_list_store_new (N_TASKCOLS,   /* One entry for each element*/
	 							   G_TYPE_STRING,/*  of the Tasks structure   */
								   G_TYPE_STRING,
								   G_TYPE_STRING,	
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,
								   G_TYPE_STRING,	
								   G_TYPE_STRING,
								   G_TYPE_INT);
	
	/* Associate the tree view with the list model */
	
	trvTasks = GTK_TREE_VIEW (xml_get_widget (xml, "trvTasks"));
	gtk_tree_view_set_model (trvTasks, GTK_TREE_MODEL (lisTasks));

	/* Set the columns to be displayed in the tree view */

	renderer = gtk_cell_renderer_text_new ();	
	column = gtk_tree_view_column_new_with_attributes ("Task",
                                                      renderer,
                                                      "text", TASK,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("ExpType",
                                                      renderer,
                                                      "text", EXP_TYPE,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Filter",
                                                      renderer,
                                                      "text", FILTER,
                                                      NULL);													  
	gtk_tree_view_append_column (trvTasks, column);
	renderer = gtk_cell_renderer_text_new ();
	//gkt_cell_renderer_set_alignment (renderer, 1.0, 0.0); /* right align */
	g_object_set (renderer, "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("Seconds",
                                                      renderer,
                                                      "text", TASK_TIME,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("H1",
                                                      renderer,
                                                      "text", H1,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("V1",
                                                      renderer,
                                                      "text", V1,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("H2",
                                                      renderer,
                                                      "text", H2,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("V2",
                                                      renderer,
                                                      "text", V2,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Hbin",
                                                      renderer,
                                                      "text", H_BIN,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Vbin",
                                                      renderer,
                                                      "text", V_BIN,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	//gkt_cell_renderer_set_alignment (renderer, 1.0, 0.0); /* right align */
	g_object_set (renderer, "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes ("degC.",
                                                      renderer,
                                                      "text", CCDTEMP,
                                                      NULL);													  
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("No.",
                                                      renderer,
                                                      "text", NUM,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("RA/Time",
                                                      renderer,
                                                      "text", RA,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
		renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Dec",
                                                      renderer,
                                                      "text", DEC,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Value",
                                                      renderer,
                                                      "text", VALUE,
                                                      NULL);
	gtk_tree_view_append_column (trvTasks, column);
	
	/* Initialise task flags */
	
	Flags.Task = T_NULL;
}

void tasks_start (void)
{
	/* This routine is called to begin execution at the first task in
	 * the list.
	 */
	
	Flags.Task |= T_START;
	Flags.Task &= ~(T_STOP | T_EXEC);
	init_task_params ();
	
	ip = -1;   /* Counters for checking if/while/loop statements */
	iep = -1;
	wp = -1;
	wep = -1;
	lc = -1;
	lec = -1;
	
	loop.depth = -1;  /* Initial BeginLoop depth */
	seq_expno = 1;    /* First exposure in sequence */
	seq_start = loop_elapsed_since_first_iteration ();
}

void tasks_pause (gboolean pause)
{
	/* This routine is called to pause/continue the task list */
	
	if (pause)
		Flags.Task |= T_PAUSE;
	else
		Flags.Task &= ~T_PAUSE;
}

void tasks_stop (void)
{
	/* This routine is called to stop execution of tasks in the list */
	
	gint i;

	for (i = 0; i < MAX_NEST; i++) {
		if (loop.path[i]) {
			gtk_tree_path_free (loop.path[i]);
			loop.path[i] = NULL;
		}
	}
	
	for (i = 0; i < MAX_IF; i++) {
		if (path_endif[i]) {
			gtk_tree_path_free (path_endif[i]);
			path_endif[i] = NULL;
		}
	}
	
	for (i = 0; i < MAX_WHILE; i++) {
		if (path_while[i]) {
			gtk_tree_path_free (path_while[i]);
			path_while[i] = NULL;
		}
		if (path_endwhile[i]) {
			gtk_tree_path_free (path_endwhile[i]);
			path_endwhile[i] = NULL;
		}
	}
	
	for (i = 0; i < 2 * (MAX_IF + MAX_WHILE); i++) {
		loop_check[i] = 0;
	}
	
	Flags.Task |= T_STOP;
	Flags.Task &= ~T_NEXT;
	free_task_params ();
}

void tasks_task_done (enum TaskTypes Type)
{
	/* This routine is called by an executing task when it has completed */
	
	if (Flags.Type == Type)
		Flags.Task &= ~T_EXEC;
}

void tasks_move_up (void)
{
	/* Move the selected task up in the list and keep the task selected */
	
	GtkTreeIter iter, p_iter;
	GtkTreePath *path;
	gboolean valid;
	
	if (get_selected_iter (&iter)) {
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (lisTasks), &iter);
		gtk_tree_path_prev (path);
		gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), &p_iter, path);
		gtk_list_store_move_before (lisTasks, &iter, &p_iter);
		if ((valid = (iter.stamp != 0 ? TRUE : FALSE)))
			select_current_row (&iter);
		gtk_tree_path_free (path);
	}	
}

void tasks_move_down (void)
{
	/* Move the selected task down in the list and keep the task selected */
	
	GtkTreeIter iter, p_iter;
	GtkTreePath *path;
	gboolean valid;
	
	if (get_selected_iter (&iter)) {
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (lisTasks), &iter);
		gtk_tree_path_next (path);
		gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), &p_iter, path);
		gtk_list_store_move_after (lisTasks, &iter, &p_iter);
		if ((valid = (iter.stamp != 0 ? TRUE : FALSE)))
			select_current_row (&iter);
		gtk_tree_path_free (path);
	}	
}

void tasks_delete (void)
{
	/* Delete the selected task from the list */
	
	GtkTreeIter iter;
	gboolean valid;
	
	if (get_selected_iter (&iter)) {
		gtk_list_store_remove (lisTasks, get_selected_iter (&iter));
		if ((valid = (iter.stamp != 0 ? TRUE : FALSE)))
			select_current_row (&iter);
	}		
}

void tasks_clear (void)
{
	/* This routine clears all the tasks from the list */

	GtkTreeIter iter;
	
	while (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (lisTasks), &iter))
		gtk_list_store_remove (lisTasks, &iter);
	
	seq_expno = 1;
	seq_start = loop_elapsed_since_first_iteration ();	
}

void tasks_add_Object (gchar *str)
{
	/* Adds an object name command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "Object",
						VALUE, str,
						-1);	
}

void tasks_add_BeginSequence (void)
{
	/* Adds a BeginSequence marker to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);		
	gtk_list_store_set (lisTasks, &iter,
						TASK, "BeginSequence",
						-1);
}

void tasks_add_WaitUntil (gchar *str)
{
	/* Adds a WaitUntil command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);			
	gtk_list_store_set (lisTasks, &iter,
						TASK, "WaitUntil",
						TASK_TIME, str,
						-1);
}

void tasks_add_PauseFor (gchar *str)
{
	/* Adds a PauseFor command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);			
	gtk_list_store_set (lisTasks, &iter,
						TASK, "PauseFor",
						TASK_TIME, str,
						-1);
}

void tasks_add_At (gchar *str)
{
	/* Adds an At command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "At",
						RA, str,
						-1);	
}

void tasks_add_Exposure (gchar **stra)
{
	/* Adds an exposure to the task list */
	
	GtkTreeIter s_iter, iter;
	gchar *str_num;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);	
	str_num = g_strdup_printf ("%d", seq_expno++);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "Expose",
						EXP_TYPE, stra[0],
						FILTER, stra[1],
						TASK_TIME, stra[2],
						H1, stra[3],
						V1, stra[4],
						H2, stra[5],
						V2, stra[6],
						H_BIN, stra[7],
						V_BIN, stra[8],
						CCDTEMP, stra[9],
						NUM, str_num,
						-1);
	g_free (str_num);
}

void tasks_add_FocusTo (gchar *str)
{
	/* Adds a command to move focuser to given position to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "FocusTo",
						VALUE, str,
						-1);	
}

void tasks_add_FocusMove (gchar *str)
{
	/* Adds a command to move focuser by given amount to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "FocusMove",
						VALUE, str,
						-1);	
}

void tasks_add_BeginLoop (gchar *str)
{
	/* Adds a command to begin a loop to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "BeginLoop",
						NUM, str,
						-1);	
}

void tasks_add_EndLoop (void)
{
	/* Adds a command to end a loop to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "EndLoop",
						-1);	
}

void tasks_add_IfTrue (gchar *str)
{
	/* Adds a command to begin an If segment to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "IfTrue",
						VALUE, str,
						-1);	
}

void tasks_add_IfFalse (gchar *str)
{
	/* Adds a command to begin an If segment to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "IfFalse",
						VALUE, str,
						-1);	
}

void tasks_add_EndIf (void)
{
	/* Adds a command to end an If segment to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "EndIf",
						-1);	
}

void tasks_add_While (gchar *str)
{
	/* Adds a command to start a while loop to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "While",
						VALUE, str,
						-1);	
}

void tasks_add_EndWhile (void)
{
	/* Adds a command to end a while loop to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "EndWhile",
						-1);	
}

void tasks_add_Aug (gboolean on)
{
	/* Adds a command to turn autoguider on/off to task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, (on ? "AugOn" : "AugOff"),
						-1);	
}

void tasks_add_Guide (gboolean start)
{
	/* Adds a command to start/stop autoguiding to task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, (start ? "GuideStart" : "GuideStop"),
						-1);		
}

void tasks_add_GoTo (gchar *str1, gchar *str2)
{
	/* Adds a GoTo command to the task list */

	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "GoTo",
						RA, str1,
						DEC, str2,
						-1);	
}

void tasks_add_Move (gchar *str1, gchar *str2)
{
	/* Adds a Move command to the task list */

	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "Move",
						RA, str1,
						DEC, str2,
						-1);	
}

void tasks_add_Exec (gchar *filename, gboolean Exec)
{
	/* Adds an Exec command if Exec == TRUE
	 * Adds an ExecAsync command if Exec == FALSE
	 */
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, Exec ? "Exec" : "ExecAsync",
						VALUE, filename,
						-1);	
}

void tasks_add_SetParam (gchar *param, gchar *str)
{
	/* Adds a SetParam command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "SetParam",
						NUM, param,
						VALUE, str,
						-1);	
}

void tasks_add_WarmRestart (void)
{
	/* Adds a WarmRestart command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "WarmRestart",
						-1);	
}

void tasks_add_ParkMount (void)
{
	/* Adds a ParkMount command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "ParkMount",
						-1);
}

void tasks_add_Record (gboolean record)
{
	/* Adds a command to start/stop recording to task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, (record ? "RecordStart" : "RecordStop"),
						-1);	
}

void tasks_add_YellowButton (void)
{
	/* Adds a YellowButton command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "YellowButton",
						-1);	
}

void tasks_add_Shutdown (void)
{
	/* Adds a Shutdown command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "Shutdown",
						-1);		
}

void tasks_add_Exit (void)
{
	/* Adds an Exit command to the task list */
	
	GtkTreeIter s_iter, iter;
	
	gtk_list_store_insert_after (lisTasks, &iter, get_selected_iter (&s_iter));
	select_current_row (&iter);
	gtk_list_store_set (lisTasks, &iter,
						TASK, "Exit",
						-1);		
}

void tasks_activate_watch (void)
{
	/* Initialise the file modification time variable */
	
	time (&mtime);
}

void tasks_watch_file (void)
{
	/* Watch the specified file, load the contents into the task list if the
	 * file is updated and execute the tasks.
	 */
	
	struct stat fileinfo;
	gboolean error;
	
	/* If the task list is already running, return without doing anything */
	
	if (tasks_get_status () & TSK_ACTIVE)
		return;
		
	/* If the 'watch' file doesn't exist, return without doing anything */
	
	if (!g_file_test (WatchFile, G_FILE_TEST_EXISTS))
		return;
		
	/* Compare the latest modification time of the file with its previous
	 * value.  If it's been modified, load the contents into the task list
	 * and execute it.
	 */
	
	if (stat (WatchFile, &fileinfo) < 0) {
		L_print ("{r}Unable to get modification time for watched file\n");
		return;
	}
	
	if (difftime (fileinfo.st_mtime, mtime) > 0.0) { /* File has been modified*/
		mtime = fileinfo.st_mtime;
		tasks_clear ();
		tasks_load_file (WatchFile);
		
		/* Check for errors in task list */
		
		L_print ("{b}Checking task list for errors...\n");
		tasks_start ();
		while (tasks_execute_tasks (TRUE, &error))
			;
		
		/* Start tasks if no error */
		
		if (!error) {
			//L_print ("{b}Task list OK\n");
			set_task_buttons (TRUE);
			tasks_start ();
			//L_print ("{b}\n\nTask execution STARTED for GoQat %s...\n",VERSION);
		} else
			L_print ("{o}Task list contains errors. Can't execute tasks!\n");
	}
}

gboolean tasks_execute_tasks (gboolean TestOnly, gboolean *error)
{
	/* Execute the next task in the list, if no other tasks are currently
	 * in progress.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	struct cam_img *aug = get_aug_image_struct ();
	
	struct exposure_data exd;
	struct focus f;
	static GtkTreeIter iter;
	GtkTreePath *path;
	guint elapsed;
	static gint line = 0;
	gint i, n, o, intval;
	gdouble val, val1;
	static gchar filter[10];
	static gchar *stra[15];
	static gchar sRA[9], sDec[10];
	static gchar obj[128];
	gchar *tp[NUMTPARAMS];
	gboolean IsExec, BadFilter;
	static gboolean WhileBreak = FALSE;
	static gboolean Error = FALSE;
	static gboolean GetNextIter = TRUE;
	
	elapsed = (guint) floor ((loop_elapsed_since_first_iteration () - 
							                               seq_start) / 1000.0);
	set_elapsed_time (elapsed);
	
	/* Return if task execution is stopped */

	if (Flags.Task & T_STOP)
		return FALSE;

	/* Return if task execution is paused */
	
	if (Flags.Task & T_PAUSE)
		return TRUE;	

	/* Return if a task is currently executing */

	if (Flags.Task & T_EXEC)
		return TRUE;

	/* Get the first task */
	
	if (Flags.Task & T_START) {
		Flags.Task &= ~T_START;
		if (TestOnly)
			Flags.Task |= T_TESTONLY;
		else
			Flags.Task &= ~T_TESTONLY;
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (lisTasks), &iter)) {
			Flags.Task |= T_NEXT;
		    line = 0;
			Error = FALSE;
		} else
			Flags.Task &= ~T_NEXT;
	}

	if (Flags.Task & T_NEXT) {
		line++;
		
		/* Select the row */
	
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (lisTasks), &iter);
		gtk_tree_view_set_cursor (trvTasks, path, NULL, FALSE);
	
		/* Scroll selected row to top of window */

		gtk_tree_view_scroll_to_cell (trvTasks, path, NULL, TRUE, 0.0, 0.0);
		gtk_tree_path_free (path);
		
		/* Get the task type */
			
		gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
							TASK, &task.type,
							-1);			

		/* Schedule the task to be done */	
		
		if (!(strcmp (task.type, "Object"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[0],
								-1);
			strcpy (obj, g_strstrip (stra[0]));
			stra[0] = get_task_param_value (FALSE, g_strstrip (stra[0]), &n);
			strcpy (obj, stra[0]);
			if (!strcmp (obj, "")) {
				L_print ("{r}Task 'Object' has missing name at line %d\n",
						 line);
				Error = TRUE;
			}
			if (n < 0)  /* Invalid task parameter */
				Error = TRUE;
			if (!TestOnly && !Error) {
				L_print ("{b}TASK Object %s\n", obj);			
				set_entry_string ("txtObject", obj);
				set_entry_string ("txtCCDFile", obj);
				set_entry_string ("txtAUGFile", obj);
			}
		} else if (!(strcmp (task.type, "BeginSequence"))) {
			if (!TestOnly && !Error) {
				L_print ("{b}TASK BeginSequence\n");			
				seq_start = loop_elapsed_since_first_iteration ();
			}
		} else if (!(strcmp (task.type, "WaitUntil"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								TASK_TIME, &stra[0],
								-1);
			if (!get_entry_float (g_strstrip (stra[0]), 
								  WAI_MIN, WAI_MAX, WAI_DEF, 
								  TSK_PAGE, &task.time)) {
				L_print ("{r}Task 'WaitUntil' has invalid number of "
						 "seconds at line %d\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK WaitUntil %.1fs\n", task.time);			
				Flags.Type = T_WTU;
				Flags.Task |= T_EXEC;
				loop_tasks_wait ();
			}
		} else if (!(strcmp (task.type, "PauseFor"))) {	
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								TASK_TIME, &stra[0],
								-1);
			if (!get_entry_float (g_strstrip (stra[0]), 
							      WAI_MIN, WAI_MAX, WAI_DEF, 
							      TSK_PAGE, &task.time)) {
				L_print ("{r}Task 'PauseFor' has invalid number of seconds "
						 "at line %d\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK PauseFor %.1fs (starting at %ds elapsed)\n", 
														    task.time, elapsed);			
				task.now = loop_elapsed_since_first_iteration ();
				Flags.Type = T_PSF;
				Flags.Task |= T_EXEC;
				loop_tasks_pause ();
			}
		} else if (!(strcmp (task.type, "At"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								RA, &stra[0],
								-1);
			strcpy (sRA, g_strstrip (stra[0]));
			if (!check_format (TRUE, sRA)) {
				L_print ("{r}Task 'At' has invalid time at line %d\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK At %s\n", sRA);
				task.time = (gdouble) stof_RA (sRA) * 12.0 / M_PI;
				Flags.Type = T_EAT;
				Flags.Task |= T_EXEC;
				loop_tasks_at ();
			}
		} else if (!(strcmp (task.type, "Expose"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								EXP_TYPE, &stra[0],
								FILTER, &stra[1],
								TASK_TIME, &stra[2],
								H1, &stra[3],
								V1, &stra[4],
								H2, &stra[5],
								V2, &stra[6],
								H_BIN, &stra[7],
								V_BIN, &stra[8],
								CCDTEMP, &stra[9],
								NUM, &stra[10],
								-1);
			if (!menu.OpenCCDCam) {
				L_print ("{r}Task 'Expose' at line %d requires CCD camera "
						 "to be open\n", line);
				Error = TRUE;
			}
			
			if (menu.OpenCCDCam) {
				
				exd.ExpType = g_strstrip (stra[0]);
				if (strcmp ("TARGET", exd.ExpType) && 
					strcmp ("FLAT", exd.ExpType) &&
					strcmp ("DARK", exd.ExpType) && 
					strcmp ("BIAS", exd.ExpType)) {
						L_print ("{r}Task 'Expose' has invalid exposure "
								 "type at line %d\n", line);
						Error = TRUE;
				}
				
				exd.filter = g_strstrip (stra[1]);
				BadFilter = TRUE;
				for (i = 0; i < MAX_FILTERS; i++) {
					if (!strcmp (exd.filter, get_filter_info (ccd, i, filter, 
															             &o))) {
						BadFilter = FALSE;
						break;
					}
				}
				if (BadFilter) {
					L_print ("{r}Task 'Expose' has invalid filter type at "
							 "line %d\n", line);
					Error = TRUE;
				}
				
				if (!get_entry_float (g_strstrip (stra[2]), 
									  ccd->cam_cap.min_exp, 
									  ccd->cam_cap.max_exp, 1.0, TSK_PAGE, 
									  &exd.req_len)) {
					L_print ("{r}Task 'Expose' has invalid exposure time at "
							 "line %d\n", line);
					Error = TRUE;
				}
				
				if (!get_entry_int (g_strstrip (stra[3]), 1,
									ccd->cam_cap.max_h - 1, 1, TSK_PAGE, 
									&intval)) {
					L_print ("{r}Task 'Expose' has invalid H1 coordinate at "
							 "line %d\n", line);
					Error = TRUE;
				} else
				    exd.h_top_l = (gushort) intval;

				if (!get_entry_int (g_strstrip (stra[4]), 1,
									ccd->cam_cap.max_v - 1, 1, TSK_PAGE,
									&intval)) {
					L_print ("{r}Task 'Expose' has invalid V1 coordinate at "
							 "line %d\n", line);
					Error = TRUE;
				} else
					exd.v_top_l = (gushort) intval;
				
				if (!get_entry_int (g_strstrip (stra[5]), exd.h_top_l + 1, 
									ccd->cam_cap.max_h, ccd->cam_cap.max_h, 
								    TSK_PAGE, &intval)) {
					L_print ("{r}Task 'Expose' has invalid H2 coordinate at "
							 "line %d\n", line);
					Error = TRUE;
				} else
				    exd.h_bot_r = (gushort) intval;				
				
				if (!get_entry_int (g_strstrip (stra[6]), exd.v_top_l + 1,
									ccd->cam_cap.max_v, ccd->cam_cap.max_v,
									TSK_PAGE, &intval)) {
					L_print ("{r}Task 'Expose' has invalid V2 coordinate at "
							 "line %d\n", line);
					Error = TRUE;
				} else
				    exd.v_bot_r = (gushort) intval;	
				
				if (!get_entry_int (g_strstrip (stra[7]), 1,
									ccd->cam_cap.max_binh, 1, TSK_PAGE, 
									&intval)) {
					L_print ("{r}Task 'Expose' has invalid H-bin coordinate at "
							 "line %d\n", line);
					Error = TRUE;
				} else
				    exd.h_bin = (gushort) intval;
				
				if (!get_entry_int (g_strstrip (stra[8]), 1,
									ccd->cam_cap.max_binv, 1, TSK_PAGE, 
									&intval)) {
					L_print ("{r}Task 'Expose' has invalid V-bin coordinate at "
							 "line %d\n", line);
					Error = TRUE;
				} else
				    exd.v_bin = (gushort) intval;

				if (!get_entry_float (g_strstrip (stra[9]), TPR_MIN, TPR_MAX, 
									  TPR_DEF, TSK_PAGE, &exd.ccdtemp)) {
					L_print ("{r}Task 'Expose' has invalid CCD temperature at "
							 "line %d\n", line);
					Error = TRUE;
				}
			}
			
			/* Start the exposure */
			
			if (!TestOnly && !Error) {
				L_print ("{b}TASK Expose (%s)\n", stra[10]);
				ccdcam_set_exposure_data (&exd);
				Flags.Type = T_EXP;
				Flags.Task |= T_EXEC;
				loop_ccd_start ();
			}
		} else if (!(strcmp (task.type, "FocusTo"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[0],  
								-1);
			if (!menu.OpenFocusPort) {
				L_print ("{r}Task 'FocusTo' at line %d requires focuser link "
						 "to be open\n", line);
				Error = TRUE;
			} else {
				f.cmd = FC_MAX_TRAVEL_GET;
				focus_comms->focus (&f);
				if (!get_entry_int (g_strstrip (stra[0]), 
									1, f.max_travel, f.max_travel / 2, 
									TSK_PAGE, &f.move_to)) {
				    L_print ("{r}Task 'FocusTo' has invalid focuser position "
							 "at line %d\n", line);
				    Error = TRUE;
			    }
			}
			
			if (!TestOnly && !Error) {
				L_print ("{b}TASK FocusTo %d\n", f.move_to);
				Flags.Type = T_FOC;
				Flags.Task |= T_EXEC;
				f.cmd = FC_MOVE_TO;
				focus_comms->focus (&f);
				loop_focus_check_done ();
			}
		} else if (!(strcmp (task.type, "FocusMove"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[0],  
								-1);
			if (!menu.OpenFocusPort) {
				L_print ("{r}Task 'FocusMove' at line %d requires focuser link "
						 "to be open\n", line);
				Error = TRUE;
			} else {
				f.cmd = (FC_MAX_TRAVEL_GET | FC_CUR_POS_GET);
				focus_comms->focus (&f);
				if (!get_entry_int (g_strstrip (stra[0]), 
									-f.max_travel, f.max_travel, 0, 
									TSK_PAGE, &f.move_by)) {
				    L_print ("{r}Task 'FocusMove' has invalid focuser motion "
							 "value at line %d\n", line);
				    Error = TRUE;
			    }
				if ((f.cur_pos + f.move_by) < 1 || 
					(f.cur_pos + f.move_by) > f.max_travel) {
				    L_print ("{r}Task 'FocusMove' has invalid focuser motion "
							 "value at line %d\n", line);
				    Error = TRUE;
			    }
			}
			
			if (!TestOnly && !Error) {
				L_print ("{b}TASK FocusMove %d\n", f.move_by);
				Flags.Type = T_FOC;
				Flags.Task |= T_EXEC;
				f.cmd = FC_MOVE_BY;
				focus_comms->focus (&f);
				loop_focus_check_done ();
			}
		} else if (!(strcmp (task.type, "BeginLoop"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								NUM, &stra[0],  
								-1);
			if (!get_entry_int (g_strstrip (stra[0]), 
							    REP_MIN, REP_MAX, REP_DEF, 
							    TSK_PAGE, &intval)) {
				L_print ("{r}Task 'BeginLoop' has invalid repeat value at "
						 "line %d\n", line);
				Error = TRUE;
			}
			
			++loop.depth;
			if (TestOnly) {
				if (loop.depth >= MAX_NEST) {
					L_print ("{r}Task 'BeginLoop' exceeds maximum nesting "
							 "depth at line %d\n", line);
					Error = TRUE;
				}
				loop_check[++lc] = EL;
				lec = lc;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK BeginLoop\n");
				loop.repeat[loop.depth] = (gushort) intval;
				loop.count[loop.depth] = 1;
				loop.path[loop.depth] = 
					 gtk_tree_model_get_path (GTK_TREE_MODEL (lisTasks), &iter);
				L_print ("Loop depth %d, beginning iteration %d...\n",
								        loop.depth + 1, loop.count[loop.depth]);
			}
		} else if (!(strcmp (task.type, "EndLoop"))) {
			if (TestOnly) {
				++lec;
				while (--lec >= 0) {
					if (loop_check[lec]) {
						if (loop_check[lec] == EL) {
							loop_check[lec] = 0;
						} else {
							L_print ("{r}Unclosed BeginLoop, If or While "
									 "statement at line %d\n", line);
							Error = TRUE;
						}
						break;
					}
				}
				if (loop.depth < 0) {
					L_print ("{r}Task 'EndLoop' has no corresponding "
							 "'BeginLoop' at line %d\n", line);
					Error = TRUE;
				}
				loop.depth--;
			} 
			if (!TestOnly && !Error) {
				L_print ("{b}TASK EndLoop\n");
				if (loop.depth > -1) {
					if (loop.count[loop.depth]++ < loop.repeat[loop.depth]){
						if (loop.path[loop.depth] != NULL)
							gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), 
													 &iter, 
													 loop.path[loop.depth]);
						L_print ("Loop depth %d, beginning iteration %d...\n",
								 loop.depth + 1, loop.count[loop.depth]);
					} else {
						L_print ("Loop depth %d, iterations completed\n", 
															    loop.depth + 1);
						gtk_tree_path_free (loop.path[loop.depth]);
						loop.path[loop.depth--] = NULL;
					}
				}
		    }
		} else if (!(strcmp (task.type, "IfTrue"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[0],
								-1);
			if (!get_entry_int (g_strstrip (stra[0]), 
							    G_MININT, G_MAXINT, 0, 
							    TSK_PAGE, &intval)) {
				L_print ("{r}Task 'IfTrue' has invalid parameter at "
						 "line %d\n", line);
				Error = TRUE;
			}
			if (TestOnly) {
				if (++ip >= MAX_IF) {  /* ip set to -1 initially */
					L_print ("{r}Task 'IfTrue' exceeds maximum number of "
							 "statements at line %d\n", line);
					Error = TRUE;
				}
				gtk_list_store_set (lisTasks, &iter,
									TAG, ip,
									-1);
				iep = ip;
				loop_check[++lc] = EI;
				lec = lc;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK IfTrue %d\n", intval);			
				if (!intval) {
					gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
										TAG, &ip,
										-1);
					gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), 
											 &iter, 
											 path_endif[ip]);
					GetNextIter = FALSE;
				}
			}
		} else if (!(strcmp (task.type, "IfFalse"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[0],
								-1);
			if (!get_entry_int (g_strstrip (stra[0]), 
								G_MININT, G_MAXINT, 0, 
								TSK_PAGE, &intval)) {
				L_print ("{r}Task 'IfFalse' has invalid parameter at "
						 "line %d\n", line);
				Error = TRUE;
			}
			if (TestOnly) {
				if (++ip >= MAX_IF) {  /* ip set to -1 initially */
					L_print ("{r}Task 'IfFalse' exceeds maximum number of "
							 "statements at line %d\n", line);
					Error = TRUE;
				}
				gtk_list_store_set (lisTasks, &iter,
									TAG, ip,
									-1);
				iep = ip;
				loop_check[++lc] = EI;
				lec = lc;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK IfFalse %d\n", intval);			
				if (intval) {
					gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
										TAG, &ip,
										-1);
					gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), 
											 &iter, 
											 path_endif[ip]);
					GetNextIter = FALSE;
				}
			}
		} else if (!(strcmp (task.type, "EndIf"))) {
			if (TestOnly) {
				++lec;
				while (--lec >= 0) {
					if (loop_check[lec]) {
						if (loop_check[lec] == EI) {
							loop_check[lec] = 0;
						} else {
							L_print ("{r}Unclosed BeginLoop, If or While "
									 "statement at line %d\n", line);
							Error = TRUE;
						}
						break;
					}
				}
				path = gtk_tree_model_get_path(GTK_TREE_MODEL (lisTasks),&iter);
				while (iep >= 0) {
					if (!path_endif[iep]) {
						path_endif[iep] = path;
						gtk_list_store_set (lisTasks, &iter,
											TAG, iep,
											-1);
						break;
					}
					iep--;
				}
				if (iep < 0) {
					L_print ("{r}Task 'EndIf' has no corresponding 'If' "
							 "at line %d\n", line);
					Error = TRUE;
				}
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK EndIf\n");			
			}
		} else if (!(strcmp (task.type, "While"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[0],
								-1);
			if (!get_entry_int (g_strstrip (stra[0]), 
							    G_MININT, G_MAXINT, 0, 
							    TSK_PAGE, &intval)) {
				L_print ("{r}Task 'While' has invalid parameter at "
						 "line %d\n", line);
				Error = TRUE;
			}
			if (TestOnly) {
				if (++wp >= MAX_WHILE) {  /* wp set to -1 initially */
					L_print ("{r}Task 'While' exceeds maximum number of "
							 "statements at line %d\n", line);
					Error = TRUE;
				}
				gtk_list_store_set (lisTasks, &iter,
									TAG, wp,
									-1);
				wep = wp;
				path_while[wp] = gtk_tree_model_get_path (
												GTK_TREE_MODEL(lisTasks),&iter);
				loop_check[++lc] = EW;
				lec = lc;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK While %d\n", intval);			
				gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
									TAG, &wp,
									-1);
				wep = wp;
				if (!intval) {
					gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), 
											 &iter, 
											 path_endwhile[wp]);
					GetNextIter = FALSE;
					WhileBreak = TRUE;
				} else {
					WhileBreak = FALSE;
				}
			}
		} else if (!(strcmp (task.type, "EndWhile"))) {
			if (TestOnly) {
				++lec;
				while (--lec >= 0) {
					if (loop_check[lec]) {
						if (loop_check[lec] == EW) {
							loop_check[lec] = 0;
						} else {
							L_print ("{r}Unclosed BeginLoop, If or While "
									 "statement at line %d\n", line);
							Error = TRUE;
						}
						break;
					}
				}
				path = gtk_tree_model_get_path(GTK_TREE_MODEL (lisTasks),&iter);
				while (wep >= 0) {
					if (!path_endwhile[wep]) {
						path_endwhile[wep] = path;
						gtk_list_store_set (lisTasks, &iter,
											TAG, wep,
											-1);
						break;
					}
					wep--;
				}
				if (wep < 0) {
					L_print ("{r}Task 'EndWhile' has no corresponding 'While' "
							 "at line %d\n", line);
					Error = TRUE;
				}
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK EndWhile\n");			
				gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
									TAG, &wep,
									-1);
				if (!WhileBreak) {
					gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), 
											 &iter, 
											 path_while[wep]);
					GetNextIter = FALSE;
				} else {
					WhileBreak = FALSE;
				}
			}
		} else if (!(strcmp (task.type, "AugOn"))) {
			if (!TestOnly && !Error) {
				L_print ("{b}TASK AugOn\n");			
				Flags.Type = T_AGN;
				Flags.Task |= T_EXEC;
				set_autog_on (TRUE);
			}
		} else if (!(strcmp (task.type, "AugOff"))) {
			if (!TestOnly && !Error) {
				L_print ("{b}TASK AugOff\n");			
				Flags.Type = T_AGF;
				Flags.Task |= T_EXEC;
				set_autog_on (FALSE);
			}
		} else if (!(strcmp (task.type, "GuideStart"))) {
			if (!menu.OpenTelPort && !menu.OpenAutogPort) {
				L_print ("{r}Task 'GuideStart' at line %d requires "
						 "telescope link or autoguider link to be open\n", 
						 line);
				Error = TRUE;
			}
			if (!aug->Open) {
				L_print ("{r}Task 'GuideStart' at line %d requires "
						 "autoguider camera to be open\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK GuideStart\n");			
				Flags.Type = T_GST;
				Flags.Task |= T_EXEC;
				set_guide_on (TRUE);
			}
		} else if (!(strcmp (task.type, "GuideStop"))) { 
			if (!TestOnly && !Error) {
				L_print ("{b}TASK GuideStop\n");			
				Flags.Type = T_GSP;
				Flags.Task |= T_EXEC;
				set_guide_on (FALSE);
			}
		} else if (!(strcmp (task.type, "GoTo"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								RA, &stra[0],
								DEC, &stra[1],			
								-1);
			if (!menu.OpenTelPort) {
				L_print ("{r}Task 'GoTo' at line %d requires the telescope "
						 "link to be open\n", line);
				Error = TRUE;
			}
			strcpy (sRA, g_strstrip (stra[0]));
			if (!check_format (TRUE, sRA)) {
				L_print ("{r}Task 'GoTo' has invalid RA at line %d\n", line);
				Error = TRUE;
			}
			strcpy (sDec, g_strstrip (stra[1]));
			if (!check_format (FALSE, sDec)) {
				L_print("{r}Task 'GoTo' has invalid Dec at line %d\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK GoTo %s %s\n", sRA, sDec);
				Flags.Type = T_GTO;
				Flags.Task |= T_EXEC;
				loop_telescope_goto (sRA, sDec);
			}
		} else if (!(strcmp (task.type, "Move"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								RA, &stra[0],
								DEC, &stra[1],			
								-1);
			if (!menu.OpenTelPort) {
				L_print ("{r}Task 'Move' at line %d requires the telescope "
						 "link to be open\n", line);
				Error = TRUE;
			}
			if (!get_entry_float (g_strstrip (stra[0]), 
							      MOV_MIN, MOV_MAX, MOV_DEF, 
							      TSK_PAGE, &val)) {
				L_print ("{r}Task 'Move' has invalid RA motion value at "
						 "line %d\n", line);
				Error = TRUE;
			}
			if (!get_entry_float (g_strstrip (stra[1]), 
							      MOV_MIN, MOV_MAX, MOV_DEF, 
							      TSK_PAGE, &val1)) {
				L_print ("{r}Task 'Move' has invalid Dec motion value at "
						 "line %d\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK Move %.2f %.2f\n", val, val1);
				Flags.Type = T_GTO;
				Flags.Task |= T_EXEC;
				loop_telescope_move (val, val1);
			}
		} else if (!(strcmp (task.type, "Exec")) || 
				   !(strcmp (task.type, "ExecAsync"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[0],
								-1);
			IsExec = !(strcmp (task.type, "Exec")) ? TRUE : FALSE;
			if (!stra[0]) {
				L_print ("{r}Task '%s' has missing script file at "
						 "line %d\n", IsExec ? "Exec" : "ExecAsync", line);
				Error = TRUE;
			} else {
				if (!g_file_test (stra[0], G_FILE_TEST_EXISTS)) {
					L_print ("{r}Task '%s' script file does not exist at "
							 "line %d\n", IsExec ? "Exec":"ExecAsync",line);
					Error = TRUE;
				}
				if (!g_file_test (stra[0], G_FILE_TEST_IS_EXECUTABLE)) {
					L_print("{r}Task '%s' script file is not executable at "
							 "line %d\n", IsExec ? "Exec":"ExecAsync",line);
					Error = TRUE;
				}
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK %s %s\n", IsExec? "Exec":"ExecAsync",stra[0]);
				tasks_script_execute (IsExec, stra[0]);
				if (IsExec) {
					Flags.Type = T_SCR;
					Flags.Task |= T_EXEC;
					loop_tasks_script ();
				}
			}
		} else if (!(strcmp (task.type, "SetParam"))) {
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								NUM, &stra[0],
								-1);
			gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
								VALUE, &stra[1],
								-1);
			if (!get_entry_int (g_strstrip (stra[0]), 
								0, NUMTPARAMS - 1, 0, 
								TSK_PAGE, &intval)) {
				L_print ("{r}Task SetParam has invalid parameter at line "
						 "%d\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK SetParam %%%d %s\n", intval, stra[1]);
				get_task_params (tp);
				tp[intval] = g_strstrip (stra[1]);
				set_task_params (tp);
			}
		} else if (!(strcmp (task.type, "WarmRestart"))) {
			if (!menu.OpenTelPort) {
				L_print ("{r}Task 'WarmRestart' at line %d requires the "
						 "telescope link to be open\n", line);
				Error = TRUE;
			}
			if (!menu.Gemini) {
				L_print ("{r}Task 'WarmRestart' at line %d requires the "
						 "'Gemini commands' option to be selected\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK WarmRestart\n");			
				Flags.Type = T_WRT;
				Flags.Task |= T_EXEC;
				loop_telescope_restart ();
			}
		} else if (!(strcmp (task.type, "ParkMount"))) {
			if (!menu.OpenTelPort) {
				L_print ("{r}Task 'ParkMount' at line %d requires the "
						 "telescope link to be open\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK ParkMount\n");			
				Flags.Type = T_PMT;
				Flags.Task |= T_EXEC;
				loop_telescope_park ();
			}
		#ifdef HAVE_UNICAP
		} else if (!(strcmp (task.type, "RecordStart"))) { 
			if (!menu.LiveView) {
				L_print ("{r}Task 'RecordStart' at line %d requires "
						 "Live View window to be open\n", line);
				Error = TRUE;
			} else {
				if (!liveview_record_is_writeable ()) {
					L_print ("{r}Task 'RecordStart' at line %d requires "
							 "destination for recording file to be "
							 "writeable\n", line);
					Error = TRUE;
				}
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK RecordStart\n");			
				Flags.Type = T_RST;
				Flags.Task |= T_EXEC;
				set_entry_string ("txtLVComment", obj);
				set_record_on (TRUE);
			}
		} else if (!(strcmp (task.type, "RecordStop"))) { 
			if (!menu.LiveView) {
				L_print ("{r}Task 'RecordStop' at line %d requires "
						 "Live View window to be open\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK RecordStop\n");			
				Flags.Type = T_RSP;
				Flags.Task |= T_EXEC;
				set_record_on (FALSE);
			}
		#endif /*HAVE_UNICAP*/
		} else if (!(strcmp (task.type, "YellowButton"))) {
			if (!menu.OpenAutogPort) {
				L_print ("{r}Task 'YellowButton' at line %d requires "
						 "autoguider link to be open\n", line);
				Error = TRUE;
			}
			if (!TestOnly && !Error) {
				L_print ("{b}TASK YellowButton\n");
				Flags.Type = T_YBT;
				Flags.Task |= T_EXEC;
				loop_telescope_yellow ();
			}
		} else if (!(strcmp (task.type, "Shutdown"))) {
			if (!TestOnly && !Error) {
				L_print ("{b}TASK Shutdown\n");			
				Flags.Type = T_SDN;
				Flags.Task |= T_EXEC;
				exit_and_shutdown ();
			}
		} else if (!(strcmp (task.type, "Exit"))) {
			if (!TestOnly && !Error) {
				L_print ("{b}TASK Exit\n");			
				tasks_stop ();
				finished_tasks ();
			}
		} else
			L_print ("{r}Invalid task type: %s\n", task.type); 

		/* Get the next row of the task list, but not if this has already been
		 * set via an If or While segment.
		 */
	    
		if (GetNextIter)
			if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (lisTasks), &iter))
				Flags.Task &= ~T_NEXT;  /* Must have reached end of list */
			
		GetNextIter = TRUE;

		if (!(Flags.Task & T_NEXT) && TestOnly) {
			while (lc >= 0) {
				if (loop_check[lc--]) {
					L_print ("{r}There are BeginLoop, If or While "
						 	 "statements with no corresponding "
							 "EndLoop, EndIf or EndWhile statements!\n");
					Error = TRUE;
					break;
				}
			}
		}
			
	} else {
		L_print ("{b}Task execution FINISHED\n");
		tasks_stop ();
		finished_tasks ();
	}
	
	*error = Error;
	return Flags.Task & T_NEXT;
}

guint tasks_get_status (void)
{
	/* Return task list status */
	
	Flags.Status = TSK_INACTIVE;
	
	if (Flags.Task & T_NEXT)
		Flags.Status |= TSK_ACTIVE;
	else
		Flags.Status &= ~TSK_ACTIVE;
	
	if (Flags.Task & T_TESTONLY)
		Flags.Status |= TSK_TESTONLY;
	else
		Flags.Status &= ~TSK_TESTONLY;
	
	return Flags.Status;
}

gboolean tasks_task_wait (void)
{
	/* Check to see if the wait time has been reached */

	if ((loop_elapsed_since_first_iteration()-seq_start) / 1000.0 > task.time) {
		tasks_task_done (T_WTU);
		return TRUE;
	} else
		return FALSE;	
}

gboolean tasks_task_pause (void)
{
	/* Check to see if the pause time has been reached */
	
	if ((loop_elapsed_since_first_iteration ()-task.now) / 1000.0 > task.time) {
		tasks_task_done (T_PSF);
		return TRUE;
	} else
		return FALSE;	
}

gboolean tasks_task_at (void)
{
	struct tm *dt;
	gdouble now;		
	
	dt = get_time (UseUTC);
	now = (gdouble) dt->tm_hour + ((gdouble) dt->tm_min + (gdouble) dt->tm_sec /
	                                                               60.0) / 60.0;

	/* If now >= 12:00:00, then we haven't reached a time earlier than 12:00:00
	 * yet (i.e. any such time is tomorrow morning rather than earlier today).
	 */
	
	if (now >= 12.0 && task.time < 12.0)
		return FALSE;
	
	if (now >= task.time) {
		tasks_task_done (T_EAT);
		return TRUE;
	}

	return FALSE;	
}

gboolean tasks_load_file (gchar *filename)
{
	/* Load tasks into the task list from a file */
	
	GString *gs;
	gint i, k;
	gchar *buffer, *buf, *line, **strings, **tokens;
	gchar *cmd, *token, *stra[10];
	
	if (!g_file_get_contents (filename, &buffer, NULL, NULL))
		return show_error (__func__, "Couldn't get file contents");
	
	if (!g_str_has_suffix (buffer, "\n"))      /* Add terminating new-line if */
		buf = g_strconcat (buffer, "\n", NULL);/*  there isn't one       */
	else	
		buf = g_strdup (buffer);
	g_free (buffer);
	
	strings = g_strsplit (buf, "\n", -1); /* Get array of lines from the file */
	g_free (buf);
	
	i = -1;
	while (strings[++i]) {
		if (!g_str_has_suffix (strings[i], "\t")) /* Add terminating tab if */
			line = g_strconcat (strings[i], "\t", NULL); /*  there isn't one  */
		else
			line = g_strdup (strings[i]);

		tokens = g_strsplit_set (line, " \t", -1);  /* Split on white space */
		g_free (line);

		/* Load the data */	

		if (!(cmd = next_token (tokens, TRUE)))
			continue;
		
		if (!strcmp (cmd, "Object")) {
			gs = g_string_new (NULL);
			while ((token = next_token (tokens, FALSE)))
				g_string_append_printf (gs, "%s ", token);
			tasks_add_Object (gs->str);
			g_string_free (gs, TRUE);
		} else if (!strcmp (cmd, "BeginSequence")) {
			tasks_add_BeginSequence ();
		} else if (!strcmp (cmd, "WaitUntil")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_WaitUntil (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "PauseFor")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_PauseFor (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "At")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_At (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "Expose")) {
			for (k = 0; k < 10; k++) {
				if ((token = next_token (tokens, FALSE)))
					stra[k] = token;
				else
					goto token_error;
			}
			tasks_add_Exposure (stra);
		} else if (!strcmp (cmd, "FocusTo")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_FocusTo (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "FocusMove")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_FocusMove (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "BeginLoop")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_BeginLoop (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "EndLoop")) {
			tasks_add_EndLoop ();
		} else if (!strcmp (cmd, "IfTrue")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_IfTrue (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "IfFalse")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_IfFalse (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "EndIf")) {
			tasks_add_EndIf ();
		} else if (!strcmp (cmd, "While")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_While (token);
			else
				goto token_error;
		} else if (!strcmp (cmd, "EndWhile")) {
			tasks_add_EndWhile ();
		} else if (!strcmp (cmd, "AugOn")) {
			tasks_add_Aug (TRUE);			
		} else if (!strcmp (cmd, "AugOff")) {			
			tasks_add_Aug (FALSE);			
		} else if (!strcmp (cmd, "GuideStart")) {
			tasks_add_Guide (TRUE);			
		} else if (!strcmp (cmd, "GuideStop")) {			
			tasks_add_Guide (FALSE);			
		} else if (!strcmp (cmd, "GoTo")) {
			for (k = 0; k < 2; k++) {
				if ((token = next_token (tokens, FALSE)))
					stra[k] = token;
				else
					goto token_error;
			}
			tasks_add_GoTo (stra[0], stra[1]);
		} else if (!strcmp (cmd, "Move")) {
			for (k = 0; k < 2; k++) {
				if ((token = next_token (tokens, FALSE)))
					stra[k] = token;
				else
					goto token_error;
			}
			tasks_add_Move (stra[0], stra[1]);
		} else if (!strcmp (cmd, "Exec")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_Exec (token, TRUE);
			else
				goto token_error;
		} else if (!strcmp (cmd, "ExecAsync")) {
			if ((token = next_token (tokens, FALSE)))
				tasks_add_Exec (token, FALSE);
			else
				goto token_error;
		} else if (!strcmp (cmd, "SetParam")) {
			if ((token = next_token (tokens, FALSE)))
				stra[0] = token;
			else
				goto token_error;
			gs = g_string_new (NULL);
			while ((token = next_token (tokens, FALSE)))
				g_string_append_printf (gs, "%s ", token);
			tasks_add_SetParam (stra[0], gs->str);
			g_string_free (gs, TRUE);
		} else if (!strcmp (cmd, "WarmRestart")) {
			tasks_add_WarmRestart ();
		} else if (!strcmp (cmd, "ParkMount")) {
			tasks_add_ParkMount ();
		} else if (!strcmp (cmd, "RecordStart")) {
			tasks_add_Record (TRUE);
		} else if (!strcmp (cmd, "RecordStop")) {
			tasks_add_Record (FALSE);
		} else if (!strcmp (cmd, "YellowButton")) {
			tasks_add_YellowButton ();
		} else if (!strcmp (cmd, "Shutdown")) {
				tasks_add_Shutdown ();
		} else if (!strcmp (cmd, "Exit")) {
				tasks_add_Exit ();
		} else if (strncmp (cmd, "#", 1)) {
			L_print ("{r}Invalid task: '%s' in file on line %d\n", cmd, i + 1);
			return show_error (__func__, "Error reading file");
		}			
		g_strfreev (tokens);
	}
	g_strfreev (strings);
	return TRUE;
	
token_error:

	L_print ("{r}Missing parameter for task '%s' in file on line %d\n", 
			                                                        cmd, i + 1);
	return show_error (__func__, "Error reading file");
}

gboolean tasks_write_file (gchar *filename)
{
	/* Write the task list to the specified file */
	
	GtkTreeIter iter;
	GtkTreePath *path;
	gushort i;
	gint numbytes;
	gchar *s1, *s2, *stra[15];
	gboolean next = FALSE;
	
	FILE *fp;
	
	/* Get the first task */
	
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (lisTasks), &iter)) {
		next = TRUE;
    	if (!(fp = fopen (filename, "w")))
			return show_error (__func__, "Error opening tasks file");
	}

	while (next) {
		
		/* Select the row */
	
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (lisTasks), &iter);
		gtk_tree_view_set_cursor (trvTasks, path, NULL, FALSE);
	
		/* Scroll selected row to top of window */

		gtk_tree_view_scroll_to_cell (trvTasks, path, NULL, TRUE, 0.0, 0.0);
		gtk_tree_path_free (path);			
		
		/* Get the task details */
			
		gtk_tree_model_get (GTK_TREE_MODEL (lisTasks), &iter,
							TASK, &stra[0],
							EXP_TYPE, &stra[1],
							FILTER, &stra[2],
							TASK_TIME, &stra[3],
							H1, &stra[4],
							V1, &stra[5], 
							H2, &stra[6], 
							V2, &stra[7], 
							H_BIN, &stra[8], 
							V_BIN, &stra[9], 
							CCDTEMP, &stra[10], 
							NUM, &stra[11], 
							RA, &stra[12], 
							DEC, &stra[13], 
							VALUE, &stra[14], 							
							-1);

		/* Write to file */
		
		s1 = g_strconcat (g_strstrip (stra[0]), " ", NULL);
		for (i = 1; i < 15; i++) {
			if (!strcmp (stra[0], "Expose") && i > 10) /* Don't write exposure*/
				break;                                 /*  number - confusing!*/
			if (stra[i]) {
				s2 = g_strconcat (s1, " ", g_strstrip (stra[i]), NULL);
				g_free (s1);
				s1 = s2;
			}
		}
		s2 = g_strconcat (s1, "\n", NULL);
		g_free (s1);
		
		numbytes = g_utf8_strlen (s2, -1);	
    	if (fwrite (s2, sizeof (gchar), numbytes, fp) < numbytes) {
			g_free (s2);
			fclose (fp);
			return show_error (__func__, "Error writing tasks file");
		}
		g_free (s2);
		
		/* Get the next task */
		
		if (!(next = gtk_tree_model_iter_next (GTK_TREE_MODEL(lisTasks),&iter)))
			fclose (fp);
	}
	return TRUE;
}

static void tasks_script_execute (gboolean IsExec, gchar *script)
{
	/* Execute the script */

	struct cam_img *ccd = get_ccd_image_struct ();
	
	struct focus f;
	GString *cmd;
	gushort i;
	gdouble epoch, fRA, fDec;
	gchar sRA[9], sDec[10];
	gchar *pid, *s;
    gchar *tp[NUMTPARAMS];	
	gboolean OK_RA, OK_Dec;
	
	/* Remove any pre-existing files */
	
	pid = g_strdup_printf ("%d", getpid ());
	if (!sr) /* Results */
		sr = g_strconcat (PrivatePath, "/", pid, SCRIPT_RESULTS, NULL);
	if (!sd) /* 'Done'  */
		sd = g_strconcat (PrivatePath, "/", pid, SCRIPT_DONE, NULL);
	g_remove (sr);
	g_remove (sd);
	g_free (pid);
	
	/* Compose the command string.  If a parameter contains spaces, surround it
	 * with single quotes.
	 */
	
	s = g_strconcat ("'", script, "'", NULL);           /* Script to execute  */
	cmd = g_string_new (s);
	g_free (s);
	
	g_string_append_printf (cmd, IsExec ? " 1" : " 0"); /* Exec or ExecAsync? */
	
	g_string_append_printf (cmd, " '%s' '%s'", sr, sd); /* Results/done files */
	
	g_string_append_printf (cmd, " %d", NUMTPARAMS);    /* No. of task params */
	get_task_params (tp);
	for (i = 0; i < NUMTPARAMS; i++) {                  /* Task parameters    */
		if (!g_strrstr (tp[i], " "))
			g_string_append_printf (cmd, " %s", tp[i]);
		else
			g_string_append_printf (cmd, " '%s'", tp[i]);
	}
	
	g_string_append_printf (cmd, " '%s'", ccd->exd.filename); /* CCD filename */
	g_string_append_printf (cmd, " '%s'", ccd->exd.ExpType);  /* Exposure type*/
	g_string_append_printf (cmd, " '%s'", ccd->exd.filter);   /* Filter       */
	g_string_append_printf (cmd, " %f", ccd->exd.req_len);    /* Exp. length  */
	g_string_append_printf (cmd, " %d", ccd->exd.h_top_l + 1);/* Chip area    */
	g_string_append_printf (cmd, " %d", ccd->cam_cap.max_v - ccd->exd.v_bot_r);
	g_string_append_printf (cmd, " %d", ccd->exd.h_bot_r + 1);
	g_string_append_printf (cmd, " %d", ccd->cam_cap.max_v - ccd->exd.v_top_l);
	g_string_append_printf (cmd, " %d", ccd->exd.h_bin);      /* Binning      */
	g_string_append_printf (cmd, " %d", ccd->exd.v_bin);
	g_string_append_printf (cmd, " %f", ccd->exd.ccdtemp);    /* Chip temp.   */

	telescope_get_RA_Dec(FALSE,&epoch, sRA, sDec, &fRA, &fDec, &OK_RA, &OK_Dec);
	g_string_append_printf (cmd, " %s", sRA);                 /* Current RA   */
	g_string_append_printf (cmd, " %s", sDec);                /* Current Dec  */
	if (menu.OpenFocusPort) {
		f.cmd = FC_VERSION;
		focus_comms->focus (&f);
		if (f.version >= 3.0)
			f.cmd = FC_MAX_TRAVEL_GET | FC_CUR_POS_GET | FC_TEMP_GET;
		else
			f.cmd = FC_MAX_TRAVEL_GET | FC_CUR_POS_GET;
		focus_comms->focus (&f);
	} else
	    memset (&f, 0, sizeof (struct focus));
	g_string_append_printf (cmd, " %d", f.max_travel);/* Focuser max. travel  */
	g_string_append_printf (cmd, " %d", f.cur_pos);   /* Focuser current pos. */
	if (menu.OpenFocusPort && f.version >= 3.0)
		g_string_append_printf (cmd, " %f", f.temp);  /* Focuser current temp.*/
	else
		g_string_append_printf (cmd, " %f", 0.0);     /* Focuser current temp.*/
	
	/* Execute the script */
	
    sp = popen (cmd->str, "r");
	
	g_string_free (cmd, TRUE);
}

gboolean tasks_script_done (void)
{
	/* Check whether the script has finished executing.  If so, read the
	 * results of the script and remove the files.  Return FALSE if executing, 
	 * TRUE if done.
	 */
	
	gushort n;
	gint i, k;
	gchar *buffer, **strings;
	gchar *tp[NUMTPARAMS] = { NULL };
	gboolean Valid = TRUE;
	
	if (!g_file_test (sd, G_FILE_TEST_EXISTS)) /* 'Done' file not there yet */
		return FALSE;
    pclose (sp);
	
	/* Get the contents of the results file, and split on new lines */
	
	g_file_get_contents (sr, &buffer, NULL, NULL);
	
	strings = g_strsplit (buffer, "\n", -1); /* Get array of lines from file */
	g_free (buffer);
	
	i = -1;
	while (strings[++i]) {
		if (!strncmp (strings[i], "%", 1)) {         /* Task parameter? */
			Valid = TRUE;
			k = 0;
			while (strings[i][++k] && !isspace (strings[i][k])) {
		        if (!isdigit (strings[i][k])) {
					Valid = FALSE;
					break;
				}
			}
			if (k == strlen (strings[i]))
				Valid = FALSE;
			if (Valid) {
				if ((n = (gushort) strtol (&strings[i][1], NULL, 10)) < 
					                                               NUMTPARAMS) {
					tp[n] = &strings[i][k + 1];
				} else {
					L_print ("{r}Task parameter specification '%s' exceeds "
							 "maximum allowed value [%d] at line %d\n", 
							 strings[i], NUMTPARAMS, i + 1);
				}
			} else {
				L_print("{r}Invalid task parameter specification '%s' in "
						"script results file at line %d\n", strings[i], i + 1);
			}
		} else if (!strncmp (strings[i], "#", 1)) {  /* Comment line? */
			L_print ("{g}%s\n", &strings[i][1]);
		}
	}
	L_print ("Set parameters: ");
	for (i = 0; i < NUMTPARAMS; i++)
		if (tp[i])
			L_print ("{.}%%%d: %s  ", i, tp[i]);
	L_print ("{.}\n");
	set_task_params (tp);
	g_strfreev (strings);
	
	g_remove (sr);    /* Remove 'results' file */
	if (sr) {
		g_free (sr);
		sr = NULL;
	}
	g_remove (sd);    /* Remove 'done' file */
	if (sd) {
		g_free (sd);
		sd = NULL;
	}
	
	tasks_task_done (T_SCR);
	return TRUE;
}

static GtkTreeIter *get_selected_iter (GtkTreeIter *iter)
{
	/* Return the iter corresponding to the currently selected row, or NULL
	 * if no row at all is selected.
	 */
	
	GtkTreePath *path;
	
	gtk_tree_view_get_cursor (trvTasks, &path, NULL);
	if (path != NULL) {
		if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (lisTasks), iter, path))
			iter = NULL;
	} else
		iter = NULL;
	gtk_tree_path_free (path);	

	return iter;
}	

static void select_current_row (GtkTreeIter *iter)
{
	/* Select the current row (called after adding or deleting at the previously
	 * selected row).
	 */
	
	GtkTreePath *path;
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (lisTasks), iter);
	gtk_tree_view_set_cursor (trvTasks, path, NULL, FALSE);
	gtk_tree_path_free (path);
}

static gchar *next_token (gchar **tokens, gboolean Init)
{
	/* The g_strsplit_set function splits at every occurence of the splitting
	 * characters rather than grouping them together, giving blank tokens if 
	 * multiple splitting characters follow each other.  This routine works 
	 * through the list of split tokens and returns the next token that isn't 
	 * blank; or NULL if the end of the list of tokens has been reached.
	 *
	 * Call with Init = TRUE for the first token and Init = FALSE for the
	 * subsequent ones.
	 */

	static guint i = -1;
	
	if (Init)
		i = -1;
	
	while (tokens[++i]) {
		if (strcmp (tokens[i], ""))
			return tokens[i];
	}
	return NULL;	
}
