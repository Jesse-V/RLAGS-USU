/******************************************************************************/
/*                                EVENT LOOP                                  */
/*                                                                            */
/* The routines in this file govern the operation of the event loop.  The     */
/* loop is started when the application initialises.                          */
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
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#define GOQAT_LOOP
#include "interface.h"

#define INTERVAL 25   /* Timer tick interval in milliseconds   */

#define ODL 0x00000000     /* lOop is iDLe                     */
#define OCL 0x00000001     /* lOop CanceL                      */
#define OIQ 0x00000002     /* lOop Is Quitting                 */

#define CCO 0x00000001     /* Ccd Camera Open                  */
#define CCA 0x00000002     /* Ccd Camera Available             */
#define CST 0x00000004     /* Ccd Set Temperature              */
#define CSF 0x00000008     /* Ccd Set Filter                   */
#define CSE 0x00000010     /* Ccd Start Exposure               */
#define CEP 0x00000020     /* Ccd Exposure in Progress         */
#define CIE 0x00000040     /* Ccd Interrupt Exposure           */
#define CCE 0x00000080     /* Ccd Cancel Exposure              */
#define CIR 0x00000100     /* Ccd Image Ready                  */
#define CDI 0x00000200     /* Ccd Display single Image         */
#define CAP 0x00000400     /* Ccd Autofocus calibration stoP   */
#define CFP 0x00000800     /* Ccd autoFocus stoP               */
#define CDT 0x00001000     /* Ccd Display Temperatures         */
#define CSI 0x00002000     /* Ccd get Status Information       */
#define CUS 0x00004000     /* Ccd Unable to get Status info    */
#define CCC 0x00008000     /* Ccd Camera Close                 */

#define AGO 0x00000001     /* AutoGuider camera Open           */
#define AGA 0x00000002     /* AutoGuider camera Available      */
#define ASE 0x00000004     /* Autoguider Start Exposure        */
#define AEP 0x00000008     /* Autoguider Exposure in Progress  */
#define AIR 0x00000010     /* Autoguider Image Ready           */
#define ACC 0x00000020     /* Autog. Calib. Calibrate          */
#define ACI 0x00000040     /* Autog. Calib. In progress        */
#define ACG 0x00000080     /* Autog. Calib. Get next coords    */
#define ACM 0x00000100     /* Autog. Calib. Moved telescope    */
#define ACE 0x00000200     /* Autog. Calib. Exposure           */
#define ACD 0x00000400     /* Autog. Calib. got all coorDs     */
#define ACP 0x00000800     /* Autog. Calib. stoP               */
#define AGB 0x00001000     /* AutoGuider Begin guiding         */
#define AGG 0x00002000	   /* AutoGuider issue Guide commands  */
#define AGE 0x00004000     /* AutoGuider idlE                  */
#define AGU 0x00008000     /* AutoGuider paUse guiding         */
#define AGP 0x00010000     /* AutoGuider is Paused             */
#define AGN 0x00020000     /* AutoGuider coNtinue guiding      */
#define AGQ 0x00040000	   /* AutoGuider Quit guiding          */
#define AGI 0x00080000     /* AutoGuider display single Image  */
#define ATP 0x00100000     /* Autoguider camera Thread stoP    */
#define AGC 0x00200000     /* AutoGuider Close                 */

#define FCO 0x00000001     /* FoCuser is Open                  */
#define FCM 0x00000002     /* Focuser Check Moving             */
#define FSM 0x00000004     /* Focuser Stop Moving              */
#define FIM 0x00000008     /* Focuser Is Moving                */
#define FSP 0x00000010     /* Focuser motion is StoPped        */
#define FFF 0x00000020     /* Focusing (Focus calib./autoFocus)*/
#define FAO 0x00000040     /* Focuser Apply filter Offset      */
#define FAT 0x00000080     /* Focuser Apply Temp. comp offset  */

#define LVO 0x00000001     /* Live View Open                   */
#define LVR 0x00000002     /* Live View Read                   */
#define LVE 0x00000004     /* Live View rEcord                 */
#define LVI 0x00000008     /* Live View Is recording           */
#define LVD 0x00000010     /* Live View Flush to disk          */
#define LVS 0x00000020     /* Live View Stop recording         */
#define LVC 0x00000040     /* Live View Close                  */

#define PBI 0x00000001     /* PlayBack Iterate thru' frames    */
#define PBF 0x00000002     /* PlayBack Iterate Final time      */

#define TGO 0x00000001     /* Telescope GOto                   */
#define TGP 0x00000002     /* Telescope Goto in Progress       */
#define TMV 0x00000004     /* Telescope MoVe                   */
#define TWR 0x00000008     /* Telescope Warm Restart           */
#define TWS 0x00000010     /* Telescope is Warm reStarting     */
#define TPM 0x00000020     /* Telescope Park Mount             */
#define TPK 0x00000040     /* Telescope is ParKing             */
#define TYB 0x00000080     /* Telescope Yellow Button          */

#define SCI 0x00000001     /* Save CCD Image                   */
#define SSA 0x00000002     /* Save Single Autoguider image     */
#define SPA 0x00000004     /* Save Periodic Autoguider image   */

#define DBR 0x00000001     /* Disp. Blink selection Rectangle  */
#define DSR 0x00000002     /* Disp. Stop blinking Rectangle    */

#define WAC 0x00000001     /* WAtch aCtivate                   */
#define WAR 0x00000002     /* WAtch is Running                 */
#define WAS 0x00000004     /* WAtch Stop                       */

#define KWT 0x00000001     /* tasKs WaiT                       */
#define KPS 0x00000002     /* tasKs PauSe                      */
#define KAT 0x00000004	   /* tasKs AT                         */
#define KEP 0x00000008     /* tasKs Execute script in Progress */

static struct LoopFlags {
	guint Loop;
	guint CCD;
	guint Aug;
	guint Foc;
	guint Lvw;
	guint Pbw;
	guint Tel;
	guint Img;
	guint Dsp;
	guint Wat;
	guint Tsk;
} Flags;

static struct AFCalibThreadData {/* Data for autofocus calibration thread     */
	gint start_pos;
	gint end_pos;
	gint step;
	gint repeat;
	gint box;
	gdouble exp_len;
} AFCalib_thread_data;

static struct AFFocusThreadData {/* Data for autofocus focusing thread        */
	gint start_pos;
	gint box;
	gdouble LHSlope;
	gdouble RHSlope;
	gdouble PID;
	gdouble near_HFD;
	gdouble exp_len;
	gboolean Inside;
} AFFocus_thread_data;

static GThread *ccd_thread = NULL; /* CCD camera thread                       */
static GThread *AFC_thread = NULL; /* Autofocus calibration thread            */
static GThread *AFF_thread = NULL; /* Autofocus focusing thread               */
static GThreadPool *thread_pool_autog_calib = NULL; /* Autog. calib. threads  */
static GThreadPool *thread_pool_focuser_moving = NULL; /* Foc. move threads   */
static gushort vid_buf;         /* Number of video buffer to flush to disk    */
static guint save_period;       /* Period for saving autoguider images (s)    */
static guint handler_id;        /* Timeout handler id                         */
static gint filter_offset;      /* Focus offset when changing camera filter   */
static gint tempcomp_pos;  /* Focus position after applying focus temp. comp. */
static gint cam_type;           /* Camera type for filter focus offset        */
static gfloat calib_coords[5][2]; /* Autoguider calibration star coordinates  */
static gdouble MoveRA, MoveDec; /* RA and Dec motion for telescope Move       */
static gchar *sRA, *sDec;       /* Coordinates of object for telescope GoTo   */
static gchar *VideoDir = NULL;  /* Directory for saving video file            */

/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void loop_start_loop (void);
void loop_ccd_open (gboolean Open);
void loop_ccd_start (void);
void loop_ccd_interrupt (void);
void loop_ccd_cancel (void);
void loop_ccd_display_image (void);
void loop_ccd_calibrate_autofocus (gboolean Start, gint start_pos, gint end_pos,
								   gint step, gint repeat, gdouble exp_len, 
								   gint box);
void loop_ccd_autofocus (gboolean Start, gdouble LHSlope, gdouble RHSlope, 
						 gdouble PID, gint start_pos, gboolean Inside, 
						 gdouble near_HFD, gdouble exp_len, gint box); 
void loop_ccd_temps (gboolean display, guint period);
void loop_autog_open (gboolean Open);
void loop_autog_calibrate (gboolean Calibrate);
void loop_autog_exposure_wait (gboolean Wait, enum MotionDirection dirn);
void loop_autog_guide (gboolean guide);
void loop_autog_pause (gboolean pause);
void loop_autog_DS9 (void);
void loop_focus_open (gboolean Open);
void loop_focus_stop (void);
gboolean loop_focus_is_focusing (void);
void loop_focus_apply_filter_offset (enum CamType type, gint offset);
void loop_focus_apply_temp_comp (gint pos);
void loop_focus_check_done (void);
void loop_LiveView_open (gboolean open);
void loop_LiveView_record (gboolean record, gchar *dirname);
void loop_LiveView_flush_to_disk (gushort buf);
void loop_video_iter_frames (gboolean Iter);
void loop_telescope_goto (gchar *sRA, gchar *sDec);
void loop_telescope_move (gdouble RA, gdouble Dec);
void loop_telescope_restart (void);
void loop_telescope_park (void);
void loop_telescope_yellow (void);
void loop_save_image (gint id);
void loop_save_periodic (gboolean save, guint period);
void loop_stop_loop (void);
void loop_watch_activate (gboolean Activate);
void loop_tasks_wait (void);
void loop_tasks_pause (void);
void loop_tasks_at (void);
void loop_tasks_script (void);
void loop_display_blinkrect (gboolean Blink);
guint loop_elapsed_since_first_iteration (void);
static gboolean timeout (guint *timer, guint msec);
static gpointer thread_func_ccd (gpointer data);
static gpointer thread_func_AFCalib (gpointer data);
static gpointer thread_func_AFFocus (gpointer data);
static void AF_focuser_move_and_wait (gint pos);
static gboolean AF_measure_HFD (gboolean Init, gboolean Plot, gint box,
								struct exposure_data *exd, gdouble *hfd);
static void thread_pool_autog_calib_func (gpointer data, gpointer user_data);
static void thread_pool_focuser_moving_func (gpointer data, gpointer user_data);
static gint event_loop (gpointer data);


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

void loop_start_loop (void)
{
	/* This routine is called when the application initialises, in order to
	 * start the execution loop.
	 */

	memset (&Flags, 0, sizeof (Flags));
	handler_id = g_timeout_add ((guint32) INTERVAL, event_loop, NULL);
	Flags.Loop = ODL;
}

void loop_ccd_open (gboolean Open)
{
	/* This routine is called when the user chooses to connect to a CCD
	 * camera from the 'Settings' menu.
	 */
	
	if (Open)
		Flags.CCD |= CCO;
	else
		Flags.CCD |= CCC;
}

void loop_ccd_start (void)
{
	/* This routine is called by the task list to start a CCD exposure.
	 * Setting the temperature and the filter position can be done 
	 * simultaneously.
	 */
	
	gboolean AtTemperature;
	
	Flags.CCD |= (CSF | CSE);
	ccdcam_set_temperature (&AtTemperature);
	if (!AtTemperature)
		Flags.CCD |= CST;
	set_exposure_buttons (TRUE);
}

void loop_ccd_interrupt (void)
{
	/* This routine is called to interrupt the current exposure */
	
	Flags.CCD |= CIE;
}

void loop_ccd_cancel (void)
{
	/* This routine is called to cancel an exposure */
	
	Flags.CCD |= CCE;
}

void loop_ccd_display_image (void)
{
	/* This routine is called to redisplay the image if the debayering option
	 * has been changed.
	 */
	
	Flags.CCD |= CDI;
}

void loop_ccd_calibrate_autofocus (gboolean Start, gint start_pos, gint end_pos,
								   gint step, gint repeat, gdouble exp_len, 
								   gint box)
{
	/* This routine is called to start/stop autofocus calibration */
	
	if (Start && !AFC_thread) {
		Flags.Foc |= FFF;
		AFCalib_thread_data.start_pos = start_pos;
		AFCalib_thread_data.end_pos = end_pos;
		AFCalib_thread_data.step = step;
		AFCalib_thread_data.repeat = repeat;
		AFCalib_thread_data.exp_len = exp_len;
		AFCalib_thread_data.box = box;
		AFC_thread = g_thread_create (thread_func_AFCalib, NULL, TRUE, NULL);
	}
	
	if (!Start && AFC_thread)
			Flags.CCD |= CAP;
}

void loop_ccd_autofocus (gboolean Start, gdouble LHSlope, gdouble RHSlope,
						 gdouble PID, gint start_pos, gboolean Inside, 
						 gdouble near_HFD, gdouble exp_len, gint box)
{
	/* This routine is called to start/stop autofocusing */

	if (Start && !AFF_thread) {
		Flags.Foc |= FFF;
		AFFocus_thread_data.LHSlope = LHSlope;
		AFFocus_thread_data.RHSlope = RHSlope;
		AFFocus_thread_data.PID = PID;
		AFFocus_thread_data.start_pos = start_pos;
		AFFocus_thread_data.Inside = Inside;
		AFFocus_thread_data.near_HFD = near_HFD;
		AFFocus_thread_data.exp_len = exp_len;
		AFFocus_thread_data.box = box;
		AFF_thread = g_thread_create (thread_func_AFFocus, NULL, TRUE, NULL);
	}
	
	if (!Start && AFF_thread)
			Flags.CCD |= CFP;
}

void loop_ccd_temps (gboolean display, guint period)
{
	/* This routine is called when CCD camera temperatures are to be
	 * displayed.
	 */
	
	if (display)
		Flags.CCD |= CDT;
	else
	    Flags.CCD &= ~CDT;
}

void loop_autog_open (gboolean Open)
{
	/* This routine is called when the user toggles the 'Use autoguider?'
	 * check box in the user interface.
	 */
	
	if (Open)
		Flags.Aug |= AGO;
	else
		Flags.Aug |= AGC;
}

void loop_autog_calibrate (gboolean Calibrate)
{
	/* This routine is called when the user clicks the autoguider calibration
	 * button on the autoguider tab.
	 */
	
	if (Calibrate)
		Flags.Aug |= (ACC | ACG);
	else
		Flags.Aug |= ACP;
}

void loop_autog_exposure_wait (gboolean Wait, enum MotionDirection dirn)
{
	/* This routine is called to pause reading from the autoguider camera
	 * until the current guide correction has ended.  The reason for doing this
	 * is because the guide routine in telescope.c rejects any images whose
	 * exposure started before the last guide correction ended.  Therefore, a
	 * wait essentially equal to the exposure length is imposed there if an
	 * autoguider exposure starts too soon.  (For free-running devices, the
	 * guide routine is obliged to wait an exposure length since GoQat has no
	 * control over when such exposures start).
	 */
	 
	static gushort guide_motion = 0;
	
	if (Wait) { 
		Flags.Aug &= ~ASE;
		guide_motion |= dirn;
	} else {
		guide_motion &= ~dirn;
		if (!guide_motion)
			Flags.Aug |= ASE;
	}
}

void loop_autog_guide (gboolean guide)
{
	/* This routine is called when the user toggles the 'Start autoguiding'
	 * button in the user interface.
	 */
	
	if (guide)
		Flags.Aug |= AGB;
	else
		Flags.Aug |= AGQ;
}

void loop_autog_pause (gboolean pause)
{
	/* This routine is called when the user toggles the 'Pause autoguiding'
	 * button in the user interface.
	 */
	
	if (pause)
		Flags.Aug |= AGU;
	else
		Flags.Aug |= AGN;
}

void loop_autog_DS9 (void)
{
	/* This routine is called when the user clicks the 'Display in DS9' button
	 * in the user interface.
	 */
	
	Flags.Aug |= AGI;
}

void loop_focus_open (gboolean Open)
{
	/* This routine is called when the focuser is opened/closed, so that e.g.
	 * the temperature can be automatically read at given intervals.
	 */
	
	if (Open)
		Flags.Foc |= FCO;
	else
		Flags.Foc &= ~FCO;
}

void loop_focus_stop (void)
{
	/* This routine is called when the user clicks the 'Stop' button on the
	 * Focus tab.
	 */
	
	Flags.Foc |= FSM;
}

gboolean loop_focus_is_focusing (void)
{
	/* This is called by any routine that wishes to check whether a focuser
	 * motion is in progress, or autofocus calibration or autofocusing is 
	 * taking place.
	 */
	
	return Flags.Foc & (FIM | FFF);
}

void loop_focus_apply_filter_offset (enum CamType type, gint offset)
{
	/* This routine is called to apply a focus offset after changing a filter.
	 * It has to be done via the loop code to prevent it clashing with an
	 * automatic temperature focusing movement.
	 */
	
	cam_type = type;
	filter_offset = offset;
	Flags.Foc |= FAO;
}

void loop_focus_apply_temp_comp (gint pos)
{
	/* This routine is called to apply a focus motion after a given change in
	 * temperature.  It has to be done via the loop code to prevent it clashing 
	 * with a filter focus offset movement.
	 */
	
	tempcomp_pos = pos;
	Flags.Foc |= FAT;
}

void loop_focus_check_done (void)
{
	/* This routine is called after a focuser motion is initiated, to check
	 * when the motion has finished.
	 */
	
	Flags.Foc |= (FCM | FIM);
}

void loop_LiveView_open (gboolean open)
{
	/* This routine is called when the user clicks the 'Live View' menu item in
	 * the user interface.
	 */
	
	if (open)
		Flags.Lvw |= LVO;
	else
		Flags.Lvw |= LVC;
}

void loop_LiveView_record (gboolean record, gchar *dirname)
{
	/* This routine is called when the user clicks the 'Record' button in the
	 * LiveView window.
	 */
	
	if (record) {
		Flags.Lvw |= LVE;
	    VideoDir = g_strdup (dirname);
	} else {
		Flags.Lvw |= LVS;
		if (VideoDir) {
			g_free (VideoDir);
		    VideoDir = NULL;
		}
	}
}

void loop_LiveView_flush_to_disk (gushort buf)
{
	/* This routine is called when a video buffer has been filled and is ready
	 * for flushing to disk.
	 */
	
	Flags.Lvw |= LVD;
    vid_buf = buf;	
}

void loop_video_iter_frames (gboolean Iter)
{
	/* This routine is called to start or stop iterating through selected
	 * video frames in the Playback window.
	 */
	
	if (Iter)
		Flags.Pbw |= PBI;
	else
		Flags.Pbw |= PBF;
}

void loop_telescope_goto (gchar *RA, gchar *Dec)
{
	/* This routine is called when the task list executes a GoTo command */
	
	sRA = RA;
	sDec = Dec;
	Flags.Tel |= TGO;	
}

void loop_telescope_move (gdouble RA, gdouble Dec)
{
	/* This routine is called when the task list excecutes a Move command */
	
	MoveRA = RA;
	MoveDec = Dec;
	Flags.Tel |= TMV;	
}

void loop_telescope_restart (void)
{
	/* This routine is called when the task list executes a WarmRestart 
	 * command.
	 */
	
	Flags.Tel |= TWR;
}

void loop_telescope_park (void)
{
	/* This routine is called when the task list executes a ParkMount command */
	
	Flags.Tel |= TPM;
}

void loop_telescope_yellow (void)
{
	/* This routine is called when the task list executes a YellowButton 
	 * command.
	 */
	
	Flags.Tel |= TYB;
}

void loop_save_image (gint id)
{
	/* This routine is called when an image is to be autosaved */
	
	if (id == AUG)
		Flags.Img |= SSA;
	
	if (id == CCD)
		Flags.Img |= SCI;
}

void loop_save_periodic (gboolean save, guint period)
{
	/* This routine is called when an autoguider image is to be
	 * saved periodically.
	 */

	if (save) {
		Flags.Img |= SPA;
		save_period = period;
	} else
		Flags.Img &= ~SPA;
}

void loop_stop_loop (void)
{
	/* This routine is called when the application terminates in order to
	 * stop the event loop.
	 */
	
	Flags.Loop |= OCL;
}

void loop_watch_activate (gboolean Activate)
{
	/* Activate/deactivate watching the designated folder for incoming tasks */
	
	if (Activate)
		Flags.Wat |= WAC;
	else
		Flags.Wat |= WAS;
}

void loop_tasks_wait (void)
{
	/* This routine is called when a wait command is encountered in the task
	 * list.
	 */
	
	Flags.Tsk |= KWT;
}

void loop_tasks_pause (void)
{
	/* This routine is called when a pause command is encountered in the task
	 * list.
	 */
	
	Flags.Tsk |= KPS;	
}

void loop_tasks_at (void)
{
	/* This routine is called when an 'at' command is encountered in the task
	 * list.
	 */
	
	Flags.Tsk |= KAT;
}

void loop_tasks_script (void)
{
	/* This routine is called when an 'Exec' command is encountered in the 
	 * task list.
	 */
	
	Flags.Tsk |= KEP;
}

void loop_display_blinkrect (gboolean Blink)
{
	/* This routine is called to blink the selection rectangle on the autoguider
	 * image dispay.
	 */
	
	if (Blink)
		Flags.Dsp |= DBR;
	else
		Flags.Dsp |= DSR;
}

guint loop_elapsed_since_first_iteration (void)
{
	/* Returns the number of milliseconds that have elapsed since
	 * first called.
	 */
	
	GTimeVal time;
	static gulong init_msec;
	gulong current_msec;
	static gboolean first_call = TRUE;
	
	g_get_current_time (&time);
	
	if (first_call) {
		first_call = FALSE;
		init_msec = time.tv_sec * 1000 + time.tv_usec / 1000;
	}
	
	current_msec = time.tv_sec * 1000 + time.tv_usec / 1000;
	
	return ((guint) (current_msec - init_msec));
}

static gboolean timeout (guint *timer, guint msec)
{
	/* Time-out timer */
	
	if (!*timer) {
		*timer = loop_elapsed_since_first_iteration ();
		return FALSE;
	}
	
	if (loop_elapsed_since_first_iteration () - *timer > msec) {
		*timer = 0;
		return TRUE;
	} else
	    return FALSE;
}

static gpointer thread_func_ccd (gpointer data)
{
	/* CCD camera monitoring thread function.  Periodic calls to the CCD camera
	 * to monitor its status are made from this thread.  Calls to the QSI and SX
	 * cameras are blocked while they are downloading an image.  Consquently,
	 * any such calls made from the main thread would hang the application while
	 * the camera was being read.
	 * 
	 * Guiding commands issued via the CCD camera are made from the guide 
	 * timing thread in telescope.c.  They will block in that thread whilst the
	 * image is being downloaded.
	 */
	
	while (TRUE) {
		usleep (50*1000);
		
		if (Flags.CCD & CCC)
			g_thread_exit (NULL);
		
		if (Flags.CCD & CSI) {
			Flags.CCD &= ~CSI;
			if (!ccdcam_get_status ())
				Flags.CCD |= CUS;
		}
		
		if (Flags.CCD & CEP) {
			if (ccdcam_image_ready ())
				Flags.CCD |= CIR;
		}
	}
	return NULL;
}

static gpointer thread_func_AFCalib (gpointer data)
{
	/* Autofocus calibration thread */
	
	struct exposure_data exd;
	
	gint repeat;
	gdouble hfd;
	gboolean NextStep = TRUE, Stopped = FALSE;
	
	/* Move focuser to initial position */
	
	AF_focuser_move_and_wait (AFCalib_thread_data.start_pos);

	/* Capture initial CCD image and set bounds for first HFD image */
	
	exd.req_len = AFCalib_thread_data.exp_len;
	ccdcam_set_fast_readspeed (TRUE);
	AF_measure_HFD (TRUE, TRUE, AFCalib_thread_data.box, &exd, &hfd);
	
	/* Loop over requested focus range */
	
	exd.focus_pos = AFCalib_thread_data.start_pos;
	while (NextStep) {
		
		/* Move focuser */
		
		AF_focuser_move_and_wait (exd.focus_pos);
		
	    for (repeat = 0; repeat < AFCalib_thread_data.repeat; repeat++) {
			
			/* Get CCD image and measure HFD.  Keep trying once per second
			 * if no star detected, perhaps due to temporary cloud.
			 */

			while (!AF_measure_HFD (FALSE, TRUE, AFCalib_thread_data.box, 
								    &exd, &hfd)) {
				sleep (1);
				L_print ("Waiting for star... Click 'Stop' to exit "
						 "autofocus calibration\n");
				if (Flags.CCD & CAP) {  /* Test if user selects to stop */
					Stopped = TRUE;
					goto stopped;
				}
			}
		}
			
		/* Any more steps to do? */
		
		exd.focus_pos += (AFCalib_thread_data.end_pos > 
						  AFCalib_thread_data.start_pos) ? 
				          AFCalib_thread_data.step : 
		                 -AFCalib_thread_data.step;
		
		if (AFCalib_thread_data.end_pos > AFCalib_thread_data.start_pos)
			NextStep = (exd.focus_pos <= AFCalib_thread_data.end_pos) ? 
			                                                       TRUE : FALSE;
		else
			NextStep = (exd.focus_pos >= AFCalib_thread_data.end_pos) ? 
			                                                       TRUE : FALSE;
			
		/* Test to see if end of thread has been requested */
		
		if (Flags.CCD & CAP) {
			Stopped = TRUE;
			goto stopped;
		}
	}

stopped:
	Flags.CCD &= ~CAP;
	ccdcam_set_fast_readspeed (FALSE);
	L_print ("{b}Autofocus calibration %s\n", Stopped ? "stopped" : "finished");
	AFC_thread = NULL;
	g_thread_exit (NULL);
	Flags.Foc &= ~FFF;
	return NULL;	
}

static gpointer thread_func_AFFocus (gpointer data)
{
	/* Autofocus focusing thread */

	struct exposure_data exd;
	
	gushort i, retry = 0;
	gint old_p;
	gdouble hfd, hfd_ave, old_hfd;
	gboolean Stopped = FALSE;
	
	/* Set camera readout speed */
	
	ccdcam_set_fast_readspeed (TRUE);  /* Must call this before restart! */
	
	/* Move focuser to initial position */
	
restart:
	AF_focuser_move_and_wait (AFFocus_thread_data.start_pos);
	exd.focus_pos = AFFocus_thread_data.start_pos;

	/* Capture initial CCD image and set bounds for first HFD image */
	
	exd.req_len = AFFocus_thread_data.exp_len;
	AF_measure_HFD (TRUE, FALSE, AFFocus_thread_data.box, &exd, &hfd);
	
	/* Now make first HFD measurement */
	
	AF_measure_HFD (FALSE, FALSE, AFFocus_thread_data.box, &exd, &hfd);
	
	/* Alter focus postion until HFD is within a factor of two of the near-HFD
	 * value.
	 */
	
	old_p = AFFocus_thread_data.start_pos;
	while (hfd > 2.0 * AFFocus_thread_data.near_HFD) {
		exd.focus_pos = rint (old_p + (AFFocus_thread_data.near_HFD - hfd) / 
							   (2.0 * (AFFocus_thread_data.Inside ? 
		                               AFFocus_thread_data.LHSlope : 
		                               AFFocus_thread_data.RHSlope)));
		old_p = exd.focus_pos;
		old_hfd = hfd;
		
		/* Move focuser */
		
		AF_focuser_move_and_wait (exd.focus_pos);
		
		/* Take next exposure */
		
		if (!AF_measure_HFD(FALSE, FALSE, AFFocus_thread_data.box, &exd, &hfd)){
			Stopped = TRUE;
			goto stopped;
		}
		
		/* If HFD got bigger, assume we moved focuser wrong way, so try
		 * again, just once.
		 */
		
		if (hfd > old_hfd) {
			if (++retry < 2) {
				AFFocus_thread_data.Inside = !AFFocus_thread_data.Inside;
				goto restart;
			} else {
				L_print ("{b}Autofocus confused! Stopping...\n");
				Stopped = TRUE;
				goto stopped;
			}
		}
		
		/* Test to see if end of thread has been requested */
		
		if (Flags.CCD & CFP) {
			Stopped = TRUE;
			goto stopped;
		}
	}
	
	/* Now move to estimated position to give near-HFD value */
	
	exd.focus_pos = rint (old_p + (AFFocus_thread_data.near_HFD - hfd) / 
	                              (AFFocus_thread_data.Inside ? 
		                           AFFocus_thread_data.LHSlope : 
		                           AFFocus_thread_data.RHSlope));
	old_p = exd.focus_pos;
	AF_focuser_move_and_wait (exd.focus_pos);
	
	/* Take average of five HFD measurements... */
	
	for (i = 0, hfd_ave = 0.0; i < 5; i++) {
		if (!AF_measure_HFD(FALSE, FALSE, AFFocus_thread_data.box, &exd, &hfd)){
			Stopped = TRUE;
			goto stopped;
		}
		hfd_ave += hfd;
		
		/* Test to see if end of thread has been requested */
		
		if (Flags.CCD & CFP) {
			Stopped = TRUE;
			goto stopped;
		}
	}
	hfd_ave /= 5.0;
	
	/* Move to final position */
	
	exd.focus_pos = rint (old_p - hfd_ave / 
						  (AFFocus_thread_data.Inside ? 
		                   AFFocus_thread_data.LHSlope : 
		                   AFFocus_thread_data.RHSlope) +
	                      (AFFocus_thread_data.Inside ? -1 : 1) * 
	                       AFFocus_thread_data.PID / 2.0);

	AF_focuser_move_and_wait (exd.focus_pos);
	
	/* Get final HFD value for display in log window */
	
	if (!AF_measure_HFD (FALSE, FALSE, AFFocus_thread_data.box, &exd, &hfd)) {
		Stopped = TRUE;
		goto stopped;
	}
	
	/* Store current focuser temperature */
	
	focus_store_temp_and_pos ();
	
stopped:
	Flags.CCD &= ~CFP;
	ccdcam_set_fast_readspeed (FALSE);
	L_print ("{b}Autofocus %s\n", Stopped ? "stopped" : "finished");
	AFF_thread = NULL;
	g_thread_exit (NULL);
	Flags.Foc &= ~FFF;
	return NULL;	
}
	
static void AF_focuser_move_and_wait (gint pos)
{
	/* Move focuser to given position and wait until motion complete */
	
	struct focus f;
		
	f.cmd = FC_MOVE_TO;
	f.move_to = pos;
	focus_comms->focus (&f);
	loop_focus_check_done ();
	while (Flags.Foc & FIM)
		usleep (50*1000);
}

static gboolean AF_measure_HFD (gboolean Init, gboolean Plot, gint box,
								struct exposure_data *exd, gdouble *hfd)
{
	/* Make a CCD exposure to measure and report the current HFD value */
	
	gdouble exp_len;
	gboolean OK;
	
	if (Init) {
		/* The image parameter data read by get_ccd_image_params includes the
		 * exposure length from the CCD tab; we want to preserve the value set
		 * in this function's calling routine.
		 */
		exp_len = exd->req_len;
		gdk_threads_enter ();   /* Try to avoid these calls in future release */
		get_ccd_image_params (exd);
		gdk_threads_leave ();   /* Try to avoid these calls in future release */
		exd->req_len = exp_len;
	}
	ccdcam_set_exposure_data (exd);
	if (ccdcam_start_exposure ())
		Flags.CCD |= CEP;
	while (Flags.CCD & CEP)
		usleep (50*1000);
	OK = ccdcam_measure_HFD (Init, Plot, box, exd, hfd);
	
	return OK;
}

static void thread_pool_autog_calib_func (gpointer data, gpointer user_data)
{
	/* This is the thread pool function for timing autoguider calibration
	 * steps.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	enum TelMotion direction = TM_NONE;
	struct timespec length;
	static gushort calib_step = 0;
	guint duration, now;
	gchar *dir;
	
	/* Initialise if first step, or return if all steps done */
	
	if (GPOINTER_TO_INT (data) > 0) {
		calib_step = 0;
		memset (calib_coords, 0, sizeof (calib_coords));
		get_autog_guide_params ();
		telescope_set_guide_speed (aug->autog.s.GuideSpeed);
	}
	if (calib_step > 4)
		return;
	
	/* Store current star coordinates.  At the first pass through this thread,
	 * the star coordinates are the initial starting position. For subsequent
	 * passes, the coordinates are the result of moving the telescope.
	 */
	
    calib_coords[calib_step][0] = aug->rect.mean[GREY].h;
    calib_coords[calib_step][1] = aug->rect.mean[GREY].v;
	
	/* Issue next 'move' instruction to telescope */
	
	switch (calib_step++) {
		case 0:
		    direction = TM_WEST;
		    duration = (guint) (1000.0 * aug->autog.s.EWCalibDuration);
		    dir = "west";
		    break;
		case 1:
		    direction = TM_EAST;
		    duration = (guint) (1000.0 * aug->autog.s.EWCalibDuration);
		    dir = "east";
		    break;
		case 2:
		    direction = TM_SOUTH;
		    duration = (guint) (1000.0 * aug->autog.s.NSCalibDuration);
		    dir = "south";
		    break;
		case 3:
		    direction = TM_NORTH;
		    duration = (guint) (1000.0 * aug->autog.s.NSCalibDuration);
		    dir = "north";
		    break;
		case 4:
			/* Have captured final star position so set flag and return */
			Flags.Aug |= ACD;
			return;
			break;
		default:
			return;
	}

	L_print ("Moving telescope %s for %d ms\n", dir, duration);
	length.tv_sec = (time_t) (duration / 1000);
	length.tv_nsec = 1e+06 * (duration - 1000 * length.tv_sec);
	/* If the autoguider camera is an SX camera and the guide signals are being
	 * sent via that camera, then ignore the Remote Timing setting and send
	 * separate start and stop signals.  Otherwise the guide pulse timing may
	 * interfere with the exposure timing and in addition the user will not see
	 * the star move since exposures cannot be captured while the timed guide
	 * signal is in progress.
	 */
	if (aug->autog.s.RemoteTiming && 
	  !(autog_comms->pnum == USBAUG && aug->device == SX)) {
		now = loop_elapsed_since_first_iteration ();
		telescope_guide_pulse (direction, (gint) duration);
		if (loop_elapsed_since_first_iteration () - now < duration)
			/* Assume guide_pulse returned immediately; sleep here instead */
			nanosleep (&length, NULL);
	} else {
		telescope_guide_start (direction);
		nanosleep (&length, NULL);
		telescope_guide_stop (direction);
	}
	
	/* For free-running devices, i.e. where we don't control the start/end of
	 * each exposure, we need to allow at least twice the exposure length to 
	 * elapse before grabbing the next frame.  (i.e. one exposure might begin
	 * just before the end of the telescope motion, and we need to get the star
	 * position at the end of the exposure after that).  Measuring the star
	 * position in the correct exposure for devices where we control the start
	 * of the exposure is handled by the loop code.
	 */
	
	if (aug->exd.FreeRunning)
		usleep (1000 * ((gint)(2000.0 * aug->exd.req_len) + 500));/* plus 0.5s*/
	
	/* Set flag to indicate a telescope motion has been made and therefore we
	 * need to get the next star position.  This will be immediate for free
	 * running devices (we've already introduced a delay above), but will be
	 * for the first exposure that starts after this point for devices where we
	 * control the exposure.
	 */
	
	Flags.Aug |= ACM;
}

static void thread_pool_focuser_moving_func (gpointer data, gpointer user_data)
{
	/* This is the thread pool function for checking to see if the focuser is 
	 * moving.  Execution of the main event loop becomes sluggish if this is
	 * done there.  Stopping focuser motion (and checking that it has stopped)
	 * is also done from this thread.
	 */
	
	struct focus f;

	if (Flags.Foc & FSM) {
		Flags.Foc &= ~FSM;
		f.cmd = FC_STOP;
		while (focus_is_moving ())
			focus_comms->focus (&f);
	}
	
	if (focus_is_moving ())
		Flags.Foc |= FCM;
	else
		Flags.Foc |= FSP;
}

static gint event_loop (gpointer data)
{
	/* This is the main event loop that is called periodically by the timer and
	 * controls the sequential operation of events.  It runs in the main gtk
	 * thread.
	 */
	
	static gushort ccd_temp_check = 0;
	static guint wait_temp_t = 0, wait_autog_t = 0, save_autog_t = 0;
	static guint check_status_t = 0, blink_rect_t = 0;
	static guint check_autog_calib_t = 0, check_goto_done_t = 0;
	static guint check_script_done_t = 0, read_focuser_temp_t = 0;
	static gboolean Show = FALSE;
	gboolean AtTemperature, ignore;
	
	tasks_execute_tasks (FALSE, &ignore); /* Anything in the task list? */
	
	if (Flags.Loop & OCL) { /* Cancel event loop */
		Flags.Loop &= ~OCL;
		if (Flags.CCD & CCA) Flags.CCD |= CCC;
		if (Flags.Aug & AGA) Flags.Aug |= AGC;
		if (Flags.Foc & FIM) Flags.Foc |= FSM;
		if (Flags.Lvw & LVI) Flags.Lvw |= (LVS | LVC);
		Flags.Loop |= OIQ;
	}
	
	if (Flags.CCD & CCO) { /* Open CCD camera */
		Flags.CCD &= ~CCO; /* Don't keep trying! */
		if (ccdcam_open ()) {
			set_ccd_gui (TRUE);  /* Set the various elements in the GUI */
			Flags.CCD |= CCA;
			ccd_thread = g_thread_create (thread_func_ccd, NULL, TRUE, NULL);
		} else
			reset_checkbox_state (RCS_OPEN_CCD_LINK, FALSE);
	}
	
	if ((Flags.CCD & CSF) && !(Flags.CCD & CCE)) { /* Set CCD filter */
		if (set_filter (CCD, (gchar *) NULL, &filter_offset)) {
			Flags.CCD &= ~CSF;
			if (get_apply_filter_offset ()) {
				L_print ("{b}Applying filter focus offset of %d steps...\n", 
						                                         filter_offset);
				loop_focus_apply_filter_offset (CCD, filter_offset);
			}
		} else
		    Flags.CCD |= CCE;
	}
	
	if ((Flags.CCD & CST) && !(Flags.CCD & CCE)) { /* Set CCD temperature */
		if (timeout (&wait_temp_t, 5000)) {
		    if (ccdcam_set_temperature (&AtTemperature)) {
				if (AtTemperature) {
					if (++ccd_temp_check == 4) {
						ccd_temp_check = 0;
						Flags.CCD &= ~CST;
					}
				} else
					ccd_temp_check = 0;
			} else 
		        Flags.CCD |= CCE;
		}
	}
	
	if ((Flags.CCD & CSE) && !(Flags.CCD & (CST | CSF))  /* Start exposure */
		                  && !(Flags.Foc & (FAO | FIM))) {
		if (Flags.Aug & AGG) {      /*  If autoguiding and autoguider idle */
			if (Flags.Aug & AGE) {  /*   (i.e. not just made a correction) */
				Flags.CCD &= ~CSE;
				if (ccdcam_start_exposure ())
					Flags.CCD |= CEP;
			} else {                               /* Add message to log once */
			    if (timeout (&wait_autog_t, 1000)) /* per second              */
					L_print ("Waiting for autoguider...\n");
			}
		} else {
			Flags.CCD &= ~CSE;
			if (ccdcam_start_exposure ())
				Flags.CCD |= CEP;
		}
	}
	
	if (Flags.CCD & CIE) { /* Interrupt CCD exposure */
		Flags.CCD &= ~CIE;
		set_progress_bar (TRUE, loop_elapsed_since_first_iteration ());
		ccdcam_interrupt_exposure ();
	}
		
	if (Flags.CCD & CCE) { /* Cancel CCD exposure */
		Flags.CCD &= ~(CCE | CEP | CSF | CST | CSE);
		set_progress_bar (TRUE, loop_elapsed_since_first_iteration ());
		ccdcam_cancel_exposure ();
	}
	
	if (Flags.CCD & CIR) { /* CCD camera image ready */
		Flags.CCD &= ~(CIR | CEP);
		set_progress_bar (TRUE, loop_elapsed_since_first_iteration ());
		if (ccdcam_capture_exposure ()) {
			Flags.CCD |= CDI;
			Flags.Aug &= ~AGE; /* Unset autog. idle flag here; next exposure */
		}                      /* can't start until autog. is next idle      */
	}
	
	if (Flags.CCD & CDI) { /* Save CCD image for display and display it */
		Flags.CCD &= ~CDI;
		if ((get_ccd_image_struct ())->Debayer && 
			(get_ccd_image_struct ())->exd.h_bin == 1 &&
			(get_ccd_image_struct ())->exd.v_bin == 1) {
			if (save_file (get_ccd_image_struct (), R, TRUE))
				xpa_display_image (get_ccd_image_struct (), R);
			if (save_file (get_ccd_image_struct (), G, TRUE))
				xpa_display_image (get_ccd_image_struct (), G);
			if (save_file (get_ccd_image_struct (), B, TRUE))
				xpa_display_image (get_ccd_image_struct (), B);
		} else {
			if (save_file (get_ccd_image_struct (), GREY, TRUE))
				xpa_display_image (get_ccd_image_struct (), GREY);
		}
	}
	
	if (Flags.Img & SCI) { /* Save CCD image if autosave requested */
		Flags.Img &= ~SCI;
		if ((get_ccd_image_struct ())->Debayer && 
			(get_ccd_image_struct ())->exd.h_bin == 1 &&
			(get_ccd_image_struct ())->exd.v_bin == 1) {
			save_file (get_ccd_image_struct (), R, FALSE);
			save_file (get_ccd_image_struct (), G, FALSE);
			save_file (get_ccd_image_struct (), B, FALSE);
		} else
			save_file (get_ccd_image_struct (), GREY, FALSE);
	}
	
	if (Flags.CCD & CDT) { /* Plot CCD temperatures */
		if (!ccdcam_plot_temperatures ())
			Flags.CCD &= ~CDT;
	}
	
	if (Flags.CCD & CCC) { /* Close CCD camera */
		if (Flags.CCD & CEP)
			Flags.CCD |= CCE; /* Cancel any exposure in progress first */
		else {
			if (ccd_thread) {
				g_thread_join (ccd_thread);
				ccd_thread = NULL;
			}
			Flags.CCD &= ~CCC;
			if (ccdcam_close ()) {
				set_ccd_gui (FALSE); /* Reset the various elements in the GUI */
				set_exposure_buttons (FALSE);
				show_camera_status (FALSE);
				Flags.CCD &= ~CCA;
			} else
				reset_checkbox_state (RCS_OPEN_CCD_LINK, TRUE);
		}
	}
	
	if (Flags.Aug & AGO) { /* Open autoguider */
		Flags.Aug &= ~AGO;
		if (!(Flags.Lvw & LVR)) { /* Open only if not already being used by   */
			if (augcam_open ()) { /* live view                                */
				ui_show_aug_window ();
				set_autog_sensitive (TRUE, TRUE);
			    Flags.Aug |= (AGA | ASE);
			} else {
			    Flags.Aug |= AGC; /* Error condition - reset checkbox state   */
				reset_checkbox_state (RCS_USE_AUTOGUIDER, FALSE);
			}
		} else {
			ui_show_aug_window ();
			set_autog_sensitive (TRUE, TRUE);
		    Flags.Aug |= (AGA | ASE);
		}
	}
	
	if ((Flags.Aug & AGA) && (Flags.Aug & ASE)) {/* Start autog. cam. exposure*/
		Flags.Aug &= ~ASE;
		if (Flags.Aug & ACM) { /* Need to store star position in this exposure*/
			Flags.Aug &= ~ACM; /* for autoguider calibration.                 */
			Flags.Aug |= ACE;
		}
		if (augcam_start_exposure ()) {
			Flags.Aug |= AEP;
		}
	}
	
	if (Flags.Aug & AEP) {/* Check for autoguider camera image */
		if (augcam_image_ready ())
			Flags.Aug |= AIR;
	}
	
	if (Flags.Aug & AIR) { /* Autoguider image ready */
		Flags.Aug &= ~(AIR | AEP);
		if (augcam_capture_exposure ()) {
			Flags.Aug |= ASE;
		    if (Flags.Aug & ACE) { /* Finished an exposure following telescope*/
			    Flags.Aug &= ~ACE; /*  motion by the autoguider calibration   */
			    Flags.Aug |= ACG;  /*  procedure, so store this star position.*/
			}
		} else {
			Flags.Aug |= AGC; /* Error condition - reset checkbox state       */
			reset_checkbox_state (RCS_USE_AUTOGUIDER, FALSE);
		}
	}
	
	if ((Flags.Aug & ACC) && (Flags.Aug & ACG)) { /* Calibrate autoguider */
		Flags.Aug &= ~ACG;
		if (!thread_pool_autog_calib) {
			Flags.Aug |= ACI;
			thread_pool_autog_calib = g_thread_pool_new (
							                    &thread_pool_autog_calib_func, 
											    NULL,
											    1, 
												TRUE, 
												NULL);
			g_thread_pool_push (thread_pool_autog_calib, 
								GINT_TO_POINTER (1), 
								NULL);
		} else
			g_thread_pool_push (thread_pool_autog_calib, 
								GINT_TO_POINTER (-1), 
								NULL);
	}
	
	if (Flags.Aug & ACP) { /* Stop autoguider calibration */
		Flags.Aug &= ~(ACP | ACC | ACI);
		if (thread_pool_autog_calib) {
			g_thread_pool_free (thread_pool_autog_calib, TRUE, TRUE);
			thread_pool_autog_calib = NULL;
		}
		Flags.Aug &= ~ACD; /* Just in case thread sets */
	}                      /*  this before exiting...  */
	
	if (Flags.Aug & ACD) { /* Got autoguider calibration star positions */
		Flags.Aug &= ~(ACD | ACC);
		if (thread_pool_autog_calib) {
			g_thread_pool_free (thread_pool_autog_calib, TRUE, TRUE);
			thread_pool_autog_calib = NULL;
		}
		if (telescope_autog_calib ((gfloat *) calib_coords))
			Flags.Aug &= ~ACI;
	}		

	if (Flags.Aug & AGB) { /* Begin autoguiding */
		if (Flags.Aug & ACI) {
			if (timeout (&check_autog_calib_t, 1000))
				L_print("{o}Autoguider waiting for calibration to finish...\n");
		} else if (Flags.Tel & TGP) {
			L_print("{o}Can't begin autoguiding while GoTo in progress!\n");
		} else {
			if (telescope_guide (INIT, loop_elapsed_since_first_iteration ())) {
				set_autog_sensitive (FALSE, FALSE);
				Flags.Aug &= ~AGB;
				Flags.Aug |= AGG;
			}
		}
	}
	
	if (Flags.Aug & AGU) { /* If autoguiding, pause issuing guiding commands */
		Flags.Aug &= ~AGU;
		telescope_guide (PAUSE, loop_elapsed_since_first_iteration ());
		Flags.Aug |= AGP;
	}
	
	if (Flags.Aug & AGN) { /* If paused autoguiding, continue issuing cmds */
		Flags.Aug &= ~AGN;
		telescope_guide (CONT, loop_elapsed_since_first_iteration ());
		Flags.Aug &= ~AGP;
	}	
	
	if ((Flags.Aug & AGG) &&     /* Autoguiding and...                   */
		!(Flags.Aug & AGP) &&    /*  autoguider not paused and...        */
		!(Flags.Aug & AEP)) {    /*  autoguider exposure not in progress */
		if (telescope_guide (GUIDE, loop_elapsed_since_first_iteration ()))
			Flags.Aug |= AGE;
		else
			Flags.Aug &= ~AGE;
	} 
	
	if (Flags.Aug & AGQ) { /* If autoguiding, quit */
		Flags.Aug &= ~(AGQ | AGG | ACI);		
		telescope_guide (QUIT, loop_elapsed_since_first_iteration ());
		set_autog_sensitive (TRUE, FALSE);
	}
	
	if (Flags.Aug & AGC) { /* Close autoguider */
		Flags.Aug &= ~AGC;
		if (Flags.Aug & AGA) {
			Flags.Aug &= ~(AGA | ASE | AEP | AIR);
			ui_hide_aug_window ();
			set_autog_sensitive (FALSE, TRUE);
			if (Flags.Aug & AGG) {
				Flags.Aug &= ~(AGQ | AGG | ACI);
				telescope_guide (QUIT, loop_elapsed_since_first_iteration ());
			}
			if (!(Flags.Lvw & LVR)) /* Close only if camera not being used  */
				augcam_close ();    /* by live view                         */
		}
	}
	
	if (Flags.Aug & AGI) { /* Save autoguider image for display and display it*/
		Flags.Aug &= ~AGI;
		set_fits_data (get_aug_image_struct (), NULL, 
			  (get_aug_image_struct ()->exd.FreeRunning ? FALSE : TRUE), FALSE);			
		if (save_file (get_aug_image_struct (), GREY, TRUE))
			xpa_display_image (get_aug_image_struct (), GREY);
	}
	
	if (Flags.Aug & AGA) { /* Save autoguider image if autosave requested */
		if (Flags.Img & SSA) {
			Flags.Img &= ~SSA;
			set_fits_data (get_aug_image_struct (), NULL, 
			  (get_aug_image_struct ()->exd.FreeRunning ? FALSE : TRUE), FALSE);			
			save_file (get_aug_image_struct (), GREY, FALSE);
		}
	}
	
	if (Flags.Foc & FCM) { /* Check to see if focuser is moving */
		Flags.Foc &= ~FCM;    /* Reset in focuser_moving thread */
		if (Flags.Foc & FCO) {  /* User might have closed focuser link... */
			if (!thread_pool_focuser_moving)
				thread_pool_focuser_moving = g_thread_pool_new (
											   &thread_pool_focuser_moving_func, 
											   NULL,
											   1,
											   TRUE, 
											   NULL);
			g_thread_pool_push (thread_pool_focuser_moving,
									GINT_TO_POINTER (1), /*Dummy data to      */
									NULL);               /*satisfy func. call.*/
		} else
		    Flags.Foc |= FSP;
	}
	
	if (Flags.Foc & FSP) { /* Focuser stopped */
		Flags.Foc &= ~(FSP | FIM);
		g_thread_pool_free (thread_pool_focuser_moving, TRUE, FALSE);
		thread_pool_focuser_moving = NULL;
		set_focus_done ();
	}
	
	if ((Flags.Foc & FCO) && (Flags.Foc & FAO)) { /* Apply filter focus offset*/
		if (!(Flags.Foc & FIM)) {
			apply_filter_focus_offset (cam_type, filter_offset);
			Flags.Foc &= ~FAO;
		}
	}
	
	if ((Flags.Foc & FCO) && (Flags.Foc & FAT)) { /* Apply focus temp. comp. */
		if (!(Flags.Foc & FIM)) {
			struct focus f;
			f.cmd = FC_MOVE_TO;
			f.move_to = tempcomp_pos;
			focus_comms->focus (&f);
			loop_focus_check_done ();
			Flags.Foc &= ~FAT;
		}
	}
	
	#ifdef HAVE_UNICAP
	if (Flags.Lvw & LVO) { /* Open live view display */
		Flags.Lvw &= ~LVO;
		if (!(Flags.Aug & AGA)) { /* Open only if not already being used by   */
			if (augcam_open ())   /* autoguider                               */
				Flags.Lvw |= LVR;
			else {
			    Flags.Aug |= AGC; /* Error condition - reset checkbox state   */
				reset_checkbox_state (RCS_OPEN_LIVEVIEW_WINDOW, FALSE);
			}
		} else {
		    show_liveview_window ();
			Flags.Lvw |= LVR;
		}
	}
	
	if (Flags.Lvw & LVE) { /* Start recording to disk */
		Flags.Lvw &= ~LVE;
		if (video_record_start (VideoDir))
			Flags.Lvw |= LVI;
	}
	
	if (Flags.Lvw & LVI)
		if (Flags.Lvw & LVD) { /* Flush buffer to disk */
			Flags.Lvw &= ~LVD;
			video_flush_buffer (vid_buf);
		}
		
	if (Flags.Lvw & LVS) { /* Stop recording to disk */
		Flags.Lvw &= ~(LVS | LVI);
		video_record_stop ();
	}
	
	if (Flags.Lvw & LVC && !(Flags.Lvw & LVI)) { /* Close live view display */
		Flags.Lvw &= ~LVC;
		if (Flags.Lvw & LVR) {
			Flags.Lvw &= ~LVR;
			hide_liveview_window ();
			if (!(Flags.Aug & AGA)) /* Close only if camera not being used  */
				augcam_close ();    /* by autoguider                        */
		}
	}
	#endif /*HAVE_UNICAP*/
	
	if (Flags.Pbw & PBI) { /* Iterate through video frames */
		video_iter_frames (FALSE);
	}
	
	if (Flags.Pbw & PBF) { /* Final call to video_iter_frames */
		Flags.Pbw &= ~(PBI | PBF);
		video_iter_frames (TRUE);
	}
	
	if (Flags.Tel & TWR) { /* Telescope Warm Restart */
		Flags.Tel &= ~TWR;
		telescope_warm_restart ();
		Flags.Tel |= TWS;
	}
	
	if (Flags.Tel & TWS) { /* Telescope is restarting */
		if (telescope_warm_restart_done ()) {
			Flags.Tel &= ~TWS;
			L_print ("Re-initialising telescope comms link after warm "
					                                               "restart\n");
			telescope_close_comms_port ();  /* Re-initialise telescope link */
			if (!telescope_open_comms_port ())
				reset_checkbox_state (RCS_OPEN_COMMS_LINK, FALSE);
		}
	}			
	
	if (Flags.Tel & TGO) { /* Telescope GoTo */
		Flags.Tel &= ~TGO;
		if (Flags.Aug & AGG)
			L_print ("{o}Can't execute GoTo whilst autoguiding!\n");
		else {
			telescope_goto (sRA, sDec);
			Flags.Tel |= TGP;
		}
	}

	if (Flags.Tel & TGP) { /* Telescope GoTo is in progress */
		if (timeout (&check_goto_done_t, 5000)) { /* Check every 5s, waiting  */
			if (telescope_goto_done ())           /* 5s initially to make     */
				Flags.Tel &= ~TGP;                /* sure GoTo has started!   */
		}
	}
	
	if (Flags.Tel & TMV) { /* Telescope Move */
		Flags.Tel &= ~TMV;
		if (Flags.Aug & AGG)
			L_print ("{o}Can't execute Move whilst autoguiding!\n");
		else {
			telescope_move_by (MoveRA, MoveDec);
			Flags.Tel |= TGP;
		}
	}

	if (Flags.Tel & TPM) { /* Telescope Park Mount */
		Flags.Tel &= ~TPM;
		if (Flags.Aug & AGG)
			L_print ("{o}Can't park mount whilst autoguiding!\n");
		else {
			telescope_park_mount ();
		    Flags.Tel |= TPK;
		}
	}
	
	if (Flags.Tel & TPK) { /* Telescope is parking */
		if (telescope_park_mount_done ()) {
			Flags.Tel &= ~TPK;
			save_RA_worm_pos (telescope_get_RA_worm_pos ());
		}
	}

	if (Flags.Tel & TYB) { /* Telescope Yellow Button */
		Flags.Tel &= ~TYB;
		telescope_yellow_button ();
	}
	
	if (Flags.Wat & WAC) { /* Watch activate */
		Flags.Wat &= ~WAC;
		tasks_activate_watch ();
		Flags.Wat |= WAR;
	}
	
	if (Flags.Wat & WAS) /* Stop watching file */
		Flags.Wat &= ~(WAS | WAR);
	
	if (Flags.Wat & WAR) /* Watch is running */
		tasks_watch_file ();
	
	if (Flags.Tsk & KEP) { /* Check if script execution is in progress */
		if (timeout (&check_script_done_t, 1000)) { /* Check once per second */
			if (tasks_script_done ())
				Flags.Tsk &= ~KEP;
		}
	}
	
	if (Flags.CCD & CEP) /* Set exposure progress bar on main window */
		set_progress_bar (FALSE, loop_elapsed_since_first_iteration ());
	
	/***************************************************/
	/* Perform any checks at the end of this iteration */
	/***************************************************/
	
	if (Flags.Img & SPA) {     /* Save a periodic autoguider image */
		if (Flags.Aug & AGA) { /* provided autoguider is idle      */
			if (timeout (&save_autog_t, save_period * 1000)) {
				set_fits_data (get_aug_image_struct (), NULL, 													  
							  (get_aug_image_struct ()->exd.FreeRunning ? 
								FALSE : TRUE), FALSE);			
				if (Flags.Aug & AGG) {
					if (Flags.Aug & AGE) {
						save_file (get_aug_image_struct (), GREY, FALSE);
					}
				} else {
					save_file (get_aug_image_struct (), GREY, FALSE);
				}
			}
		}
	}
		
	if (Flags.CCD & CCA) { /* Display the CCD camera status */
		if (timeout (&check_status_t, 1000)) {
			Flags.CCD |= CSI;
		    show_camera_status (TRUE);
		}
	}
	
	if (Flags.CCD & CUS) { /* Unable to obtain camera status */
		Flags.CCD &= ~CUS;
		L_print ("{r}Error - Unable to obtain CCD camera status information\n");
	    reset_checkbox_state (RCS_OPEN_CCD_LINK, FALSE);  /* Error condition -*/
		Flags.CCD |= CCC;            /* Close camera and reset checkbox state */
	}
	
	if ((Flags.Foc & FCO) && !(Flags.Foc & FIM) && !(Flags.Foc & FFF)) {
		if (timeout (&read_focuser_temp_t, 60000)) /* Read focuser temperature*/
			check_focuser_temp ();
	}
	
	if (Flags.Dsp & DBR) { /* Blink selection rectangle */
		if (timeout (&blink_rect_t, 500)) {
			ui_show_augcanv_rect (Show);
			Show = !Show;
		}
	}
	
	if (Flags.Dsp & DSR) { /* Restore selection rectangle after blinking */
		Flags.Dsp &= ~(DSR | DBR);
		ui_show_augcanv_rect (TRUE);
	}		
	
	if (Flags.Tsk & KWT)  /* Check if task Wait time has elapsed */
		if (tasks_task_wait ())
			Flags.Tsk &= ~KWT;
		
	if (Flags.Tsk & KPS)  /* Check if task Pause time has elapsed */
		if (tasks_task_pause ())
			Flags.Tsk &= ~KPS;	

	if (Flags.Tsk & KAT)  /* Check if task At time has been reached */
		if (tasks_task_at ())
			Flags.Tsk &= ~KAT;	
			
	FlushLog ();  /* Flush pending messages from other threads to log window */
			
	if (Flags.Loop & OIQ) {
		if (!(Flags.CCD & CCC) && !(Flags.Aug & AGC) && !(Flags.Foc & FIM) &&
			                                               !(Flags.Lvw & LVC)) {
			Flags.Loop &= ~OIQ;
			gtk_main_quit ();
		    return 0;  /* Timer automatically removed when returning zero */
		}
	}
	
	/***************************************************/
	/* End of checks at the end of this iteration      */
	/***************************************************/
	
	return 1; /* Return non-zero to keep event loop alive */
}
