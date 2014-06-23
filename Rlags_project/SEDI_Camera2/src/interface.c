/******************************************************************************/
/*                    MAIN ROUTINES FOR GRAPHICAL INTERFACE                   */
/*                                                                            */
/* The routines in this file cover all the interactions between the User and  */
/* the UI (e.g. data entry, button presses etc) and display of data on the    */
/* canvas.                                                                    */
/*                                                                            */
/* Copyright (C) 2009 - 2014  Edward Simonson                                 */
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
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include <goocanvas.h>
#ifdef HAVE_LIBGRACE_NP
#include <grace_np.h>
#endif
#ifdef HAVE_LIBUSB
#include "gqusb.h"
#endif

#define GOQAT_INTERFACE
#include "interface.h"

#ifdef HAVE_SX_CAM
#include <libusb-1.0/libusb.h>
#endif

#ifndef HAVE_CONFIG_H
#define PACKAGE "GoQat"
#define VERSION "X.xx"
#endif

#ifdef HAVE___AUTOGEN_SH
#undef GOQAT_DATDIR
#define GOQAT_DATADIR "../" 
#undef GLADE_INTERFACE
#define GLADE_INTERFACE "../GoQat-gtkbuilder.glade"
#undef GRACE_TEMPS
#define GRACE_TEMPS "../data/temps.agr"
#undef GRACE_TRACE
#define GRACE_TRACE "../data/trace.agr"
#undef GRACE_HFD
#define GRACE_HFD "../data/hfd.agr"
#undef GOQAT_HFD_PL
#define GOQAT_HFD_PL "../data/Analyse_HFD.pl"
#endif

#define CONFIG_FILE "GoQat.conf"                /* Configuration file name    */
                                                /* Default font for canvas    */
#define FONT "-*-courier-medium-r-normal-*-*-120-*-*-*-*-*"  
#define MAX_LOG_BUF_MSG 100                     /* Max. messages in log buffer*/

                                                /* Reading config string      */
#define R_config_d(string1, arg) atoi(ReadCString(string1, "%d", arg))
#define R_config_f(string1, arg) atof(ReadCString(string1, "%f", arg))
#define R_config_s(string1, arg) strcpy(arg, ReadCString(string1, "%s", arg))
                                                /* Writing config string      */
#define W_config_d(string1, arg) WriteCString(string1, g_strdup_printf("%d", arg))
#define W_config_f(string1, arg) WriteCString(string1, g_strdup_printf("%f", arg))
#define W_config_s(string1, arg) WriteCString(string1, g_strdup_printf("%s", arg))

#define set_spin_int(string, arg) set_spin_float(string, (gdouble) arg)

enum AugSettings {TELESCOPE, INSTRUMENT, NSCALIBDURATION, EWCALIBDURATION,
	              GUIDESPEED, MAXSHIFT, MAXDRIFT, MAXMOVE, MAXOFFSET, 
				  DRIFTSAMPLE, CORRFAC, UPDATE, N_COLUMNS};
				                      /* Autoguider settings data fields      */
				  
static GtkBuilder *xml_app = NULL;    /* Main application window              */
static GtkBuilder *xml_img = NULL;    /* Autoguider image window              */
static GtkBuilder *xml_set = NULL;    /* Autoguider settings window           */
static GtkBuilder *xml_agt = NULL;	  /* Autoguider trace window              */
static GtkBuilder *xml_dev = NULL;    /* Device selection window              */
static GtkBuilder *xml_con = NULL;    /* CCD camera configuration window      */
#ifdef HAVE_UNICAP
static GtkBuilder *xml_uni = NULL;    /* Unicap device selection window       */
#endif
static GtkBuilder *xml_V4L = NULL;    /* V4L configuration window             */
static GtkBuilder *xml_fil = NULL;    /* Filter wheel configuration window    */
static GtkBuilder *xml_fcw = NULL;    /* Focuser configuration window         */
static GtkBuilder *xml_ppd = NULL;    /* Parallel port settings window        */
static GtkBuilder *xml_tsk = NULL;    /* Tasks editing window                 */
static GtkBuilder *xml_lvw = NULL;    /* Live view window                     */
static GtkBuilder *xml_pbw = NULL;    /* Video playback window                */
static GtkBuilder *xml_svf = NULL;    /* Save video file types window         */
static GtkBuilder *xml_pht = NULL;    /* Photometry settings window           */
static GtkBuilder *xml_font = NULL;   /* Font selection dialog                */

static GtkWindow *ccdApp;             /* Window for main application          */
static GtkStatusbar *stsAppStatus;    /* Main application status bar          */
static GtkProgressBar *prgAppBar;     /* Main application progress bar        */
static GtkStatusbar *stsImageStatus;  /* Image window status bar              */
static GHashTable *hshCombo;          /* Hash table for combo box pointers    */
static GKeyFile *KeyFileData;         /* Configuration data                   */
static GtkTreeView *trvSetting;       /* Autog. settings dialog tree view     */
static GtkListStore *lisSetting;      /* List store - autoguider config. data */
static GtkWidget *CCDConfigWin = NULL;/* Currently open CCD config. window    */
static GtkWidget *cnvImage = NULL;    /* Canvas for autoguider images         */
static GtkWidget *cnvHist = NULL;     /* Canvas for autoguider histogram      */
static GooCanvasItem *cgpImage;       /* Autoguider image canvas root group   */
static GooCanvasItem *cgpHist;        /* Autog. histogram canvas root group   */
static GtkWidget *cnvPlayback = NULL; /* Canvas for video playback            */
static GooCanvasItem *cgpPlayback;    /* Video canvas root group              */
static GooCanvasItem *cgpPhotom = NULL; /* Photometric detections group       */
static GtkTextView *txtStatus;        /* CCD camera status window             */
static GtkTextBuffer *txtStatusBuffer;/* Camera status window text buffer     */
static GtkTextView *txtLog;           /* Main log window                      */
static GtkTextBuffer *txtLogBuffer;   /* Main log window text buffer          */
static GtkTextView *txtPhot;          /* Photometry log window                */
static GtkTextBuffer *txtPhotBuffer;  /* Photmetry log window text buffer     */
static GtkTextTag *red;               /* Colour tag for text buffer           */
static GtkTextTag *orange;            /* Colour tag for text buffer           */
static GtkTextTag *blue;              /* Colour tag for text buffer           */
static GtkTextTag *green;             /* Colour tag for text buffer           */
static GtkTextTag *magenta;           /* Colour tag for text buffer           */
static GtkTextTag *magenta_1;         /* Colour tag for text buffer           */
static GtkTextTag *courier;           /* Font tag for text buffer             */

struct cam_img *CCDConfigOwner;       /* Owner of CCD config. window (ccd/aug)*/
static GStaticMutex LogMutex = G_STATIC_MUTEX_INIT; /* Log buffer mutex       */
static pid_t main_tid;                /* Thread id for main (gtk) thread      */
static gint MsgNum = 0;               /* Number of next message in log buffer */
static gchar *LogBuf[MAX_LOG_BUF_MSG];/* Buffer for pending log messages      */
static gchar font[MCSL];              /* Canvas font                          */
const gchar *UserDir;                 /* Path to user's home folder           */
gchar *PrivatePath;                   /* Path to folder for private files     */
gchar *ConfigFile;                    /* Path to configuration file           */
gchar *WatchFile;                     /* Path to 'watch' file                 */
static gchar *tp[NUMTPARAMS];         /* Task parameters (%1, %2 etc)         */
static gboolean CCDAppWinConf = FALSE;/* TRUE if main app. window configured  */
static gboolean AFWinConf = FALSE;    /* TRUE if autofocus conf. window conf'd*/
static gboolean PPWinConf = FALSE;    /* TRUE if parallel port window conf'd  */
static gboolean EdWinConf = FALSE;    /* TRUE if 'Edit Tasks' window config'd */
static gboolean CCDConfigWinConf = FALSE; /* TRUE if 'CCD config.' window conf*/
static gboolean V4LWinConf = FALSE;   /* TRUE if 'V4L config.' window config'd*/
static gboolean FilWinConf = FALSE;   /* TRUE if 'filter config.' window conf.*/
static gboolean PBWinConf = FALSE;    /* TRUE if playback window configured   */
static gboolean PhotWinConf = FALSE;  /* TRUE if 'Photometry' window config'd */
static gboolean ResetChkState = FALSE;/* TRUE if checkbox state is being reset*/
gboolean GeminiCmds = FALSE;          /* TRUE if Gemini commands to be used   */
gboolean UseUTC = FALSE;              /* TRUE if log files and 'At' use UTC   */
gboolean Debug = FALSE;               /* TRUE if debug messages written to log*/
static FILE *f_log = NULL;            /* Pointer to log file                  */

/******************************************************************************/
/*                                 EVENTS                                     */
/******************************************************************************/

gboolean on_ccdApp_configure (GtkWidget *widget, GdkEventExpose *event,
                              gpointer data);
gboolean on_ccdApp_delete (GtkWidget *widget, GdkEventAny *event,
                           gpointer data);
gboolean on_ccdApp_key_press (GtkWidget *widget, GdkEventKey *event,
                              gpointer data);
gboolean on_txtVBin_focus_out (GtkWidget *widget, GdkEventKey *event,
                               gpointer data);
gboolean on_wndImage_configure (GtkWidget *widget, GdkEventExpose *event,
                                gpointer data);
gboolean on_wndImage_delete (GtkWidget *widget, GdkEventAny *event,
                             gpointer data);
gboolean on_wndImage_key_press (GtkWidget *widget, GdkEventKey *event, 
                                gpointer data);
gboolean on_cnvImage_button_press (GtkWidget *widget, GdkEventButton *event,
                                   gpointer data);
gboolean on_cnvImage_button_release (GtkWidget *widget, GdkEventButton *event,
                                     gpointer data);
gboolean on_cnvImage_motion_notify (GtkWidget *widget, GdkEventMotion *event,
                                    gpointer data);
gboolean on_wndSetting_delete (GtkWidget *widget, GdkEventAny *event,
                               gpointer data);
gboolean on_wndConfigAutofocus_configure (GtkWidget *widget, 
										  GdkEventExpose *event, gpointer data);
gboolean on_wndConfigAutofocus_delete (GtkWidget *widget, GdkEventAny *event,
                                       gpointer data);	
gboolean on_wndParallelPort_configure (GtkWidget *widget, GdkEventExpose *event,
                               	       gpointer data);
gboolean on_wndParallelPort_delete (GtkWidget *widget, GdkEventAny *event,
                                    gpointer data);	
gboolean on_wndEditTasks_configure (GtkWidget *widget, GdkEventExpose *event,
                               	    gpointer data);
gboolean on_wndEditTasks_delete (GtkWidget *widget, GdkEventAny *event,
                                 gpointer data);	
gboolean on_wndQSIConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                    gpointer data);								 
gboolean on_wndQSIConfig_delete (GtkWidget *widget, GdkEventAny *event,
                                 gpointer data);
gboolean on_wndSXConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                   gpointer data);
gboolean on_wndSXConfig_delete (GtkWidget *widget, GdkEventAny *event,
                                gpointer data);
gboolean on_wndV4LConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                    gpointer data);
gboolean on_wndV4LConfig_delete (GtkWidget *widget, GdkEventAny *event,
                                 gpointer data);
gboolean on_wndFilterConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                       gpointer data);
gboolean on_wndFilterConfig_delete (GtkWidget *widget, GdkEventExpose *event,
                                    gpointer data);
gboolean on_trvTasks_button_release (GtkWidget *widget, GdkEventButton *event,
                                     gpointer data);
#ifdef HAVE_UNICAP
gboolean on_wndLiveView_configure (GtkWidget *widget, GdkEventExpose *event,
                               	   gpointer data);
gboolean on_wndLiveView_delete (GtkWidget *widget, GdkEventAny *event,
                                gpointer data);	
#endif
gboolean on_wndPlayback_configure (GtkWidget *widget, GdkEventExpose *event,
                               	   gpointer data);
gboolean on_wndPlayback_delete (GtkWidget *widget, GdkEventAny *event,
                                gpointer data);
gboolean on_cnvPlayback_button_press (GtkWidget *widget, GdkEventButton *event, 
                                      gpointer data);
gboolean on_cnvPlayback_button_release (GtkWidget *widget,GdkEventButton *event, 
                                        gpointer data);
gboolean on_cnvPlayback_motion_notify (GtkWidget *widget, GdkEventMotion *event, 
                                       gpointer data);
gboolean on_wndPhotom_configure (GtkWidget *widget, GdkEventExpose *event,
                               	 gpointer data);
gboolean on_wndPhotom_delete (GtkWidget *widget, GdkEventAny *event,
                              gpointer data);
 
/******************************************************************************/
/*                                 SIGNALS                                    */
/******************************************************************************/

void on_ccdApp_destroy (GtkWidget *widget, gpointer data);
void on_FileSaveCCD_activate (GtkWidget *widget, gpointer data);
void on_FileSaveAUG_activate (GtkWidget *widget, gpointer data);
void on_WriteDebugToLog_activate (GtkWidget *widget, gpointer data);
void on_WriteLog_activate (GtkWidget *widget, gpointer data);
void on_ClearLog_activate (GtkWidget *widget, gpointer data);
void on_FileExit_activate (GtkWidget *widget, gpointer data);
void on_Communications_activate (GtkWidget *widget, gpointer data);
void on_TelescopeOpen_activate (GtkWidget *widget, gpointer data);
void on_GeminiCommands_activate (GtkWidget *widget, gpointer data);
void on_AutogOpen_activate (GtkWidget *widget, gpointer data);
void on_FilterWheelOpen_activate (GtkWidget *widget, gpointer data);
void on_FocusOpen_activate (GtkWidget *widget, gpointer data);
void on_ParallelPort_activate (GtkWidget *widget, gpointer data);
void on_ParPortOpen_activate (GtkWidget *widget, gpointer data);
void on_CCDCamType_activate (GtkWidget *widget, gpointer data);
void on_CCDSelect_activate (GtkWidget *widget, gpointer data);
void on_CCDOpen_activate (GtkWidget *widget, gpointer data);
void on_CCDConfig_activate (GtkWidget *widget, gpointer data);
void on_FullFrame_activate (GtkWidget *widget, gpointer data);
void on_Debayer_activate (GtkWidget *widget, gpointer data);
void on_AutogCamType_activate (GtkWidget *widget, gpointer data);
void on_UnicapDevice_activate (GtkWidget *widget, gpointer data);
void on_UnicapProperties_activate (GtkWidget *widget, gpointer data);
void on_V4LProperties_activate (GtkWidget *widget, gpointer data);
void on_SXCamSelect_activate (GtkWidget *widget, gpointer data);
void on_Greyscale_activate (GtkWidget *widget, gpointer data);
void on_Setting_activate (GtkWidget *widget, gpointer data);
void on_FilterWheelType_activate (GtkWidget *widget, gpointer data);
void on_FilterWheelSelect_activate (GtkWidget *widget, gpointer data);
void on_FiltersConfig_activate (GtkWidget *widget, gpointer data);
void on_FocuserType_activate (GtkWidget *widget, gpointer data);
void on_PEC_activate (GtkWidget *widget, gpointer data);
void on_PrecessCoords_activate (GtkWidget *widget, gpointer data);
void on_UseUTC_activate (GtkWidget *widget, gpointer data);
void on_SetCanvasFont_activate (GtkWidget *widget, gpointer data);
void on_LiveView_activate (GtkWidget *widget, gpointer data);
void on_Playback_activate (GtkWidget *widget, gpointer data);
void on_CCDTemps_activate (GtkWidget *widget, gpointer data);
void on_AutogTrace_activate (GtkWidget *widget, gpointer data);
void on_ShowToolbar_activate (GtkWidget *widget, gpointer data);
void on_HelpAbout_activate (GtkWidget *widget, gpointer data);
void on_btnFullFrame_clicked (GtkButton *button, gpointer data);
void on_btnGetRegion_clicked (GtkButton *button, gpointer data);
void on_spbBin_value_changed (GtkSpinButton *spin, gpointer data);
void on_txtExposure_activate (GtkEditable *editable, gpointer data);
void on_cmbExpType_changed (GtkWidget *widget, gpointer data);
void on_chkIgnoreCCDCooling_toggled (GtkButton *button, gpointer data);
void on_chkDisplayCCDImage_toggled (GtkButton *button, gpointer data);
void on_chkBeepExposure_toggled (GtkButton *button, gpointer data);
void on_btnStart_clicked (GtkButton *button, gpointer data);
void on_btnCancel_clicked (GtkButton *button, gpointer data);
void on_btnInterrupt_clicked (GtkButton *button, gpointer data);
void on_chkAutoSave_toggled (GtkButton *button, gpointer data);
void on_chkSaveEvery_toggled (GtkButton *button, gpointer data);
void on_chkWatchActive_toggled (GtkButton *button, gpointer data);
void on_btnPPClose_clicked (GtkButton *button, gpointer data);
void on_btnZoomIn_clicked (GtkButton *button, gpointer data);
void on_btnZoomOut_clicked (GtkButton *button, gpointer data);
void on_btnZoom1to1_clicked (GtkButton *button, gpointer data);
void on_btnResetArea_clicked (GtkButton *button, gpointer data);
void on_txtImgSatLevel_activate (GtkEntry *entry, gpointer data);
void on_txtImgExpLength_activate (GtkEntry *entry, gpointer data);
void on_optImgBin_toggled (GtkButton *button, gpointer data);
void on_hscBackground_changed (GtkRange *range, gpointer data);
void on_hscBrightness_changed (GtkRange *range, gpointer data);
void on_hscContrast_changed (GtkRange *range, gpointer data);
void on_hscGamma_changed (GtkRange *range, gpointer data);
void on_hscGain_changed (GtkRange *range, gpointer data);
void on_btnFntSel_clicked (GtkButton *button, gpointer data);
void on_chkAutogOpen_toggled (GtkButton *button, gpointer data);
void on_chkAutogWrite_toggled (GtkButton *button, gpointer data);
void on_spbAutogGuideSpeed_changed (GtkSpinButton *spin, gpointer data);
void on_btnAutogDS9_clicked (GtkButton *button, gpointer data);
void on_btnAutogCapDark_clicked (GtkButton *button, gpointer data);
void on_chkAutogSubDark_toggled (GtkButton *button, gpointer data);
void on_txtAutogSkySigma_activate (GtkEntry *entry, gpointer data);
void on_txtAutogCentroidSize_activate (GtkEntry *entry, gpointer data);
void on_chkAutogShowCentroid_toggled (GtkButton *button, gpointer data);
void on_tglAutogStart_toggled (GtkButton *button, gpointer data);
void on_tglAutogPause_toggled (GtkButton *button, gpointer data);
void on_btnAutog_pressed (GtkButton *button, gpointer data);
void on_btnAutog_released (GtkButton *button, gpointer data);
void on_btnAutogCalibrate_clicked (GtkButton *button, gpointer data);
void on_btnSaveSetting_clicked (GtkButton *button, gpointer data);
void on_btnLoadSetting_clicked (GtkButton *button, gpointer data);
void on_btnDeleteSetting_clicked (GtkButton *button, gpointer data);
void on_btnCloseSetting_clicked (GtkButton *button, gpointer data);
void on_btnFocusIn_clicked (GtkButton *button, gpointer data);
void on_btnFocusOut_clicked (GtkButton *button, gpointer data);
void on_btnFocusMoveTo_clicked (GtkButton *button, gpointer data);
void on_btnFocusStop_clicked (GtkButton *button, gpointer data);
void on_btnFocusConfig_clicked (GtkButton *button, gpointer data);
void on_btnFocusFocus_clicked (GtkButton *button, gpointer data);
void on_btnFocusAFStop_clicked (GtkButton *button, gpointer data);
void on_btnFocusMaxTravelSet_clicked (GtkButton *button, gpointer data);
void on_btnFocusCurrentPosSet_clicked (GtkButton *button, gpointer data);
void on_btnFocusBacklashSet_clicked (GtkButton *button, gpointer data);
void on_btnFocusMotorConfigSet_clicked (GtkButton *button, gpointer data);
void on_btnFocusGetCurrentSettings_clicked (GtkButton *button, gpointer data);
void on_chkFocusApplyFilterOffsets_toggled (GtkButton *button, gpointer data);
void on_chkFocusTempComp_toggled (GtkButton *button, gpointer data);
void on_chkFocusFastReadout_toggled (GtkButton *button, gpointer data);
void on_btnAFConfigMeasureHFD_clicked (GtkButton *button, gpointer data);
void on_btnAFConfigClearData_clicked (GtkButton *button, gpointer data);
void on_btnAFConfigStopHFD_clicked (GtkButton *button, gpointer data);
void on_btnAFConfigCalculate_clicked (GtkButton *button, gpointer data);
void on_btnAFConfigUseResults_clicked (GtkButton *button, gpointer data);
void on_btnGemModelLoad_clicked (GtkButton *button, gpointer data);
void on_btnGemModelSave_clicked (GtkButton *button, gpointer data);
void on_btnGemModelRead_clicked (GtkButton *button, gpointer data);
void on_btnGemModelWrite_clicked (GtkButton *button, gpointer data);
void on_btnGemPECLoad_clicked (GtkButton *button, gpointer data);
void on_btnGemPECSave_clicked (GtkButton *button, gpointer data);
void on_btnGemDefaultsLoad_clicked (GtkButton *button, gpointer data);
void on_btnGemSetTime_clicked (GtkButton *button, gpointer data);
void on_btnGemStatus_clicked (GtkButton *button, gpointer data);
void on_btnTaskEdit_clicked (GtkButton *button, gpointer data);
void on_btnTaskUp_clicked (GtkButton *button, gpointer data);
void on_btnTaskDown_clicked (GtkButton *button, gpointer data);
void on_btnTaskDelete_clicked (GtkButton *button, gpointer data);
void on_btnTaskClear_clicked (GtkButton *button, gpointer data);
void on_btnTaskStart_clicked (GtkButton *button, gpointer data);
void on_btnTaskPause_clicked (GtkButton *button, gpointer data);
void on_btnTaskStop_clicked (GtkButton *button, gpointer data);
void on_btnTObject_clicked (GtkButton *button, gpointer data);
void on_btnTBeginSequence_clicked (GtkButton *button, gpointer data);
void on_btnTWaitUntil_clicked (GtkButton *button, gpointer data);
void on_btnTPauseFor_clicked (GtkButton *button, gpointer data);
void on_btnTAt_clicked (GtkButton *button, gpointer data);
void on_btnTExpose_clicked (GtkButton *button, gpointer data);
void on_cmbTExpType_changed (GtkWidget *widget, gpointer data);
void on_btnTBeginLoop_clicked (GtkButton *button, gpointer data);
void on_btnTEndLoop_clicked (GtkButton *button, gpointer data);
void on_btnTFocusTo_clicked (GtkButton *button, gpointer data);
void on_btnTFocusMove_clicked (GtkButton *button, gpointer data);
void on_btnTIfTrue_clicked (GtkButton *button, gpointer data);
void on_btnTIfFalse_clicked (GtkButton *button, gpointer data);
void on_btnTEndIf_clicked (GtkButton *button, gpointer data);
void on_btnTWhile_clicked (GtkButton *button, gpointer data);
void on_btnTEndWhile_clicked (GtkButton *button, gpointer data);
void on_btnTAugOn_clicked (GtkButton *button, gpointer data);
void on_btnTAugOff_clicked (GtkButton *button, gpointer data);
void on_btnTGuideStart_clicked (GtkButton *button, gpointer data);
void on_btnTGuideStop_clicked (GtkButton *button, gpointer data);
void on_btnTGoTo_clicked (GtkButton *button, gpointer data);
void on_btnTMove_clicked (GtkButton *button, gpointer data);
void on_btnTExec_clicked (GtkButton *button, gpointer data);
void on_btnTExecAsync_clicked (GtkButton *button, gpointer data);
void on_btnTWarmRestart_clicked (GtkButton *button, gpointer data);
void on_btnTParkMount_clicked (GtkButton *button, gpointer data);
void on_btnTRecordStart_clicked (GtkButton *button, gpointer data);
void on_btnTRecordStop_clicked (GtkButton *button, gpointer data);
void on_btnTYellow_clicked (GtkButton *button, gpointer data);
void on_btnTShutdown_clicked (GtkButton *button, gpointer data);
void on_btnTExit_clicked (GtkButton *button, gpointer data);
void on_btnTLoadTasks_clicked (GtkButton *button, gpointer data);
void on_btnTSaveTasks_clicked (GtkButton *button, gpointer data);
void on_btnTCloseWindow_clicked (GtkButton *button, gpointer data);
void on_btnLVClose_clicked (GtkButton *button, gpointer data);
void on_btnPBZoomIn_clicked (GtkButton *button, gpointer data);
void on_btnPBZoomOut_clicked (GtkButton *button, gpointer data);
void on_btnPBZoom1to1_clicked (GtkButton *button, gpointer data);
void on_btnPBResetArea_clicked (GtkButton *button, gpointer data);
void on_btnPBOpenFile_clicked (GtkButton *button, gpointer data);
void on_txtPBFrameNum_activate (GtkEditable *editable, gpointer data);
void on_txtPBTimeStamp_activate (GtkEditable *editable, gpointer data);
void on_btnPBSetTimes_clicked (GtkButton *button, gpointer data);
void on_hscPBFrames_changed (GtkRange *range, gpointer data);
void on_btnPBPlay_clicked (GtkButton *button, gpointer data);
void on_btnPBStop_clicked (GtkButton *button, gpointer data);
void on_btnPBPrev_clicked (GtkButton *button, gpointer data);
void on_btnPBNext_clicked (GtkButton *button, gpointer data);
void on_btnPBMarkFirst_clicked (GtkButton *button, gpointer data);
void on_btnPBMarkLast_clicked (GtkButton *button, gpointer data);
void on_cmbPBfps_changed (GtkComboBox *combo, gpointer data);
void on_btnPBSaveFrames_clicked (GtkButton *button, gpointer data);
void on_btnPBPhotom_clicked (GtkButton *button, gpointer data);
void on_btnPBClose_clicked (GtkButton *button, gpointer data);
void on_btnPhotSingle_clicked (GtkButton *button, gpointer data);
void on_btnPhotRange_clicked (GtkButton *button, gpointer data);
void on_btnPhotClearLog_clicked (GtkButton *button, gpointer data);
void on_btnPBClose_clicked (GtkButton *button, gpointer data);
void on_btnSVOK_clicked (GtkButton *button, gpointer data);
void on_btnDevSelect_clicked (GtkButton *button, gpointer data);
void on_btnUnicapSelect_clicked (GtkButton *button, gpointer data);
void on_btnV4LConfigApply_clicked (GtkButton *button, gpointer data);
void on_btnV4LConfigClose_clicked (GtkButton *button, gpointer data);
void on_chkCConfCoolOnConnect_toggled (GtkButton *button, gpointer data);
void on_btnCConfSetDefTemp_clicked (GtkButton *button, gpointer data);
void on_btnCConfSetTempTol_clicked (GtkButton *button, gpointer data);
void on_btnCConfCoolerOn_clicked (GtkButton *button, gpointer data);
void on_btnCConfCoolerOff_clicked (GtkButton *button, gpointer data);
void on_optCConfFans_toggled (GtkButton *button, gpointer data);
void on_btnCConfSetFans_clicked (GtkButton *button, gpointer data);
void on_cmbCConfShutPrior_changed (GtkComboBox *combo, gpointer data);
void on_cmbCConfShutMode_changed (GtkComboBox *combo, gpointer data);
void on_cmbCConfShutOpen_changed (GtkComboBox *combo, gpointer data);
void on_cmbCConfPreFlush_changed (GtkComboBox *combo, gpointer data);
void on_cmbCConfFastMode_changed (GtkComboBox *combo, gpointer data);
void on_cmbCConfCameraGain_changed (GtkComboBox *combo, gpointer data);
void on_cmbCConfReadoutSpeed_changed (GtkComboBox *combo, gpointer data);
void on_cmbCConfAntiBlooming_changed (GtkComboBox *combo, gpointer data);
void on_btn_CConfSetShutPrior_clicked (GtkButton *button, gpointer data);
void on_btn_CConfSetShutMode_clicked (GtkButton *button, gpointer data);
void on_btn_CConfSetShutOpen_clicked (GtkButton *button, gpointer data);
void on_btn_CConfSetPreFlush_clicked (GtkButton *button, gpointer data);
void on_btn_CConfSetFastMode_clicked (GtkButton *button, gpointer data);
void on_btn_CConfSetCamGain_clicked (GtkButton *button, gpointer data);
void on_btnCConfSetReadoutSpeed_clicked (GtkButton *button, gpointer data);
void on_btn_CConfSetAntiBloom_clicked (GtkButton *button, gpointer data);
void on_cmbCConfWheel_changed (GtkComboBox *combo, gpointer data);
void on_btnCConfSaveFilterSettings_clicked (GtkButton *button, gpointer data);
void on_btnCConfRotate_clicked (GtkButton *button, gpointer data);
void on_chkCConfInvertImage_toggled (GtkButton *button, gpointer data);
void on_chkCConfInvertDS9h_toggled (GtkButton *button, gpointer data);
void on_chkCConfInvertDS9v_toggled (GtkButton *button, gpointer data);
void on_chkCConfDebayer_toggled (GtkButton *button, gpointer data);
void on_cmbCConfBayerPattern_changed (GtkComboBox *combo, gpointer data);
void on_btnCConfClose_clicked (GtkButton *button, gpointer data);
gboolean IsCConfCombo (gpointer key, gpointer value, gpointer data);
void on_cmbFConfWheel_changed (GtkComboBox *combo, gpointer data);
void on_btnFConfSaveFilterSettings_clicked (GtkButton *button, gpointer data);
void on_btnFConfRotate_clicked (GtkButton *button, gpointer data);
void on_btnFConfClose_clicked (GtkButton *button, gpointer data);
gboolean IsFConfCombo (gpointer key, gpointer value, gpointer data);
#ifdef HAVE_UNICAP
void on_tglLVRecord_toggled (GtkButton *button, gpointer data);
#endif  /* UNICAP */


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

gboolean get_entry_int (const gchar *name, gint minval, gint maxval,
                        gint defval, gint page, gint *val);
gboolean get_entry_float (const gchar *name,
                          gdouble minval, gdouble maxval, 
                          gdouble defval, gint page, gdouble *val);
gchar *get_entry_string (const gchar *name);
static void get_spin_int (const gchar *name, gint *val);
static void get_spin_float (const gchar *name, gdouble *val);
static void set_entry_int (const gchar *name, gint val);
void set_entry_float (const gchar *name, gdouble val);
void set_entry_string (const gchar *name, gchar *string);
static void set_spin_float (const gchar *name, gdouble val);
static GtkEntry *get_entry_widget (const gchar *name);
static GtkSpinButton *get_spin_widget (const gchar *name);
void set_ccd_gui (gboolean set);
void set_exposure_buttons (gboolean active);
void set_task_buttons (gboolean active);
void set_focus_done (void);
void set_progress_bar (gboolean zeroise, gint elapsed);
void set_status_bar (GtkStatusbar *bar, gchar *message, gboolean Clear);
void set_range_minmaxstep (enum Range range, gdouble min, gdouble max, 
						   gdouble step, gushort dp);
void set_range_value (enum Range range, gboolean Sensitive, gdouble value);
static void set_notebook_page (gint page);
void set_elapsed_time (guint elapsed);
void set_fits_data (struct cam_img *img, struct timeval *time, 
	                gboolean UseDateobs, gboolean QueryHardware);
	
void set_autog_on (gboolean on);
void set_guide_on (gboolean on);
void get_autog_guide_params (void);
void get_autog_movespeed (gboolean *CenterSpeed, gfloat *speed);
void set_autog_sensitive (gboolean sensitive, gboolean autogopened);
void set_autog_calibrate_done (void);

gboolean ui_control_action (gchar *cmd, gchar *token);
static void canvas_button_press (GtkWidget *widget, GdkEventButton *event,
                                 struct cam_img *img);
static void canvas_button_release (struct cam_img *img);
static void canvas_motion_notify (GdkEventMotion *event, struct cam_img *img);
static void canvas_zoom_image (GtkWidget *canv, struct cam_img *img, 
                               enum ZoomType zoom);
gboolean ui_show_augcanv_image (void);
void ui_show_augcanv_rect (gboolean Show);
void ui_set_augcanv_rect_colour (gchar *colour);
void ui_set_augcanv_rect_full_area (void);
void ui_set_augcanv_crosshair (gdouble x, gdouble y);
GooCanvasItem *ui_show_augcanv_plot (GooCanvasPoints *points,
                                     GooCanvasItem *plot);
static void ui_show_augcanv_plot_titles (void);
GooCanvasItem *ui_show_augcanv_text (gdouble x, gdouble y, gchar *string, 
                                     gdouble val, gushort type, gint sigfig, 
                                     gchar *colour, GooCanvasItem *text);
void ui_show_augcanv_centroid (gboolean show, gboolean saturated, 
                               gfloat h, gfloat v,
                               gushort x1, gushort x2, gushort y1, gushort y2);
void ui_set_aug_window_controls (enum HWDevice dev, gboolean Binning);
                               
void ui_show_video_frame (guchar *frame, gchar *timestamp, guint num, 
					      gushort h, gushort v);
void ui_show_photom_points (gchar *filename, gfloat aperture);
void set_video_range_adjustment (guint num_frames);
void set_video_range_value (guint frame_num);
gushort get_video_framebufsize (void);
static void ui_show_status_bar_info (void);

static gchar *get_open_filename (GtkWindow *window, gchar *filename);
static gchar *get_save_filename (GtkWindow *window, gchar *filename);
gboolean save_file (struct cam_img *img, enum Colour colour, gboolean display);
void file_saved (struct cam_img *img, gboolean saved);
static gboolean query_file_not_saved (void);
static gboolean check_file (gchar *filename, gboolean Overwrite);

void ui_show_aug_window (void);
void ui_hide_aug_window (void);

void select_device (void);
void get_ccd_image_params (struct exposure_data *exd);
void set_camera_state (struct cam_img *img);
void set_ccd_deftemp (void);
gboolean show_camera_status (gboolean show);
gboolean get_V4L_settings (gchar *device);
gboolean set_filter (gboolean ForceInternal, gchar *filter, gint *fo_diff);
gboolean get_filter_info (struct cam_img *img, gchar *filter, gint *pos, 
	                      gint *offset);
gboolean get_apply_filter_offset (void);
void apply_filter_focus_offset (gint offset);
void check_focuser_temp (void);

static void select_entry_region (const gchar *name);
static void common_keyboard_shortcuts (GdkEventKey *event);
static void comms_menus_update_ports (void);
static void comms_ports_set_active (GtkWidget *widget, gpointer data);
static void comms_ports_activate_cb (GtkWidget* widget, gpointer data);
static void comms_menus_remove_ports (GtkWidget *widget, gpointer data);
static void widget_set_sensitive (GtkWidget *widget, gpointer data);
static void widget_set_insensitive (GtkWidget *widget, gpointer data);
static void restore_config_data (void);
static void save_config_data (void);
static void restore_watch_file (void);
void save_PEC_guide_speed (gfloat GuideSpeed);
void save_RA_worm_pos (gushort WormPos);
gfloat get_goto_motion_limit (void);
void reset_checkbox_state (enum CheckBox chkbox, gboolean active);
gboolean check_format (gboolean is_RA, gchar s[]);
struct tm *get_time (gboolean UTC);
gint get_UTC_offset (void);
void finished_tasks (void);

static void warn_PEC_guidespeed (gboolean MsgBox);
void msg (const gchar *message);
void err_msg (gchar *entry_text, const gchar *message);
gboolean show_error (const gchar *routine, const gchar *message);
static void InitApp (void);
void WriteLog (char *string);
void FlushLog (void);
void WritePhot (char *string);
static gchar *ReadCString (gchar *string, gchar *fmt, ...);
static gchar **ReadCArray (gchar *string);
static void WriteCString (gchar *string, gchar *val);
static void DeleteCGroup (gchar *string);
static gchar *CCDConfigKey (struct cam_img *img, const gchar *str);
	
void init_task_params (void);
void free_task_params (void);
void get_task_params (gchar **tparams);
void set_task_params (gchar **tparams);
gchar *get_task_param_value (gboolean FreeString, gchar *string, gint *status);

void exit_and_shutdown (void);

#ifdef HAVE_UNICAP
static void unicap_device_change_cb (UnicapgtkDeviceSelection *selection,
									 gchar *device_id, GtkWidget *format);
static void unicap_format_change_cb (GtkWidget *ugtk, unicap_format_t *format,
                                     gpointer *p);
void set_record_on (gboolean on);
gboolean liveview_record_is_writeable (void);
gboolean open_liveview_window (void);
void show_liveview_window (void);
void hide_liveview_window (void);
void close_liveview_window (void);
#endif

static GtkBuilder *xml_load_new(GtkBuilder *xml, gchar *file, gchar *objects[]);
GtkWidget *xml_get_widget (GtkBuilder *xml, const gchar *name);
static GtkComboBox *create_text_combo_box (GtkTable *tblTable, guint l, guint r,
                                           guint t, guint b, 
                                           GtkComboBox *cmbBox, 
                                           const gchar *list, guint index,
                                           GCallback func);


/******************************************************************************/
/*                                 EVENTS                                     */
/******************************************************************************/

gboolean on_ccdApp_configure (GtkWidget *widget, GdkEventExpose *event,
                              gpointer data)
{
	/* Set up some initial values when the application is loaded */
	
	static GtkComboBox *cmbExpType, *cmbFilType;
	gushort j;
	
	if (!CCDAppWinConf) {
	
        /* Create and fill exposure type and filter combo boxes */
        
        cmbExpType = create_text_combo_box (
                              GTK_TABLE (xml_get_widget (xml_app, "tblCCD")),
                              1, 2, 5, 6, cmbExpType, "ListCCDExposureTypes", 
                              0, (GCallback) on_cmbExpType_changed);
        g_hash_table_insert (hshCombo, "cmbExpType", cmbExpType);
        cmbFilType = create_text_combo_box (
                              GTK_TABLE (xml_get_widget (xml_app, "tblCCD")),
                              3, 4, 5, 6, cmbFilType, "ListCCDFilterTypes", 
                              0, NULL);
        g_hash_table_insert (hshCombo, "cmbFilType", cmbFilType);
		
		/* Initialise task parameters */
		
		for (j = 0; j < NUMTPARAMS; j++)
			tp[j] = NULL;
		
		CCDAppWinConf = TRUE;
	}
	
	return FALSE;
}
							  
gboolean on_ccdApp_delete (GtkWidget *widget, GdkEventAny *event, gpointer data)
{
	/* Tidy up */
	
	/* Ask the user whether to save any unsaved file before exiting */
	
	if (!query_file_not_saved ()) {
		loop_stop_loop ();
		CCDAppWinConf = FALSE;
	}
	
	/* Don't let the main window be destroyed at this point, otherwise the event
	 * loop might still try to write to it even though it's not there.
	 */
	
	return TRUE;
}

gboolean on_ccdApp_key_press (GtkWidget *widget, GdkEventKey *event,
                              gpointer data)
{
	struct cam_img *aug = get_aug_image_struct ();
		
	/* Implement keyboard shortcuts */
	
	common_keyboard_shortcuts (event);
	
	/* Shortcuts pertaining to the main window only */
	
	switch (event->keyval) {
		
		case GDK_Escape:  /* Switch to image window */
			if (gtk_widget_get_visible (aug->aug_window))
				gtk_window_present (GTK_WINDOW (aug->aug_window));
			break;
	}		
	
	return FALSE;
}

gboolean on_txtVBin_focus_out (GtkWidget *widget, GdkEventKey *event,
                               gpointer data)
{
	/* Warn the user if the entered value is invalid and modify it */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	gint val_h, val_v;
	
	get_entry_int ("txtHBin", 0, ccd->cam_cap.max_binh, 1, NO_PAGE, &val_h);
	get_entry_int ("txtVBin", 0, ccd->cam_cap.max_binv, 1, NO_PAGE, &val_v);
	if (ccd->device == SX && ccd->cam_cap.IsInterlaced && val_v > 1 && val_v%2)
		L_print ("{o}Vertical binning value restricted to even numbers "
				 "for interlaced camera: using %dx%d\n",
				  val_h, --val_v);
	set_entry_int ("txtVBin", val_v);
	
	return FALSE;
}
	
gboolean on_wndImage_configure (GtkWidget *widget, GdkEventExpose *event,
                                gpointer data)
{
	/* Create and initialise the autoguider canvases */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	GdkCursor *cursor;
	gushort i;
	gint level;
	
	if (cnvImage)
		return FALSE;

	/* Image canvas... */
	
	cnvImage = goo_canvas_new ();
	gtk_widget_set_size_request (cnvImage, AUGCANV_H, AUGCANV_V);
	goo_canvas_set_bounds (GOO_CANVAS (cnvImage), 0, 0, AUGCANV_H, AUGCANV_V);
    g_object_set (G_OBJECT (cnvImage), "background-color", "black", NULL);
    gtk_widget_set_events (cnvImage, GDK_MOTION_NOTIFY | 
                                     GDK_BUTTON_PRESS  | 
                                     GDK_BUTTON_RELEASE);
    g_signal_connect (cnvImage, "motion-notify-event", 
                      (GCallback) on_cnvImage_motion_notify,
                      NULL);
    g_signal_connect (cnvImage, "button-press-event", 
                      (GCallback) on_cnvImage_button_press,
                      NULL);
    g_signal_connect (cnvImage, "button-release-event", 
                      (GCallback) on_cnvImage_button_release,
                      NULL);

	gtk_widget_show (cnvImage);
	gtk_container_add (GTK_CONTAINER (xml_get_widget (
	                                           xml_img, "scwImage")), cnvImage);
	cgpImage = goo_canvas_get_root_item (GOO_CANVAS (cnvImage));
    
	/* Draw the selection rectangle */
		
	aug->canv.cviRect = goo_canvas_rect_new (cgpImage,
						                     0.0,
						                     0.0,
						                     (gdouble) (AUGCANV_H - 1),
						                     (gdouble) (AUGCANV_V - 1),
						                     "stroke-color", "green",
                                             "line-width", 2.0,
						                     NULL);

	aug->canv.NewRect = TRUE;
	
	/* Initialise the crosshair */
	
	aug->canv.chx_line = NULL;
	aug->canv.chy_line = NULL;		
	ui_set_augcanv_crosshair (AUGCANV_H / 2.0, AUGCANV_V / 2.0);
	
	/* Set cursor for canvas */
						  
	cursor = gdk_cursor_new (GDK_CROSSHAIR);		
	gdk_window_set_cursor (gtk_widget_get_window (cnvImage), cursor);
    
    /* Set initial zoom factor */
    
    aug->canv.zoom = 1.0;
    
	/* Histogram canvas... */
		
	cnvHist = goo_canvas_new ();
	gtk_widget_set_size_request (cnvHist, HWID, -1);
	goo_canvas_set_bounds (GOO_CANVAS (cnvHist), 0, 0, HWID, AUGCANV_V + 20);
    g_object_set (G_OBJECT (cnvHist), "background-color", "black", NULL);
	gtk_widget_show (cnvHist);
	gtk_box_pack_end (GTK_BOX (xml_get_widget (
	                   xml_img, "hbxImageCanvases")), cnvHist, FALSE, FALSE, 2);
	cgpHist = goo_canvas_get_root_item (GOO_CANVAS (cnvHist));	
		
	/* Draw the histogram and x/y flux plot windows */
		
	for (i = YHIST; i < 600 - BOXSIZE; i+= YGAP + BOXSIZE)
		goo_canvas_rect_new (cgpHist,
							 (gdouble) (XPLOT - 1),
							 (gdouble) (i - 1),
							 (gdouble) (BOXSIZE + 1),
							 (gdouble) (BOXSIZE + 1),
							 "stroke-color", "green",
		    				 "line-width", 1.5,
							 NULL);
	
	/* Label the plots */
		
	strcpy (font, FONT);  /* Set the default value for font */
	R_config_s ("Canvas/Font", font);
	ui_show_augcanv_plot_titles ();
	
	/* Get the initial values of the exposure length and saturation level */
	
	get_entry_float("txtImgExpLength", 0.001, 99.9,1,NO_PAGE,&aug->exd.req_len);
	get_entry_int ("txtImgSatLevel", aug->imdisp.B, aug->imdisp.W,
					                            aug->imdisp.W, NO_PAGE, &level);
    aug->imdisp.satlevel = (gushort) level;
		
	/* Finally, display status bar info... */
		
	ui_show_status_bar_info ();
	
	return FALSE;
}	

gboolean on_wndImage_delete (GtkWidget *widget, GdkEventAny *event,
                             gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */

	if (!gtk_toggle_button_get_active (
		 GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "tglAutogStart"))) &&
		!gtk_toggle_button_get_active (
		 GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "tglAutogPause"))))
		gtk_widget_activate (xml_get_widget (xml_app, "chkAutogOpen"));
	return TRUE;
}

gboolean on_wndImage_key_press (GtkWidget *widget, GdkEventKey *event,
                                gpointer data)
{
	/* Implement keyboard shortcuts */
	
	/* Common keyboard shortcuts */
	
	common_keyboard_shortcuts (event);

	/* Shortcuts pertaining to the image window only */
	
	switch (event->keyval) {
		
		case GDK_Escape:  /* Switch to main window */
			gtk_window_present (ccdApp);
			break;
	}
			
	return FALSE;	
}

gboolean on_cnvImage_button_press (GtkWidget *widget, GdkEventButton *event,
                                   gpointer data)
{
	/* Perform mouse operations */

	struct cam_img *aug = get_aug_image_struct ();
	
	/* Return if guiding */
	
	if (aug->autog.Guide)
		return TRUE;
    
    /* Left mouse button down... */
    
    if (event->button == 1)
        canvas_button_press (widget, event, aug);
		
    return TRUE;
}

gboolean on_cnvImage_button_release (GtkWidget *widget, GdkEventButton *event,
                                     gpointer data)
{
	/* When the left mouse button is released, ungrab the pointer */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	/* Return if guiding */

	if (aug->autog.Guide)
		return TRUE;
	
    /* Left mouse button up... */
        
    if (event->button == 1)
        canvas_button_release (aug);
	
    return TRUE;
}

gboolean on_cnvImage_motion_notify (GtkWidget *widget, GdkEventMotion *event,
                                    gpointer data)
{
	/* Grab the cursor coordinates.  Draw the selection rectangle when the mouse
     * is moved with the left button pressed, and move it if the shift button is
     * held down.
     */

	struct cam_img *aug = get_aug_image_struct ();
		
    /* Return if guiding */
	
	if (aug->autog.Guide)
		goto write_info;
        
	aug->canv.cursor_x = (gint) (event->x / aug->canv.zoom);
    aug->canv.cursor_y = (gint) (event->y / aug->canv.zoom);
	
    /* Draw/move rectangle */
    
    canvas_motion_notify (event, aug);
		
	/* Write pointer location and other info to status bar */
	
write_info:
	ui_show_status_bar_info ();
		
    return TRUE;
}

gboolean on_wndSetting_delete (GtkWidget *widget, GdkEventAny *event,
                               gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_set, "btnCloseSetting"));
	return TRUE;
}

gboolean on_wndConfigAutofocus_configure (GtkWidget *widget, 
										  GdkEventExpose *event, gpointer data)
{
	/* Configure the autofocus configuration window */
	
	if (!AFWinConf) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (xml_get_widget (
						              xml_fcw, "optAFConfigSetRange")), TRUE);
		AFWinConf = TRUE;
	}
	
	return FALSE;
}

gboolean on_wndConfigAutofocus_delete (GtkWidget *widget, GdkEventAny *event,
                                       gpointer data)
{
	/* Close focus configuration window and tidy up */
	
	loop_ccd_calibrate_autofocus (FALSE, 0, 0, 0, 0, 0.0, 0); /* Stop calib. */
	
	gtk_widget_destroy (xml_get_widget (xml_fcw, "wndConfigAutofocus"));
	g_object_unref (G_OBJECT (xml_fcw));
	xml_fcw = NULL;
	AFWinConf = FALSE;
	return FALSE;
}

gboolean on_wndParallelPort_configure (GtkWidget *widget, GdkEventExpose *event,
                               	       gpointer data)
{
	/* Configure the parallel port settings window */
	
	if (!PPWinConf) {
		set_entry_string ("txtPPAddress", ports[LPT].address);
		set_entry_int ("txtPPRAp", ports[LPT].RAp);
		set_entry_int ("txtPPRAm", ports[LPT].RAm);
		set_entry_int ("txtPPDecp", ports[LPT].Decp);
		set_entry_int ("txtPPDecm", ports[LPT].Decm);
		//set_entry_int ("txtPPSC1LongExp", ports[LPT].SC1LongExp);
		//set_entry_int ("txtPPSC1Pause", ports[LPT].SC1Pause);
		PPWinConf = TRUE;
	}
	
	return FALSE;
}

gboolean on_wndParallelPort_delete (GtkWidget *widget, GdkEventAny *event,
                                    gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_ppd, "btnPPClose"));
	return TRUE;	
}

gboolean on_wndEditTasks_configure (GtkWidget *widget, GdkEventExpose *event,
                               	    gpointer data)
{
	/* Configure the tasks editing window */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	static GtkComboBox *cmbTExpType, *cmbTFilType;
	
	if (!EdWinConf) {
		
		#ifndef YELLOW_BUTTON  /* Private option for Kiwi-OSD box */
		gtk_widget_hide (xml_get_widget (xml_tsk, "btnTYellow"));
		#endif
		
		#ifndef HAVE_UNICAP
		gtk_widget_set_sensitive (xml_get_widget (xml_tsk, 
												     "btnTRecordStart"), FALSE);
		gtk_widget_set_sensitive (xml_get_widget (xml_tsk, 
													  "btnTRecordStop"), FALSE);
		#endif
		
		gtk_widget_set_sensitive (xml_get_widget (xml_tsk, 
											    "btnTWarmRestart"), GeminiCmds);
		
        /* Create and fill exposure type and filter combo boxes */
        
        cmbTExpType = create_text_combo_box (
                              GTK_TABLE (xml_get_widget (xml_tsk,"tblTExpose")),
                              1, 2, 1, 2, cmbTExpType, "ListCCDExposureTypes", 
                              0, (GCallback) on_cmbTExpType_changed);
        g_hash_table_insert (hshCombo, "cmbTExpType", cmbTExpType);
        cmbTFilType = create_text_combo_box (
                              GTK_TABLE (xml_get_widget (xml_tsk,"tblTExpose")),
                              2, 3, 1, 2, cmbTFilType, "ListCCDFilterTypes", 
                              0, NULL);
        g_hash_table_insert (hshCombo, "cmbTFilType", cmbTFilType);
        
        /* Fill in CCD chip dimensions if camera is open */
        
		if (ccd->Open) {
			set_entry_int ("txtTH2", ccd->cam_cap.max_h);
			set_entry_int ("txtTV2", ccd->cam_cap.max_v);
		}
		
		EdWinConf = TRUE;
	}

	return FALSE;	
}

gboolean on_wndEditTasks_delete (GtkWidget *widget, GdkEventAny *event,
                                 gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_tsk, "btnTCloseWindow"));
	return TRUE;	
}

gboolean on_wndQSIConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                    gpointer data)
{
	/* Configure the CCD Configuration window */
	
	//~ struct cam_img *img;  (see below)
    static GtkComboBox *cmbCConfFil[8];
	static GtkComboBox *cmbCConfWheel, *cmbCConfSetFilter;
    static GtkComboBox *cmbCConfBayerPattern;
    static GtkComboBox *cmbCConfShutPrior, *cmbCConfShutMode, *cmbCConfShutOpen;
    static GtkComboBox *cmbCConfPreFlush, *cmbCConfFastMode;
    static GtkComboBox *cmbCConfReadoutSpeed, *cmbCConfCameraGain;
    static GtkComboBox *cmbCConfAntiBlooming;
	gushort i;
	gint fans;
	gdouble def_temp;
	gchar *title;
    static gchar *CConfFil_name[8] = {
                                 "cmbCConfFil0", "cmbCConfFil1", "cmbCConfFil2", 
                                 "cmbCConfFil3", "cmbCConfFil4", "cmbCConfFil5", 
                                 "cmbCConfFil6", "cmbCConfFil7"};
	
	if (!CCDConfigWinConf) {
		
		title = g_strconcat((CCDConfigOwner->id == CCD ? "Main" : "Autoguider"),
		                     " camera configuration for ", 
							 CCDConfigOwner->cam_cap.camera_manf, " ", 
							 CCDConfigOwner->cam_cap.camera_snum, NULL);
		gtk_window_set_title (GTK_WINDOW (widget), title);
		g_free (title);
		
		/* Set cooling controls */
		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfCoolOnConnect")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "CoolOnConnect"), FALSE));			  
		
		def_temp = R_config_f (CCDConfigKey (CCDConfigOwner,"DefTemp"),TPR_DEF);
		set_entry_float ("txtCConfDefTemp", def_temp);
		
		fans = R_config_d (CCDConfigKey (CCDConfigOwner, "Fans"), CCD_FAN_AUTO);
		switch (fans) {
			case CCD_FAN_HIGH:
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
					(xml_get_widget (xml_con, "optCConfFansHigh")), TRUE);
					break;
			case CCD_FAN_AUTO:
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
					(xml_get_widget (xml_con, "optCConfFansAuto")), TRUE);
					break;
			case CCD_FAN_OFF:
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
					(xml_get_widget (xml_con, "optCConfFansOff")), TRUE);
					break;
		}
		
        /* Create and fill filter wheel combo boxes */
        
        for (i = 0; i < 8; i++) {  /* Filter for given wheel position */
            cmbCConfFil[i] = create_text_combo_box (
                           GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                           5, 6, 3 + i, 4 + i, cmbCConfFil[i], 
                           "ListCCDFilterTypes", 0, NULL);
            g_hash_table_insert (hshCombo, CConfFil_name[i], cmbCConfFil[i]);
        }
        
        cmbCConfSetFilter = create_text_combo_box ( /* Rotate to this filter */
                           GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                           6, 7, 13, 14, cmbCConfSetFilter, 
                           "ListCCDFilterTypes", 0, NULL);
        g_hash_table_insert (hshCombo, "cmbCConfSetFilter", cmbCConfSetFilter);
            
		/* Create, fill and activate the active filter wheel number combo box.
         * This causes the filter wheel combo boxes for each filter position to 
         * be restored to their previous values for that filter wheel via the 
         * on_cmbCConfWheel_changed callback when the filter wheel number combo 
         * box is activated.
		 */
		
        cmbCConfWheel = create_text_combo_box (
                           GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                           6, 7, 1, 2, cmbCConfWheel, "ListCCDFilterWheels",
                           R_config_d (CCDConfigKey (CCDConfigOwner, 
                           "Filter/ActiveWheel"), 1) -1, 
                           (GCallback) on_cmbCConfWheel_changed);
                           
		/* Set the debayering options */
		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfDebayer")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "Debayer"), FALSE));			  
		
        cmbCConfBayerPattern = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      6, 7, 17, 18, cmbCConfBayerPattern,"ListCCDBayerPatterns",
                      R_config_d(CCDConfigKey(CCDConfigOwner,"BayerPattern"),0), 
                      (GCallback) on_cmbCConfBayerPattern_changed);
        g_hash_table_insert (hshCombo, "cmbCConfBayerPattern", 
                             cmbCConfBayerPattern);
                           
		/* Set the exposure control and miscellaneous combo boxes.  Ideally
		 * these would be set based on the values read from the camera.
		 * Unfortunately, for cameras that do not support setting some of these
		 * values, the corresponding "get_" calls return "random" values, some 
		 * of which are within the valid range.  (Such calls should really
		 * return a pre-defined non-valid value so that we would know to ignore
		 * them).  So instead we display "-" in the combo boxes unless a value 
		 * has been specifically set by the user, in which case it can be
		 * recalled from the configuration database.
		 */
		
		//~ switch (CCDConfigOwner->id) {
			//~ case CCD:
				//~ img = get_ccd_image_struct ();
				//~ break;
			//~ case AUG:
				//~ img = get_aug_image_struct ();
				//~ break;
			//~ default:
			    //~ CCDConfigWinConf = TRUE;
			    //~ return show_error (__func__, "Unknown camera type!");
		//~ }
		//~ img->get_state (&img->state, TRUE);
		
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfShutPrior")), 
								 //~ img->state.shut_prior + 1);
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfShutMode")),
								 //~ img->state.shut_mode + 1);
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfShutOpen")),
								 //~ img->state.shut_open + 1);
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfPreFlush")),
								 //~ img->state.pre_flush + 1);
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfFastMode")),
								 //~ img->state.host_timed + 1);
		
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfCameraGain")),
								 //~ img->state.cam_gain + 1);
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfReadoutSpeed")),
								 //~ img->state.read_speed + 1);
		//~ gtk_combo_box_set_active (GTK_COMBO_BOX
		  //~ (xml_get_widget (xml_con, "cmbCConfAntiBlooming")),
								 //~ img->state.anti_bloom + 1);
		
        cmbCConfShutPrior = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 11, 12, cmbCConfShutPrior, "ListCCDShutterPriority",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "ShutterPriority"), -1) + 1, 
                      (GCallback) on_cmbCConfShutPrior_changed);
        cmbCConfShutMode = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 12, 13, cmbCConfShutMode, "ListCCDShutterMode",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "ShutterMode"), -1) + 1, 
                      (GCallback) on_cmbCConfShutMode_changed);
        cmbCConfShutOpen = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 13, 14, cmbCConfShutOpen, "ListCCDShutterOpen",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "ShutterOpen"), -1) + 1, 
                      (GCallback) on_cmbCConfShutOpen_changed);
        cmbCConfPreFlush = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 14, 15, cmbCConfPreFlush, "ListCCDPreFlush",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "PreFlush"), -1) + 1, 
                      (GCallback) on_cmbCConfPreFlush_changed);
        cmbCConfFastMode = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 15, 16, cmbCConfFastMode, "ListCCDHostTimed",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "HostTimed"), -1) + 1, 
                      (GCallback) on_cmbCConfFastMode_changed);
		
        cmbCConfCameraGain = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 16, 17, cmbCConfCameraGain, "ListCCDCameraGain",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "CameraGain"), -1) + 1, 
                      (GCallback) on_cmbCConfCameraGain_changed);
        cmbCConfReadoutSpeed = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 17, 18, cmbCConfReadoutSpeed, "ListCCDReadoutSpeed",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "ReadoutSpeed"), -1) + 1, 
                      (GCallback) on_cmbCConfReadoutSpeed_changed);
        cmbCConfAntiBlooming = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblQSIConfig")),
                      1, 2, 18, 19, cmbCConfAntiBlooming, "ListCCDAntiBlooming",
                      R_config_d (CCDConfigKey (CCDConfigOwner, 
                      "AntiBloom"), -1) + 1, 
                      (GCallback) on_cmbCConfAntiBlooming_changed);
		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfInvertDS9h")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "InvertDS9h"), FALSE));			  
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfInvertDS9v")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "InvertDS9v"), FALSE));	
		  		  
		CCDConfigWinConf = TRUE;
	}
	return FALSE;
}

gboolean on_wndQSIConfig_delete (GtkWidget *widget, GdkEventAny *event,
                                 gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_con, "btnCConfClose"));
	return TRUE;	
}

gboolean on_wndSXConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                   gpointer data)
{
	/* Configure the CCD Configuration window */
	
    static GtkComboBox *cmbCConfBayerPattern;
	gdouble val;
	gchar *title;
	gboolean Active;
	
	if (!CCDConfigWinConf) {
		
		title = g_strconcat((CCDConfigOwner->id == CCD ? "Main" : "Autoguider"),
		                     " camera configuration for ", 
							 CCDConfigOwner->cam_cap.camera_manf, " ", 
							 CCDConfigOwner->cam_cap.camera_desc, NULL);
		gtk_window_set_title (GTK_WINDOW (widget), title);
		g_free (title);
		
		/* Set cooling controls */
		
		Active = CCDConfigOwner->cam_cap.CanSetCCDTemp;
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "chkCConfCoolOnConnect1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "lblCConfDefTemp1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "txtCConfDefTemp1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "btnCConfSetDefTemp1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "lblCConfTempTol1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "txtCConfTempTol1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "btnCConfSetTempTol1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "btnCConfCoolerOn1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "btnCConfCoolerOff1"), Active);
		  			  
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfCoolOnConnect1")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "CoolOnConnect"), FALSE));			  
		
		val = R_config_f (CCDConfigKey(CCDConfigOwner,"DefTemp"),TPR_DEF);
		set_entry_float ("txtCConfDefTemp1", val);
		
		val = R_config_f (CCDConfigKey(CCDConfigOwner,"TempTol"), 1.0);
		set_entry_float ("txtCConfTempTol1", val);
		
		/* Set miscellaneous options */
		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfInvertImage1")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "InvertImage"), FALSE));			  
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfInvertDS9h1")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "InvertDS9h"), FALSE));			  
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfInvertDS9v1")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "InvertDS9v"), FALSE));			  
		
		/* Set the debayering options */
		
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
		  (xml_get_widget (xml_con, "chkCConfDebayer1")),
		  R_config_d (CCDConfigKey (CCDConfigOwner, "Debayer"), FALSE));			  
		
        cmbCConfBayerPattern = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_con, "tblSXConfig")),
                      3, 4, 12, 13, cmbCConfBayerPattern,"ListCCDBayerPatterns",
                      R_config_d(CCDConfigKey(CCDConfigOwner,"BayerPattern"),0), 
                      (GCallback) on_cmbCConfBayerPattern_changed);
        g_hash_table_insert (hshCombo, "cmbCConfBayerPattern", 
                             cmbCConfBayerPattern);
		
		Active = CCDConfigOwner->cam_cap.IsColour;
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "chkCConfDebayer1"), Active);
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_con, "lblCConfPattern1"), Active);
		gtk_widget_set_sensitive (GTK_WIDGET (cmbCConfBayerPattern), Active);
								  
		CCDConfigWinConf = TRUE;
	}
	return FALSE;
}

gboolean on_wndSXConfig_delete (GtkWidget *widget, GdkEventAny *event,
                                gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_con, "btnCConfClose1"));
	return TRUE;	
}

gboolean on_wndV4LConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                    gpointer data)
{
	/* Configure the V4L configuration window */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	static GtkComboBox *cmbV4LVidStd, *cmbV4LVidInp;
	gushort i;
	gchar *str;
	
	if (!V4LWinConf) {
		set_entry_string ("txtV4LConfigDevice", aug->vid_dat.card);
		
		/* Create video standard combo box */
		
		cmbV4LVidStd = create_text_combo_box (
						   GTK_TABLE (xml_get_widget (xml_V4L, "tblV4LConfig")),
						   1, 2, 1, 2, cmbV4LVidStd, "ListDummyComboBox", 
						   0, NULL);
		g_hash_table_insert (hshCombo, "cmbV4LVidStd", cmbV4LVidStd);
		
		/* Fill combo box with an entry for each standard found */
		
		if (!aug->vid_dat.vid_std.num) {
			gtk_combo_box_append_text (cmbV4LVidStd, "None");
			gtk_combo_box_set_active (cmbV4LVidStd, 0);
			aug->vid_dat.HasVideoStandard = FALSE;
		} else {
			for (i = 0; i < aug->vid_dat.vid_std.num; i++) {
				str = g_strdup_printf ("%s", aug->vid_dat.vid_std.name[i]);
				gtk_combo_box_append_text (cmbV4LVidStd, str);
				g_free (str);
			}
			gtk_combo_box_set_active (cmbV4LVidStd, 
			                          aug->vid_dat.vid_std.selected);
			aug->vid_dat.HasVideoStandard = TRUE;
		}
		
		/* Create video input combo box */
		
		cmbV4LVidInp = create_text_combo_box (
						   GTK_TABLE (xml_get_widget (xml_V4L, "tblV4LConfig")),
						   1, 2, 2, 3, cmbV4LVidInp, "ListDummyComboBox", 
						   0, NULL);
		g_hash_table_insert (hshCombo, "cmbV4LVidInp", cmbV4LVidInp);
		
		/* Fill combo box with an entry for each input found */
		
		if (!aug->vid_dat.vid_input.num)
			gtk_combo_box_append_text (cmbV4LVidInp, "None");
		else {
			for (i = 0; i < aug->vid_dat.vid_input.num; i++) {
				str = g_strdup_printf ("%s", aug->vid_dat.vid_input.name[i]);
				gtk_combo_box_append_text (cmbV4LVidInp, str);
				g_free (str);
			}
		}
		gtk_combo_box_set_active(cmbV4LVidInp, aug->vid_dat.vid_input.selected);
		
		/* Set width, height, fps */
		
		set_entry_int ("txtV4LConfigWidth", aug->vid_dat.width);
		set_entry_int ("txtV4LConfigHeight", aug->vid_dat.height);
		set_entry_float ("txtV4LConfigFps", aug->vid_dat.fps);

		V4LWinConf = TRUE;
	}
	return FALSE;
}

gboolean on_wndV4LConfig_delete (GtkWidget *widget, GdkEventAny *event,
                                 gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_V4L, "btnV4LConfigClose"));
	return TRUE;	
}

gboolean on_wndFilterConfig_configure (GtkWidget *widget, GdkEventExpose *event,
                                       gpointer data)
{
	/* Configure the filter wheel configuration window */
	
    static GtkComboBox *cmbFConfFil[9];
	static GtkComboBox *cmbFConfWheel, *cmbFConfSetFilter;
	gushort i;
    static gchar *FConfFil_name[9] = {
                                "cmbFConfFil0", "cmbFConfFil1", "cmbFConfFil2", 
                                "cmbFConfFil3", "cmbFConfFil4", "cmbFConfFil5", 
                                "cmbFConfFil6", "cmbFConfFil7", "cmbFConfFil8"};
	
	if (!FilWinConf) {
		
        /* Create and fill filter wheel combo boxes */
        
        for (i = 0; i < 9; i++) {  /* Filter for given wheel position */
            cmbFConfFil[i] = create_text_combo_box (
                        GTK_TABLE (xml_get_widget (xml_fil, "tblFilterConfig")),
                        1, 2, 3 + i, 4 + i, cmbFConfFil[i], 
                        "ListCCDFilterTypes", 0, NULL);
            g_hash_table_insert (hshCombo, FConfFil_name[i], cmbFConfFil[i]);
        }
        
        cmbFConfSetFilter = create_text_combo_box ( /* Rotate to this filter */
                        GTK_TABLE (xml_get_widget (xml_fil, "tblFilterConfig")),
                        2, 3, 15, 16, cmbFConfSetFilter, 
                        "ListCCDFilterTypes", 0, NULL);
        g_hash_table_insert (hshCombo, "cmbFConfSetFilter", cmbFConfSetFilter);
            
		/* Create, fill and activate the active filter wheel number combo box.
         * This causes the filter wheel combo boxes for each filter position to 
         * be restored to their previous values for that filter wheel via the 
         * on_cmbFConfWheel_changed callback when the filter wheel number combo 
         * box is activated.
		 */
		
        cmbFConfWheel = create_text_combo_box (
                        GTK_TABLE (xml_get_widget (xml_fil, "tblFilterConfig")),
                        1, 2, 1, 2, cmbFConfWheel, "ListCCDFilterWheels",
                        R_config_d ("FilterWheel/ActiveWheel", 1) -1, 
                        (GCallback) on_cmbFConfWheel_changed);
                           
		FilWinConf = TRUE;
	}
	
	/* Disable position zero for SX filterwheels - they start at position 1 */
	
	if (!(strcmp (menu.filterwheel, "filterwheel_sx"))) {
		gtk_widget_set_sensitive (
		                 g_hash_table_lookup (hshCombo, "cmbFConfFil0"), FALSE);
		gtk_widget_set_sensitive (
		                 xml_get_widget (xml_fil, "txtFConfFoc0"), FALSE);
	}
	
	return FALSE;
}

gboolean on_wndFilterConfig_delete (GtkWidget *widget, GdkEventExpose *event,
                                    gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_fil, "btnFConfClose"));
	return TRUE;	
}

gboolean on_trvTasks_button_release (GtkWidget *widget, GdkEventButton *event,
                                     gpointer data)
{
	/* Open task list in external editor if user right-clicks in tasks tree
	 * view.  Re-load task file when user is done.
	 */
	
	gint status;
	gchar *TempPath = NULL, *cmd = NULL;
	gchar *editor;
	
    if (event->button == 3) {
		TempPath = g_build_filename (PrivatePath, "tasks.lis", NULL);
		if (tasks_write_file (TempPath)) {
			editor = (gchar *) g_malloc0 (128 * sizeof (gchar));
			strcpy (editor, "gedit");
			R_config_s ("Misc/Editor", editor);
			cmd = g_strconcat (editor, " ", TempPath, NULL);
			g_free (editor);
			if (g_spawn_command_line_sync(cmd, (gchar **) NULL, (gchar **) NULL,
									      &status, (GError **) NULL)) {
				tasks_clear ();
				if (tasks_load_file (TempPath)) {
					g_free (cmd);
					cmd = g_strconcat ("rm ", TempPath, NULL);
					g_spawn_command_line_sync (cmd, (gchar **) NULL, 
									(gchar **) NULL, &status, (GError **) NULL);
					L_print ("Loaded tasks list!\n");
				}
			} else
				L_print ("{r}Error executing external editor\n");
		} else
		    L_print ("{r}Error writing task list to temporary file\n");
		if (cmd)
			g_free (cmd);
		if (TempPath)
			g_free (TempPath);
	}
	
	return TRUE;
}

#ifdef HAVE_UNICAP
gboolean on_wndLiveView_configure (GtkWidget *widget, GdkEventExpose *event,
                               	   gpointer data)
{
	/* Configure the live view window */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	GtkWidget *vbxVbox, *scwWindow;
	gchar *uri, *u;
	
	if (aug->ugtk_window == NULL) {
		
		/* Create a new scrolled window, with scrollbars only if needed */
		
		scwWindow = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scwWindow),
										GTK_POLICY_AUTOMATIC, 
										GTK_POLICY_AUTOMATIC);
		
		/* Pack the display widget into the scrolled window */
		
		aug->ugtk_display = unicapgtk_video_display_new_by_handle (
		                                                       aug->ucp_handle);
		gtk_scrolled_window_add_with_viewport (
					        GTK_SCROLLED_WINDOW (scwWindow), aug->ugtk_display);
		
		/* Pack the scrolled window into the Vbox */
		
		vbxVbox = xml_get_widget (xml_lvw, "vbxLVDisplay");
		gtk_box_pack_start (GTK_BOX (vbxVbox), scwWindow, TRUE, TRUE, 0);
		
		/* Register a callback to be called for each new frame */
		
		unicapgtk_video_display_set_new_frame_callback (
					   UNICAPGTK_VIDEO_DISPLAY (aug->ugtk_display), 
					   UNICAPGTK_CALLBACK_FLAGS_BEFORE, 
					   (unicap_new_frame_callback_t) augcam_unicap_new_frame_cb, 
					   NULL);
	 
		/* Set the specified display format */
		
		if (!SUCCESS (unicapgtk_video_display_set_format 
			   (UNICAPGTK_VIDEO_DISPLAY (aug->ugtk_display), &aug->ucp_format)))
			goto config_error;
		
		/* Start the display */
		
		if (!SUCCESS (unicapgtk_video_display_start (
					              UNICAPGTK_VIDEO_DISPLAY (aug->ugtk_display))))
			goto config_error;

		/* Realize all the widgets, but hide the window unless opened via
		 * the Live View menu option.
		 */
		
		aug->ugtk_window = xml_get_widget (xml_lvw, "wndLiveView");
		gtk_widget_show_all (aug->ugtk_window);
		if (!menu.LiveView)
			hide_liveview_window ();
		
		/* Set the directory for saving video files */
		
		uri = (gchar *) g_malloc0 (256 * sizeof (gchar));
		R_config_s ("Video/VideoDir", uri);
		if (!strlen (uri)) {
			u = g_filename_to_uri (UserDir, NULL, NULL);
			strncpy (uri, u, 256);
			g_free (u);
		}
		gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER 
		                   (xml_get_widget (xml_lvw, "flcVideoFile")), uri);
		g_free (uri);
		
		/* Initialise video record buffer */
		
		video_init ();
	}
	
	return FALSE;

  /* Handle config errors here */	
	
  config_error:

  L_print ("{r}%s: Error starting video device\n", __func__);
  if (menu.LiveView)  /* Turn off live view if opened via live view menu */
	  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
						        (xml_get_widget (xml_app, "live_view")), FALSE);
  else                /* Turn off autoguider if opened via autoguider tab */
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (xml_get_widget
											 (xml_app, "chkAutogOpen")), FALSE);
  return FALSE;
}
#endif

#ifdef HAVE_UNICAP
gboolean on_wndLiveView_delete (GtkWidget *widget, GdkEventAny *event,
                                gpointer data)	
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_lvw, "btnLVClose"));
	return TRUE;	
}
#endif

gboolean on_wndPlayback_configure (GtkWidget *widget, GdkEventExpose *event,
                               	   gpointer data)
{
	/* Create and initialise the video playback canvas */
	
	struct cam_img *vid = get_vid_image_struct ();
	
	GdkCursor *cursor;
    static GtkComboBox *cmbPBfps;
	
	if (!PBWinConf) {

        /* Image canvas... */
        
        cnvPlayback = goo_canvas_new ();
        gtk_widget_set_size_request (cnvPlayback, VIDCANV_H, VIDCANV_V);
        goo_canvas_set_bounds (GOO_CANVAS(cnvPlayback), 0, 0, 
                                                          VIDCANV_H, VIDCANV_V);
        g_object_set (G_OBJECT (cnvPlayback), "background-color", "black",NULL);
        gtk_widget_set_events (cnvPlayback, GDK_MOTION_NOTIFY | 
                                            GDK_BUTTON_PRESS  | 
                                            GDK_BUTTON_RELEASE);
        g_signal_connect (cnvPlayback, "motion-notify-event", 
                          (GCallback) on_cnvPlayback_motion_notify,
                          NULL);
        g_signal_connect (cnvPlayback, "button-press-event", 
                          (GCallback) on_cnvPlayback_button_press,
                          NULL);
        g_signal_connect (cnvPlayback, "button-release-event", 
                          (GCallback) on_cnvPlayback_button_release,
                          NULL);

        gtk_widget_show (cnvPlayback);
        gtk_container_add (GTK_CONTAINER (xml_get_widget (
                                         xml_pbw, "scwPlayback")), cnvPlayback);
        cgpPlayback = goo_canvas_get_root_item (GOO_CANVAS (cnvPlayback));
        
        /* Draw the selection rectangle */
            
        vid->canv.cviRect = goo_canvas_rect_new (cgpPlayback,
                                                 0.0,
                                                 0.0,
                                                 (gdouble) (VIDCANV_H - 1),
                                                 (gdouble) (VIDCANV_V - 1),
                                                 "stroke-color", "green",
                                                 "line-width", 2.0,
                                                 NULL);
        vid->canv.NewRect = TRUE;
        
        /* Set cursor for canvas */
                              
        cursor = gdk_cursor_new (GDK_CROSSHAIR);		
        gdk_window_set_cursor (gtk_widget_get_window (cnvPlayback), cursor);
        
        /* Set initial zoom factor */
        
        vid->canv.zoom = 1.0;
        
        /* Create frame rate combo box and set default frame rate */
            
        cmbPBfps = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_pbw, "tblPBPlayControls")),
                      1, 2, 0, 1, cmbPBfps, "ListVideoPlaybackRates", 
                      2, (GCallback) on_cmbPBfps_changed);
        video_set_frame_rate (25);
        
        PBWinConf = TRUE;
    }
    
    return FALSE;	
}

gboolean on_wndPlayback_delete (GtkWidget *widget, GdkEventAny *event,
                                gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_pbw, "btnPBClose"));
	return TRUE;	
}

gboolean on_cnvPlayback_button_press (GtkWidget *widget, GdkEventButton *event, 
                                      gpointer data)
{
	/* Perform mouse operations */
	
	struct cam_img *vid = get_vid_image_struct ();
    
    /* Return if no video file displayed */
		
	if (!vid->Open)
		return TRUE;
        
    /* Left mouse button down... */

    if (event->button == 1) {
        canvas_button_press (widget, event, vid);
        if (xml_pht)
            gtk_widget_set_sensitive (
                                xml_get_widget(xml_pht, "btnPhotRange"), FALSE);
    }
		
    return TRUE;
}

gboolean on_cnvPlayback_button_release (GtkWidget *widget,GdkEventButton *event, 
                                        gpointer data)
{
	/* When the left mouse button is released, ungrab the pointer */
	
	struct cam_img *vid = get_vid_image_struct ();
	
	if (!vid->Open)
		return TRUE;
	
    /* Left mouse button up... */
        
    if (event->button == 1)
        canvas_button_release (vid);
	
    return TRUE;
}

gboolean on_cnvPlayback_motion_notify (GtkWidget *widget, GdkEventMotion *event, 
                                       gpointer data)
{
	/* Draw the selection rectangle when the mouse is moved with the
	 * left button pressed.
	 */

	struct cam_img *vid = get_vid_image_struct ();
		
	gchar *s;
	
	if (!vid->Open)
		return TRUE;
	
	vid->canv.cursor_x = (gint) (event->x / vid->canv.zoom);
    vid->canv.cursor_y = (gint) (event->y / vid->canv.zoom);
	
    /* Draw/move rectangle */
    
    canvas_motion_notify (event, vid);
		
	/* Set the cursor coordinates in the photometry window (if open) */
	
	if (PhotWinConf) {
		s = g_strdup_printf ("%4d,%4d", vid->canv.cursor_x, vid->canv.cursor_y);
		set_entry_string ("txtPhotCoords", s);
		g_free (s);
	}
		
    return TRUE;
}

gboolean on_wndPhotom_configure (GtkWidget *widget, GdkEventExpose *event,
                               	   gpointer data)
{
	/* Configure the photometry window */
	
	GdkColor gdk_magenta = {0, 49151, 0, 49151};
	gfloat f;
	
	if (!PhotWinConf) {
		
		/* Restore saved photometry parameters */
		
		f = R_config_f ("Photom/Aperture", 5.0);
		set_entry_float ("txtPhotAperture", f);
		f = R_config_f ("Photom/MinArea", 5.0);
		set_entry_float ("txtPhotMinarea", f);
		f = R_config_f ("Photom/Thresh", 2.0);
		set_entry_float ("txtPhotThresh", f);
	
		/* Get a pointer to the log window */
		
		txtPhot = GTK_TEXT_VIEW (xml_get_widget (xml_pht, "txtPhot"));
		
		/* Set up the text buffer for the log window */
		
		txtPhotBuffer = gtk_text_view_get_buffer (txtPhot); 
		
		/* Create colour tag */
		
		magenta_1 = gtk_text_buffer_create_tag (txtPhotBuffer, "magenta",
										  "foreground-gdk", &gdk_magenta, NULL);
		
		PhotWinConf = TRUE;
	}
	
	return FALSE;
}

gboolean on_wndPhotom_delete (GtkWidget *widget, GdkEventAny *event,
                                gpointer data)
{
	/* Close window in a controlled fashion if the user tries to close this 
	 * window via the window menu.
 	 */
	
	gtk_widget_activate (xml_get_widget (xml_pht, "btnPhotClose"));
	return TRUE;	
}


/******************************************************************************/
/*                                 SIGNALS                                    */
/******************************************************************************/

void on_FileSaveCCD_activate (GtkWidget *widget, gpointer data)
{
	/* Save the CCD image */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	if (ccd->Debayer && ccd->exd.h_bin == 1 && ccd->exd.v_bin == 1) {
		save_file (ccd, R, FALSE);
		save_file (ccd, G, FALSE);
		save_file (ccd, B, FALSE);
	} else
	    save_file (ccd, GREY, FALSE);
}

void on_FileSaveAUG_activate (GtkWidget *widget, gpointer data)
{
	/* Save the autoguider image */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	set_fits_data (aug, NULL, (aug->exd.FreeRunning ? FALSE : TRUE), TRUE);
	save_file (aug, GREY, FALSE);
}

void on_WriteDebugToLog_activate (GtkWidget *widget, gpointer data)
{
	/* Toggle writing of debug messages to the log window */
	
	Debug = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
}

void on_WriteLog_activate (GtkWidget *widget, gpointer data)
{
	/* Write the current content of the log window to a file */
	
	GtkTextIter start, end;	
	gchar *string;
	
	gtk_text_buffer_get_iter_at_offset (txtLogBuffer, &start, 0);
	gtk_text_buffer_get_iter_at_offset (txtLogBuffer, &end, -1);
	string = gtk_text_buffer_get_text (txtLogBuffer, &start, &end, FALSE);
	
	if (f_log)
		fprintf (f_log, "%s", string);
	g_free (string);
}

void on_ClearLog_activate (GtkWidget *widget, gpointer data)
{
	/* Clear out the current content of the log window */
	
	GtkTextIter start, end;
	
	gtk_text_buffer_get_iter_at_offset (txtLogBuffer, &start, 0);
	gtk_text_buffer_get_iter_at_offset (txtLogBuffer, &end, -1);
	gtk_text_buffer_delete (txtLogBuffer, &start, &end);
}

void on_FileExit_activate (GtkWidget *widget, gpointer data)
{
	/* Close the application unless the user elects not to */
	
	if (!query_file_not_saved ())
		loop_stop_loop ();
}

void on_Communications_activate (GtkWidget *widget, gpointer data)
{
	/* Rebuild submenus with current /dev/ttyUSB ports */
	
	comms_menus_update_ports ();
}

void on_TelescopeOpen_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close the telescope link */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gboolean Open;
	
	Open = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
	menu.OpenTelPort = Open;
	
	/* Set sensitive/insensitive any menu items that depend on the telescope
	 * link state.
	 */
	
	gtk_widget_set_sensitive (xml_get_widget (
                                   xml_app, "telescope_link"), !Open);
    if (Open)
		gtk_widget_set_sensitive (xml_get_widget (
								   xml_app, "gemini_commands"), FALSE);
	else {
		if (!aug->Open)
			gtk_widget_set_sensitive (xml_get_widget (
								   xml_app, "gemini_commands"), TRUE);
	}
	if (GeminiCmds)
		gtk_widget_set_sensitive (xml_get_widget (
								   xml_app, "periodic_error_correction"), Open);
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	/* Always attempt to close link; then reopen if we're supposed to be 
	 * opening it.
	 */
	
	telescope_close_comms_port ();

	if (Open) 
		if (!telescope_open_comms_port ())
			reset_checkbox_state (RCS_OPEN_COMMS_LINK, FALSE);
}

void on_GeminiCommands_activate (GtkWidget *widget, gpointer data)
{
	/* Toggle flag for using Losmandy Gemini commands */
	
	menu.Gemini =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (widget));
	GeminiCmds = menu.Gemini;
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "tblGemini"),GeminiCmds);
}

void on_AutogOpen_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close the guide signal comms link */
	
	gboolean Open;
	
	Open = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
	menu.OpenAutogPort = Open;
	
	/* Set sensitive/insensitive any menu items that depend on the autoguider
	 * link state.
	 */
	
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"autoguider_link"),!Open);
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}

	/* Always attempt to close link; then reopen if we're supposed to be 
	 * opening it.
	 */
	
	telescope_close_guide_port ();
	
	if (Open) {
		if (!telescope_open_guide_port ())
			reset_checkbox_state (RCS_OPEN_AUTOG_LINK, FALSE);
	}
}

void on_FilterWheelOpen_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close filterwheel link */
	
	gboolean Open;
	
	Open = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
	menu.OpenFilterwheelPort = Open;
		
	/* Set sensitive/insensitive any menu items that depend on the filter wheel
	 * link state.
	 */
	
	/* Filter wheel comms options permanently greyed out at present because
	 * there is only one valid option for each filterwheel type and that is
	 * selected automatically.
	 */
	//gtk_widget_set_sensitive (xml_get_widget (
	//                                 xml_app, "filter_wheel_link"), !Open);
	if (Open)
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			       xml_app, "filterwheel_menu")), widget_set_insensitive, NULL);
	else
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			       xml_app, "filterwheel_menu")), widget_set_sensitive, NULL);
	if (menu.OpenCCDCam)
		gtk_widget_set_sensitive (xml_get_widget (
		           xml_app, "filterwheel_int"), FALSE);
	gtk_widget_set_sensitive (xml_get_widget (
									   xml_app, "select_filter_wheel"),  !Open);
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}

	/* Now open the port */
	
	if (Open) {
		if (!filter_open_comms_port ())
			reset_checkbox_state (RCS_OPEN_FILTER_LINK, FALSE);
	} else {
		if (!filter_close_comms_port ())
			reset_checkbox_state (RCS_OPEN_FILTER_LINK, TRUE);
	}
}

void on_FocusOpen_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close the focuser link */
	
	struct focus f;
	gboolean Open;
	
	Open = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
	menu.OpenFocusPort = Open;
		
	loop_focus_open (Open);
	
	/* Set sensitive/insensitive any menu items that depend on the focuser
	 * link state.
	 */
	
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "focuser_link"), !Open);
	if (Open)
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			           xml_app, "focuser_menu")), widget_set_insensitive, NULL);
	else
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			           xml_app, "focuser_menu")), widget_set_sensitive, NULL);
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "tblFocus"),      Open);
	if (!Open)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (xml_get_widget (
							               xml_app, "chkFocusTempComp")),FALSE);																		  
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}

	/* Always attempt to close link; then reopen if we're supposed to be 
	 * opening it.
	 */
	
	focus_close_comms_port ();
	
	if (Open) {
		if (!focus_open_comms_port ())
			reset_checkbox_state (RCS_OPEN_FOCUS_LINK, FALSE);
		else {
		    if (!strcmp (menu.focuser, "focuser_robofocus")) {
				f.cmd = FC_VERSION;
				focus_comms->focus (&f);
				if (f.version >= 3.0)
					f.cmd = FC_MAX_TRAVEL_GET | FC_CUR_POS_GET | 
				            FC_BACKLASH_GET | FC_MOTOR_GET | FC_TEMP_GET;
				else
					f.cmd = FC_MAX_TRAVEL_GET | FC_CUR_POS_GET | 
				            FC_BACKLASH_GET;
				focus_comms->focus (&f);
				if (!f.Error) {
					set_entry_int ("txtFocusMaxTravel", f.max_travel);
					set_entry_int ("txtFocusCurrentPos", f.cur_pos);
					set_entry_int ("txtFocusBacklashSteps", f.backlash_steps);
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
										  xml_get_widget (xml_app, 
									     "optFocusBacklashIn")), f.BacklashIn);
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
										  xml_get_widget (xml_app, 
										 "optFocusBacklashOut")),!f.BacklashIn);
					if (f.version >= 3.0) {
						set_entry_int ("txtFocusStepSize", f.step_size);
						set_entry_int ("txtFocusStepPause", f.step_pause);
						set_entry_int ("txtFocusDutyCycle", f.duty_cycle);
						set_entry_float ("txtFocusTemperature", f.temp);
					} else
					    L_print ("{o}Can read motor configuration and "
								 "temperature only for Robofocus version 3\n");
				} else
				    L_print ("{r}Error reading focuser configuration\n");
			} else {
				/* Some other focuser */
			}
		}
	}
}

void on_ParallelPort_activate (GtkWidget *widget, gpointer data)
{
	/* Open the parallel port settings dialog box */

	if (xml_ppd != NULL) {  /* Present window if it's already open */
		gtk_window_present (GTK_WINDOW (xml_get_widget 
		                                         (xml_ppd, "wndParallelPort")));
		return;
	}
	
	gchar *objects[] = {"wndParallelPort", NULL};
	xml_ppd = xml_load_new (xml_ppd, GLADE_INTERFACE, objects);
}

void on_ParPortOpen_activate (GtkWidget *widget, gpointer data)
{
	#ifdef HAVE_LIBPARAPIN
	/* Open/close the parallel port */
	
	gboolean Open;
	
	Open = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
	menu.OpenParPort = Open;
	
	/* Set sensitive/insensitive any dependent menu items */
	
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "parallel_port"), !Open);
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	if (Open) {
		if (!telescope_open_parallel_port ())
			reset_checkbox_state (RCS_OPEN_PARPORT, FALSE);
	} else {
		if (!telescope_close_parallel_port ())
			reset_checkbox_state (RCS_OPEN_PARPORT, TRUE);
	}
	#endif
}

void on_CCDCamType_activate (GtkWidget *widget, gpointer data)
{
	/* Set the CCD camera type.  Any menu item that calls this function
	 * is available only if GoQat was compiled with the supporting libraries for
	 * the corresponding hardware already installed.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	const gchar *name;
	
	if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)))
		return;
		
	name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
	
	if (!(strcmp (name, "ccd_qsi"))) {
		ccd->device = QSI;
	} else if (!(strcmp (name, "ccd_sx"))) {
		ccd->device = SX;
	} else 
	    L_print ("{r}%s: Unknown CCD camera type: "
				 " %s\n", __func__, name);
	
	strcpy (menu.ccd_camera, name);
}

void on_CCDSelect_activate (GtkWidget *widget, gpointer data)
{
	/* Search for connected cameras and ask the user to choose one */
	
	if (ccdcam_get_cameras ()) {
		ds.idx = &(get_ccd_image_struct ())->devnum;
		select_device ();
	}
}

void on_CCDOpen_activate (GtkWidget *widget, gpointer data)
{
	/* Open the selected CCD camera */
	
	menu.OpenCCDCam = gtk_check_menu_item_get_active (
												  GTK_CHECK_MENU_ITEM (widget)); 
	
	/* Set sensitive/insensitive any menu items that depend on the CCD camera
	 * state.
	 */
	
	if (menu.OpenCCDCam)
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			        xml_app, "ccd_camera_menu")), widget_set_insensitive, NULL);
	else
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			        xml_app, "ccd_camera_menu")), widget_set_sensitive, NULL);
	gtk_widget_set_sensitive (xml_get_widget (
							 xml_app, "select_ccd_camera"), !menu.OpenCCDCam);
	gtk_widget_set_sensitive (xml_get_widget (
							 xml_app, "configure_ccd_camera"), menu.OpenCCDCam);
	gtk_widget_set_sensitive (xml_get_widget (
							 xml_app, "tblCCD"), menu.OpenCCDCam);
	
	if (!strcmp (menu.filterwheel, "filterwheel_int")) {						 
		if (menu.OpenCCDCam)
			gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			       xml_app, "filterwheel_menu")), widget_set_insensitive, NULL);
		else
			gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			       xml_app, "filterwheel_menu")), widget_set_sensitive, NULL);
	} else
		gtk_widget_set_sensitive (xml_get_widget (
							     xml_app, "filterwheel_int"), !menu.OpenCCDCam);
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	loop_ccd_open (menu.OpenCCDCam);
}

void on_CCDConfig_activate (GtkWidget *widget, gpointer data)
{
	/* Open (or show) the CCD configuration window.  Note that this window could
	 * be opened to configure a camera as the main imaging camera, or as an
	 * autoguider camera, but only one instance of it can be opened at once.
	 */
	 
	struct cam_img *img;
	
	gchar *window = NULL;
	 
	if (!strcmp (gtk_menu_item_get_label (GTK_MENU_ITEM (widget)),
												   "Configure CCD camera...")) {
		/* Configuring any CCD camera used as the main camera */
		img = get_ccd_image_struct ();
	} else if (!strcmp (gtk_menu_item_get_label (GTK_MENU_ITEM (widget)),
												   "Configure SX camera...")) {
		/* Configuring a SX camera used as an autoguider camera */
		img = get_aug_image_struct ();
	} else {
		L_print ("{r}Unrecognised menu item\n");
		return;
	}
	
	if (img->device == QSI)
		window = "wndQSIConfig";
	else if (img->device == SX)
		window = "wndSXConfig";
	else {
		L_print ("{r}Unknown camera type - can't configure it!\n");
		return;
	}
	
	/* Present window if it is already open; otherwise open it */
	
	if (xml_con) {
		gtk_window_present (GTK_WINDOW (CCDConfigWin));
	} else {
		gchar *objects[] = {window, NULL};
		xml_con = xml_load_new (xml_con, GLADE_INTERFACE, objects);
	}
	
	/* Set the owner of the configuration window and a pointer to the window */
	
	CCDConfigOwner = img;
	CCDConfigWin = xml_get_widget (xml_con, window);
}

void on_FullFrame_activate (GtkWidget *widget, gpointer data)
{
	/* Toggle the option to display CCD camera data embedded in the full frame
	 * chip area.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	menu.FullFrame=gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (widget));
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "btnGetRegion"),
	                                                            menu.FullFrame);
	ccd->FullFrame = menu.FullFrame;
}

void on_Debayer_activate (GtkWidget *widget, gpointer data)
{
	/* Set flag for debayering.  The names below must match the names in the
	 * Glade interface file.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
		
	const gchar *name;
	
	name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
	if (!(strcmp (name, "simple")))
		ccd->imdisp.debayer = DB_SIMP;
	else if (!(strcmp (name, "nearest")))
		ccd->imdisp.debayer = DB_NEAR;
	else if (!(strcmp (name, "bilinear")))
		ccd->imdisp.debayer = DB_BILIN;	
	else if (!(strcmp (name, "quality")))
		ccd->imdisp.debayer = DB_QUAL;	
	/*else if (!(strcmp (name, "downsample")))
		ccd->imdisp.debayer = DB_DOWN;	*/
	else if (!(strcmp (name, "gradients")))
		ccd->imdisp.debayer = DB_GRADS;	
	else
		msg ("Warning - Invalid option for debayer method!");	

	strcpy (menu.debayer, name);
	
	/* Debayer and redisplay the image (the ccdcam_debayer routine checks to 
	 * see whether any image data actually exists before attempting to debayer
	 * it).
	 */
	
	if (ccd->Debayer && ccd->exd.h_bin == 1 && ccd->exd.v_bin == 1) {
		if (ccdcam_debayer ())
			loop_ccd_display_image ();
	}
}

void on_AutogCamType_activate (GtkWidget *widget, gpointer data)
{
	/* Set the autoguider camera type.  Any menu item that calls this function
	 * is available only if GoQat was compiled with the supporting libraries for
	 * the corresponding hardware already installed.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	const gchar *name;
	
	if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)))
		return;
		
	name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
	aug->device = OTHER;
	
	#ifdef HAVE_LIBUNICAP
	if (!strcmp (name, "unicap")) {
		aug->device = UNICAP;
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "greyscale_conversion"), TRUE); 
	}
	gtk_widget_set_sensitive (xml_get_widget (
		                      xml_app, "unicap_device"), 
		                      aug->device == UNICAP ? TRUE : FALSE);
	#endif /* HAVE_LIBUNICAP */
    #ifdef HAVE_LIBV4L2
	if (!strcmp (name, "V4L_(/dev/video0)") ||
		!strcmp (name, "V4L_(/dev/video1)") ||
		!strcmp (name, "V4L_(/dev/video2)") ||
		!strcmp (name, "V4L_(/dev/video3)")) {
		aug->device = V4L;
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "greyscale_conversion"), TRUE); 
	}
	gtk_widget_set_sensitive (xml_get_widget (
		                      xml_app, "v4l_properties"), 
		                      aug->device == V4L ? TRUE : FALSE);
    #endif
    #ifdef HAVE_SX_CAM
	if (!strcmp (name, "SX")) {
		aug->device = SX;
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "greyscale_conversion"), FALSE); 
	}
	gtk_widget_set_sensitive (xml_get_widget (
		                      xml_app, "select_sx_camera"), 
		                      aug->device == SX ? TRUE : FALSE);
	if (!strcmp (name, "SX_GuideHead")) {
		aug->device = SX_GH;
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "greyscale_conversion"), FALSE);
	} 
	/* Don't 'un-grey' the autoguider sx camera selection menu because in 
	 * this case we're using the guide head attached to the CCD camera.
	 */
	#endif /* HAVE_SX_CAM */
	
	if (aug->device == OTHER)
	    L_print ("{r}%s: Unknown autoguider camera type\n", __func__);
	
	strcpy (menu.aug_camera, name);
}

void on_UnicapDevice_activate (GtkWidget *widget, gpointer data)
{
	/* Let the user select the desired unicap device and format */
	
	#ifdef HAVE_UNICAP /* Prevents "can't find signal handler" message if here*/
	GtkWidget *device_selection, *format_selection;
	GtkWidget *hbxHbox;
	
	/* Open the selection window */
	
	gchar *objects[] = {"wndUnicapSelect", NULL};
	xml_uni = xml_load_new (xml_uni, GLADE_INTERFACE, objects);
	
	/* Add an hbox and pack in the device and format selection widgets */
	
	hbxHbox = gtk_hbox_new (FALSE, 10);
	gtk_container_add (GTK_CONTAINER (xml_get_widget (
										 xml_uni, "vbxUnicapSelect")), hbxHbox);
	format_selection = unicapgtk_video_format_selection_new ();
	device_selection = unicapgtk_device_selection_new (TRUE);
    gtk_box_pack_start (GTK_BOX (hbxHbox), device_selection, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbxHbox), format_selection, TRUE, TRUE, 0);
	unicapgtk_device_selection_rescan (
							     UNICAPGTK_DEVICE_SELECTION (device_selection));
	
	/* Connect signal handlers and show the window contents */
	
    g_signal_connect (G_OBJECT (device_selection),
					  "unicapgtk_device_selection_changed",
                      G_CALLBACK (unicap_device_change_cb), format_selection);	
	gtk_combo_box_set_active (GTK_COMBO_BOX (device_selection), 0);
    g_signal_connect (G_OBJECT (format_selection),
                      "unicapgtk_video_format_changed",
                      G_CALLBACK (unicap_format_change_cb), NULL);	
	gtk_widget_show_all (xml_get_widget (xml_uni, "wndUnicapSelect"));
	#endif
}

void on_UnicapProperties_activate (GtkWidget *widget, gpointer data)
{
	/* Display the Unicap properties dialog */
	
	#ifdef HAVE_UNICAP /* Prevents "can't find signal handler" message if here*/
	struct cam_img *aug = get_aug_image_struct ();
	
	GtkWidget *property_dialog;
	
	property_dialog = unicapgtk_property_dialog_new ();
	unicapgtk_property_dialog_set_handle (
				  UNICAPGTK_PROPERTY_DIALOG (property_dialog), aug->ucp_handle);
	#endif
}

void on_V4LProperties_activate (GtkWidget *widget, gpointer data)
{
	/* Display the V4L properties dialog */

	if (xml_V4L) {
		gtk_window_present (GTK_WINDOW (xml_get_widget (
		                                             xml_V4L, "wndV4LConfig")));
	} else {
		gchar *objects[] = {"wndV4LConfig", NULL};
		xml_V4L = xml_load_new (xml_V4L, GLADE_INTERFACE, objects);
	}
}

void on_SXCamSelect_activate (GtkWidget *widget, gpointer data)
{
	/* Select an sx camera to use as an autoguider */
	
	#ifdef HAVE_SX_CAM /* Prevents "can't find signal handler" message if here*/
	if (augcam_get_sx_cameras ()) {
		ds.idx = &(get_aug_image_struct ())->devnum;
		select_device ();
	}
	#endif
}

void on_Greyscale_activate (GtkWidget *widget, gpointer data)
{
	/* Set flag for greyscale conversion.  The names below must match the
	 * names in the Glade gtkbuilder file.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	const gchar *name;
	
	name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
	if (!(strcmp (name, "luminance")))
		aug->imdisp.greyscale = LUMIN;
	else if (!(strcmp (name, "desaturate")))
		aug->imdisp.greyscale = DESAT;
	else if (!(strcmp (name, "maximum")))
		aug->imdisp.greyscale = MAXIM;
	else if (!(strcmp (name, "mono")))
		aug->imdisp.greyscale = MONO;	
	else
		msg ("Warning - Invalid option for greyscale setting!");	

	strcpy (menu.greyscale, name);
}

void on_Setting_activate (GtkWidget *widget, gpointer data)
{
	/* Open (or show) the settings dialog and load the list contents */
	
	struct autog_config *s; 
	GtkTreeIter iter;
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	gushort SetNumEntries = 0;	
	gchar *s1, *s2;
	
	if (xml_set) {
		gtk_window_present (GTK_WINDOW (xml_get_widget (xml_set,"wndSetting")));
		return;
	}
	
	gchar *objects[] = {"wndSetting", NULL};
	xml_set = xml_load_new (xml_set, GLADE_INTERFACE, objects);
    
	/* Set up the list store for holding the settings data */
	
	lisSetting = gtk_list_store_new (N_COLUMNS,    /* One entry for each element*/
	 							     G_TYPE_STRING,/*  of the Setting structure */
								     G_TYPE_STRING,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT,
								     G_TYPE_FLOAT);
	
	/* Associate the tree view with the list model */
	
	trvSetting = GTK_TREE_VIEW (xml_get_widget (xml_set, "trvSetting"));
	gtk_tree_view_set_model (trvSetting, GTK_TREE_MODEL (lisSetting));

	/* Set the columns to be displayed in the tree view */

	renderer = gtk_cell_renderer_text_new ();	
	column = gtk_tree_view_column_new_with_attributes ("Telescope",
                                                      renderer,
                                                      "text", TELESCOPE,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (trvSetting), column);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Instrument",
                                                      renderer,
                                                      "text", INSTRUMENT,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (trvSetting), column);	

	/* Populate each list store row with settings data */
	
	s = (struct autog_config *) g_malloc0 (sizeof(struct autog_config));	
	s->Telescope = (gchar *) g_malloc0 (MCSL * sizeof (gchar));
	s->Instrument = (gchar *) g_malloc0 (MCSL * sizeof (gchar));	

	while (TRUE) {
		
		/* Load the data, checking to see whether the initial value assigned to
         * the 'Telescope' entry has been overwritten or not.  If it has been
         * overwritten, there's a set of data to be read from the configuration
         * file; if it hasn't, there isn't.
		 */
		
		s1 = g_strdup_printf ("Setting/%i", SetNumEntries++);
		s2 = g_strconcat (s1, "/Telescope", NULL);

		strcpy (&s->Telescope[0], "~"); /* Set initial value for 'Telescope' */
		R_config_s (s2, s->Telescope);
		if (strncmp (s->Telescope, "~", 1) != 0) { /* Overwritten? */
			g_free (s2);
			s2 = g_strconcat (s1, "/Instrument", NULL);
			R_config_s (s2, s->Instrument);
			g_free (s2);
			s2 = g_strconcat (s1, "/NSCalibDuration", NULL);
			s->NSCalibDuration = R_config_f (s2, 1.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/EWCalibDuration", NULL);
			s->EWCalibDuration = R_config_f (s2, 1.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/GuideSpeed", NULL);
			s->GuideSpeed = R_config_f (s2, 0.5);
			g_free (s2);
			s2 = g_strconcat (s1, "/MaxShift", NULL);
			s->MaxShift = R_config_f (s2, 1.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/MaxDrift", NULL);
			s->MaxDrift = R_config_f (s2, 1.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/MaxMove", NULL);
			s->MaxMove = R_config_f (s2, 1.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/MaxOffset", NULL);
			s->MaxOffset = R_config_f (s2, 1.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/DriftSample", NULL);
			s->DriftSample = R_config_f (s2, 7.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/CorrFac", NULL);
			s->CorrFac = R_config_f (s2, 1.0);
			g_free (s2);
			s2 = g_strconcat (s1, "/Update", NULL);
			s->Update = R_config_f (s2, 1.0);
			g_free (s2);
            
            /* Delete this group of entries from the configuration file.  The
             * entries will be re-created when the user closes the dialog, or
             * will not be re-created if the user chooses to delete this set of
             * configuration values.
             */
            
            DeleteCGroup (s1);			
			g_free (s1);
			
			/* Create a list entry using the values read from the configuration
             * file.
             */
			
			gtk_list_store_append (lisSetting, &iter);  /* Acquire an iterator */				
			gtk_list_store_set (lisSetting, &iter,
								TELESCOPE, s->Telescope,
								INSTRUMENT, s->Instrument,
								NSCALIBDURATION, s->NSCalibDuration,
								EWCALIBDURATION, s->EWCalibDuration,
								GUIDESPEED, s->GuideSpeed,
								MAXSHIFT, s->MaxShift,
								MAXDRIFT, s->MaxDrift,
								MAXMOVE, s->MaxMove,
								MAXOFFSET, s->MaxOffset,
								DRIFTSAMPLE, s->DriftSample,
								CORRFAC, s->CorrFac,
								UPDATE, s->Update,
								-1);
		} else {
			g_free (s->Telescope);
			g_free (s->Instrument);
			break;
		}
	}
}

void on_FilterWheelType_activate (GtkWidget *widget, gpointer data)
{
	/* Set selected filter wheel type to be active device */
	
	const gchar *name;
	
	if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)))
		return;
	
	name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
	
	if (!(strcmp (name, "filterwheel_int"))) {
		filter_set_device (INTERNAL);
		/* Set filter wheel selection and configuration insensitive */
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "select_filter_wheel"), FALSE);
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "configure_filters"), FALSE);
		/* Set filter wheel comms to "Via CCD camera" */
		strcpy (menu.filterwheel_port, "w_CCDCam");
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			 xml_app, "filter_wheel_link_menu")), comms_ports_set_active, NULL);
	} else if (!(strcmp (name, "filterwheel_sx"))) {
		filter_set_device (SX);
		/* Set filter wheel selection and configuration sensitive */
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "select_filter_wheel"), TRUE);
		gtk_widget_set_sensitive (xml_get_widget (
		                          xml_app, "configure_filters"), TRUE);
		/* Set filter wheel comms to "USB direct" */
		strcpy (menu.filterwheel_port, "w_USBdirect");
		gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			 xml_app, "filter_wheel_link_menu")), comms_ports_set_active, NULL);
	} else 
	    L_print ("{r}%s: Unknown filter wheel type: "
				 " %s\n", __func__, name);
	
	strcpy (menu.filterwheel, name);
}

void on_FilterWheelSelect_activate (GtkWidget *widget, gpointer data)
{
	/* Search for connected filter wheels and ask the user to choose one */
	
	if (filter_get_filterwheels ()) {
		ds.idx = get_filter_devnumptr ();
		select_device ();
	}
}

void on_FiltersConfig_activate (GtkWidget *widget, gpointer data)
{
	/* Open (or show) the filters configuration window.  Note that for QSI
	 * cameras, filter configuration is managed by the CCD camera 
	 * configuration window.
	 */
	 
	/* Present window if it is already open; otherwise open it */
	
	if (xml_fil) {
		gtk_window_present (GTK_WINDOW (xml_get_widget (
		                                          xml_fil, "wndFilterConfig")));
	} else {
		gchar *objects[] = {"wndFilterConfig", NULL};
		xml_fil = xml_load_new (xml_fil, GLADE_INTERFACE, objects);
	}
}

void on_FocuserType_activate (GtkWidget *widget, gpointer data)
{
	/* Set selected focuser type to be active device */
	
	const gchar *name;
	
	if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget)))
		return;
	
	name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
	strcpy (menu.focuser, name);
}

void on_PEC_activate (GtkWidget *widget, gpointer data)
{
	/* Switch PEC on/off */
	
	gboolean Activate;
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}

	/* Turn PEC on/off */
	
	Activate = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
	if (!telescope_PEC_on (Activate)) {
	    reset_checkbox_state (RCS_PEC_ON, !Activate);
		return;
	}

	/* If turning PEC on, advise the user if the guide speed associated with the
	 * most recently uploaded data does not match the currently set guide speed.
	 */
	
	if (Activate)
		warn_PEC_guidespeed (TRUE);
}

void on_PrecessCoords_activate (GtkWidget *widget, gpointer data)
{
	/* Toggle flag for precession of telescope coordinates */
	
	menu.Precess = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (widget));
}

void on_UseUTC_activate (GtkWidget *widget, gpointer data)
{
	/* Toggle flag for using UTC in log files and for 'At' task */
	
	menu.UTC =  gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM (widget));
	UseUTC = menu.UTC;
}

void on_SetCanvasFont_activate (GtkWidget *widget, gpointer data)
{
	/* Display the font selection dialog */
	
	GtkWidget *dlgFontSelection;
	
    gchar *objects[] = {"dlgFontSelection", NULL};
    xml_font = xml_load_new (xml_font, GLADE_INTERFACE, objects);
	
	dlgFontSelection = xml_get_widget (xml_font, "dlgFontSelection");
	gtk_font_selection_dialog_set_preview_text (GTK_FONT_SELECTION_DIALOG 
	                     (dlgFontSelection), "Histogram Flux Scale 0123456789");
}

void on_LiveView_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close the live view window.  Note that the corresponding menu item
	 * is not shown if the application has been compiled without Unicap
	 * support.
	 */
	
	#ifdef HAVE_UNICAP /* Prevents "can't find signal handler" message if here*/
	struct cam_img *aug = get_aug_image_struct ();
	
	GtkCheckMenuItem *mnuItem;
	
	menu.LiveView = gtk_check_menu_item_get_active (
												  GTK_CHECK_MENU_ITEM (widget));
	
	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (xml_get_widget (
	                                               xml_app, "chkAutogOpen")))) {
		gtk_widget_set_sensitive (xml_get_widget (
                             xml_app, "autoguider_camera_menu"),!menu.LiveView);
	
		if (aug->device == UNICAP) {
			gtk_widget_set_sensitive (xml_get_widget (
								      xml_app, "unicap_device"),!menu.LiveView);
			gtk_widget_set_sensitive (xml_get_widget (
								  xml_app, "unicap_properties"), menu.LiveView);
		}
	}	
	
	mnuItem = GTK_CHECK_MENU_ITEM (xml_get_widget (xml_app, "playback"));
	gtk_widget_set_sensitive (GTK_WIDGET (mnuItem), !menu.LiveView);
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	 
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	if (menu.LiveView)
		switch (aug->device) {
			case UNICAP:
				unicap_void_device (&aug->ucp_device);				
				strcpy (aug->ucp_device.identifier, "NONE");
			    R_config_s ("Unicap/Device/ID", aug->ucp_device.identifier);
				unicap_void_format (&aug->ucp_format_spec);
			    aug->ucp_format_spec.fourcc = R_config_d (
											   "Unicap/Format/Fourcc", 0);
			    aug->ucp_format_spec.size.width = R_config_d (
											   "Unicap/Format/Size/Width", 0);
			    aug->ucp_format_spec.size.height = R_config_d (
											   "Unicap/Format/Size/Height", 0);
			    break;
			default:
				L_print ("{r}Error - Can't open Live View unless a Unicap "
						                             "device is being used!\n");
			    reset_checkbox_state (RCS_OPEN_LIVEVIEW_WINDOW, FALSE);
			    return;
		}
	else
		video_close_file ();
	
	/* Now open/close LiveView window */
	
	loop_LiveView_open (menu.LiveView);
	#endif
}

void on_Playback_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close the playback window */
	
	gboolean Active;
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	Active = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget));
	
	if (Active) {
		gchar *objects[] = {"hscPBFrames_adj", "wndPlayback", NULL};
        xml_pbw = xml_load_new (xml_pbw, GLADE_INTERFACE, objects); 
		video_init ();
    } else {                   /* Close */
		if (xml_pht)
            gtk_widget_activate (xml_get_widget(xml_pht, "btnPhotClose"));
	    video_close_file ();
        gtk_widget_destroy (xml_get_widget (xml_pbw, "wndPlayback"));
        g_object_unref (G_OBJECT (xml_pbw));
        xml_pbw = NULL;
        PBWinConf = FALSE;
	}
	
	gtk_widget_set_sensitive (GTK_WIDGET (GTK_CHECK_MENU_ITEM (
						      xml_get_widget (xml_app, "live_view"))), !Active);
}

void on_CCDTemps_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close the CCD temperatures window */
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, do that and then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
		#ifdef HAVE_LIBGRACE_NP
		struct cam_img *ccd = get_ccd_image_struct (); 
		gboolean AlreadyOpen = FALSE;
		if (!Grace_Open (GRACE_TEMPS, &AlreadyOpen)) {
			reset_checkbox_state (RCS_OPEN_TEMPS_WINDOW, FALSE);
			return;
		}
		ccd->graph.reset = !AlreadyOpen;
		loop_ccd_temps (TRUE, 1);  /* Update every second */
		#else
		L_print("{o}Sorry! You need to install Grace to display this window\n");
		#endif
	} else {
		#ifdef HAVE_LIBGRACE_NP
		loop_ccd_temps (FALSE, 1);  /* Pause updating */
		#endif
	}
}

void on_AutogTrace_activate (GtkWidget *widget, gpointer data)
{
	/* Open/close the autoguider trace window */
	
	struct cam_img *aug = get_aug_image_struct (); 
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, do that and then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
		#ifdef HAVE_LIBGRACE_NP            /* Use Grace if available */
		gboolean AlreadyOpen = FALSE;
		if (!Grace_Open (GRACE_TRACE, &AlreadyOpen)) {
			reset_checkbox_state (RCS_OPEN_TRACE_WINDOW, FALSE);
			return;
		}
		aug->graph.reset = !AlreadyOpen;
		aug->graph.draw = TRUE;
		#endif
	} else {
		aug->graph.draw = FALSE;
	}
}

void on_ShowToolbar_activate (GtkWidget *widget, gpointer data)
{
	/* Show/hide the main window toolbar */
	
	GtkWidget *tlbToolbar;
	
	tlbToolbar = xml_get_widget (xml_app, "tlbToolbar");
	menu.ShowToolbar = gtk_check_menu_item_get_active (
	                                              GTK_CHECK_MENU_ITEM (widget));
	if (menu.ShowToolbar)
		gtk_widget_show (tlbToolbar);
	else
		gtk_widget_hide (tlbToolbar);
		
	gtk_window_resize (GTK_WINDOW (xml_get_widget (xml_app, "ccdApp")), 1, 1);
}

void on_HelpAbout_activate (GtkWidget *widget, gpointer data)
{
    /* Show the 'About' dialog */
	
	gchar *authors[] = {"Edward Simonson;", "please see website and code file "
	                    "headers\nfor full credits and acknowledgements", NULL};
	gchar *license = 
	"GoQat is free software; you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation; either version 3 of the License, or\n"
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program; if not, see <http://www.gnu.org/licenses/> .";

	gtk_show_about_dialog (GTK_WINDOW (xml_get_widget (xml_app, "ccdApp")),
	                       "name", "GoQat",
						   "version", VERSION,
						   "authors", authors,
						   "copyright", "(c) Edward Simonson, 2009 - 2014",
						   "license", license,
	                       NULL);
}

void on_btnFullFrame_clicked (GtkButton *button, gpointer data)
{
	/* Set the text fields for the CCD image coordinates to their full-frame
	 * values.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();		
	
	set_entry_int ("txtH1", 1);
	set_entry_int ("txtV1", 1);
	set_entry_int ("txtH2", ccd->cam_cap.max_h);
	set_entry_int ("txtV2", ccd->cam_cap.max_v);
}

void on_btnGetRegion_clicked (GtkButton *button, gpointer data)
{
	/* Get the selected imaging region for the CCD chip from DS9 */
	
	struct cam_img *ccd = get_ccd_image_struct ();	
	
	gint h_size, v_size;
	gfloat x, y, w, h;
	
	/* Get the region */

	if (xpa_get_rect_coords (&x, &y, &w, &h)) {

		h_size = ccd->cam_cap.max_h;
		v_size = ccd->cam_cap.max_v;
		
		set_entry_int ("txtH1", CLAMP ((gint) (x - w/2.0), 1, h_size));
		set_entry_int ("txtV1", CLAMP ((gint) (y - h/2.0), 1, v_size));
		set_entry_int ("txtH2", CLAMP ((gint) (x + w/2.0), 1, h_size));
		set_entry_int ("txtV2", CLAMP ((gint) (y + h/2.0), 1, v_size));
	}
}

void on_spbBin_value_changed (GtkSpinButton *spin, gpointer data)
{
	/* Set the horizontal and vertical binning text fields to this value, when
	 * the content is changed.
	 */
	 
	struct cam_img *ccd = get_ccd_image_struct ();
	
	gint bin;
	
	bin = gtk_spin_button_get_value_as_int (spin);
	set_entry_int ("txtHBin", bin);
	if (ccd->device == SX && ccd->cam_cap.IsInterlaced && bin > 1 && bin%2) {
		L_print ("{o}Vertical binning value restricted to even numbers "
				 "for interlaced camera: using %dx%d\n",
				  bin, bin - 1);
		bin--;
	}
	set_entry_int ("txtVBin", bin);
}

void on_txtExposure_activate (GtkEditable *editable, gpointer data)
{
	/* Start the exposure if the user presses the Return key in the
	 * Exposure field.
	 */
	
	gtk_widget_activate (xml_get_widget (xml_app, "btnStart"));
}

void on_cmbExpType_changed (GtkWidget *widget, gpointer data)
{
	/* Set the minimum exposure time in the text field on the CCD camera tab
	 * if a camera is connected and a bias frame is selected.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	if (!strcmp ("BIAS", gtk_combo_box_get_active_text (
													 GTK_COMBO_BOX (widget)))) {
	    if (ccd->Open)
			set_entry_float ("txtExposure", ccd->cam_cap.min_exp);
	}
}

void on_chkIgnoreCCDCooling_toggled (GtkButton *button, gpointer data)
{
	/* Set flag to ignore cooling state when making an exposure */
	
	struct cam_img *ccd = get_ccd_image_struct ();
		
	ccd->state.IgnoreCooling = gtk_toggle_button_get_active (
	                                                GTK_TOGGLE_BUTTON (button));
	W_config_d ("Misc/IgnoreCCDCooling", ccd->state.IgnoreCooling);
}

void on_chkDisplayCCDImage_toggled (GtkButton *button, gpointer data)
{
	/* Toggle display of the CCD image */
	
	struct cam_img *ccd = get_ccd_image_struct ();
		
	ccd->Display = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	W_config_d ("Misc/DisplayCCDImage", ccd->Display);
}

void on_chkBeepExposure_toggled (GtkButton *button, gpointer data)
{
	/* Set flag to beep after each CCD exposure */
	
	struct cam_img *ccd = get_ccd_image_struct ();
		
	ccd->Beep = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	W_config_d ("Misc/Beep", ccd->Beep);
}

void on_btnStart_clicked (GtkButton *button, gpointer data)
{
	/* Initiate the exposure */ 
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	struct exposure_data exd;
	GtkWidget /**dlgLightPath,*/ *dlgAutoSave;
	gushort i;
	gint num, ret;
	gchar *stra[10];
	
	/* Abort if the task list is running */
	
	if (tasks_get_status () & TSK_ACTIVE) {
		L_print ("{r}Error - Can't start an exposure while the task list "
				                                               "is running!\n");
		return;
	}		
	
	/* Prompt the user if more than one exposure is selected in the
	 * 'Num. Exposures' field, and autosave has not been selected.
	 */
	 
	get_entry_int ("txtNumExposures", 1, 999, 1, NO_PAGE, &num);
	if (num > 1 && !ccd->AutoSave) {
		dlgAutoSave = gtk_message_dialog_new (ccdApp,
											 GTK_DIALOG_DESTROY_WITH_PARENT,
											 GTK_MESSAGE_QUESTION,
											 GTK_BUTTONS_YES_NO,
											 "Do you want to autosave the "
		                                     "sequence of images?\n");
		ret = gtk_dialog_run (GTK_DIALOG (dlgAutoSave));
		gtk_widget_destroy (dlgAutoSave);
		switch (ret) {
			case GTK_RESPONSE_YES:
				set_notebook_page (FIL_PAGE);
				return;
				break;
			case GTK_RESPONSE_NO:
				break;
			default: /* User closed dialog with window manager */
				break;
		}
	}
	
	/* Get the exposure parameters */
	
	get_ccd_image_params (&exd);

	/* Add the exposure(s) to the task list */
	
	tasks_clear ();
	stra[0] = exd.ExpType;
	stra[1] = exd.filter;
	stra[2] = g_strdup_printf ("%.3f", exd.req_len);
	stra[3] = g_strdup_printf ("%d", exd.h_top_l);
	stra[4] = g_strdup_printf ("%d", exd.v_top_l);
	stra[5] = g_strdup_printf ("%d", exd.h_bot_r);
	stra[6] = g_strdup_printf ("%d", exd.v_bot_r);
	stra[7] = g_strdup_printf ("%d", exd.h_bin);
	stra[8] = g_strdup_printf ("%d", exd.v_bin);
	stra[9] = g_strdup_printf ("%.2f", exd.ccdtemp);

	while (num--)
		tasks_add_Exposure (stra);
	for (i = 2; i < 10; i++)
		g_free (stra[i]);
	set_task_buttons (TRUE);	
	tasks_start ();
}

void on_btnCancel_clicked (GtkButton *button, gpointer data)
{
	/* Cancel the exposure */
	
	loop_ccd_cancel ();
}

void on_btnInterrupt_clicked (GtkButton *button, gpointer data)
{
	/* Interrupt the existing exposure and read the camera */

	loop_ccd_interrupt ();
}

void on_chkAutoSave_toggled (GtkButton *button, gpointer data)
{
	/* Toggle the autosave flag on and off.  If on, then images are
	 * automatically saved with the specified file name.
	 */
	
	struct cam_img *img;
	const gchar *ButtonName;
	
	ButtonName = gtk_buildable_get_name (GTK_BUILDABLE (button));	
	if (!g_ascii_strcasecmp (ButtonName, "chkAutoSaveCCD"))
		img = get_ccd_image_struct ();
	else if (!g_ascii_strcasecmp (ButtonName, "chkAutoSaveAUG"))
		img = get_aug_image_struct ();
	else {
		G_print ("{r}on_chkAutoSave_toggled: Not a valid button name: %s\n", 
		                                                            ButtonName);
		return;
	}
	
	img->AutoSave = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
}

void on_chkSaveEvery_toggled (GtkButton *button, gpointer data)
{
	/* Schedule the autoguider image to be saved periodically,
	 * if requested.
	 */
	 
	gint period;
	
	struct cam_img *aug = get_aug_image_struct ();

	get_entry_int ("txtSaveEvery", 1, 3600, 60, FIL_PAGE, &period);
	aug->SavePeriodic = gtk_toggle_button_get_active (
	                                                GTK_TOGGLE_BUTTON (button));
	loop_save_periodic (aug->SavePeriodic, (guint) period);
}

void on_chkWatchActive_toggled (GtkButton *button, gpointer data)
{
	/* Get the watch file path and activate watching the file for incoming 
	 * tasks.
	 */
	 
	gchar *u, *s, *s1;
	gboolean active;
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, do that and then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	if (active) {
		u = gtk_file_chooser_get_uri (
				  GTK_FILE_CHOOSER (xml_get_widget(xml_app, "flcWatchFolder")));
		s = g_filename_from_uri (u, NULL, NULL);
		g_free (u);
		
		s1 = get_entry_string ("txtWatchFile");
		if (!strlen (s1)) {
			L_print ("{o}Please enter a file name\n");
			reset_checkbox_state (RCS_ACTIVATE_WATCH, FALSE);
			g_free (s1);
			g_free (s);
			return;
		}
		WatchFile = g_strconcat (s, "/", s1, NULL);
		g_free (s1);
		g_free (s);
	}
	gtk_widget_set_sensitive (xml_get_widget(xml_app,"flcWatchFolder"),!active);
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "txtWatchFile"),!active);
	loop_watch_activate (active);
}

void on_btnPPClose_clicked (GtkButton *button, gpointer data)
{
	/* Save settings and close the parallel port settings window */
	
	gchar *s;
	
	s = get_entry_string ("txtPPAddress");
	strcpy (ports[LPT].address, s);
	g_free (s);
	get_entry_int ("txtPPRAp", 2, 9, 6, NO_PAGE, &ports[LPT].RAp);
	get_entry_int ("txtPPRAm", 2, 9, 7, NO_PAGE, &ports[LPT].RAm);
	get_entry_int ("txtPPDecp", 2, 9, 8, NO_PAGE, &ports[LPT].Decp);
	get_entry_int ("txtPPDecm", 2, 9, 9, NO_PAGE, &ports[LPT].Decm);
	//get_entry_int ("txtPPSC1LongExp", 2, 9, 2, NO_PAGE, &ports[LPT].SC1LongExp);
	//get_entry_int ("txtPPSC1Pause", 0, 999, 10, NO_PAGE, &ports[LPT].SC1Pause);
	
	gtk_widget_destroy (xml_get_widget (xml_ppd, "wndParallelPort"));
	g_object_unref (G_OBJECT (xml_ppd));
	xml_ppd = NULL;
	PPWinConf = FALSE;
}

void on_btnZoomIn_clicked (GtkButton *button, gpointer data)
{
	/* Zoom in to the selected area of the autoguider image canvas and 
     * centre it
     */
	
	canvas_zoom_image (cnvImage, get_aug_image_struct (), ZOOM_IN);
	ui_show_status_bar_info ();
}

void on_btnZoomOut_clicked (GtkButton *button, gpointer data)
{
	/* Zoom out from the selected canvas area of the autoguider image canvas */
	
	canvas_zoom_image (cnvImage, get_aug_image_struct (), ZOOM_OUT);
	ui_show_status_bar_info ();
}

void on_btnZoom1to1_clicked (GtkButton *button, gpointer data)
{
    /* Set the zoom scale to 1:1 for the autoguider image canvas*/
    
    canvas_zoom_image (cnvImage, get_aug_image_struct (), ZOOM_1x1);
	ui_show_status_bar_info ();
}

void on_btnResetArea_clicked (GtkButton *button, gpointer data)
{
	/* Reset the selected area rectangle to full frame */
	
	ui_set_augcanv_rect_full_area ();
}

void on_txtImgSatLevel_activate (GtkEntry *entry, gpointer data)
{
	/* Get the saturation level for the autoguider image display */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	gint level;
		
	get_entry_int ("txtImgSatLevel", aug->imdisp.B, aug->imdisp.W, 
				   aug->imdisp.W, NO_PAGE, &level);
	aug->imdisp.satlevel = (gushort) level;
}

void on_txtImgExpLength_activate (GtkEntry *entry, gpointer data)
{
	/* Set the exposure length for autoguider camera exposures (where this is
	 * user-adjustable), when the user presses the Enter key in the exposure
	 * length field in the Image window.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	get_entry_float ("txtImgExpLength",0.001,99.9,1, NO_PAGE,&aug->exd.req_len);
}

void on_optImgBin_toggled (GtkButton *button, gpointer data)
{
	/* Set the binning for the autoguider camera */
	
	const gchar *name;
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
		name = gtk_buildable_get_name (GTK_BUILDABLE (button));
		if (!strcmp (name, "optImgBin1x1")) {
			augcam_set_camera_binning (1, 1);
			canvas_zoom_image (cnvImage, get_aug_image_struct (), ZOOM_1x1);
			ui_show_status_bar_info ();
		} else if (!strcmp (name, "optImgBin2x2")) {
			augcam_set_camera_binning (2, 2);
			canvas_zoom_image (cnvImage, get_aug_image_struct (), ZOOM_2x2);
			ui_show_status_bar_info ();
		} else
			L_print ("{r}%s: Invalid button name: %s\n", __func__, name);		
	}
}

void on_hscBackground_changed (GtkRange *range, gpointer data)
{
	/* Adjust background level for autoguider image data */
	
	augcam_set_vidval (AIW_BACKGROUND, gtk_range_get_value (range));
}

void on_hscBrightness_changed (GtkRange *range, gpointer data)
{
	/* Set brightness for autoguider */
	
	augcam_set_vidval (AIW_BRIGHTNESS, gtk_range_get_value (range));
}

void on_hscContrast_changed (GtkRange *range, gpointer data)
{
	/* Set contrast for autoguider */
	
	augcam_set_vidval (AIW_CONTRAST, gtk_range_get_value (range));
}

void on_hscGamma_changed (GtkRange *range, gpointer data)
{
	/* Set gamma for autoguider */

	augcam_set_vidval (AIW_GAMMA, gtk_range_get_value (range));
}

void on_hscGain_changed (GtkRange *range, gpointer data)
{
	/* Set the gain for the autoguider */
	
	augcam_set_vidval (AIW_GAIN, gtk_range_get_value (range));
}

void on_btnFntSel_clicked (GtkButton *button, gpointer data)
{
	/* Set a new canvas font, if one has been selected */
	
	GtkWidget *dlgFontSelection;
	static gchar oldfont[MCSL];
	const gchar *ButtonName;
	
	dlgFontSelection = xml_get_widget (xml_font, "dlgFontSelection");

	ButtonName = gtk_buildable_get_name (GTK_BUILDABLE (button));	
	
	if (!g_ascii_strcasecmp (ButtonName, "btnFntSelOK")) {
		strcpy (font, gtk_font_selection_dialog_get_font_name 
		                        (GTK_FONT_SELECTION_DIALOG (dlgFontSelection)));
		if (strncmp (font, "", 1) == 0)
			strcpy (font, FONT);
		
		gtk_widget_destroy (dlgFontSelection);
		strcpy (oldfont, font);
	}
	
	if (!g_ascii_strcasecmp (ButtonName, "btnFntSelApply")) {
		strcpy (font, gtk_font_selection_dialog_get_font_name 
		                        (GTK_FONT_SELECTION_DIALOG (dlgFontSelection)));
		if (strncmp (font, "", 1) == 0)
			strcpy (font, FONT);
	}
	
	if (!g_ascii_strcasecmp (ButtonName, "btnFntSelCancel")) {
		if (strncmp (oldfont, "", 1) == 0)
			strcpy (oldfont, FONT);
		strcpy (font, oldfont);
		
		gtk_widget_destroy (dlgFontSelection);
	}
	
	ui_show_augcanv_plot_titles ();
	W_config_s ("Canvas/Font", font);
}

void on_chkAutogOpen_toggled (GtkButton *button, gpointer data)
{
	/* Open the autoguider camera if this option is toggled on, else close
	 * the autoguider camera if toggled off.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();

	gint period;
	gboolean Open;
	
	Open = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	
	if (!menu.LiveView) {
		if (Open)
			gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			 xml_app, "autoguider_camera_menu")), widget_set_insensitive, NULL);
		else
			gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			 xml_app, "autoguider_camera_menu")), widget_set_sensitive, NULL);
	
		#ifdef HAVE_UNICAP
		if (aug->device == UNICAP) {
			gtk_widget_set_sensitive (xml_get_widget (
								      xml_app, "unicap_device"), !Open);
			gtk_widget_set_sensitive (xml_get_widget (
								      xml_app, "unicap_properties"), Open);
		}
		#endif
		#ifdef HAVE_LIBV4L2
		if (aug->device == V4L) {
			gtk_widget_set_sensitive (xml_get_widget (
								      xml_app, "v4l_properties"), Open);
		}
		#endif
	}	
	
	#ifdef HAVE_SX_CAM
	if (aug->device == SX) {
		gtk_widget_set_sensitive (xml_get_widget (
							  xml_app, "select_sx_camera"), !Open);
		gtk_widget_set_sensitive (xml_get_widget (
							  xml_app, "configure_sx_camera"), Open);
	}
	#endif
	
    if (Open) {
		gtk_widget_set_sensitive (xml_get_widget (
								  xml_app, "gemini_commands"), FALSE);
	} else {
		if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (
			                  xml_get_widget (xml_app, "open_telescope_link"))))
			gtk_widget_set_sensitive (xml_get_widget (
                              xml_app, "gemini_commands"), TRUE);
    }
                              
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	if (Open) {
		
	    /* Warn the user if the guide speed is set to a different value from the
		 * one associated with the most recently uploaded PEC data.
		 */
		
		warn_PEC_guidespeed (TRUE);
	
		/* Get Unicap device/format if selected */
	
		#ifdef HAVE_UNICAP
		if (aug->device == UNICAP) {
			unicap_void_device (&aug->ucp_device);
			strcpy (aug->ucp_device.identifier, "NONE");
			R_config_s ("Unicap/Device/ID", aug->ucp_device.identifier);
			unicap_void_format (&aug->ucp_format_spec);
			aug->ucp_format_spec.fourcc = R_config_d (
											   "Unicap/Format/Fourcc", 0);
			aug->ucp_format_spec.size.width = R_config_d (
											   "Unicap/Format/Size/Width", 0);
			aug->ucp_format_spec.size.height = R_config_d(
											   "Unicap/Format/Size/Height", 0);
		}
		#endif
		
		/* Schedule the autoguider image to be saved periodically,
		 * if requested.
		 */

		get_entry_int ("txtSaveEvery", 1, 3600, 60, FIL_PAGE, &period);
		aug->SavePeriodic = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON ( 
							         xml_get_widget (xml_app, "chkSaveEvery")));
		loop_save_periodic (aug->SavePeriodic, (guint) period);
	}
    
	/* Open/close the autoguider camera */
	
	loop_autog_open (Open);
}

void on_chkAutogWrite_toggled (GtkButton *button, gpointer data)
{
	/* Set the flag to indicate that the star position and guide correction
	 * data should be written to a file.
	 */

	struct cam_img *aug = get_aug_image_struct ();
		
	GtkWidget *widget;
	
	aug->autog.Write = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (button)); 
	if (aug->autog.Write)
		augcam_write_starpos_time ();
	
	/* Sensitise/de-sensitise 'Write worm pos.' checkbox if telescope link is
	 * open, and turn off 'Write worm pos.' if it's on and 'Write star pos.' is
	 * being turned off.
	 */
	
	widget = xml_get_widget (xml_app, "chkAutogWorm");
	gtk_widget_set_sensitive (widget, tel_comms->user & PU_TEL && 
							                    aug->autog.Write && GeminiCmds);
	if (!aug->autog.Write && aug->autog.Worm)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
}

void on_chkAutogWorm_toggled (GtkButton *button, gpointer data)
{
	/* Set flag to write RA worm position data in addition to star position
	 * and guide correction data when the autoguider camera is open.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	aug->autog.Worm = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (button)); 
}

void on_spbAutogGuideSpeed_changed (GtkSpinButton *spin, gpointer data)
{
	/* Warn the user if the guide speed is set to a different value from the
	 * one associated with the most recently uploaded PEC data.
	 */
	
	warn_PEC_guidespeed (FALSE);
}

void on_btnAutogDS9_clicked (GtkButton *button, gpointer data)
{
	/* Save a temporary autoguider image and display it in DS9 */

	loop_autog_DS9 ();
}

void on_btnAutogCapDark_clicked (GtkButton *button, gpointer data)
{
	/* Set the flag to capture a dark exposure and toggle dark frame
	 * subtraction on.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gint val;
	
	augcam_flush_dark_frame ();
	get_entry_int ("txtAutogDarkExps", 1, 99, 1, AUG_PAGE, &val);	
	aug->dark.num = (gushort) val;
	aug->dark.Capture = TRUE;
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
	                       (xml_get_widget (xml_app, "chkAutogSubDark")), TRUE);	
}

void on_chkAutogSubDark_toggled (GtkButton *button, gpointer data)
{
	/* Toggle the flag to indicate whether the dark exposure should be
	 * subtracted from all other exposures.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	aug->dark.Subtract = gtk_toggle_button_get_active 
		                                           (GTK_TOGGLE_BUTTON (button));
	if (aug->dark.Subtract)
		augcam_read_dark_frame ();
}

void on_txtAutogSkySigma_activate (GtkEntry *entry, gpointer data)
{
	/* Get the number of standard deviations above the sky background for
	 * the centroid calculation.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	gdouble sd;
	
	get_entry_float ("txtAutogSkySigma", 0.0, 5.0, 3.0, AUG_PAGE, &sd);
	aug->imdisp.stdev = (gfloat) sd;
}

void on_txtAutogCentroidSize_activate (GtkEntry *entry, gpointer data)
{
	/* Get the size of the centroid box when the user presses the enter key
	 * in this field.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gint csize;
	
	get_entry_int ("txtAutogCentroidSize", 3, 99, 17, AUG_PAGE, &csize);
	if (!(csize%2))
		csize++;
	
	aug->canv.csize = (gushort) csize;	
}

void on_chkAutogShowCentroid_toggled (GtkButton *button, gpointer data)
{
	/* Set the image centroid flag.  If set to TRUE, the centroid marker is
	 * shown for the region within the selection rectangle on the autoguider
	 * display.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	aug->canv.Centroid =gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)); 
}

void on_tglAutogStart_toggled (GtkButton *button, gpointer data)
{
	/* Toggle autoguiding on/off.  Is set to off initially in the Glade 
	 * gtkbuilder file.
	 */
	
	GtkWidget *widget;
	gboolean Guide;
	
	/* Grey-out/un-grey the autoguider 'Pause' button */
	
	Guide = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)); 
	widget = xml_get_widget (xml_app, "tglAutogPause");
	gtk_widget_set_sensitive (widget, Guide);
	
	/* Set the button text */
	
	if (Guide) {
		warn_PEC_guidespeed (TRUE);  /* Warn if guide speed doesn't match PEC */
		gtk_button_set_label (button, "Stop autoguiding");
	} else {
		gtk_button_set_label (button, "Start autoguiding");
	}
	
	/* The following code sets the flag to start/stop autoguider calibration if
	 * requested prior to starting/stopping autoguiding.  We can't just activate
	 * btnAutogCalibrate and have that routine call loop_autog_calibrate because
	 * there's no guarantee that it will be called before loop_autog_guide is
	 * called here.  So we have to call loop_autog_calibrate explicitly, and
	 * then activate the button to set the state of the GUI correctly.  This is
	 * OK to do because having the btnAutogCalibrate routine call 
	 * loop_autog_calibrate again has no additional effect.
	 */
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (xml_get_widget(xml_app, 
		                                           "chkAutogAutoCalibrate")))) {
		loop_autog_calibrate (Guide);
		widget = xml_get_widget (xml_app, "btnAutogCalibrate");
		if (Guide || (!Guide && !strcmp (gtk_button_get_label (
								     GTK_BUTTON (widget)), "Stop calibration")))
			gtk_widget_activate (xml_get_widget (xml_app, "btnAutogCalibrate"));
	}
	
	/* Now set the flag to start/stop guiding */

	loop_autog_guide (Guide);
}

void on_tglAutogPause_toggled (GtkButton *button, gpointer data)
{
	/* Pause autoguiding */
	
	GtkWidget *widget;
	gboolean Pause;
	
	if (tasks_get_status () & TSK_ACTIVE) {
		L_print ("{r}Error - Can't pause autoguiding while the task list is "
				                                                  "running!\n");
		return;
	}		
	
	Pause = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	loop_autog_pause (Pause);
	
	if (Pause)
		gtk_button_set_label (button, "Continue autoguiding");
	else
		gtk_button_set_label (button, "Pause autoguiding");		

	widget = xml_get_widget (xml_app, "tglAutogStart");
	gtk_widget_set_sensitive (widget, !Pause);
}

void on_btnAutog_pressed (GtkButton *button, gpointer data)
{
	/* Move the telescope in the desired direction */
	
	gfloat speed;
	const gchar *ButtonName;
	gboolean CenterSpeed;
	
	ButtonName = gtk_buildable_get_name (GTK_BUILDABLE (button));	
	get_autog_movespeed (&CenterSpeed, &speed);
	
	if (CenterSpeed && !(tel_comms->user & PU_TEL)) {
		L_print ("{r}Can't move telescope at centering speed - telescope serial"
				                                         " link is not open\n");
		return;
	}
	
	if (CenterSpeed) {/* Move at centering speed via telescope port because */
		telescope_set_center_speed (speed); /* autoguider link might be to  */
		if (!g_ascii_strcasecmp (ButtonName, "btnAutogEast")) /* relay box  */
		   telescope_move_start (TM_EAST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogWest"))
		   telescope_move_start (TM_WEST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogNorth"))
		   telescope_move_start (TM_NORTH);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogSouth"))
		   telescope_move_start (TM_SOUTH);
		else
			G_print ("{r}on_btnAutog_pressed: Not a valid button name: %s\n", 
																    ButtonName);
	} else {          /* Move at guide speed via guide signals port */
		telescope_set_guide_speed (speed);
		if (!g_ascii_strcasecmp (ButtonName, "btnAutogEast"))
		   telescope_guide_start (TM_EAST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogWest"))
		   telescope_guide_start (TM_WEST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogNorth"))
		   telescope_guide_start (TM_NORTH);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogSouth"))
		   telescope_guide_start (TM_SOUTH);
		else
			G_print ("{r}on_btnAutog_pressed: Not a valid button name: %s\n", 
																    ButtonName);
	}
}

void on_btnAutog_released (GtkButton *button, gpointer data)
{
	/* Stop telescope movement */
	
	gfloat speed;
	const gchar *ButtonName;
	gboolean CenterSpeed;
	
	ButtonName = gtk_buildable_get_name (GTK_BUILDABLE (button));	
	get_autog_movespeed (&CenterSpeed, &speed);
	
	if (CenterSpeed) {  /* Moving at centering speed via telescope port */
		if (!g_ascii_strcasecmp (ButtonName, "btnAutogEast"))
		   telescope_move_stop (TM_EAST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogWest"))
		   telescope_move_stop (TM_WEST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogNorth"))
		   telescope_move_stop (TM_NORTH);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogSouth"))
		   telescope_move_stop (TM_SOUTH);
		else
			G_print ("{r}on_btnAutog_pressed: Not a valid button name: %s\n", 
																    ButtonName);
	} else {          /* Moving at guide speed via guide signals port */
		if (!g_ascii_strcasecmp (ButtonName, "btnAutogEast"))
		   telescope_guide_stop (TM_EAST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogWest"))
		   telescope_guide_stop (TM_WEST);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogNorth"))
		   telescope_guide_stop (TM_NORTH);
		else if (!g_ascii_strcasecmp (ButtonName, "btnAutogSouth"))
		   telescope_guide_stop (TM_SOUTH);
		else
			G_print ("{r}on_btnAutog_pressed: Not a valid button name: %s\n", 
																    ButtonName);
	}
	
	if (tel_comms->user & PU_TEL && GeminiCmds)
		telescope_restore_motion_rates ();
}

void on_btnAutogCalibrate_clicked (GtkButton *button, gpointer data)
{
	/* Start/stop autoguider calibration */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	static gboolean Calibrate = FALSE;

	Calibrate = !Calibrate;
	loop_autog_calibrate (Calibrate);
	if (Calibrate) {
		get_autog_guide_params ();
		if (aug->autog.s.RemoteTiming && autog_comms->pnum >= TTY0) {
			L_print ("{r}WARNING: The 'remote timing' guide command permits "
					 "a maximum duration of 9.999 seconds\n");
			L_print ("{r}Some hardware may allow a maximum duration even less "
					 "than this\n");
		}
		L_print ("{b}Starting autoguider calibration...\n");
		gtk_button_set_label (button, "Stop calibration");
	} else {
		L_print ("{b}Autoguider calibration stopped.\n");
		gtk_button_set_label (button, "Calibrate autoguider");
	}
}

void on_btnSaveSetting_clicked (GtkButton *button, gpointer data)
{
	/* Add the current settings to the list of settings data */

	struct cam_img *aug = get_aug_image_struct ();
	
	GtkTreeIter iter;
	GtkTreePath *path;
	
	/* Get the settings data */
	
	aug->autog.s.Telescope = get_entry_string ("txtSetTelescope");
	aug->autog.s.Instrument = get_entry_string ("txtSetInstrument");		
	get_autog_guide_params ();
	
	/* Add a row containing this data */
	
	gtk_list_store_append (lisSetting, &iter);  /* Acquire an iterator */	
	gtk_list_store_set (lisSetting, &iter,
						TELESCOPE, aug->autog.s.Telescope,
						INSTRUMENT, aug->autog.s.Instrument,
						NSCALIBDURATION, aug->autog.s.NSCalibDuration,
						EWCALIBDURATION, aug->autog.s.EWCalibDuration,
						GUIDESPEED, aug->autog.s.GuideSpeed,
						MAXSHIFT, aug->autog.s.MaxShift,
						MAXDRIFT, aug->autog.s.MaxDrift,
						MAXMOVE, aug->autog.s.MaxMove,
						MAXOFFSET, aug->autog.s.MaxOffset,
						DRIFTSAMPLE, aug->autog.s.DriftSample,
						CORRFAC, aug->autog.s.CorrFac,
						UPDATE, aug->autog.s.Update,
						-1);
						
	g_free (aug->autog.s.Telescope);
	g_free (aug->autog.s.Instrument);
	
	/* Select the created row */
	
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (lisSetting), &iter);
	gtk_tree_view_set_cursor (trvSetting, path, NULL, FALSE);
	
	/* Scroll selected row to bottom of window */

	gtk_tree_view_scroll_to_cell (trvSetting, path, NULL, FALSE, 0.0, 0.0);
	gtk_tree_path_free (path);
}

 void on_btnLoadSetting_clicked (GtkButton *button, gpointer data)
{
	/* Set the settings data associated with the selected row as the
	 * active values.
	 */
	
	struct autog_config *s;	
	GtkTreePath *path;
	GtkTreeIter iter;
	
	/* Get the selected row in the tree view */
	
	gtk_tree_view_get_cursor (trvSetting, &path, NULL);
	
	if (path != NULL)
		if (gtk_tree_model_get_iter (GTK_TREE_MODEL (lisSetting), &iter, path)){
			s = (struct autog_config *) g_malloc0 (sizeof(struct autog_config));	
			s->Telescope = (gchar *) g_malloc0 (80 * sizeof (gchar));
			s->Instrument = (gchar *) g_malloc0 (80 * sizeof (gchar));
			
			/* Get the data */
			
			gtk_tree_model_get (GTK_TREE_MODEL (lisSetting), &iter,
								TELESCOPE, &s->Telescope,
								INSTRUMENT, &s->Instrument,
								NSCALIBDURATION, &s->NSCalibDuration,
								EWCALIBDURATION, &s->EWCalibDuration,
								GUIDESPEED, &s->GuideSpeed,
								MAXSHIFT, &s->MaxShift,
								MAXDRIFT, &s->MaxDrift,
								MAXMOVE, &s->MaxMove,
								MAXOFFSET, &s->MaxOffset,
								DRIFTSAMPLE, &s->DriftSample,
								CORRFAC, &s->CorrFac,
								UPDATE, &s->Update,
								-1);
			
			set_entry_float ("txtAutogNSCalibDuration", s->NSCalibDuration);
			set_entry_float ("txtAutogEWCalibDuration", s->EWCalibDuration);
			set_spin_float ("spbAutogGuideSpeed", s->GuideSpeed);
			set_spin_float ("spbAutogMaxShift", s->MaxShift);
			set_spin_float ("spbAutogMaxDrift", s->MaxDrift);
			set_spin_float ("spbAutogMaxMove", s->MaxMove);
			set_spin_float ("spbAutogMaxOffset", s->MaxOffset);
			set_spin_int ("spbAutogDriftSample", s->DriftSample);			
			set_spin_float ("spbAutogCorrFac", s->CorrFac);
			set_entry_float ("txtAutogUpdate", s->Update);
			set_entry_string ("txtSetTelescope", s->Telescope);
			set_entry_string ("txtSetInstrument", s->Instrument);
			
			g_free (s->Telescope);
			g_free (s->Instrument);
			g_free (s);
		}
	gtk_tree_path_free (path);
}

void on_btnDeleteSetting_clicked (GtkButton *button, gpointer data)
{
	/* Delete the selected settings */
	
	GtkTreePath *path;
	GtkTreeIter iter;
	
	gtk_tree_view_get_cursor (trvSetting, &path, NULL);
	if (path != NULL)
		if (gtk_tree_model_get_iter (GTK_TREE_MODEL (lisSetting), &iter, path))
			gtk_list_store_remove (lisSetting, &iter);
	gtk_tree_path_free (path);
}

void on_btnCloseSetting_clicked (GtkButton *button, gpointer data)
{
	/* Save the settings data and close the window */
	
	struct autog_config *s;
	GtkTreeIter iter;
	gint i = 0;
	gchar *s1, *s2;
	gboolean valid;
	
	s = (struct autog_config *) g_malloc0 (sizeof (struct autog_config));
	s->Telescope = (gchar *) g_malloc0 (80 * sizeof (gchar));
	s->Instrument = (gchar *) g_malloc0 (80 * sizeof (gchar));
	
	valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (lisSetting), &iter);
	while (valid) {

		/* Walk through the list, reading each row */
		
		gtk_tree_model_get (GTK_TREE_MODEL (lisSetting), &iter,
							TELESCOPE, &s->Telescope,
							INSTRUMENT, &s->Instrument,
							NSCALIBDURATION, &s->NSCalibDuration,
							EWCALIBDURATION, &s->EWCalibDuration,
							GUIDESPEED, &s->GuideSpeed,
							MAXSHIFT, &s->MaxShift,
							MAXDRIFT, &s->MaxDrift,
							MAXMOVE, &s->MaxMove,
							MAXOFFSET, &s->MaxOffset,
							DRIFTSAMPLE, &s->DriftSample,
							CORRFAC, &s->CorrFac,
							UPDATE, &s->Update,
							-1);

		/* Save the data */		
		
		s1 = g_strdup_printf ("Setting/%i", i++);
		
		s2 = g_strconcat (s1, "/Telescope", NULL);
		W_config_s (s2, s->Telescope);		
		g_free (s2);
		s2 = g_strconcat (s1, "/Instrument", NULL);
		W_config_s (s2, s->Instrument);		
		g_free (s2);
		s2 = g_strconcat (s1, "/NSCalibDuration", NULL);
		W_config_f (s2, s->NSCalibDuration);		
		g_free (s2);
		s2 = g_strconcat (s1, "/EWCalibDuration", NULL);
		W_config_f (s2, s->EWCalibDuration);		
		g_free (s2);
		s2 = g_strconcat (s1, "/GuideSpeed", NULL);
		W_config_f (s2, s->GuideSpeed);		
		g_free (s2);
		s2 = g_strconcat (s1, "/MaxShift", NULL);
		W_config_f (s2, s->MaxShift);		
		g_free (s2);
		s2 = g_strconcat (s1, "/MaxDrift", NULL);
		W_config_f (s2, s->MaxDrift);		
		g_free (s2);
		s2 = g_strconcat (s1, "/MaxMove", NULL);
		W_config_f (s2, s->MaxMove);		
		g_free (s2);
		s2 = g_strconcat (s1, "/MaxOffset", NULL);
		W_config_f (s2, s->MaxOffset);		
		g_free (s2);
		s2 = g_strconcat (s1, "/DriftSample", NULL);
		W_config_f (s2, s->DriftSample);		
		g_free (s2);
		s2 = g_strconcat (s1, "/CorrFac", NULL);
		W_config_f (s2, s->CorrFac);		
		g_free (s2);
		s2 = g_strconcat (s1, "/Update", NULL);
		W_config_f (s2, s->Update);		
		g_free (s2);		

		g_free (s1);	

		valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (lisSetting), &iter);
    }
	
	g_free (s->Telescope);
	g_free (s->Instrument);
	g_free (s);
	g_object_unref (lisSetting);
	
	/* Close the settings window */
	
	gtk_widget_destroy (xml_get_widget (xml_set, "wndSetting"));
	g_object_unref (G_OBJECT (xml_set));
	xml_set = NULL;
}

void on_btnFocusIn_clicked (GtkButton *button, gpointer data)
{
	/* Move focuser in */
	
	struct focus f;
		
	if (loop_focus_is_focusing ()) {
		L_print ("{o}Command ignored - focuser is already moving!\n");
		return;
	}
	
	f.cmd = FC_CUR_POS_GET;
	focus_comms->focus (&f);
	
	if (!strcmp (menu.focuser, "focuser_robofocus"))
		get_entry_int ("txtFocusInOut", 0, f.cur_pos - 1, 0, 
		                                                  FOC_PAGE, &f.move_by);
	/* else use appropriate range limits for some other focuser etc */

	L_print ("{b}Moving focuser in %d steps...\n", f.move_by);
	f.cmd = FC_MOVE_BY;
	f.move_by = -f.move_by; /* Moving in */
	focus_comms->focus (&f);
	loop_focus_check_done ();  /* Start checking to see if focusing is done */
}

void on_btnFocusOut_clicked (GtkButton *button, gpointer data)
{
	/* Move focuser out */
	
	struct focus f;
	
	if (loop_focus_is_focusing ()) {
		L_print ("{o}Command ignored - focuser is already moving!\n");
		return;
	}
	
	f.cmd = (FC_MAX_TRAVEL_GET | FC_CUR_POS_GET);
	focus_comms->focus (&f);
	
	if (!strcmp (menu.focuser, "focuser_robofocus"))
		get_entry_int ("txtFocusInOut", 0, f.max_travel - f.cur_pos, 0, 
					                                      FOC_PAGE, &f.move_by);
	/* else use appropriate range limits for some other focuser etc */

	L_print ("{b}Moving focuser out %d steps...\n", f.move_by);
	f.cmd = FC_MOVE_BY;
	focus_comms->focus (&f);
	loop_focus_check_done ();  /* Start checking to see if focusing is done */
}

void on_btnFocusMoveTo_clicked (GtkButton *button, gpointer data)
{
	/* Move focuser to given position */
	
	struct focus f;
	
	if (loop_focus_is_focusing ()) {
		L_print ("{o}Command ignored - focuser is already moving!\n");
		return;
	}
	
	f.cmd = FC_MAX_TRAVEL_GET;
	focus_comms->focus (&f);
	
	if (!strcmp (menu.focuser, "focuser_robofocus"))
		get_entry_int ("txtFocusMoveTo", 1, f.max_travel, f.max_travel / 2, 
					                                      FOC_PAGE, &f.move_to);
	/* else use appropriate range limits for some other focuser etc */

	L_print ("{b}Moving focuser to position %d...\n", f.move_to);
	f.cmd = FC_MOVE_TO;
	focus_comms->focus (&f);
	loop_focus_check_done ();  /* Start checking to see if focusing is done */
}

void on_btnFocusStop_clicked (GtkButton *button, gpointer data)
{
	/* Stop focuser motion */
	
	loop_focus_stop ();  /* This has to go via the focuser motion checking */
	                     /*  thread in loop.c                              */
}

void on_btnFocusConfig_clicked (GtkButton *button, gpointer data)
{
	/* Open the focus configuration window */
	
	if (xml_fcw != NULL) {  /* Present window if it's already open */
		gtk_window_present (GTK_WINDOW (xml_get_widget 
		                                      (xml_fcw, "wndConfigAutofocus")));
		return;
	}
	
	gchar *objects[] = {"wndConfigAutofocus", NULL};
	xml_fcw = xml_load_new (xml_fcw, GLADE_INTERFACE, objects);
}

void on_btnFocusFocus_clicked (GtkButton *button, gpointer data)
{
	/* Initiate autofocus */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	struct focus f;	
	gint start_pos;
	gdouble LHSlope, RHSlope, PID, near_HFD, exp_len;
	gboolean Inside;
	
	/* Check CCD camera is open */
	
	if (!ccd->Open) {
		L_print ("{o}Can't autofocus - CCD camera isn't open\n");
		return;
	}
	
	/* Get settings from Focus tab */
	
	f.cmd = FC_CUR_POS_GET | FC_MAX_TRAVEL_GET;
	focus_comms->focus (&f);
	get_entry_float ("txtFocusLHSlope", -9999.0, 0.0, -0.1, NO_PAGE, &LHSlope);	
	get_entry_float ("txtFocusRHSlope", 0.0, 9999.0, 0.1, NO_PAGE, &RHSlope);	
	get_entry_float ("txtFocusPID", -65535.0, 65535.0, 0.1, NO_PAGE, &PID);	
	get_entry_int ("txtFocusAFStart", 0, f.max_travel , f.cur_pos, 
				                                           NO_PAGE, &start_pos);
	Inside = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
						         xml_get_widget (xml_app, "optFocusAFInside")));
	get_entry_float ("txtFocusNearHFD", 0.1, 9999.0, 10.0, NO_PAGE, &near_HFD);
	get_entry_float ("txtFocusExpLength", 0.01, 10.0, 1.0, NO_PAGE, &exp_len);
	
	/* Start autofocus */
		
	L_print ("{b}Beginning autofocus...\n");
	loop_ccd_autofocus (TRUE, LHSlope, RHSlope, PID, start_pos, 
						Inside, near_HFD, exp_len, 
						R_config_d ("Focus/Config/AFBox", 40)); 
}

void on_btnFocusAFStop_clicked (GtkButton *button, gpointer data)
{
	/* Stop autofocusing */
	
	loop_ccd_autofocus (FALSE, 0.0, 0.0, 0.0, 0, TRUE, 0.0, 0.0, 0);
}

void on_btnFocusMaxTravelSet_clicked (GtkButton *button, gpointer data)
{
	/* Set the focuser maximum travel */
	
	struct focus f;
	
	if (!strcmp (menu.focuser, "focuser_robofocus"))
		get_entry_int ("txtFocusMaxTravel", 1, 65535, 32000, 
					                                   FOC_PAGE, &f.max_travel);
	/* else use appropriate range limits for some other focuser etc */

	f.cmd = FC_MAX_TRAVEL_SET;
	focus_comms->focus (&f);
	if (!f.Error)  /* Display set value as confirmed by Robofocus */
		set_entry_int ("txtFocusMaxTravel", f.max_travel);
	else
		L_print ("{r}Error setting maximum travel for focuser\n");
}

void on_btnFocusCurrentPosSet_clicked (GtkButton *button, gpointer data)
{
	/* Set (re-define) the current focuser position */
	
	struct focus f;
	
	f.cmd = FC_MAX_TRAVEL_GET;
	focus_comms->focus (&f);
	
	if (!strcmp (menu.focuser, "focuser_robofocus"))
		get_entry_int ("txtFocusCurrentPos", 1, f.max_travel, f.max_travel / 2, 
					                                      FOC_PAGE, &f.cur_pos);
	/* else use appropriate range limits for some other focuser etc */

	f.cmd = FC_CUR_POS_SET;
	focus_comms->focus (&f);
	if (!f.Error)  /* Display set value as confirmed by Robofocus */
		set_entry_int ("txtFocusCurrentPos", f.cur_pos);
	else
		L_print ("{r}Error setting current position for focuser\n");
}

void on_btnFocusBacklashSet_clicked (GtkButton *button, gpointer data)
{
	/* Set the current backlash settings */
	
	struct focus f;
		
	f.cmd = FC_BACKLASH_SET;
	if (!strcmp (menu.focuser, "focuser_robofocus")) {
		get_entry_int ("txtFocusBacklashSteps", 0, 32767, 20, 
					                               FOC_PAGE, &f.backlash_steps);
		f.BacklashIn = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
						       xml_get_widget (xml_app, "optFocusBacklashIn")));
	}
	/* else use appropriate range limits for some other focuser etc */
	
	focus_comms->focus (&f);
	if (!f.Error) {  /* Display set values as confirmed by Robofocus */
		set_entry_int ("txtFocusBacklashSteps", f.backlash_steps);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (xml_get_widget (
					           xml_app, "optFocusBacklashIn")), f.BacklashIn);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (xml_get_widget (
					           xml_app, "optFocusBacklashOut")), !f.BacklashIn);
	} else
		L_print ("{r}Error setting backlash for focuser\n");
}

void on_btnFocusMotorConfigSet_clicked (GtkButton *button, gpointer data)
{
	/* Set the focus motor configuration settings */
	
	struct focus f;
		
	if (!strcmp (menu.focuser, "focuser_robofocus")) {
		f.cmd = FC_VERSION;
		focus_comms->focus (&f);
		if (f.version >= 3.0) {
			f.cmd = FC_MOTOR_SET;
			get_entry_int ("txtFocusStepSize",1,255,10, FOC_PAGE, &f.step_size);
			get_entry_int ("txtFocusStepPause",1,20,4, FOC_PAGE, &f.step_pause);
			get_entry_int ("txtFocusDutyCycle",0,100,0,FOC_PAGE, &f.duty_cycle);
			focus_comms->focus (&f);
			if (!f.Error) {  /* Display set values as confirmed by Robofocus */
				set_entry_int ("txtFocusStepSize", f.step_size);
				set_entry_int ("txtFocusStepPause", f.step_pause);
				set_entry_int ("txtFocusDutyCycle", f.duty_cycle);
			} else
				L_print ("{r}Error setting motor configuration for focuser\n");
		} else
			L_print ("{o}Can set motor configuration only for Robofocus "
					 "version 3\n");
	} else {
		/* else use appropriate range limits for some other focuser etc */
	}
}

void on_btnFocusGetCurrentSettings_clicked (GtkButton *button, gpointer data)
{
	/* Get the current focuser configuration settings */
	
	struct focus f;
	
	if (!strcmp (menu.focuser, "focuser_robofocus")) {
		f.cmd = FC_VERSION;
		focus_comms->focus (&f);
		if (f.version >= 3.0)
			f.cmd = FC_MAX_TRAVEL_GET | FC_CUR_POS_GET | FC_BACKLASH_GET |  
				    FC_MOTOR_GET;
		else	
			f.cmd = FC_MAX_TRAVEL_GET | FC_CUR_POS_GET | FC_BACKLASH_GET; 
		focus_comms->focus (&f);
		if (!f.Error) {
			set_entry_int ("txtFocusMaxTravel", f.max_travel);
			set_entry_int ("txtFocusCurrentPos", f.cur_pos);
			set_entry_int ("txtFocusBacklashSteps", f.backlash_steps);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
										  xml_get_widget ( xml_app, 
										 "optFocusBacklashIn")), f.BacklashIn);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
										  xml_get_widget ( xml_app,
										 "optFocusBacklashOut")), !f.BacklashIn);
			if (f.version >= 3.0) {
				set_entry_int ("txtFocusStepSize", f.step_size);
				set_entry_int ("txtFocusStepPause", f.step_pause);
				set_entry_int ("txtFocusDutyCycle", f.duty_cycle);
			} else
			    L_print ("{o}Can read motor configuration "
					                          "only for Robofocus version 3\n");
		} else
		    L_print ("{r}Error reading focuser configuration\n");
	} else {
			/* some other focuser */
	}
}

void on_chkFocusApplyFilterOffsets_toggled (GtkButton *button, gpointer data)
{
	/* Apply filter focus offsets when changing filter? */
	
	W_config_d ("Focus/ApplyFilterOffsets",
				gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)));
}

void on_chkFocusTempComp_toggled (GtkButton *button, gpointer data)
{
	/* Apply temperature compensation?  Don't store this setting - user must 
	 * select it when required.
	 */
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
		focus_store_temp_and_pos ();
}

void on_chkFocusFastReadout_toggled (GtkButton *button, gpointer data)
{
	/* Set/unset flag to use fast readout speed when focusing.  This is assumed
	 * to apply just for CCD camera exposures, not autoguider camera exposures.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	ccd->FastFocus = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	W_config_d ("Focus/FastReadout", ccd->FastFocus);
}

void on_btnAFConfigMeasureHFD_clicked (GtkButton *button, gpointer data)
{
	/* Start gathering HFD data for the autofocus calibration V-curve */
	
	#ifdef HAVE_LIBGRACE_NP
	struct cam_img *ccd = get_ccd_image_struct ();
	
	struct focus f;
	gint f_start, f_end, f_step, f_c, f_n, f_repeat;
	gint tick;
	gdouble exp_len;
	gboolean AlreadyOpen = FALSE;
	
	/* Check CCD camera is open and an initial exposure has been made */
	
	if (!ccd->Open) {
		L_print ("{o}Can't calibrate autofocus - CCD camera isn't open\n");
		return;
	}
	
	/* Get settings from focus configuration window */
	
	f.cmd = FC_CUR_POS_GET | FC_MAX_TRAVEL_GET;
	focus_comms->focus (&f);
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
				            xml_get_widget (xml_fcw, "optAFConfigSetRange")))) {
		get_entry_int ("txtAFConfigStartPos", 0, f.max_travel, 0, 
					                                         NO_PAGE, &f_start);
		get_entry_int ("txtAFConfigEndPos", 0, f.max_travel, f.max_travel,
					                                           NO_PAGE, &f_end);
		get_entry_int ("txtAFConfigStepSize", 0, f.max_travel, 10, 
					                                          NO_PAGE, &f_step);
	} else {
		get_entry_int ("txtAFConfigCentreFocus", 0, f.max_travel, 
					                           f.max_travel / 2, NO_PAGE, &f_c);
		get_entry_int ("txtAFConfigNumSteps", 1, f.max_travel,10,NO_PAGE, &f_n);
		get_entry_int ("txtAFConfigStepSize", 1, f.max_travel, 10, 
					                                          NO_PAGE, &f_step);
		f_start = f_c - f_n * f_step;
		f_end = f_c + f_n * f_step;
	}	
	get_entry_int ("txtAFConfigRepeat", 1, 999, 1, NO_PAGE, &f_repeat);
	get_entry_float("txtAFConfigExpLength", 0.01, 10.0, 1.0, NO_PAGE, &exp_len);
	
	/* Open Grace plot */
	
	if (!Grace_Open (GRACE_HFD, &AlreadyOpen)) {
		L_print ("{r}Error opening Grace HFD plot\n");
		return;
	}
	if (f_end > f_start)
		Grace_SetXAxis (1, (gfloat) (f_start - (f_end - f_start) / 10),
						   (gfloat) (f_end + (f_end - f_start) / 10));
	else
		Grace_SetXAxis (1, (gfloat) (f_end - (f_start - f_end) / 10),
						   (gfloat) (f_start + (f_start - f_end) / 10));
	tick = ABS (f_start - f_end) / 10;
	if (tick%2) tick += 1;
	if (!tick) tick = 1;
	Grace_XAxisMajorTick (1, tick);
	Grace_Update ();
	
	/* Start autofocus configuration */
		
	L_print ("{b}Beginning autofocus calibration...\n");
	loop_ccd_calibrate_autofocus (TRUE, f_start, f_end, f_step, 
								  f_repeat, exp_len, 
								  R_config_d ("Focus/Config/AFBox", 40));
	#else
	L_print ("{r}Can't find Grace!  You must install Grace if you want "
			 "to calibrate autofocus\n"); 
	#endif
}

void on_btnAFConfigClearData_clicked (GtkButton *button, gpointer data)
{
	/* Clear out existing HFD plots */
	
	#ifdef HAVE_LIBGRACE_NP
	Grace_ErasePlot (0, 0);
	Grace_ErasePlot (1, 0);
	Grace_ErasePlot (1, 1);
	Grace_ErasePlot (1, 2);
	Grace_ErasePlot (1, 3);
	Grace_ErasePlot (1, 4);
	Grace_ErasePlot (1, 5);
	Grace_Update ();
	#endif
}

void on_btnAFConfigStopHFD_clicked (GtkButton *button, gpointer data)
{
	/* Stop autofocus calibration */
	
	loop_ccd_calibrate_autofocus (FALSE, 0, 0, 0, 0, 0.0, 0);
}

void on_btnAFConfigCalculate_clicked (GtkButton *button, gpointer data)
{
	/* Calculate the HFD slope coefficients etc. */
	
	#ifdef HAVE_LIBGRACE_NP
	struct focus f;
	gsize length;
	gushort retry;
	guint n;
	gint f_start, f_end, f_centre;
	gint status;
	gfloat LH_m, LH_b, LH_i;
	gfloat RH_m, RH_b, RH_i;
	gdouble HFD_lower, HFD_upper;
	gchar *s, *graph, *results;
	gchar *buffer, **strings;
	gchar *p0, *p1, *p2;	
	
	/* Write the graph file */
	
	graph = g_strconcat (PrivatePath, "/grace_hfd.agr", NULL);
	g_remove (graph);
	Grace_SaveToFile (graph);
	
	/* Wait for Grace to write the graph file data.  If it's still not there
	 * after 5 seconds, the Perl script will return an error anyway.
	 */
	
	retry = 5;
	while (!g_file_test (graph, G_FILE_TEST_EXISTS) && retry-- > 0)
		sleep (1);
	
	/* Analyse the graph */
	
	get_entry_float ("txtAFConfigFitUpperHFD", 0.0, 500.0, 20.0, 
					 NO_PAGE, &HFD_upper);
	get_entry_float ("txtAFConfigFitLowerHFD", 0.0, 500.0, 20.0, 
					 NO_PAGE, &HFD_lower);
	
	results = g_strconcat (PrivatePath, "/grace_hfd_results.txt", NULL);
	s = g_strdup_printf ("%s --graph_file %s "
						 "--HFD_lower %f "
						 "--HFD_upper %f "
						 "--results_file %s ",
						 GOQAT_HFD_PL,
						 graph,
						 HFD_lower,
						 HFD_upper,
						 results);
	g_spawn_command_line_sync (s, NULL, NULL, &status, NULL);
	g_free (s);
	g_free (graph);
	
	/* Get the contents of the Perl results file */
	
	if (g_file_get_contents (results, &buffer, &length, NULL))
		g_free (results);
	else {
		L_print ("{r}%s: Error accessing HFD results file:\n", __func__); 
		return;
	}
	if (!length) {
		L_print ("{r}%s: HFD results file is empty!\n", __func__);
		return;
	}
	
	strings = g_strsplit (buffer, "\n", -1); /* Get array of lines from file */
	g_free (buffer);
	
	if (!strncmp (strings[0], "0", 1)) { /* Error code is zero, so get results*/
	
		n = (guint) strtod (strings[1], &p0);
		LH_m = strtod (++p0, &p1);
		LH_b = strtod (++p1, &p2);
		LH_i = strtod (++p2, (gchar **) NULL);
		
		set_entry_float ("txtAFConfigLHSlope", LH_m);
		set_entry_float ("txtAFConfigLHPosIntercept", LH_i);
		
		n = (guint) strtod (strings[2], &p0);
		RH_m = strtod (++p0, &p1);
		RH_b = strtod (++p1, &p2);
		RH_i = strtod (++p2, (gchar **) NULL);
		
		set_entry_float ("txtAFConfigRHSlope", RH_m);
		set_entry_float ("txtAFConfigRHPosIntercept", RH_i);
	
		set_entry_float ("txtAFConfigSlopeRatio", LH_m / RH_m);
	    set_entry_float ("txtAFConfigPID", LH_i - RH_i);
	
		set_entry_int ("txtAFConfigCentreFocus", (gint) 
					                    ((gfloat) (0.5 + (LH_i + RH_i) / 2.0)));
	
	    /* Get centre point of range/fit, either from start and end points or
	     * centre value.
	     */
	
	    f.cmd = FC_CUR_POS_GET | FC_MAX_TRAVEL_GET;
	    focus_comms->focus (&f);
	    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
					        xml_get_widget (xml_fcw, "optAFConfigSetRange")))) {
			get_entry_int ("txtAFConfigStartPos", 0, f.max_travel, 0, 
						                                     NO_PAGE, &f_start);
			get_entry_int("txtAFConfigEndPos", 0, f.max_travel, f.max_travel,
						                                        NO_PAGE,&f_end);
			f_centre = (f_start + f_end) / 2;
		} else
		    get_entry_int ("txtAFConfigCentreFocus", 0, f.max_travel, 
						                  f.max_travel / 2, NO_PAGE, &f_centre);
		
		L_print ("{.}\n");
		L_print ("HFD range: %f to %f\n", HFD_lower, HFD_upper);
		L_print ("LH slope: %f, position intercept: %f\n", LH_m, LH_i);
		L_print ("RH slope: %f, position intercept: %f\n", RH_m, RH_i);
		L_print ("Slope ratio: %f, intercept difference: %f, best focus: %d\n",
				 LH_m / RH_m, LH_i - RH_i, (gint) 
					                    ((gfloat) (0.5 + (LH_i + RH_i) / 2.0)));
		
		/* Plot fitted lines */
		
		Grace_ErasePlot (1, 1);
		Grace_ErasePlot (1, 2);
		Grace_ErasePlot (1, 3);
		Grace_ErasePlot (1, 4);
		Grace_ErasePlot (1, 5);
		
	    Grace_PlotPoints (1, 1, f_centre, 0);
	    Grace_PlotPoints (1, 1, f_centre, 65535);
	
	    Grace_PlotPoints (1, 2, 0, HFD_upper);
	    Grace_PlotPoints (1, 2, 65535, HFD_upper);
	    Grace_PlotPoints (1, 3, 0, HFD_lower);
	    Grace_PlotPoints (1, 3, 65535, HFD_lower);
	
	    Grace_PlotPoints (1, 4, LH_i, 0);
	    Grace_PlotPoints (1, 4, (HFD_upper - LH_b) / LH_m, HFD_upper);
	    Grace_PlotPoints (1, 5, RH_i, 0);
	    Grace_PlotPoints (1, 5, (HFD_upper - RH_b) / RH_m, HFD_upper);
	
		Grace_Update ();
	
	} else if (!strcmp (strings[0], "-1")) /* Error conditions... */
		L_print ("{o}HFD graph doesn't contain any points in given range!\n");
	else if (!strcmp (strings[0], "-2"))
		L_print ("{o}Fit range too small: left or right-hand side contains "
			     "fewer than 2 points!\n");
	else if (!strcmp (strings[0], "-3"))
		L_print ("{o}Fit range too large: increase lower limit to distinguish "
			     "left and right-hand sides!\n");
	else if (!strcmp (strings[0], "-999"))
		L_print ("{r}Can't find Grace graph data!\n");
	
	g_strfreev (strings);
	#else
	L_print ("{r}Can't find Grace!  You must install Grace if you want "
			 "to calibrate autofocus\n"); 
	#endif
}

void on_btnAFConfigUseResults_clicked (GtkButton *button, gpointer data)
{
	/* Transfer chosen results to Focus tab */
	
	gdouble val;
	
	get_entry_float ("txtAFConfigLHSlope", -9999.0, 0.0, -0.1, NO_PAGE, &val);	
	set_entry_float ("txtFocusLHSlope", val);
	get_entry_float ("txtAFConfigRHSlope", 0.0, 9999.0, 0.1, NO_PAGE, &val);	
	set_entry_float ("txtFocusRHSlope", val);
	get_entry_float ("txtAFConfigPID", -65535.0, 65535.0, 0.1, NO_PAGE, &val);	
	set_entry_float ("txtFocusPID", val);
}

void on_btnGemModelLoad_clicked (GtkButton *button, gpointer data)
{
	/* Load the Gemini pointing model from the configuration file */
	
	set_entry_int ("txtGemA", R_config_d ("Gemini/A", 0));
	set_entry_int ("txtGemE", R_config_d ("Gemini/E", 0));
	set_entry_int ("txtGemNP", R_config_d ("Gemini/NP", 0));
	set_entry_int ("txtGemNE", R_config_d ("Gemini/NE", 0));
	set_entry_int ("txtGemIH", R_config_d ("Gemini/IH", 0));
	set_entry_int ("txtGemID", R_config_d ("Gemini/ID", 0));
	set_entry_int ("txtGemFR", R_config_d ("Gemini/FR", 0));
	set_entry_int ("txtGemFD", R_config_d ("Gemini/FD", 0));
	set_entry_int ("txtGemCF", R_config_d ("Gemini/CF", 0));
	set_entry_int ("txtGemTF", R_config_d ("Gemini/TF", 0));
}

void on_btnGemModelSave_clicked (GtkButton *button, gpointer data)
{
	/* Save the Gemini pointing model to the configuration file */
	
	gint val;
	
	get_entry_int ("txtGemA", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/A", val);
	get_entry_int ("txtGemE", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/E", val);
	get_entry_int ("txtGemNP", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/NP", val);
	get_entry_int ("txtGemNE", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/NE", val);
	get_entry_int ("txtGemIH", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/IH", val);
	get_entry_int ("txtGemID", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/ID", val);
	get_entry_int ("txtGemFR", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/FR", val);
	get_entry_int ("txtGemFD", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/FD", val);
	get_entry_int ("txtGemCF", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/CF", val);
	get_entry_int ("txtGemTF", -65535, 65535, 0, TEL_PAGE, &val);
	W_config_d ("Gemini/TF", val);
}

void on_btnGemModelRead_clicked (GtkButton *button, gpointer data)
{
	/* Read the pointing model from the Gemini system */
	
	struct GemModel gm;
	gchar *s1;
	
	if (telescope_get_gemini_model (&gm)) {
	
		s1 = g_strdup_printf ("%i", gm.A);
		set_entry_string ("txtGemA", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.E);
		set_entry_string ("txtGemE", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.NP);
		set_entry_string ("txtGemNP", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.NE);
		set_entry_string ("txtGemNE", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.IH);
		set_entry_string ("txtGemIH", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.ID);
		set_entry_string ("txtGemID", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.FR);
		set_entry_string ("txtGemFR", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.FD);
		set_entry_string ("txtGemFD", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.CF);
		set_entry_string ("txtGemCF", s1);
		g_free (s1);
		s1 = g_strdup_printf ("%i", gm.TF);
		set_entry_string ("txtGemTF", s1);
		g_free (s1);
	} else
		L_print ("{r}Error - Unable to get Gemini pointing model parameters\n");
}

void on_btnGemModelWrite_clicked (GtkButton *button, gpointer data)
{
	/* Write the parameters to the Gemini system */
	
	GtkWidget *dlgMsg;
	struct GemModel gm;
	gint ret;
	const gchar *string ="You are about to overwrite the Gemini pointing model "
	                     "parameters.\nDo you really want to do this?";
	
	/* Display warning message box */
	
	dlgMsg = gtk_message_dialog_new (ccdApp,
									GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_WARNING,
									GTK_BUTTONS_YES_NO,
									string);
    ret = gtk_dialog_run (GTK_DIALOG (dlgMsg));
	gtk_widget_destroy (dlgMsg);
      
	/* Now write parameters */
	
	if (ret == GTK_RESPONSE_YES) {
		get_entry_int ("txtGemA", -65535, 65535, 0, TEL_PAGE, &gm.A);
		get_entry_int ("txtGemE", -65535, 65535, 0, TEL_PAGE, &gm.E);
		get_entry_int ("txtGemNP", -65535, 65535, 0, TEL_PAGE, &gm.NP);
		get_entry_int ("txtGemNE", -65535, 65535, 0, TEL_PAGE, &gm.NE);
		get_entry_int ("txtGemIH", -65535, 65535, 0, TEL_PAGE, &gm.IH);
		get_entry_int ("txtGemID", -65535, 65535, 0, TEL_PAGE, &gm.ID);
		get_entry_int ("txtGemFR", -65535, 65535, 0, TEL_PAGE, &gm.FR);
		get_entry_int ("txtGemFD", -65535, 65535, 0, TEL_PAGE, &gm.FD);
		get_entry_int ("txtGemCF", -65535, 65535, 0, TEL_PAGE, &gm.CF);
		get_entry_int ("txtGemTF", -65535, 65535, 0, TEL_PAGE, &gm.TF);
	
		if (telescope_set_gemini_model (&gm))
			L_print ("{b}Loaded Gemini model parameters OK\n");
		else
			L_print ("{r}Error - Unable to set Gemini pointing model "
					                                            "parameters\n");
	}
}

void on_btnGemPECLoad_clicked (GtkButton *button, gpointer data)
{
	/* Load PEC data from file */
	
	GtkWindow *parent;
	gchar *filename = NULL;
	
	parent = GTK_WINDOW (xml_get_widget (xml_app, "ccdApp"));
	filename = get_open_filename (parent, filename);
    if (filename != NULL) {
		if (telescope_set_gemini_PEC (filename))
			L_print ("{b}Loaded PEC data from %s\n", filename);
		else
			L_print ("{r}Error - Unable to load PEC data from file!\n");
		g_free (filename);
	}
}

void on_btnGemPECSave_clicked (GtkButton *button, gpointer data)
{
	/* Save PEC data to file */
	
	GtkWindow *parent;
	gchar *filename = NULL;
	
	parent = GTK_WINDOW (xml_get_widget (xml_app, "ccdApp"));
	filename = get_save_filename (parent, filename);
    if (filename != NULL) {
		if (telescope_get_gemini_PEC (filename))
			L_print ("{b}Saved PEC data to %s\n", filename);
		else
			msg ("Error saving PEC data to file!");
		g_free (filename);
	}
}

void on_btnGemDefaultsLoad_clicked (GtkButton *button, gpointer data)
{
	/* Load Gemini default settings from file */
	
	GtkWindow *parent;
	gchar *filename = NULL;
	
	parent = GTK_WINDOW (xml_get_widget (xml_app, "ccdApp"));
	filename = get_open_filename (parent, filename);
    if (filename != NULL) {
		if (telescope_set_gemini_defaults (filename))
			L_print ("{b}Loaded Gemini defaults from %s\n", filename);
		else
			L_print ("{r}Error - Unable to load Gemini defaults from file!\n");
		g_free (filename);
	}
}

void on_btnGemSetTime_clicked (GtkButton *button, gpointer data)
{
	/* Synchronise the Gemini unit to PC time */
	
	telescope_set_time ();
}

void on_btnGemStatus_clicked (GtkButton *button, gpointer data)
{
	/* Show the Gemini status */
	
	telescope_query_status (TRUE);
}

void on_btnTaskEdit_clicked (GtkButton *button, gpointer data)
{
	/* Open the tasks editing window */

	if (xml_tsk != NULL) {  /* Present window if it's already open */
		gtk_window_present (GTK_WINDOW (xml_get_widget 
		                                            (xml_tsk, "wndEditTasks")));
		return;
	}
	
	gchar *objects[] = {"wndEditTasks", NULL};
	xml_tsk = xml_load_new (xml_tsk, GLADE_INTERFACE, objects);
}

void on_btnTaskUp_clicked (GtkButton *button, gpointer data)
{
	/* Move the selected task up in the list */
	
	tasks_move_up ();
}

void on_btnTaskDown_clicked (GtkButton *button, gpointer data)
{
	
	/* Move the selected task down in the list */
	
	tasks_move_down ();
}

void on_btnTaskDelete_clicked (GtkButton *button, gpointer data)
{
	/* Delete selected task */
	
	tasks_delete ();
}

void on_btnTaskClear_clicked (GtkButton *button, gpointer data)
{
	/* Clear all the tasks from the list */
	
	tasks_clear ();
}

void on_btnTaskStart_clicked (GtkButton *button, gpointer data)
{
	/* Start execution of tasks in the list */
	
	GtkWidget *w;
	gboolean error;
	
	/* Close task editing window, if it's open */
	
	if (xml_tsk && (w = xml_get_widget (xml_tsk, "wndEditTasks"))) {
		gtk_widget_destroy (w);
		g_object_unref (G_OBJECT (xml_tsk));
		xml_tsk = NULL;
	}
	
	/* Check for errors in task list */
	
	L_print ("{b}Checking task list for errors...\n");
	tasks_start ();
	while (tasks_execute_tasks (TRUE, &error))
		;
	
	/* Start tasks if no error */
	
	if (!error) {
		L_print ("{b}Task list OK\n");
		set_task_buttons (TRUE);
		tasks_start ();
		L_print ("{b}\n\nTask execution STARTED for GoQat %s...\n", VERSION);
	} else
	    L_print ("{o}Task list contains errors. Can't execute tasks!\n");
}

void on_btnTaskPause_clicked (GtkButton *button, gpointer data)
{
	/* Pause execution of tasks in the list */
	
	gboolean pause;
	
	pause = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	tasks_pause (pause);	
	
	if (pause)
		L_print ("{b}Task execution PAUSED...\n");
	else
		L_print ("{b}Task execution CONTINUED...\n");
}

void on_btnTaskStop_clicked (GtkButton *button, gpointer data)
{
	/* Stop execution of tasks in the list */
	
	tasks_stop ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
	                         (xml_get_widget (xml_app, "btnTaskPause")), FALSE);	
	set_task_buttons (FALSE);
	
	L_print ("{b}Task execution STOPPED\n");
}

void on_btnTObject_clicked (GtkButton *button, gpointer data)
{
	/* Add an object name to the task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTObject");
	tasks_add_Object (str);
	g_free (str);
}

void on_btnTBeginSequence_clicked (GtkButton *button, gpointer data)
{
	/* Add BeginSequence marker to task list */
	
	tasks_add_BeginSequence ();
}

void on_btnTWaitUntil_clicked (GtkButton *button, gpointer data)
{
	/* Add a WaitUntil command to the task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTWaitPause");
	tasks_add_WaitUntil (str);
	g_free (str);
}

void on_btnTPauseFor_clicked (GtkButton *button, gpointer data)
{
	/* Add a PauseFor command to the task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTWaitPause");
	tasks_add_PauseFor (str);
	g_free (str);
}

void on_btnTAt_clicked (GtkButton *button, gpointer data)
{
	/* Add an At command to the task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTAt");
	tasks_add_At (str);		
	g_free (str);	
}

void on_btnTExpose_clicked (GtkButton *button, gpointer data)
{
	/* Add exposure(s) to the task list.  The text entry fields may contain
	 * parameter values such as %1, %2 etc, so we get the contents as strings
	 * rather than using get_ccd_image_params.
	 */

	gint num;
	gushort i;
	gchar *stra[10];
	
	get_entry_int ("txtTNum", 1, 999, 1, NO_PAGE, &num);
	
	stra[0] = gtk_combo_box_get_active_text (
                                 g_hash_table_lookup (hshCombo, "cmbTExpType"));
	stra[1] = gtk_combo_box_get_active_text (
                                 g_hash_table_lookup (hshCombo, "cmbTFilType"));
	stra[2] = get_entry_string ("txtTTime");
	stra[3] = get_entry_string ("txtTH1");
	stra[4] = get_entry_string ("txtTV1");
	stra[5] = get_entry_string ("txtTH2");
	stra[6] = get_entry_string ("txtTV2");
	stra[7] = get_entry_string ("txtTh_bin");
	stra[8] = get_entry_string ("txtTv_bin");
	stra[9] = get_entry_string ("txtTChipTemp");

	while (num--)
		tasks_add_Exposure (stra);
	for (i = 2; i < 10; i++)
		g_free (stra[i]);
}

void on_cmbTExpType_changed (GtkWidget *widget, gpointer data)
{
	/* Set the minimum exposure time in the text field in the task editing
	 * window if a camera is connected and a bias frame is selected.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
    
	if (!strcmp ("BIAS", gtk_combo_box_get_active_text (
													 GTK_COMBO_BOX (widget)))) {
	    if (ccd->Open)
			set_entry_float ("txtTTime", ccd->cam_cap.min_exp);
	}
}

void on_btnTFocusTo_clicked (GtkButton *button, gpointer data)
{
	/* Add command to move focuser to given postion to the task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTFocus");
	tasks_add_FocusTo (str);		
	g_free (str);	
}

void on_btnTFocusMove_clicked (GtkButton *button, gpointer data)
{
	/* Add command to move focuser by given amount to the task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTFocus");
	tasks_add_FocusMove (str);		
	g_free (str);	
}

void on_btnTIfTrue_clicked (GtkButton *button, gpointer data)
{
	/* Add command to execute If statement to task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTIfVal");
	tasks_add_IfTrue (str);		
	g_free (str);	
}

void on_btnTIfFalse_clicked (GtkButton *button, gpointer data)
{
	/* Add command to execute If statement to task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTIfVal");
	tasks_add_IfFalse (str);		
	g_free (str);	
}

void on_btnTEndIf_clicked (GtkButton *button, gpointer data)
{
	/* Add command to end execution of If segment to task list */
	
	tasks_add_EndIf ();
}

void on_btnTBeginLoop_clicked (GtkButton *button, gpointer data)
{
	/* Add command to begin loop to task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTRepeat");
	tasks_add_BeginLoop (str);		
	g_free (str);	
}

void on_btnTEndLoop_clicked (GtkButton *button, gpointer data)
{
	/* Add command to end loop to task list */
	
	tasks_add_EndLoop ();
}

void on_btnTWhile_clicked (GtkButton *button, gpointer data)
{
	/* Add command to start while loop to task list */
	
	gchar *str;
	
	str = get_entry_string ("txtTWhileVal");
	tasks_add_While (str);		
	g_free (str);	
}

void on_btnTEndWhile_clicked (GtkButton *button, gpointer data)
{
	/* Add command to end while loop to task list */
	
	tasks_add_EndWhile ();
}

void on_btnTAugOn_clicked (GtkButton *button, gpointer data)
{
	/* Add command to turn on autoguider camera to task list */
	
	tasks_add_Aug (TRUE);
}
void on_btnTAugOff_clicked (GtkButton *button, gpointer data)
{
	/* Add command to turn off autoguider camera to task list */
	
	tasks_add_Aug (FALSE);
}

void on_btnTGuideStart_clicked (GtkButton *button, gpointer data)
{
	/* Add command to start autoguiding to task list */
	
	tasks_add_Guide (TRUE);
}

void on_btnTGuideStop_clicked (GtkButton *button, gpointer data)
{
	/* Add command to stop autoguiding to task list */
	
	tasks_add_Guide (FALSE);
}

void on_btnTGoTo_clicked (GtkButton *button, gpointer data)
{
	/* Add a GoTo command to the task list */
	
	gchar *sRA, *sDec;
	
	sRA = get_entry_string ("txtTRA");
	sDec = get_entry_string ("txtTDec");

	tasks_add_GoTo (sRA, sDec);		
	
	g_free (sDec);
	g_free (sRA);
}

void on_btnTExec_clicked (GtkButton *button, gpointer data)
{
	/* Add an Exec command to the task list */
	
	gchar *filename;
	
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER 
		                          (xml_get_widget (xml_tsk, "flcTScriptFile")));
	
	tasks_add_Exec (filename, TRUE);
	g_free (filename);	
}

void on_btnTExecAsync_clicked (GtkButton *button, gpointer data)
{
	/* Add an ExecAsync command to the task list */
	
	gchar *filename;
	
	filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER 
		                          (xml_get_widget (xml_tsk, "flcTScriptFile")));
	
	tasks_add_Exec (filename, FALSE);
	g_free (filename);	
}

void on_btnTSetParam_clicked (GtkButton *button, gpointer data)
{
	/* Sets a task parameter to the given value */
	
	gint val;
	gchar *param, *str;
	
	get_entry_int ("txtTSetParam", 0, NUMTPARAMS - 1, 0, NO_PAGE, &val);
	param = g_strdup_printf ("%d", val);
	str = get_entry_string ("txtSetParamVal");
	
	tasks_add_SetParam (param, str);
	g_free (param);
	g_free (str);	
}

void on_btnTMove_clicked (GtkButton *button, gpointer data)
{
	/* Add a Move command to the task list */
	
	gchar *sRA, *sDec;
	
	sRA = get_entry_string ("txtTMoveRA");
	sDec = get_entry_string ("txtTMoveDec");

	tasks_add_Move (sRA, sDec);		
	
	g_free (sDec);
	g_free (sRA);
}

void on_btnTWarmRestart_clicked (GtkButton *button, gpointer data)
{
	/* Add a WarmRestart command to the task list */
	
	tasks_add_WarmRestart ();
}

void on_btnTParkMount_clicked (GtkButton *button, gpointer data)
{
	/* Add a ParkMount command to the task list */
	
	tasks_add_ParkMount ();
}

void on_btnTRecordStart_clicked (GtkButton *button, gpointer data)
{
	/* Add a RecordStart command to the task list */
	
	tasks_add_Record (TRUE);
}

void on_btnTRecordStop_clicked (GtkButton *button, gpointer data)
{
	/* Add a RecordStop command to the task list */
	
	tasks_add_Record (FALSE);
}

void on_btnTYellow_clicked (GtkButton *button, gpointer data)
{
	/* Add a YellowButton command to the task list */
	
	tasks_add_YellowButton ();
}

void on_btnTShutdown_clicked (GtkButton *button, gpointer data)
{
	/* Add a Shutdown command to the task list */

	tasks_add_Shutdown ();		
}

void on_btnTExit_clicked (GtkButton *button, gpointer data)
{
	/* Add an Exit command to the task list */

	tasks_add_Exit ();		
}

void on_btnTLoadTasks_clicked (GtkButton *button, gpointer data)
{
	/* Load a tasks file */
	
	GtkWindow *parent;
	gchar *filename = NULL;

	parent = GTK_WINDOW (xml_get_widget (xml_tsk, "wndEditTasks"));
	filename = get_open_filename (parent, filename);
	if (filename != NULL) {
		if (tasks_load_file (filename))
			L_print ("{b}Loaded tasks successfully!\n");
		g_free (filename);
	}
}

void on_btnTSaveTasks_clicked (GtkButton *button, gpointer data)
{
	/* Save task list to a file */

	GtkWindow *parent;
	gchar *filename = NULL;

	parent = GTK_WINDOW (xml_get_widget (xml_tsk, "wndEditTasks"));
	filename = get_save_filename (parent, filename);
    if (filename != NULL) {
		if (tasks_write_file (filename))
			L_print ("{b}Saved tasks successfully!\n");
		g_free (filename);
	}
}

void on_btnTCloseWindow_clicked (GtkButton *button, gpointer data)
{
	/* Close the tasks editing window */
	
    g_hash_table_remove (hshCombo, "cmbTExpType");
    g_hash_table_remove (hshCombo, "cmbTFilType");
	gtk_widget_destroy (xml_get_widget (xml_tsk, "wndEditTasks"));
	g_object_unref (G_OBJECT (xml_tsk));
	xml_tsk = NULL;
	EdWinConf = FALSE;
}

void on_btnLVClose_clicked (GtkButton *button, gpointer data)
{
	/* Close the LiveView window */
	
	GtkCheckMenuItem *mnuItem;
	
	mnuItem = GTK_CHECK_MENU_ITEM (xml_get_widget (xml_app, "live_view")); 
	gtk_check_menu_item_set_active (mnuItem, FALSE);
}

void on_btnPBZoomIn_clicked (GtkButton *button, gpointer data)
{
	/* Zoom in to the selected area of the video playback image canvas and 
     * centre it
     */
	
	if (!get_vid_image_struct ()->Open)
		return;
        
	canvas_zoom_image (cnvPlayback, get_vid_image_struct (), ZOOM_IN);
}

void on_btnPBZoomOut_clicked (GtkButton *button, gpointer data)
{
	/* Zoom out from the selected canvas area of the video playback image 
     * canvas
     */
	
	if (!get_vid_image_struct ()->Open)
		return;
        
	canvas_zoom_image (cnvPlayback, get_vid_image_struct (), ZOOM_OUT);
}

void on_btnPBZoom1to1_clicked (GtkButton *button, gpointer data)
{
    /* Set the zoom scale to 1:1 for the video playback image canvas*/
    
	if (!get_vid_image_struct ()->Open)
		return;
        
    canvas_zoom_image (cnvPlayback, get_vid_image_struct (), ZOOM_1x1);
}

void on_btnPBResetArea_clicked (GtkButton *button, gpointer data)
{
	/* Reset the selected area rectangle to full frame */

	struct cam_img *vid = get_vid_image_struct ();
    
	if (!get_vid_image_struct ()->Open)
		return;
        
	vid->canv.r.htl = 0;
	vid->canv.r.vtl = 0;
	vid->canv.r.hbr = vid->exd.h_pix;
	vid->canv.r.vbr = vid->exd.v_pix;
	vid->canv.NewRect = TRUE;
    
	g_object_set (G_OBJECT (vid->canv.cviRect),
                  "x", 0.0,
                  "y", 0.0, 
                  "width", (gdouble) vid->exd.h_pix,
                  "height", (gdouble) vid->exd.v_pix,
                  NULL);
}

void on_btnPBOpenFile_clicked (GtkButton *button, gpointer data)
{
	/* Open a video file */

	GtkWindow *parent;
	gchar *filename = NULL;
    
	parent = GTK_WINDOW (xml_get_widget (xml_pbw, "wndPlayback"));
	filename = get_open_filename (parent, filename);
	if (filename != NULL) {
		video_open_file (filename);
	    g_free (filename);
		gtk_widget_set_sensitive (xml_get_widget (xml_pbw, "btnPBPlay"), TRUE);
        gtk_widget_activate (xml_get_widget (xml_pbw, "btnPBZoom1to1"));
        gtk_widget_activate (xml_get_widget (xml_pbw, "btnPBResetArea"));
	}
}

void on_txtPBFrameNum_activate (GtkEditable *editable, gpointer data)
{
	/* Show the specified frame number if the user presses 'Enter' in
	 * the 'frame number' text box.
	 */
	
	gint num;
	
	get_entry_int ("txtPBFrameNum", 1, 999999, 1, NO_PAGE, &num);
	video_show_frame (num);
}

void on_txtPBTimeStamp_activate (GtkEditable *editable, gpointer data)
{
	/* Update the time stamp if the user presses 'Enter' in the 'time stamp'
	 * text box.
	 */
	
	video_update_timestamp (get_entry_string ("txtPBTimeStamp"));
}

void on_btnPBSetTimes_clicked (GtkButton *button, gpointer data)
{
	/* Set the time stamps for frames in the selected range */
	
	video_set_timestamps ();
}

void on_hscPBFrames_changed (GtkRange *range, gpointer data)
{
	/* This is called whenever the frames slider in the playback window is moved
     * so we need to read and display the selected frame.
	 */
	
    video_show_frame ((guint) gtk_range_get_value (range));	
}

void on_btnPBPlay_clicked (GtkButton *button, gpointer data)
{
	/* Play the video */

	video_play_frames (TRUE);
}

void on_btnPBStop_clicked (GtkButton *button, gpointer data)
{
	/* Stop the video */
	
	video_play_frames (FALSE);
}

void on_btnPBPrev_clicked (GtkButton *button, gpointer data)
{
	/* Show the next frame */
	
	video_show_prev ();
}

void on_btnPBNext_clicked (GtkButton *button, gpointer data)
{
	/* Show the previous frame */
	
	video_show_next ();
}

void on_btnPBMarkFirst_clicked (GtkButton *button, gpointer data)
{
	/* Get the current frame number and display in the 'mark first' text box */
	
	gint f_num;
	
	f_num = video_get_frame_number ();
	set_entry_int ("txtPBFirst", f_num);
}

void on_btnPBMarkLast_clicked (GtkButton *button, gpointer data)
{
	/* Get the current frame number and display in the 'mark first' text box */
	
	gint f_num;
	
	f_num = video_get_frame_number ();
	set_entry_int ("txtPBLast", f_num);
}

void on_cmbPBfps_changed (GtkComboBox *combo, gpointer data)
{
	/*  Set video playback frame rate.  These values need to match the ones
	 *  defined in the configuration file.
	 */
	
	video_set_start_time ();
	switch (gtk_combo_box_get_active (combo)) {
		case 0:
			video_set_frame_rate (60);
		    break;
		case 1:
			video_set_frame_rate (30);
		    break;
		case 2:
			video_set_frame_rate (25);
		    break;
		case 3:
			video_set_frame_rate (12);
		    break;
		case 4:
			video_set_frame_rate (6);
		    break;
		case 5:
			video_set_frame_rate (3);
		    break;
	}
}

void on_btnPBSaveFrames_clicked (GtkButton *button, gpointer data)
{
	/* Open the file types window */

	gchar *objects[] = {"wndSaveVideoFileType", NULL};
	xml_svf = xml_load_new (xml_svf, GLADE_INTERFACE, objects);
}

void on_btnPBPhotom_clicked (GtkButton *button, gpointer data)
{
	/* Open the SExtractor parameters window */

	if (xml_pht) {  /* Present window if it's already open */
		gtk_window_present (GTK_WINDOW (xml_get_widget (xml_pht, "wndPhotom")));
		return;
	}
	
	gchar *objects[] = {"wndPhotom", NULL};
	xml_pht = xml_load_new (xml_pht, GLADE_INTERFACE, objects);
}

void on_btnPBClose_clicked (GtkButton *button, gpointer data)
{
	/* Close the playback window */
	
	GtkCheckMenuItem *mnuItem;
	
	mnuItem = GTK_CHECK_MENU_ITEM (xml_get_widget (xml_app, "playback")); 
	gtk_check_menu_item_set_active (mnuItem, FALSE);
}

void on_btnPhotSingle_clicked (GtkButton *button, gpointer data)
{
	/* Do photometry on a single video frame */
	
	gdouble aperture, minarea, thresh, shift;
	
	get_entry_float ("txtPhotAperture", 0.1, 100.0, 5.0, NO_PAGE, &aperture);
	get_entry_float ("txtPhotMinarea", 0.1, 100.0, 5.0, NO_PAGE, &minarea);
	get_entry_float ("txtPhotThresh", 0.0, 10.0, 2.0, NO_PAGE, &thresh);
	get_entry_float ("txtPhotMaxShift", 0.0, 100.0, 2.0, NO_PAGE, &shift);
	
	video_photom_frames ((gfloat) aperture, (gfloat) minarea, 
	                     (gfloat) thresh, (gfloat) shift, FALSE);
                         
    gtk_widget_set_sensitive (xml_get_widget (xml_pht, "btnPhotRange"), TRUE);
}

void on_btnPhotRange_clicked (GtkButton *button, gpointer data)
{
	/* Do photometry on a range of video frames */
	
	gdouble aperture, minarea, thresh, shift;
	
	get_entry_float ("txtPhotAperture", 0.1, 100.0, 5.0, NO_PAGE, &aperture);
	get_entry_float ("txtPhotMinarea", 0.1, 100.0, 5.0, NO_PAGE, &minarea);
	get_entry_float ("txtPhotThresh", 0.0, 10.0, 2.0, NO_PAGE, &thresh);
	get_entry_float ("txtPhotMaxShift", 0.0, 100.0, 2.0, NO_PAGE, &shift);
	
	video_photom_frames ((gfloat) aperture, (gfloat) minarea, 
	                     (gfloat) thresh, (gfloat) shift, TRUE);
}

void on_btnPhotClearLog_clicked (GtkButton *button, gpointer data)
{
	/* Clear photometry log window */
	
	GtkTextIter start, end;
	
	gtk_text_buffer_get_iter_at_offset (txtPhotBuffer, &start, 0);
	gtk_text_buffer_get_iter_at_offset (txtPhotBuffer, &end, -1);
	gtk_text_buffer_delete (txtPhotBuffer, &start, &end);
}

void on_btnPhotClose_clicked (GtkButton *button, gpointer data)
{
	/* Close the photometry window */
	
	gdouble f;
	
	get_entry_float ("txtPhotAperture", 0.1, 100.0, 5.0, NO_PAGE, &f);
	W_config_f ("Photom/Aperture", f);
	get_entry_float ("txtPhotMinarea", 0.1, 100.0, 5.0, NO_PAGE, &f);
	W_config_f ("Photom/MinArea", f);
	get_entry_float ("txtPhotThresh", 0.0, 10.0, 2.0, NO_PAGE, &f);
	W_config_f ("Photom/Thresh", f);

	gtk_widget_destroy (xml_get_widget (xml_pht, "wndPhotom"));
	g_object_unref (G_OBJECT (xml_pht));
	xml_pht = NULL;
	PhotWinConf = FALSE;
}

void on_btnSVOK_clicked (GtkButton *button, gpointer data)
{
	/* Get the selected file type for saving video frames and close the
	 * window.  Then request a file name for saving the video frames and
	 * initiate saving.
	 */
	
	GtkWindow *parent;
	gint filetype;
	gchar *filename = NULL;
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (xml_get_widget (
						              xml_svf, "optSVSaveAsFITS"))))
		filetype = SVF_FITS;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (xml_get_widget (
						              xml_svf, "optSVSaveAsVID"))))
		filetype = SVF_VID;
	
	gtk_widget_destroy (xml_get_widget (xml_svf, "wndSaveVideoFileType"));
	g_object_unref (G_OBJECT (xml_svf));
	xml_svf = NULL;	
	
	parent = GTK_WINDOW (xml_get_widget (xml_pbw, "wndPlayback"));
	filename = get_save_filename (parent, filename);
	if (filename != NULL) {
		video_save_frames (filename, filetype, TRUE);
	    g_free (filename);
	}
}

void on_btnDevSelect_clicked (GtkButton *button, gpointer data)
{
	/* Set the selected device number and close the window */
	
	*ds.idx = (gshort) gtk_combo_box_get_active (
                                g_hash_table_lookup (hshCombo, "cmbDevSelect"));
    g_hash_table_remove (hshCombo, "cmbDevSelect");
	gtk_widget_destroy (xml_get_widget (xml_dev, "wndDevSelect"));
	g_object_unref (G_OBJECT (xml_dev));
	xml_dev = NULL;
}

void on_btnUnicapSelect_clicked (GtkButton *button, gpointer data)
{
	#ifdef HAVE_UNICAP
	/* Save settings and close the window */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	W_config_s ("Unicap/Device/ID", aug->ucp_device.identifier);
	W_config_d ("Unicap/Format/Fourcc", aug->ucp_format_spec.fourcc);
	W_config_d ("Unicap/Format/Size/Width", aug->ucp_format_spec.size.width);
	W_config_d ("Unicap/Format/Size/Height", aug->ucp_format_spec.size.height);
	
	if (aug->ucp_handle) {
		unicap_close (aug->ucp_handle);
		aug->ucp_handle = NULL;
	}
	
	gtk_widget_destroy (xml_get_widget (xml_uni, "wndUnicapSelect"));
	g_object_unref (G_OBJECT (xml_uni));
	xml_uni = NULL;
	#endif
}

void on_btnV4LConfigApply_clicked (GtkButton *button, gpointer data)
{
	/* Apply V4L settings (save settings to configuration file and re-start
	 * the camera).
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gdouble fps;
	gchar key[128];
	
	snprintf (key, 128, "V4L/%s/4CC", aug->vid_dat.card);
	W_config_d (key, aug->vid_dat.pixfmt);

	if (aug->vid_dat.HasVideoStandard) {
		aug->vid_dat.vid_std.selected = gtk_combo_box_get_active (
		                        g_hash_table_lookup (hshCombo, "cmbV4LVidStd"));
		snprintf (key, 128, "V4L/%s/VideoStandard", aug->vid_dat.card);
		/* Note: the standard is defined as __64 in the v4l2 api and we save
		 * only the first 32 bits here (i.e. bits 0 to 31), but these cover all
		 * the common standards.  Bits 32 to 63 are for custom standards that
		 * we ignore anyway because we don't know how to treat them.
		 */
		W_config_d (key, 
	            (guint) aug->vid_dat.vid_std.id[aug->vid_dat.vid_std.selected]);
	}
		                        
	aug->vid_dat.vid_input.selected = gtk_combo_box_get_active (
		                        g_hash_table_lookup (hshCombo, "cmbV4LVidInp"));
	snprintf (key, 128, "V4L/%s/VideoInput", aug->vid_dat.card);
	W_config_d (key, aug->vid_dat.vid_input.selected);
		                        
	get_entry_int ("txtV4LConfigWidth", 1, AUGCANV_H, 640, 
	               NO_PAGE, &aug->vid_dat.width);
	snprintf (key, 128, "V4L/%s/Width", aug->vid_dat.card);
	W_config_d (key, aug->vid_dat.width);
	               
	get_entry_int ("txtV4LConfigHeight", 1, AUGCANV_V, 480, 
	               NO_PAGE, &aug->vid_dat.height);
	snprintf (key, 128, "V4L/%s/Height", aug->vid_dat.card);
	W_config_d (key, aug->vid_dat.height);
	
	get_entry_float ("txtV4LConfigFps", 1, 500, 15, 
	               NO_PAGE, &fps);
	aug->vid_dat.fps = (gfloat) fps;
	snprintf (key, 128, "V4L/%s/fps", aug->vid_dat.card);
	W_config_f (key, aug->vid_dat.fps);
	
    /* Re-start the camera with the new settings.  They will be read from the
     * configuration file when the camera is re-opened.
     */
          
    loop_autog_restart ();
}

void on_btnV4LConfigClose_clicked (GtkButton *button, gpointer data)
{
	/* Close the V4L configuration window */
	
	gtk_widget_destroy (xml_get_widget (xml_V4L, "wndV4LConfig"));
	g_object_unref (G_OBJECT (xml_V4L));
	xml_V4L = NULL;
	V4LWinConf = FALSE;
}

void on_chkCConfCoolOnConnect_toggled (GtkButton *button, gpointer data)
{
	/* Save the setting to the configuration database */
	
	W_config_d (CCDConfigKey (CCDConfigOwner, "CoolOnConnect"), 
				     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)));
}

void on_btnCConfSetDefTemp_clicked (GtkButton *button, gpointer data)
{
	/* Save the value to the configuration database.  Update the value on the 
	 * CCD camera tab to match if this is for the main imaging camera, rather 
	 * than the autoguider camera.
	 */
	
	gdouble val;
	gchar *entry = NULL;
	
	if (!strcmp (gtk_buildable_get_name (GTK_BUILDABLE (button)), 
														 "btnCConfSetDefTemp"))
		entry = "txtCConfDefTemp";
	else if (!strcmp (gtk_buildable_get_name (GTK_BUILDABLE (button)), 
														 "btnCConfSetDefTemp1"))
		entry = "txtCConfDefTemp1";
	else {
		L_print ("{r}Can't find default temperature text box\n");
		return;
	}
														  
	get_entry_float (entry, TPR_MIN, TPR_MAX, TPR_DEF, NO_PAGE, &val);
	W_config_f (CCDConfigKey (CCDConfigOwner, "DefTemp"), val);
	if (CCDConfigOwner->id == CCD)
		set_entry_float ("txtChipTemp", val);
}

void on_btnCConfSetTempTol_clicked (GtkButton *button, gpointer data)
{
	/* Save the value to the configuration database */
	
	gdouble val;
	gchar *entry = NULL;
	
	if (!strcmp (gtk_buildable_get_name (GTK_BUILDABLE (button)), 
														 "btnCConfSetTempTol"))
		entry = "txtCConfTempTol";
	else if (!strcmp (gtk_buildable_get_name (GTK_BUILDABLE (button)), 
														 "btnCConfSetTempTol1"))
		entry = "txtCConfTempTol1";
	else {
		L_print ("{r}Can't find temperature tolerance text box\n");
		return;
	}
														  
	get_entry_float (entry, 0.0, 9.9, 1.0, NO_PAGE, &val);
	W_config_f (CCDConfigKey (CCDConfigOwner, "TempTol"), val);
	(get_ccd_image_struct ())->state.c_tol = val;
}

void on_btnCConfCoolerOn_clicked (GtkButton *button, gpointer data)
{
	/* Turn the CCD cooler on and set to the new default target temperature.
	 * This routine can be called even if the cooling is already on at a 
	 * different temperature.
	 */
	
	if (CCDConfigOwner->cam_cap.CanSetCCDTemp) {
		CCDConfigOwner->state.d_ccd = R_config_f (
							 CCDConfigKey (CCDConfigOwner, "DefTemp"), TPR_DEF);
		CCDConfigOwner->set_state (S_TEMP, 0, CCDConfigOwner->state.d_ccd, 
													  CCDConfigOwner->id);
		CCDConfigOwner->set_state (S_COOL, TRUE, 0.0, CCDConfigOwner->id);
		L_print ("Requested CCD cooling at %5.1fC\n", 
				                                   CCDConfigOwner->state.d_ccd);
	} else
	    L_print ("{o}Cooling control not supported for this camera\n");
}
	
void on_btnCConfCoolerOff_clicked (GtkButton *button, gpointer data)
{
	/* Turn the CCD cooler off */

	if (CCDConfigOwner->cam_cap.CanSetCCDTemp)
		CCDConfigOwner->set_state (S_COOL, FALSE, 0.0, CCDConfigOwner->id);
	else
	    L_print ("{o}Cooling control not supported for this camera\n");
}

void on_optCConfFans_toggled (GtkButton *button, gpointer data)
{
	/* Save the fan settings */
	
	gint fans;
	const gchar *ButtonName;
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
		ButtonName = gtk_buildable_get_name (GTK_BUILDABLE (button));	
		if (!g_ascii_strcasecmp (ButtonName, "optCConfFansHigh"))
			fans = CCD_FAN_HIGH;
		else if (!g_ascii_strcasecmp (ButtonName, "optCConfFansAuto"))
			fans = CCD_FAN_AUTO;
		else if (!g_ascii_strcasecmp (ButtonName, "optCConfFansOff"))
			fans = CCD_FAN_OFF;
		else {
			G_print ("{r}on_optCConfFans_toggled: Not a valid button name: "
					                                        "%s\n", ButtonName);
			return;
		}
		
		W_config_d (CCDConfigKey (CCDConfigOwner, "Fans"), fans);
	}
}

void on_btnCConfSetFans_clicked (GtkButton *button, gpointer data)
{
	/* Set the fans state */
	
	gint fans;
	
	fans = R_config_d (CCDConfigKey (CCDConfigOwner, "Fans"), CCD_FAN_AUTO);
	CCDConfigOwner->set_state (S_FANS, fans, 0.0);
}

void on_cmbCConfShutPrior_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the shutter priority setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "ShutterPriority"), i - 1);
}

void on_cmbCConfShutMode_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the shutter mode setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "ShutterMode"), i - 1);
}

void on_cmbCConfShutOpen_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the shutter state setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "ShutterOpen"), i - 1);
}

void on_cmbCConfPreFlush_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the pre-flush setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "PreFlush"), i - 1);
}

void on_cmbCConfFastMode_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the exposure mode setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "HostTimed"), i - 1);
}

void on_cmbCConfCameraGain_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the camera gain setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "CameraGain"), i - 1);
}

void on_cmbCConfReadoutSpeed_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the readout speed setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "ReadoutSpeed"), i - 1);
}

void on_cmbCConfAntiBlooming_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the anti-blooming setting */
	
	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "AntiBloom"), i - 1);
}

void on_btnCConfSetShutPrior_clicked (GtkButton *button, gpointer data)
{
	/* Set the shutter priority */
	
	if ((CCDConfigOwner->state.shut_prior = 
	    R_config_d (CCDConfigKey (CCDConfigOwner, "ShutterPriority"), -1)) > -1)
		CCDConfigOwner->set_state (
							 S_PRIORITY, CCDConfigOwner->state.shut_prior, 0.0);
}

void on_btnCConfSetShutMode_clicked (GtkButton *button, gpointer data)
{
	/* Set the shutter mode */
	
	if ((CCDConfigOwner->state.shut_mode = 
		R_config_d (CCDConfigKey (CCDConfigOwner, "ShutterMode"), -1)) > -1)
		CCDConfigOwner->set_state(S_MODE, CCDConfigOwner->state.shut_mode, 0.0);
}

void on_btnCConfSetShutOpen_clicked (GtkButton *button, gpointer data)
{
	/* Set the shutter state */

	if ((CCDConfigOwner->state.shut_open = 
		R_config_d (CCDConfigKey (CCDConfigOwner, "ShutterOpen"), -1)) > -1)
		CCDConfigOwner->set_state(S_OPEN, CCDConfigOwner->state.shut_open, 0.0);
}

void on_btnCConfSetPreFlush_clicked (GtkButton *button, gpointer data)
{
	/* Set the expoure pre-flush */

	if ((CCDConfigOwner->state.pre_flush = 
		R_config_d (CCDConfigKey (CCDConfigOwner, "PreFlush"), -1)) > -1)
		CCDConfigOwner->set_state(S_FLUSH, CCDConfigOwner->state.pre_flush,0.0);
}

void on_btnCConfSetFastMode_clicked (GtkButton *button, gpointer data)
{
	/* Set the exposure mode */
	
	if ((CCDConfigOwner->state.host_timed = 
		R_config_d (CCDConfigKey (CCDConfigOwner, "HostTimed"), -1)) > -1)
		CCDConfigOwner->set_state(S_HOST, CCDConfigOwner->state.host_timed,0.0);
}

void on_btnCConfSetCamGain_clicked (GtkButton *button, gpointer data)
{
	/* Set the camera gain */
	
	if ((CCDConfigOwner->state.cam_gain = 
		R_config_d (CCDConfigKey (CCDConfigOwner, "CameraGain"), -1)) > -1)
		CCDConfigOwner->set_state (S_GAIN, CCDConfigOwner->state.cam_gain, 0.0);
}

void on_btnCConfSetReadoutSpeed_clicked (GtkButton *button, gpointer data)
{
	/* Set the camera readout speed */
	
	#ifdef HAVE_READOUT_SPEED
	if ((CCDConfigOwner->state.read_speed = 
		R_config_d (CCDConfigKey (CCDConfigOwner, "ReadoutSpeed"), -1)) > -1)
		CCDConfigOwner->set_state(S_SPEED,CCDConfigOwner->state.read_speed,0.0);
	#else
	L_print ("{o}Readout speed setting not available - upgrade QSI library?\n");
	#endif
}

void on_btnCConfSetAntiBloom_clicked (GtkButton *button, gpointer data)
{
	/* Set the anti-blooming */

	if ((CCDConfigOwner->state.anti_bloom = 
		R_config_d (CCDConfigKey (CCDConfigOwner, "AntiBloom"), -1)) > -1)
		CCDConfigOwner->set_state(S_ABLOOM,CCDConfigOwner->state.anti_bloom,0.0);
}

void on_cmbCConfWheel_changed (GtkComboBox *combo, gpointer data)
{
	/* Store the new filter wheel number and recall the filter names/positions
	 * and focus offsets for that wheel. 
	 */
	
    GList *keys = NULL, *current = NULL;
	gushort i = 0, j;
	gfloat fo;
	gchar *num, *key, **FilterNames, *field;
	gchar filter[MCSL];
	
	/* Store filter wheel number */
	
	num = gtk_combo_box_get_active_text (combo);
	W_config_s (CCDConfigKey (CCDConfigOwner, "Filter/ActiveWheel"), num);
    
    /* Load the available filter names from the configuration file */
    
    FilterNames = ReadCArray ("ListCCDFilterTypes");
    
	/* Iterate over each filter position for this wheel... */
    
    keys = g_hash_table_get_keys (hshCombo);
    for (current = keys; current != NULL; current = current->next) {
        
        /* Is this a valid combo box? */
        
        if (!strncmp ((gchar *) current->data, "cmbCConfFil", 11)) {
            
            /* Get the corresponding filter position from the combo box name */
            
            i = (gushort) strtol ((gchar *) current->data + 11, NULL, 10);
            key = g_strdup_printf ("%s/%s/%d", CCDConfigKey (CCDConfigOwner, 
														     "Filter"), num, i);
            strcpy (filter, "-");
            R_config_s (key, filter);  /* Get stored filter for this position */
            g_free (key);
        
            /* Set filter combo box for this position to the corresponding 
             * filter.
             */
		
            j = 0;
            while (FilterNames[j]) {
                if (!strcmp (FilterNames[j++], filter))
                    gtk_combo_box_set_active (g_hash_table_lookup (
                                              hshCombo, current->data), j - 1);
            }
            
            /* Get focus offset for this position */
		
            key = g_strdup_printf ("%s/%s/FO%d", CCDConfigKey (CCDConfigOwner, 
														     "Filter"), num, i);
            fo = R_config_d (key, 0);
		
            /* Set corresponding entry field to this value */
		
            field = g_strdup_printf ("txtCConfFoc%d", i);
            set_entry_int (field, fo);
            g_free (field);	
        }	
    }

    j = 0;
    while (FilterNames[j]) {
        g_free (FilterNames[j++]);
    }
    g_list_free (keys);
	g_free (num);
}

void on_btnCConfSaveFilterSettings_clicked (GtkButton *button, gpointer data)
{
	/* Save the filter and focus offset settings */
	
    GList *keys = NULL, *current = NULL;
	gushort ActiveW, i;
	gint val;
	gchar *key, *name, *filter;
	
	ActiveW = R_config_d (CCDConfigKey(CCDConfigOwner, "Filter/ActiveWheel"),1);
	
    keys = g_hash_table_get_keys (hshCombo);
    for (current = keys; current != NULL; current = current->next) {
        if (!strncmp ((gchar *) current->data, "cmbCConfFil", 11)) {
            i = (gushort) strtol ((gchar *) current->data + 11, NULL, 10);
            key = g_strdup_printf ("%s/%d/%d", CCDConfigKey (
                                               CCDConfigOwner, "Filter"), 
                                               ActiveW, i);
            filter = gtk_combo_box_get_active_text (
                                 g_hash_table_lookup (hshCombo, current->data));
            W_config_s (key, filter);
            g_free (filter);
            g_free (key);
        
            key = g_strdup_printf ("%s/%d/FO%d", CCDConfigKey (
                                                 CCDConfigOwner, "Filter"), 
                                                 ActiveW, i);
            name = g_strdup_printf ("txtCConfFoc%d", i);
            get_entry_int (name, -65535, 65535, 0, NO_PAGE, &val);
            W_config_d (key, val);
            g_free (name);
            g_free (key);
        }
    }
}

void on_btnCConfRotate_clicked (GtkButton *button, gpointer data)
{
	/* Set the filter wheel to the given position */
	
	gint offset;
	gchar *filter;
	
	filter = gtk_combo_box_get_active_text (
                           g_hash_table_lookup (hshCombo, "cmbCConfSetFilter"));
	set_filter (TRUE, filter, &offset);
	g_free (filter);
	if (get_apply_filter_offset ()) {
		L_print ("{b}Applying filter focus offset of %d steps...\n", offset);
		loop_focus_apply_filter_offset (offset);
	}
}

void on_chkCConfInvertImage_toggled (GtkButton *button, gpointer data)
{
	/* Save the setting to the configuration database and set the camera
	 * state.
	 */
	
	CCDConfigOwner->state.invert = gtk_toggle_button_get_active (
													GTK_TOGGLE_BUTTON (button));
	W_config_d (CCDConfigKey (CCDConfigOwner, "InvertImage"),
				              CCDConfigOwner->state.invert);
	CCDConfigOwner->set_state (S_INVERT, CCDConfigOwner->state.invert, 
							   0.0, CCDConfigOwner->id);
}

void on_chkCConfInvertDS9h_toggled (GtkButton *button, gpointer data)
{
	/* Save the setting to the configuration database and set flag */
	
	CCDConfigOwner->ds9.Invert_h = gtk_toggle_button_get_active (
	                                                GTK_TOGGLE_BUTTON (button));
	W_config_d (CCDConfigKey (CCDConfigOwner, "InvertDS9h"), 
	                          CCDConfigOwner->ds9.Invert_h);
}

void on_chkCConfInvertDS9v_toggled (GtkButton *button, gpointer data)
{
	/* Save the setting to the configuration database and set flag */
	
	CCDConfigOwner->ds9.Invert_v = gtk_toggle_button_get_active (
	                                                GTK_TOGGLE_BUTTON (button));
	W_config_d (CCDConfigKey (CCDConfigOwner, "InvertDS9v"), 
	                          CCDConfigOwner->ds9.Invert_v);
}

void on_chkCConfDebayer_toggled (GtkButton *button, gpointer data)
{
	/* Save the setting to the configuration database */
	
	CCDConfigOwner->Debayer = gtk_toggle_button_get_active (
													GTK_TOGGLE_BUTTON (button));
	W_config_d (CCDConfigKey (CCDConfigOwner, "Debayer"),
				CCDConfigOwner->Debayer);
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "debayer"),
							  CCDConfigOwner->Debayer);
}

void on_cmbCConfBayerPattern_changed (GtkComboBox *combo, gpointer data)
{
	/* Save the Bayer pattern.  
	 * The combo box is assumed to contain the following items (set in the Glade
	 * gtkbuilder file):
	 * 
	 * '-', 'RG', 'GR', 'GB', 'BG'.
	 *
	 * The value of bayer_pattern must be set as follows:
	 *
	 * 0 (R G)    1 (G R)    3 (G B)    4 (B G)
	 *
	 * See debayer_get_tile.
	 */

	gint i;
	
	i = gtk_combo_box_get_active (combo);
	W_config_d (CCDConfigKey (CCDConfigOwner, "BayerPattern"), i);
	CCDConfigOwner->bayer_pattern = (i < 3 ? --i : i);
}

void on_btnCConfClose_clicked (GtkButton *button, gpointer data)
{
	/* Close the CCD camera configuration window */
	
	GtkWidget *dlgNoTile;
	gint ret;
	
	if (CCDConfigOwner->Debayer && !gtk_combo_box_get_active (
                      g_hash_table_lookup (hshCombo, "cmbCConfBayerPattern"))) {
		dlgNoTile = gtk_message_dialog_new (
					GTK_WINDOW (CCDConfigWin),
		            GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_WARNING,
					GTK_BUTTONS_OK,
		            "You have selected to debayer the images, but have not "
					"chosen a Bayer pattern.  Please choose a Bayer pattern "
					"before continuing.");
		ret = gtk_dialog_run (GTK_DIALOG (dlgNoTile));
		gtk_widget_destroy (dlgNoTile);
		return;
	}
	
    g_hash_table_foreach_remove (hshCombo, IsCConfCombo, NULL);
	gtk_widget_destroy (CCDConfigWin);
	CCDConfigWin = NULL;
	CCDConfigWinConf = FALSE;
	g_object_unref (G_OBJECT (xml_con));
	xml_con = NULL;
}

gboolean IsCConfCombo (gpointer key, gpointer value, gpointer data)
{
    return (!strncmp ((gchar *) key, "cmbCConf", 8));
}

void on_cmbFConfWheel_changed (GtkComboBox *combo, gpointer data)
{
	/* Store the new filter wheel number and recall the filter names/positions
	 * and focus offsets for that wheel. 
	 */
	
    GList *keys = NULL, *current = NULL;
	gushort i = 0, j;
	gfloat fo;
	gchar *num, *key, **FilterNames, *field;
	gchar filter[MCSL];
	
	/* Store filter wheel number */
	
	num = gtk_combo_box_get_active_text (combo);
	W_config_s ("FilterWheel/ActiveWheel", num);
    
    /* Load the available filter names from the configuration file */
    
    FilterNames = ReadCArray ("ListCCDFilterTypes");
    
	/* Iterate over each filter position for this wheel... */
    
    keys = g_hash_table_get_keys (hshCombo);
    for (current = keys; current != NULL; current = current->next) {
        
        /* Is this a valid combo box? */
        
        if (!strncmp ((gchar *) current->data, "cmbFConfFil", 11)) {
            
            /* Get the corresponding filter position from the combo box name */
            
            i = (gushort) strtol ((gchar *) current->data + 11, NULL, 10);
            key = g_strdup_printf ("%s/%s/%d", "FilterWheel", num, i);
            strcpy (filter, "-");
            R_config_s (key, filter);  /* Get stored filter for this position */
            g_free (key);
        
            /* Set filter combo box for this position to the corresponding 
             * filter.
             */
		
            j = 0;
            while (FilterNames[j]) {
                if (!strcmp (FilterNames[j++], filter))
                    gtk_combo_box_set_active (g_hash_table_lookup (
                                              hshCombo, current->data), j - 1);
            }
            
            /* Get focus offset for this position */
		
            key = g_strdup_printf ("%s/%s/FO%d", "FilterWheel", num, i);
            fo = R_config_d (key, 0);
		
            /* Set corresponding entry field to this value */
		
            field = g_strdup_printf ("txtFConfFoc%d", i);
            set_entry_int (field, fo);
            g_free (field);	
        }	
    }

    j = 0;
    while (FilterNames[j]) {
        g_free (FilterNames[j++]);
    }
    g_list_free (keys);
	g_free (num);
}

void on_btnFConfSaveFilterSettings_clicked (GtkButton *button, gpointer data)
{
	/* Save the filter and focus offset settings */
	
    GList *keys = NULL, *current = NULL;
	gushort ActiveW, i;
	gint val;
	gchar *key, *name, *filter;
	
	ActiveW = R_config_d ("FilterWheel/ActiveWheel",1);
	
    keys = g_hash_table_get_keys (hshCombo);
    for (current = keys; current != NULL; current = current->next) {
        if (!strncmp ((gchar *) current->data, "cmbFConfFil", 11)) {
            i = (gushort) strtol ((gchar *) current->data + 11, NULL, 10);
            key = g_strdup_printf ("%s/%d/%d", "FilterWheel", ActiveW, i);
            filter = gtk_combo_box_get_active_text (
                                 g_hash_table_lookup (hshCombo, current->data));
            W_config_s (key, filter);
            g_free (filter);
            g_free (key);
        
            key = g_strdup_printf ("%s/%d/FO%d", "FilterWheel", ActiveW, i);
            name = g_strdup_printf ("txtFConfFoc%d", i);
            get_entry_int (name, -65535, 65535, 0, NO_PAGE, &val);
            W_config_d (key, val);
            g_free (name);
            g_free (key);
        }
    }
}

void on_btnFConfRotate_clicked (GtkButton *button, gpointer data)
{
	/* Set the filter wheel to the given position */
	
	gint offset;
	gchar *filter;
	
	filter = gtk_combo_box_get_active_text (
                           g_hash_table_lookup (hshCombo, "cmbFConfSetFilter"));
	set_filter (FALSE, filter, &offset);
	g_free (filter);
	if (get_apply_filter_offset ()) {
		L_print ("{b}Applying filter focus offset of %d steps...\n", offset);
		loop_focus_apply_filter_offset (offset);
	}
}

void on_btnFConfClose_clicked (GtkButton *button, gpointer data)
{
	/* Close the filter wheel configuration window */
	
    g_hash_table_foreach_remove (hshCombo, IsFConfCombo, NULL);
	gtk_widget_destroy (xml_get_widget (xml_fil, "wndFilterConfig"));
	FilWinConf = FALSE;
	g_object_unref (G_OBJECT (xml_fil));
	xml_fil = NULL;
}

gboolean IsFConfCombo (gpointer key, gpointer value, gpointer data)
{
    return (!strncmp ((gchar *) key, "cmbFConf", 8));
}

#ifdef HAVE_UNICAP
void on_tglLVRecord_toggled (GtkButton *button, gpointer data)
{
	/* Toggle recording of the Unicap video stream on/off */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	gchar *uri = NULL, *dirname = NULL;
	
	/* If this routine is being called to reset the visual appearance of the
	 * button to the correct state, then return.
	 */
	
	if (ResetChkState) {
		ResetChkState = FALSE;
		return;
	}
	
	/* Check if destination is writeable */
	
	if (!liveview_record_is_writeable ()) {
		L_print ("{r}WARNING - Destination for recording file is not "
				 "writeable.\n");
		L_print ("{r}WARNING - Unable to start recording!\n");
		gtk_window_present (ccdApp);
		reset_checkbox_state (RCS_LV_RECORD, FALSE);
		return;
	}	
	
	/* Start recording... */
	
	aug->Record = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	
	if (aug->Record) {
		uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER 
		                            (xml_get_widget (xml_lvw, "flcVideoFile")));
		W_config_s ("Video/VideoDir", uri);
		dirname = g_filename_from_uri (uri, NULL, NULL);
		g_free (uri);		
		loop_LiveView_record (aug->Record, dirname);
		if (dirname) {
		    g_free (dirname);
			dirname = NULL;
		}
		gtk_label_set_text (
		                    GTK_LABEL (xml_get_widget (xml_lvw, "lblLVRecord")),
		                    "Stop");
		gtk_image_set_from_icon_name (
		                    GTK_IMAGE (xml_get_widget (xml_lvw, "imgLVRecord")),
                            "gtk-media-stop",
                            GTK_ICON_SIZE_BUTTON);
	} else {
	    loop_LiveView_record (aug->Record, NULL);
		gtk_label_set_text (
		                    GTK_LABEL (xml_get_widget (xml_lvw, "lblLVRecord")),
		                    "Record");
		gtk_image_set_from_icon_name (
		                    GTK_IMAGE (xml_get_widget (xml_lvw, "imgLVRecord")),
                            "gtk-media-record",
                            GTK_ICON_SIZE_BUTTON);
    }
	
	gtk_widget_set_sensitive (xml_get_widget (xml_lvw, "btnLVClose"), 
							                                      !aug->Record);
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "live_view"), 
							                                      !aug->Record);
}
#endif


/******************************************************************************/
/*                           MISCELLANEOUS FUNCTIONS                          */
/******************************************************************************/

gboolean get_entry_int (const gchar *name, gint minval, gint maxval, 
                        gint defval, gint page, gint *val)
{
	/* Return the value in the entry field as a gint.
	 * If the value isn't an integer or doesn't lie in the specified range:
	 *   if defval >= minval then
	 *     report error to the user, set value to the default and return FALSE
	 *   if defval < minval then
	 *     immediately return FALSE
	 */
	
	gint value;
	gint i, n;	
    gchar *entry_text, *message;
	gboolean ReturnDefault;
	
	/* If the passed parameter is a text string from the task list (i.e.
	 * 'page' is set to TSK_PAGE), and the task list is running in TestOnly 
	 * mode, then don't return a default value; just report an error.
	 */
	
	if (page == TSK_PAGE && (tasks_get_status() & TSK_TESTONLY))
		ReturnDefault = FALSE;
	else
		ReturnDefault = TRUE;

    /* Get a pointer to the entry text string and return if empty */

	if (!strcmp ((entry_text = get_entry_string (name)), "")) {
		if (defval < minval) return FALSE;
		if (ReturnDefault) {
			message = g_strdup_printf ("Value is empty. Using default value "
									   "of %i", defval);
			err_msg (entry_text, message);
			g_free (message);
			*val = defval;
			set_entry_int (name, *val);
		} else {
		    L_print ("{r}Data item is empty!\n");
		}
	    g_free (entry_text);
		return FALSE;
	}
	
	/* If the entry text string is apparently a task parameter (i.e. begins
	 * with '%'), then if the task list is running in TestOnly mode, return TRUE
	 * if the task parameter is valid or FALSE if it is invalid.  We return at 
	 * this point if the task list is running in TestOnly mode because the value
	 * corresponding to the task parameter may not yet have been set.
	 */
	
	entry_text = get_task_param_value (TRUE, entry_text, &n);
	if (n) {
		if (tasks_get_status () & TSK_TESTONLY) {
			g_free (entry_text);
			return n > 0 ? TRUE : FALSE;
		}
	}
	
	/* Check whether the string contains an integer; return if not */
	
	for (i = 0; i < strlen (entry_text); i++) {
		if (!isdigit (entry_text[i])) {
			if (i == 0 && !strncmp (&entry_text[i], "-", 1)) continue;
			if (ReturnDefault) {
				set_notebook_page (page);
				select_entry_region (name);
				if (defval < minval) return FALSE;
				message = g_strdup_printf (" is not an integer. Using "
										   "default value of %i", defval);
				err_msg (entry_text, message);
				g_free (message);
				*val = defval;
				set_entry_int (name, *val);
			} else {
				L_print ("{r}Data item '%s' is not a valid integer!\n", 
						 entry_text);
			}
			g_free (entry_text);
			return FALSE;
		}
	}
	
	/* Check if the entered value is within bounds; return if not */
	
	if ((value = atoi (entry_text)) < minval || value > maxval) {
		if (ReturnDefault) {
			set_notebook_page (page);
			select_entry_region (name);
			if (defval < minval) return FALSE;
			message = g_strdup_printf (" lies outside the permitted range "
									   "from %i to %i. Using default value "
									   "of %i", minval, maxval, defval);
			err_msg (entry_text, message);
			g_free (message);
			*val = defval;
			set_entry_int (name, *val);
		} else {
			L_print ("{r}Data item '%s' lies outside permitted range!\n",
					 entry_text);
		}
	    g_free (entry_text);
		return FALSE;
	}
	
    g_free (entry_text);
	*val = value;
	
	return TRUE;
}

gboolean get_entry_float (const gchar *name, 
                          gdouble minval, gdouble maxval,
                          gdouble defval, gint page, gdouble *val)
{
	/* Return the value in the entry field as a float (double).
	 * If the value isn't an integer or float or doesn't lie in the
	 * specified range, then report an error to the user, set the value to 
	 * the default and return FALSE.
	 */
	
	gint i, n, decimal;
	gint v1, v2, v3;
	gdouble value;	
    gchar c, *entry_text, *message;
	gboolean ReturnDefault;
	
	/* If the passed parameter is a text string from the task list (i.e.
	 * 'page' is set to TSK_PAGE), and the task list is running in TestOnly 
	 * mode, then don't return a default value; just report an error.
	 */
	
	if (page == TSK_PAGE && (tasks_get_status() & TSK_TESTONLY))
		ReturnDefault = FALSE;
	else
		ReturnDefault = TRUE;
	
    /* Get a pointer to the entry text string and return if empty */

	if (!strcmp ((entry_text = get_entry_string (name)), "")) {
		if (ReturnDefault) {
			message = g_strdup_printf ("Value is empty. Using default value "
									   "of %f", defval);
			err_msg (entry_text, message);
			g_free (message);
			*val = defval;
			set_entry_float (name, *val);
		} else {
		    L_print ("{r}Data item is empty!\n");
		}
	    g_free (entry_text);
		return FALSE;
	}
	
	/* If the entry text string is apparently a task parameter (i.e. begins
	 * with '%'), then if the task list is running in TestOnly mode, return TRUE
	 * if the task parameter is valid or FALSE if it is invalid.  We return at 
	 * this point if the task list is running in TestOnly mode because the value
	 * corresponding to the task parameter may not yet have been set.
	 */
	
	entry_text = get_task_param_value (TRUE, entry_text, &n);
	if (n) {
		if (tasks_get_status () & TSK_TESTONLY) {
			g_free (entry_text);
			return n > 0 ? TRUE : FALSE;
		}
	}
	
	/* Check whether the string contains an integer or float; return if not */
	
	decimal = 0;
	for (i = 0; i < strlen (entry_text); i++) {
		if (!isdigit (c = entry_text[i])) {
			if (i == 0 && c == '-') continue;
			if (c == '.' || c == ',') decimal++;
			if ((c != '.' && c != ',') || decimal > 1) {  /* At most one d.p.!*/
				if (ReturnDefault) {
					set_notebook_page (page);
					select_entry_region (name);
					message = g_strdup_printf (" is not a valid number. Using "
											   "default value of %f", defval);
					err_msg (entry_text, message);
					g_free (message);
					*val = defval;
					set_entry_float (name, *val);
				} else {
					L_print ("{r}Data item '%s' is not a valid floating "
							 "point number!\n", entry_text);
				}
				g_free (entry_text);
				return FALSE;
			}
		}
	}	
	
	/* Round entered values to 4 d.p. and perform comparison tests with
	 * scaled integer values - otherwise errors can occur when comparing
	 * small numbers.
	 */
	
	v1 = rintf (atof (entry_text) * 10000.0);
	v2 = rintf (minval * 10000.0);
	v3 = rintf (maxval * 10000.0);
	value = (gdouble) v1 / 10000.0;
	
	if (v1 < v2 || v1 > v3) {
		if (ReturnDefault) {
			set_notebook_page (page);
			select_entry_region (name);
			message = g_strdup_printf (" lies outside the permitted range "
									   "from %.3f to %.3f. Using default value "
									   "of %.3f", minval, maxval, defval);
			err_msg (entry_text, message);
			g_free (message);
			*val = defval;
			set_entry_float (name, *val);
		} else {
			L_print ("{r}Data item '%s' lies outside permitted range!\n",
					 entry_text);
		}
	    g_free (entry_text);
		return FALSE;
	}
	
    g_free (entry_text);
	*val = value; 
	
	return TRUE;
}

gchar *get_entry_string (const gchar *name)
{
	/* Return a gchar pointer to the first character in the entry field.
	 * The field can contain anything or nothing, so no need for error or 
	 * bounds checking. Assume that the maximum string length is set 
	 * appropriately by the UI.  The string must be freed by the calling
	 * routine when it's finished with.  If the passed name does not
	 * correspond to any text entry field in the UI, return the content of
	 * the string.
	 */
	
    GtkEntry *entry;
	
	if ((entry = get_entry_widget (name)))
		return g_strstrip (gtk_editable_get_chars(GTK_EDITABLE (entry), 0, -1));
	else
		return g_strdup (name);
}

static void get_spin_int (const gchar *name, gint *val)
{
	/* Return the value in the spin button as an integer */
	
	GtkSpinButton *spin;
	
	if ((spin = get_spin_widget (name))) {
    	*val = gtk_spin_button_get_value_as_int (spin);
	} else {
		*val = 0;			
		G_print ("{r}get_spin_int: Not a valid spin button: %s\n", name);
	}
}

static void get_spin_float (const gchar *name, gdouble *val)
{
	/* Return the value in the spin button as a floating point value */
	
	GtkSpinButton *spin;
	
	if ((spin = get_spin_widget (name))) {
    	*val = gtk_spin_button_get_value (spin);
	} else {
		*val = 0.0;			
		G_print ("{r}get_spin_int: Not a valid spin button: %s\n", name);
	}
}

static void set_entry_int (const gchar *name, gint val)
{
	/* Write val to the specified text field */
	
    GtkEntry *entry;
	gchar *entry_text;

	if ((entry = get_entry_widget (name))) {
		entry_text = g_strdup_printf ("%i", val);  /* Write value to widget */
		gtk_entry_set_text (entry, entry_text);
		g_free (entry_text);
	} else {
		G_print ("{r}set_entry_int: Not a valid text field: %s\n", name);
	}		
}

void set_entry_float (const gchar *name, gdouble val)
{
	/* Write val to the specified text field */
	
    GtkEntry *entry;
	gchar *entry_text;
	
	if ((entry = get_entry_widget (name))) {
		entry_text = g_strdup_printf ("%f", val);  /* Write value to widget */
		gtk_entry_set_text (entry, entry_text);
		g_free (entry_text);
	} else {
		G_print ("{r}set_entry_float: Not a valid text field: %s\n", name);
	}		
}

void set_entry_string (const gchar *name, gchar *string)
{
	/* Write string to the specified field */
	
    GtkEntry *entry;

	if ((entry = get_entry_widget (name)))
		gtk_entry_set_text (entry, string);
	else
		G_print ("{r}set_entry_string: Not a valid text field: %s\n", name);
}

static void set_spin_float (const gchar *name, gdouble val)
{
	/* Write val to the specified spin button field */
	
	GtkSpinButton *spin;
	
	if ((spin = get_spin_widget (name)))
		gtk_spin_button_set_value (spin, val);
	else
		G_print ("{r}set_spin_int: Not a valid spin button: %s\n", name);
}

static GtkEntry *get_entry_widget (const gchar *name)
{
	/* Return a pointer to the desired text entry widget, or NULL if no such
	 * widget can be found.
	 */
	
	GtkWidget *entry;
	
    /* Check each window in turn for the relevant widget */

	if (xml_app && (entry = xml_get_widget (xml_app, name)))
		goto got_entry;			
	if (xml_img && (entry = xml_get_widget (xml_img, name)))
		goto got_entry;
	if (xml_ppd && (entry = xml_get_widget (xml_ppd, name)))
		goto got_entry;
	if (xml_set && (entry = xml_get_widget (xml_set, name)))
		goto got_entry;
	if (xml_tsk && (entry = xml_get_widget (xml_tsk, name)))
		goto got_entry;			
	if (xml_agt && (entry = xml_get_widget (xml_agt, name)))
		goto got_entry;
	if (xml_lvw && (entry = xml_get_widget (xml_lvw, name)))
		goto got_entry;
	if (xml_pbw && (entry = xml_get_widget (xml_pbw, name)))
		goto got_entry;
	if (xml_pht && (entry = xml_get_widget (xml_pht, name)))
		goto got_entry;
	if (xml_con && (entry = xml_get_widget (xml_con, name)))
		goto got_entry;
	if (xml_V4L && (entry = xml_get_widget (xml_V4L, name)))
		goto got_entry;
	if (xml_fil && (entry = xml_get_widget (xml_fil, name)))
		goto got_entry;
	if (xml_fcw && (entry = xml_get_widget (xml_fcw, name)))
		goto got_entry;

	return NULL;

got_entry:
	return GTK_ENTRY (entry);		
}

static GtkSpinButton *get_spin_widget (const gchar *name)
{
	/* Return a pointer to the desired spin widget, or NULL if no such
	 * widget can be found.
	 */
	
	GtkWidget *spin;
	
    /* Check each window in turn for the relevant widget */

	if (xml_app && (spin = xml_get_widget (xml_app, name)))
			goto got_spin;			
	if (xml_img && (spin = xml_get_widget (xml_img, name)))
			goto got_spin;					
	if (xml_ppd && (spin = xml_get_widget (xml_ppd, name)))
			goto got_spin;					
	if (xml_set && (spin = xml_get_widget (xml_set, name)))
			goto got_spin;					
	if (xml_tsk && (spin = xml_get_widget (xml_tsk, name)))
			goto got_spin;			
	if (xml_agt && (spin = xml_get_widget (xml_agt, name)))
			goto got_spin;
	if (xml_lvw && (spin = xml_get_widget (xml_lvw, name)))
			goto got_spin;
	if (xml_pbw && (spin = xml_get_widget (xml_pbw, name)))
			goto got_spin;
	if (xml_pht && (spin = xml_get_widget (xml_pht, name)))
			goto got_spin;
	if (xml_con && (spin = xml_get_widget (xml_con, name)))
			goto got_spin;
	if (xml_V4L && (spin = xml_get_widget (xml_V4L, name)))
		goto got_spin;
	if (xml_fil && (spin = xml_get_widget (xml_fil, name)))
			goto got_spin;
	if (xml_fcw && (spin = xml_get_widget (xml_fcw, name)))
			goto got_spin;

	return NULL;

got_spin:
	return GTK_SPIN_BUTTON (spin);		
}

void set_ccd_gui (gboolean set)
{
	/* Set the various GUI elements depending on the capabilities of the
	 * selected CCD camera.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	if (set) {
		set_entry_int ("txtH1", 1);
		set_entry_int ("txtV1", 1);
		set_entry_int ("txtH2", ccd->cam_cap.max_h);
		set_entry_int ("txtV2", ccd->cam_cap.max_v);
		set_entry_int ("txtHBin", 1);
		set_entry_int ("txtVBin", 1);
		set_spin_int ("spbBin", 1);
		gtk_spin_button_set_range (get_spin_widget ("spbBin"), 1, 
						    MIN (ccd->cam_cap.max_binh, ccd->cam_cap.max_binv));
		gtk_widget_set_sensitive (GTK_WIDGET (get_entry_widget ("txtHBin")),
								                       ccd->cam_cap.CanAsymBin);
		gtk_widget_set_sensitive (GTK_WIDGET (get_entry_widget ("txtVBin")),
								                       ccd->cam_cap.CanAsymBin);
		if (R_config_d (CCDConfigKey (ccd, "Debayer"), 0))
			gtk_widget_set_sensitive (xml_get_widget (xml_app, "debayer"),TRUE);
	} else {
		set_entry_int ("txtH1", 1);
		set_entry_int ("txtV1", 1);
		set_entry_int ("txtH2", 1000);
		set_entry_int ("txtV2", 1000);
		set_entry_int ("txtHBin", 1);
		set_entry_int ("txtVBin", 1);
		set_spin_int ("spbBin", 1);
		gtk_spin_button_set_range (get_spin_widget ("spbBin"), 1, 5); 
		gtk_widget_set_sensitive (xml_get_widget (xml_app, "tblCCD"), FALSE);
		gtk_widget_set_sensitive (xml_get_widget (xml_app, "debayer"), FALSE);
	}
	gtk_widget_set_sensitive (xml_get_widget(xml_app, "ccd_temperatures"), set);
}

void set_exposure_buttons (gboolean active)
{
	/* Set the exposure control buttons to their active/inactive state */

	struct cam_img *ccd = get_ccd_image_struct ();
	
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "btnStart"), !active);
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "btnCancel"),
                                              ccd->cam_cap.CanAbort & active);
	gtk_widget_set_sensitive (xml_get_widget (xml_app, "btnInterrupt"), 
                                              ccd->cam_cap.CanStop & active);
}	

void set_task_buttons (gboolean active)
{
	/* Set the task control buttons to their active/inactive state */
	
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskStart"), !active);
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskPause"),  active);
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskStop"),   active);	
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskEdit"),  !active);
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskDelete"),!active);
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskClear"), !active);	
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskUp"),    !active);	
	gtk_widget_set_sensitive (xml_get_widget (xml_app,"btnTaskDown"),  !active);	
}

void set_focus_done (void)
{
	/* Query for the current position and write message to log window */
	
	struct focus f;
	
	if (focus_comms->user & PU_FOCUS) {  /* Just in case focuser link closed */
		f.cmd = FC_CUR_POS_GET;	
		focus_comms->focus (&f);
		if (!f.Error)
			set_entry_int ("txtFocusCurrentPos", f.cur_pos);
		else
			L_print ("{r}Error querying current position for focuser\n");
	}
	
	L_print ("{b}Focuser stopped\n");
	tasks_task_done (T_FOC);
}

void set_progress_bar (gboolean zeroise, gint elapsed)
{
	/* Set the progress bar on the main application window during an exposure */

	struct cam_img *ccd = get_ccd_image_struct ();
	
	static gint init_elapsed;
	static gdouble p;
	gchar *s;
	static gboolean first_call = TRUE;
	
	if (zeroise) {
		p = 0.0;
		gtk_progress_bar_set_fraction (prgAppBar, p);
		set_status_bar (stsAppStatus, "", TRUE);
		first_call = TRUE;
		return;
	} else {
		if (first_call) {
			init_elapsed = elapsed;
			first_call = FALSE;
		}
		p = (gdouble) (elapsed - init_elapsed)/(1000.0 * ccd->exd.req_len);
		if (p > 1.0)
			p = 1.0;
	}
	
	gtk_progress_bar_set_fraction (prgAppBar, p);
	s = g_strdup_printf ("   Elapsed time: %d s", (gint)(p * ccd->exd.req_len));
	set_status_bar (stsAppStatus, s, FALSE);
	g_free (s);
	while (g_main_context_iteration (NULL, FALSE));
}

void set_status_bar (GtkStatusbar *bar, gchar *message, gboolean Clear)
{
	/* Set the given status bar to the given string */
	
	guint context_id;
	
	context_id = gtk_statusbar_get_context_id (bar, "");
	gtk_statusbar_pop (bar, context_id);
	if (!Clear)
		gtk_statusbar_push (bar, context_id, message);
}

void set_range_minmaxstep (enum Range range, gdouble min, gdouble max, 
						   gdouble step, gushort dp)
{
	/* Set range slider min, max and step size */
	
	GtkWidget *hscScale = NULL;

	if (xml_img != NULL) { 
		switch (range) {
			
			case AIW_BACKGROUND:
				hscScale = xml_get_widget (xml_img, "hscBackground");
				break;
			case AIW_BRIGHTNESS:
				hscScale = xml_get_widget (xml_img, "hscBrightness");
				break;
			case AIW_CONTRAST:
				hscScale = xml_get_widget (xml_img, "hscContrast");
				break;
			case AIW_GAMMA:
				hscScale = xml_get_widget (xml_img, "hscGamma");
				break;
			case AIW_GAIN:
				hscScale = xml_get_widget (xml_img, "hscGain");
				break;
		}
		
		if (hscScale) {
			gtk_range_set_range (GTK_RANGE (hscScale), min, max);
			gtk_range_set_increments (GTK_RANGE (hscScale), 0.0, step);
			gtk_scale_set_digits (GTK_SCALE (hscScale), dp);
		}
    }
}

void set_range_value (enum Range range, gboolean Sensitive, gdouble value)
{
	/* Set the appropriate range slider to the given value.  If the supplied
	 * value is less than zero, then ignore it.
	 */
	
	GtkWidget *hscScale = NULL, *lblScale = NULL;

	if (xml_img != NULL) { 
		switch (range) {
			
			case AIW_BACKGROUND:
				hscScale = xml_get_widget (xml_img, "hscBackground");
				lblScale = xml_get_widget (xml_img, "lblBackground");
				break;
			case AIW_BRIGHTNESS:
				hscScale = xml_get_widget (xml_img, "hscBrightness");
				lblScale = xml_get_widget (xml_img, "lblBrightness");
				break;
			case AIW_CONTRAST:
				hscScale = xml_get_widget (xml_img, "hscContrast");
				lblScale = xml_get_widget (xml_img, "lblContrast");
				break;
			case AIW_GAMMA:
				hscScale = xml_get_widget (xml_img, "hscGamma");
				lblScale = xml_get_widget (xml_img, "lblGamma");
				break;
			case AIW_GAIN:
				hscScale = xml_get_widget (xml_img, "hscGain");
				lblScale = xml_get_widget (xml_img, "lblGain");
				break;
			default:
				break;
		}

		if (hscScale) {
			if (value >= 0.0)
				gtk_range_set_value (GTK_RANGE (hscScale), value);
			gtk_widget_set_sensitive (hscScale, Sensitive);
			gtk_widget_set_sensitive (lblScale, Sensitive);
		}
    }
}

static void set_notebook_page (gint page)
{
	/* Simple helper function to set the notebook page */
	
	GtkWidget *ntbNotebook;
	
	ntbNotebook = xml_get_widget (xml_app, "ntbMainNotebook");			
	gtk_notebook_set_current_page (GTK_NOTEBOOK (ntbNotebook), page);
}

void set_elapsed_time (guint elapsed)
{
	/* Set the elapsed time on the 'Tasks' tab */
	
	set_entry_int ("txtElapsed", elapsed);
}

void set_fits_data (struct cam_img *img, struct timeval *time, 
	                gboolean UseDateobs, gboolean QueryHardware)
{
	/* Set the date, time, telescope pointing information (optional) and focus
	 * information to appear in the FITS header.  
	 * If UseDateobs is FALSE, then set img->fits.date_obs to the present time
	 * in this routine.
	 * If UseDateobs is TRUE, then:
	 *           if time is set to a timeval, construct fits.date_obs from that.
	 *           if time is NULL, assume that fits.date_obs has already been set
	 *           by the calling routine.
	 * If QueryHardware is true, query the telescope link for the current RA and 
	 * Dec values, and get the current position and temperature from the 
	 * focuser.
	 */
	
	struct focus f;
	struct tm *dt = NULL;	
	gfloat h, m, s;
	gdouble ignore1, ignore2;
	gchar **tokens;
	gchar *endm, *ends;
	gboolean ignore3, ignore4;
	
	if (!UseDateobs) {
		dt = get_time (TRUE);
		sprintf (img->fits.date_obs, "%4i-%02i-%02iT%02i:%02i:%02i", 
				 1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday,
				 dt->tm_hour, dt->tm_min, dt->tm_sec);
		sprintf (img->fits.utstart, "%02i:%02i:%02i", dt->tm_hour, 
				                                      dt->tm_min, dt->tm_sec);
		img->fits.tm_start = dt->tm_sec + 60.0*(dt->tm_min + 60.0*dt->tm_hour);
	} else {
		
		if (time) {
			dt = gmtime (&time->tv_sec);
			sprintf (img->fits.date_obs, "%4i-%02i-%02iT%02i:%02i:%02i.%03i", 
					 1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday,
					 dt->tm_hour, dt->tm_min, dt->tm_sec, 
					 (int) ((double) time->tv_usec / 1.0e3 + 0.5));
		}
		
		tokens = g_strsplit (img->fits.date_obs, "T", 2);
		strcpy  (img->fits.utstart, tokens[1]);
		g_strfreev (tokens);
	
		h = strtod (img->fits.utstart, &endm);
		m = strtod (++endm, &ends);
		s = strtod (++ends, (gchar **) NULL);
		img->fits.tm_start = s + 60.0 * (m + 60.0 * h);
	}
	
	if (QueryHardware) {
		
		/* Get RA and Dec from telescope controller.  This routine prints a
		 * warning and sets the values to zero if the link is not open.
		 */
		
		telescope_get_RA_Dec (menu.Precess, &img->fits.epoch, 
						      img->fits.RA, img->fits.Dec, 
							  &ignore1, &ignore2, &ignore3, &ignore4);
		
		/* No need to warn if focuser not open/available.  Just set values
		 * silently to zero.
		 */
		
		if (focus_comms->user & PU_FOCUS) {
			f.cmd = FC_VERSION;
			focus_comms->focus (&f);
			if (f.version >= 3.0)
				f.cmd = FC_CUR_POS_GET | FC_TEMP_GET;
			else
				f.cmd = FC_CUR_POS_GET;
			focus_comms->focus (&f);
			if (!f.Error) {
				img->fits.focus_pos = f.cur_pos;
				if (f.version >= 3.0)
					img->fits.focus_temp = f.temp;
			    else
					img->fits.focus_temp = 0.0;
			}
		} else {
			img->fits.focus_pos = 0;
			img->fits.focus_temp = 0.0;
		}
	} else {
		sprintf (img->fits.RA, "00:00:00");
		sprintf (img->fits.Dec, "+00:00:00");
		img->fits.focus_pos = 0;
		img->fits.focus_temp = 0.0;
	}
}

void set_autog_on (gboolean on)
{
	/* Turn autoguider camera on/off (called from task list) */

	struct cam_img *aug = get_aug_image_struct ();	
	
	GtkToggleButton *button;
	gboolean state;
	
	/* Don't turn the autoguider off if we're guiding! */
	
	if (on == FALSE && aug->autog.Guide) {
		L_print ("{o}Can't turn off autoguider while guiding!\n");
		return;
	}

	/* If the button is already active and we ask it to be set active (and vice-
	 * versa), the toggled signal never gets emitted, so we have to treat that 
	 * case here rather than issuing a warning when the instruction to turn the
	 * autoguider camera on/off reaches the loop module.
	 */
	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogOpen"));	
	state = gtk_toggle_button_get_active (button);
	if (on == state) {
		L_print ("{o}Warning - autoguider camera is already %s\n", 
		                                                     (on ? "on":"off"));
		tasks_task_done (on ? T_AGN : T_AGF);
		return;
	}
	 
	gtk_toggle_button_set_active (button, (on ? TRUE : FALSE));
}

void set_guide_on (gboolean on)
{
	/* Turn guiding on/off (called from task list and by function keypress) */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	GtkToggleButton *button;
	
	/* Check that the autoguider camera is actually running!  (Not obvious from
	 * task list).
	 */
	
	if (!aug->Open) {
		L_print ("{o}Can't %s guiding if autoguider camera isn't running!\n",
				                                       (on ? "start" : "stop"));
		tasks_task_done (on ? T_GST : T_GSP);
		return;
	}
	
	/* If the guiding button is already active and we ask it to be set active 
	 * (and vice-versa), the toggled signal never gets emitted, so we have to 
	 * treat that case here rather than issuing a warning when the instruction 
	 * to turn guiding on/off reaches the loop module.
	 */
	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app,"tglAutogStart"));	

	if (on == gtk_toggle_button_get_active (button)) {
		L_print ("{o}Warning - guiding is already %s\n", (on ? "on" : "off"));
		tasks_task_done (on ? T_GST : T_GSP);
		return;
	}
	
	/* Now start/stop autoguiding */
	
	gtk_toggle_button_set_active (button, on);			
}

void get_autog_guide_params (void)
{
	/* Get the UI settings for the autoguider guiding paramaters */
	
	struct cam_img *aug = get_aug_image_struct ();

	GtkToggleButton *button;
	gdouble val;
	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideNorth"));
	if (gtk_toggle_button_get_active (button))
		aug->autog.s.GuideDirn |= TM_NORTH;
	else
		aug->autog.s.GuideDirn &= ~TM_NORTH;
			
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideSouth"));
	if (gtk_toggle_button_get_active (button))
		aug->autog.s.GuideDirn |= TM_SOUTH;
	else
		aug->autog.s.GuideDirn &= ~TM_SOUTH;
		
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideEast"));
	if (gtk_toggle_button_get_active (button))
		aug->autog.s.GuideDirn |= TM_EAST;
	else
		aug->autog.s.GuideDirn &= ~TM_EAST;
			
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideWest"));
	if (gtk_toggle_button_get_active (button))
		aug->autog.s.GuideDirn |= TM_WEST;
	else
		aug->autog.s.GuideDirn &= ~TM_WEST;	
		
	get_entry_float("txtAutogNSCalibDuration", 0.1, 100.0, 1.0, AUG_PAGE, &val);
	aug->autog.s.NSCalibDuration = val;
	get_entry_float("txtAutogEWCalibDuration", 0.1, 100.0, 1.0, AUG_PAGE, &val);
	aug->autog.s.EWCalibDuration = val;
	get_spin_float ("spbAutogGuideSpeed", &val);
	aug->autog.s.GuideSpeed = (gfloat) val;
	get_spin_float ("spbAutogMaxShift", &val);
	aug->autog.s.MaxShift = (gfloat) val;
	get_spin_float ("spbAutogMaxDrift", &val);
	aug->autog.s.MaxDrift = (gfloat) val;
	get_spin_float ("spbAutogMaxMove", &val);
	aug->autog.s.MaxMove = (gfloat) val;
	get_spin_float ("spbAutogMaxOffset", &val);
	aug->autog.s.MaxOffset = (gfloat) val;
	get_spin_float ("spbAutogDriftSample", &val);
	aug->autog.s.DriftSample = (gfloat) val;
	get_spin_float ("spbAutogCorrFac", &val);
	aug->autog.s.CorrFac = (gfloat) val;
	get_entry_float ("txtAutogUpdate", 0.0, 10.0, 0.5, AUG_PAGE, &val);
	aug->autog.s.Update = (gfloat) val;	

	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app,"chkAutogSimulGuide"));
	aug->autog.s.SimulGuide = gtk_toggle_button_get_active (button);
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app,"chkAutogDriftNSOnly"));	
	aug->autog.s.DriftNSOnly = gtk_toggle_button_get_active (button);
	button = GTK_TOGGLE_BUTTON (xml_get_widget(xml_app,"chkAutogRemoteTiming"));
	aug->autog.s.RemoteTiming = gtk_toggle_button_get_active (button);
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogDecCorrect"));
	aug->autog.s.DecCorr = gtk_toggle_button_get_active (button);
}

void get_autog_movespeed (gboolean *CenterSpeed, gfloat *speed)
{
	/* Return the speed that the telescope should move at when the motion 
	 * buttons are pressed on the autoguider tab.  If the telescope link is 
	 * open, this is governed by the 'Use centering speed' checkbox.  If the 
	 * telescope link is closed, use the guide speed.
     */
	
	gint val;
	gdouble dval;
	
	*CenterSpeed = gtk_toggle_button_get_active (
	   GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogCenterSpeed")));
	
	if ((tel_comms->user & PU_TEL) && *CenterSpeed) {
		get_spin_int ("spbAutogCenterSpeed", &val);
		*speed = (gfloat) val;
	} else {
		get_spin_float ("spbAutogGuideSpeed", &dval);
		*speed = (gfloat) dval;
	}
}

void set_autog_sensitive (gboolean sensitive, gboolean autogopened)
{
	/* Reset fields to default values if the autoguider camera is disabled and
	 * grey-out/un-grey autoguider controls.
	 * sensitive is set to TRUE in the main event loop if the controls are to be
	 * un-greyed.
	 * autogopened is set to TRUE in the main event loop if this routine is
	 * being called as a result of the 'Use autoguider' check box being clicked,
	 * and is set to FALSE if the 'Start autoguiding' toggle button has been
	 * clicked.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	GtkWidget *widget, *label;
	gboolean Sensitise;
	
	if (!sensitive && autogopened) { /* Set/re-set some checkbox states */
		widget = (xml_get_widget (xml_app, "chkAutogDecCorrect"));	
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
		widget = (xml_get_widget (xml_app, "chkAutogWrite"));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
		widget = (xml_get_widget (xml_app, "chkAutogWorm"));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
		widget = (xml_get_widget (xml_app, "chkAutogSubDark"));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
		widget = (xml_get_widget (xml_app, "chkAutogShowCentroid"));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
	}
	
	if (autogopened) {
		widget = xml_get_widget (xml_app, "tglAutogStart");
		gtk_widget_set_sensitive (widget, sensitive);
	}
	
	if (!autogopened) {
		widget = xml_get_widget (xml_app, "chkAutogOpen");
		gtk_widget_set_sensitive (widget, sensitive);
		widget = xml_get_widget (xml_img, "lblImgBin");
		gtk_widget_set_sensitive (widget, sensitive);
		widget = xml_get_widget (xml_img, "optImgBin1x1");
		gtk_widget_set_sensitive (widget, sensitive);
		widget = xml_get_widget (xml_img, "optImgBin2x2");
		gtk_widget_set_sensitive (widget, sensitive);
	}
	
	widget = xml_get_widget (xml_app, "chkAutogRemoteTiming");
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "btnAutogEast");
	gtk_widget_set_sensitive (widget, sensitive);
	widget = xml_get_widget (xml_app, "btnAutogWest");
	gtk_widget_set_sensitive (widget, sensitive);
	widget = xml_get_widget (xml_app, "btnAutogNorth");
	gtk_widget_set_sensitive (widget, sensitive);
	widget = xml_get_widget (xml_app, "btnAutogSouth");
	gtk_widget_set_sensitive (widget, sensitive);	
	
	widget = xml_get_widget (xml_app, "btnAutogCalibrate");
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "chkAutogAutoCalibrate");
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogNSCalibDuration");
	widget = xml_get_widget (xml_app, "txtAutogNSCalibDuration");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogEWCalibDuration");
	widget = xml_get_widget (xml_app, "txtAutogEWCalibDuration");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "chkAutogDecCorrect");
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "chkAutogCenterSpeed");
	gtk_widget_set_sensitive (widget, sensitive);
	
	if (GeminiCmds) {
		label = xml_get_widget (xml_app, "lblAutogCenterSpeed");
		widget = xml_get_widget (xml_app, "spbAutogCenterSpeed");
		gtk_widget_set_sensitive (label, sensitive);
		gtk_widget_set_sensitive (widget, sensitive);
	
		label = xml_get_widget (xml_app, "lblAutogGuideSpeed");
		widget = xml_get_widget (xml_app, "spbAutogGuideSpeed");
		gtk_widget_set_sensitive (label, sensitive);
		gtk_widget_set_sensitive (widget, sensitive);
	}

	label = xml_get_widget (xml_app, "lblAutogMaxShift");
	widget = xml_get_widget (xml_app, "spbAutogMaxShift");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogMaxDrift");
	widget = xml_get_widget (xml_app, "spbAutogMaxDrift");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);	
	
	label = xml_get_widget (xml_app, "lblAutogDriftSample");
	widget = xml_get_widget (xml_app, "spbAutogDriftSample");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogMaxMove");
	widget = xml_get_widget (xml_app, "spbAutogMaxMove");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);

	label = xml_get_widget (xml_app, "lblAutogMaxOffset");
	widget = xml_get_widget (xml_app, "spbAutogMaxOffset");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogCorrFac");
	widget = xml_get_widget (xml_app, "spbAutogCorrFac");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogUpdate");
	widget = xml_get_widget (xml_app, "txtAutogUpdate");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "chkAutogGuideNorth");	
	gtk_widget_set_sensitive (widget, sensitive);
	widget = xml_get_widget (xml_app, "chkAutogGuideSouth");	
	gtk_widget_set_sensitive (widget, sensitive);	
	widget = xml_get_widget (xml_app, "chkAutogGuideEast");	
	gtk_widget_set_sensitive (widget, sensitive);	
	widget = xml_get_widget (xml_app, "chkAutogGuideWest");	
	gtk_widget_set_sensitive (widget, sensitive);
	widget = xml_get_widget (xml_app, "chkAutogSimulGuide");	
	gtk_widget_set_sensitive (widget, sensitive);
	widget = xml_get_widget (xml_app, "chkAutogDriftNSOnly");	
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "btnAutogCapDark");
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "txtAutogDarkExps");
	gtk_widget_set_sensitive (widget, sensitive);	
	
	widget = xml_get_widget (xml_app, "chkAutogSubDark");
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogSkySigma");
	widget = xml_get_widget (xml_app, "txtAutogSkySigma");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	label = xml_get_widget (xml_app, "lblAutogCentroidSize");
	widget = xml_get_widget (xml_app, "txtAutogCentroidSize");
	gtk_widget_set_sensitive (label, sensitive);
	gtk_widget_set_sensitive (widget, sensitive);
	
	widget = xml_get_widget (xml_app, "chkAutogShowCentroid");
	gtk_widget_set_sensitive (widget, sensitive);

	widget = xml_get_widget (xml_app, "chkAutogWrite");
	gtk_widget_set_sensitive (widget, sensitive);
	
	/*** Telescope port open, 'Write star pos?' checked and Gemini commands ***/
	
	Sensitise = (sensitive && tel_comms->user & PU_TEL && aug->autog.Write &&
				 GeminiCmds);
	widget = xml_get_widget (xml_app, "chkAutogWorm");
	gtk_widget_set_sensitive (widget, Sensitise);
	
	/**************************************************************************/
	
	widget = xml_get_widget (xml_app, "btnAutogDS9");
	gtk_widget_set_sensitive (widget, sensitive);

	widget = xml_get_widget (xml_img, "btnResetArea");
	gtk_widget_set_sensitive (widget, sensitive);	
}

void set_autog_calibrate_done (void)
{
	/* Reset the 'Calibrate autoguider' button.  This is called only when the
	 * calibration process has successfully completed, and has equivalent
	 * effect to a user clicking the button part way through a calibration
	 * procedure.
	 */
	
	gtk_widget_activate (xml_get_widget (xml_app, "btnAutogCalibrate"));
}

gboolean ui_control_action (gchar *cmd, gchar *token)
{
	/* Activate/deactivate the given control.
	 * (Note: text boxes should have their value set to 'token' and then
	 *  be activated).
	 */
	
	GtkWidget *w = NULL;
	
	if (!(w = xml_get_widget (xml_app, cmd)))
		return FALSE;  /* Widget doesn't exist */
	
	/* For now, we just do menu items */
	
	if (!strcmp (token, "1")) {
		if (!strcmp (G_OBJECT_TYPE_NAME (w), "GtkCheckMenuItem")) {
			if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (w)))
				gtk_menu_item_activate (GTK_MENU_ITEM (w));
		} else
			gtk_menu_item_activate (GTK_MENU_ITEM (w));
	} else if (!strcmp (token, "0")) {
		if (!strcmp (G_OBJECT_TYPE_NAME (w), "GtkCheckMenuItem")) {
			if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (w)))
				gtk_menu_item_activate (GTK_MENU_ITEM (w));
		}
	} else {
		L_print ("{r}Unrecognised command: '%s' for GUI control\n", token); 
		return FALSE;
	}
		
	return TRUE;
}

static void canvas_button_press (GtkWidget *widget, GdkEventButton *event, 
                                 struct cam_img *img)
{
    /* Perform drawing functions on the given image canvas */
        
    gint width, height;
    
    /* X automatically grabs the pointer for windows with button press and
     * button release events specified, but we can constrain the pointer to
     * remain within the window by explicitly 'grabbing' here and setting
     * appropriate options.
     */
  
    gdk_pointer_grab (gtk_widget_get_window (widget),
                      TRUE,
                      GDK_POINTER_MOTION_MASK,
                      gtk_widget_get_window (widget),
                      NULL,
                      GDK_CURRENT_TIME);
                      
    if (!(event->state & GDK_SHIFT_MASK)) { /* Start to draw new rectangle*/
    
        img->canv.r.new_htl = (gint) (event->x / img->canv.zoom);
        img->canv.r.new_vtl = (gint) (event->y / img->canv.zoom);
        img->canv.r.htl = img->canv.r.new_htl;
        img->canv.r.vtl = img->canv.r.new_vtl;
        img->canv.r.hbr = img->canv.r.htl + 1;
        img->canv.r.vbr = img->canv.r.vtl + 1;
        g_object_set (img->canv.cviRect,
                      "x", (gdouble) img->canv.r.htl,
                      "y", (gdouble) img->canv.r.vtl,
                      "width", 1.0,
                      "height", 1.0,
                      NULL);
        img->canv.NewRect = TRUE;
        
    } else {                         /* Snap existing rectangle to cursor */
    
        width = img->canv.r.hbr - img->canv.r.htl;
        height = img->canv.r.vbr - img->canv.r.vtl;
        img->canv.r.htl = MAX (0, 
                           (gint) (event->x / img->canv.zoom) - width / 2),
        img->canv.r.vtl = MAX (0, 
                           (gint) (event->y / img->canv.zoom) - height / 2),
        img->canv.r.hbr = img->canv.r.htl + width;
        img->canv.r.vbr = img->canv.r.vtl + height;
        g_object_set (img->canv.cviRect,
                    "x", (gdouble) img->canv.r.htl,
                    "y", (gdouble) img->canv.r.vtl,
                    "width", (gdouble) width,
                    "height", (gdouble) height,
                    NULL);
        img->canv.NewRect = TRUE;
        
    }
}  

void canvas_button_release (struct cam_img *img)
{
    /* Tidy up after drawing on given canvas */
    
    gdk_pointer_ungrab (GDK_CURRENT_TIME);
    img->canv.NewRect = TRUE;
}

void canvas_motion_notify (GdkEventMotion *event, struct cam_img *img)
{
    /* Perform drawing operations on given canvas */
    
    gint width, height;
    
	if (event->state & GDK_BUTTON1_MASK) {
        if (!(event->state & GDK_SHIFT_MASK)) { /* Draw rectangle lower right */
        
            if (img->canv.cursor_x >= img->canv.r.new_htl && 
                img->canv.cursor_y >= img->canv.r.new_vtl) {
                img->canv.r.htl = img->canv.r.new_htl;
                img->canv.r.vtl = img->canv.r.new_vtl;
                img->canv.r.hbr = img->canv.cursor_x + 1;
                img->canv.r.vbr = img->canv.cursor_y + 1;
                g_object_set (img->canv.cviRect,
                        "x", (gdouble) img->canv.r.htl,
                        "y", (gdouble) img->canv.r.vtl,
                        "width", (gdouble) (img->canv.r.hbr - img->canv.r.htl),
                        "height", (gdouble) (img->canv.r.vbr - img->canv.r.vtl),
                        NULL);
                img->canv.NewRect = TRUE;
            }
            
        } else {                                            /* Move rectangle */
            
            width = img->canv.r.hbr - img->canv.r.htl;
            height = img->canv.r.vbr - img->canv.r.vtl;
            img->canv.r.htl = MAX (0, img->canv.cursor_x - width / 2),
            img->canv.r.vtl = MAX (0, img->canv.cursor_y - height / 2),
            img->canv.r.hbr = img->canv.r.htl + width;
            img->canv.r.vbr = img->canv.r.vtl + height;
			g_object_set (img->canv.cviRect,
                        "x", (gdouble) img->canv.r.htl,
                        "y", (gdouble) img->canv.r.vtl,
                        "width", (gdouble) width,
                        "height", (gdouble) height,
                        NULL);
            img->canv.NewRect = TRUE;
        }
    }
}

static void canvas_zoom_image (GtkWidget *canv, struct cam_img *img, 
                        enum ZoomType zoom)
{
	/* Zoom the image */
	
	switch (zoom) {
		
		case ZOOM_IN:
            img->canv.zoom *= 2.0;
            break;
            
        case ZOOM_OUT:
           img->canv.zoom /= 2.0;
           break;
	
		case ZOOM_1x1:
			img->canv.zoom = 1.0;
			break;
			
		case ZOOM_2x2:
			img->canv.zoom = 2.0;
			break;
	}
	
    goo_canvas_set_scale (GOO_CANVAS (canv), img->canv.zoom);
    goo_canvas_scroll_to (GOO_CANVAS (canv), 
      ((img->canv.r.htl + img->canv.r.hbr) * 
        img->canv.zoom - canv->allocation.width) / (2.0 * img->canv.zoom),
      ((img->canv.r.vtl + img->canv.r.vbr) * 
        img->canv.zoom - canv->allocation.height) / (2.0 * img->canv.zoom));
        
    switch (img->id) {
        
        /* Keep line widths the same size irrespective of zoom */
        
        case AUG:
            g_object_set (G_OBJECT (img->canv.cviRect),
                          "line-width", 2.0 / img->canv.zoom,
                          NULL);           
            g_object_set (G_OBJECT (img->canv.chx_line),
                          "line-width", 2.0 / img->canv.zoom,
                          NULL);           
            g_object_set (G_OBJECT (img->canv.chy_line),
                          "line-width", 2.0 / img->canv.zoom,
                          NULL);
            break;
        case VID:
            g_object_set (G_OBJECT (img->canv.cviRect),
                          "line-width", 2.0 / img->canv.zoom,
                          NULL);           
            if (cgpPhotom)
                g_object_set (G_OBJECT (cgpPhotom), 
                              "line-width", 1.0 / img->canv.zoom,
                              NULL);
            break;
            
        default:
            ;
    }
}

gboolean ui_show_augcanv_image (void)
{
	/* Show the new image on the canvas.  This may not be the best way to stream
	 * lots of images, but goo_canvas offers useful functionality 'for free' 
	 * e.g. zooming and scrolling.  It seems to be fast enough.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	static GdkPixbuf *image = NULL;
	
	if (!aug->disp083) {
		L_print ("{o}Requested to show autoguider image, but image data "
		         "unavailable\n");
		return FALSE;
	}
	
	/* Destroy previous pixbuf */
	
	if (image != NULL) {	
		g_object_unref (image);
		image = NULL;
	}

	/* Create new pixbuf */

	if (!(image = gdk_pixbuf_new_from_data (aug->disp083,
		                                    GDK_COLORSPACE_RGB, 
	                                        FALSE,
	                                        8,
	                                        aug->exd.h_pix,
	                                        aug->exd.v_pix,
	                                        aug->exd.h_pix * 3,
	                                        NULL,
	                                        NULL)))
		return show_error (__func__, "Unable to create image from data");
	
	/* Display image on canvas */
	
	if (aug->canv.cviImage)	/* Canvas image already exists */
		g_object_set (aug->canv.cviImage, "pixbuf", image, NULL);
	else                    /* Create new canvas image item */		
		aug->canv.cviImage = goo_canvas_image_new (cgpImage,
						                           image,
	                                               (gdouble) aug->exd.h_top_l,
	                                               (gdouble) aug->exd.v_top_l,
	                                               NULL);
						   
	/* Set the image to just below the selection rectangle */
	
	goo_canvas_item_lower (aug->canv.cviImage, aug->canv.cviRect);
	
	/* Show status bar info */
	
	ui_show_status_bar_info ();
	
	while (g_main_context_iteration (NULL, FALSE));
	
	return TRUE;
}

void ui_show_augcanv_rect (gboolean Show)
{
	/* Show/hide the autoguider image selection rectangle */
	
	struct cam_img *aug = get_aug_image_struct ();
	
    g_object_set (G_OBJECT (aug->canv.cviRect), "line-width", 
                  Show ? 2.0 / aug->canv.zoom : 0.0, NULL);
}

void ui_set_augcanv_rect_colour (gchar *colour)
{
	/* Set the colour of the autoguider selection rectangle */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	g_object_set (G_OBJECT (aug->canv.cviRect),
                  "stroke-color", colour,
                  NULL);
	while (g_main_context_iteration (NULL, FALSE));
}

void ui_set_augcanv_rect_full_area (void)
{
	/* Set the selection rectangle to the full image area */
	
	struct cam_img *aug = get_aug_image_struct ();
    
	aug->canv.r.htl = aug->exd.h_top_l;
	aug->canv.r.vtl = aug->exd.v_top_l;
	aug->canv.r.hbr = aug->exd.h_top_l + aug->exd.h_pix;
	aug->canv.r.vbr = aug->exd.v_top_l + aug->exd.v_pix;
	aug->canv.NewRect = TRUE;
    
	g_object_set (G_OBJECT (aug->canv.cviRect),
                  "x", (gdouble) aug->canv.r.htl,
                  "y", (gdouble) aug->canv.r.vtl, 
                  "width", (gdouble) aug->canv.r.hbr - aug->canv.r.htl + 1,
                  "height", (gdouble) aug->canv.r.vbr - aug->canv.r.vtl + 1,
                  NULL);
}

void ui_set_augcanv_crosshair (gdouble x, gdouble y)
{
	/* Draw the autoguider crosshair at (x, y) in window coordinates, rotated
	 * according to the most recent calibration of the autoguider camera.  If
	 * x is negative, then set x to be the centre of the currently displayed
	 * autoguider image (if there is one), likewise for y.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	GooCanvasPoints *cvpLine;
	gdouble rot;
	
	if (x < 0 && aug->Open)
		x = aug->exd.h_top_l + aug->exd.h_pix / 2.0;
	if (y < 0 && aug->Open)
		y = aug->exd.v_top_l + aug->exd.v_pix / 2.0;
		
	rot = aug->autog.s.Uvec_E[1] / aug->autog.s.Uvec_E[0];
	
	cvpLine = goo_canvas_points_new (2);  /* x cross-hair... */
	cvpLine->coords[0] = 0;
	cvpLine->coords[1] = y - x * rot;
	cvpLine->coords[2] = (gdouble) AUGCANV_H;
	cvpLine->coords[3] = y + (AUGCANV_H - x) * rot;
	if (aug->canv.chx_line)
        g_object_set (G_OBJECT (aug->canv.chx_line), "points", cvpLine, NULL);
    else
        aug->canv.chx_line = goo_canvas_polyline_new (
											cgpImage,
                                            FALSE,
                                            0,
                                            "points", cvpLine,
                                            "stroke-color", "red",
                                            "line-width", 2.0,
                                            NULL);
	goo_canvas_points_unref (cvpLine);

	cvpLine = goo_canvas_points_new (2);   /* y cross-hair... */
	cvpLine->coords[0] = x + y * rot;
	cvpLine->coords[1] = 0;
	cvpLine->coords[2] = x - (AUGCANV_V - y) * rot;
	cvpLine->coords[3] = (gdouble) AUGCANV_V;
	if (aug->canv.chy_line)
        g_object_set (G_OBJECT (aug->canv.chy_line), "points", cvpLine, NULL);
    else
        aug->canv.chy_line = goo_canvas_polyline_new (
											cgpImage,
										    FALSE,
										    0,
										    "points", cvpLine,
										    "stroke-color", "yellow",
										    "line-width", 2.0,
										    NULL);
	goo_canvas_points_unref (cvpLine);
}

GooCanvasItem *ui_show_augcanv_plot (GooCanvasPoints *points, 
                                     GooCanvasItem *plot)
{
	/* Show plot made up of points on canvas group */

	if (plot)
        g_object_set (G_OBJECT (plot), "points", points, NULL);
    else
        plot = goo_canvas_polyline_new (cgpHist,
                                        FALSE,
                                        0,
                                        "points", points,
                                        "stroke-color", "red",
                                        "line-width", 2.0,
                                        NULL);
	
	goo_canvas_points_unref (points);
	
	return plot;
}

static void ui_show_augcanv_plot_titles (void)
{
	/* Display the autoguider histogram and flux plot titles (or redisplay
     * after changing font).
     */
	
	static GooCanvasItem *cviHLabel = NULL;
	static GooCanvasItem *cviYLabel = NULL;
	static GooCanvasItem *cviXLabel = NULL;
	
	cviHLabel = ui_show_augcanv_text ((gdouble) XPLOT,
	                        (gdouble) (YHIST - 3 * TGAP),
	                        "Histogram",
	                        0,
	                        0,
						    0,
			                "green",
	                        cviHLabel);
		
	cviYLabel = ui_show_augcanv_text ((gdouble) XPLOT,
	                        (gdouble) (YHIST + BOXSIZE + YGAP - 3 * TGAP),
	                        "Y-Centroid",
	                        0,
	                        0,
						    0,
			                "green",
	                        cviYLabel);
	
	cviXLabel = ui_show_augcanv_text ((gdouble) XPLOT,
	                        (gdouble) (YHIST + 2 * (BOXSIZE + YGAP) - 3 * TGAP),
	                        "X-Centroid",
	                        0,
	                        0,
						    0,
			                "green",
	                        cviXLabel);
}

GooCanvasItem *ui_show_augcanv_text (gdouble x, gdouble y, gchar *string, 
                                     gdouble val, gushort type, gint sigfig, 
                                     gchar *colour, GooCanvasItem *text)
{
	/* Display text, optionally followed by a numerical value, at position
	 * x, y on canvas group.
	 * If type = 0, show no number
	 *           1, display as integer
	 *           2, display as double
	 */

	gchar *s1, *s2, *s3;
	
	if (string == NULL)
		string = "";
	
	switch (type) {
		case 0:
			s1 = NULL;
			s2 = g_strconcat (string, "", NULL);
			s3 = NULL;
			break;
		case 1:
			s1 = g_strconcat (string, "%i", NULL);
		    s2 = g_strdup_printf (s1, (gint) val);
			s3 = NULL;
			break;
		case 2:
			s3 = g_strdup_printf ("%i", sigfig);
			s1 = g_strconcat (string, "%.", s3, "G", NULL);
		    s2 = g_strdup_printf (s1, val);
			break;
		default:
			L_print ("{r}ui_show_augcanv_text: Invalid data type\n");
			return NULL;
			break;
	}
	
	if (text)
        g_object_set (G_OBJECT (text), "text", s2, "font", font, NULL);
    else
        text = goo_canvas_text_new (cgpHist,
                                    s2,
                                    x,
                                    y,
                                    -1,
                                    GTK_ANCHOR_NW,
                                    "fill-color", colour,
                                    "font", font,
                                    NULL);
	if (s1 != NULL)
		g_free (s1);
	g_free (s2);
	if (s3 != NULL)
		g_free (s3);

	return text;
}

void ui_show_augcanv_centroid (gboolean show, gboolean saturated, 
                               gfloat h, gfloat v,
                               gushort x1, gushort x2, gushort y1, gushort y2)
{
	/* Show the box-and-dot centroid marker for the star image on the
	 * autoguider display.
	 */
	 
	 struct cam_img *aug = get_aug_image_struct ();
	
	static GooCanvasItem *cviBox = NULL;
	static GooCanvasItem *cviDot = NULL;
	gchar *red = "red";
	gchar *blue = "blue";
	
	if (cviBox != NULL) {
		goo_canvas_item_remove (cviBox);
		cviBox = NULL;
	}
	
	if (cviDot != NULL) {
		goo_canvas_item_remove (cviDot);
	    cviDot = NULL;
	}
	
	if (show) {
		cviBox = goo_canvas_rect_new (cgpImage,
					(gdouble) x1,
                    (gdouble) y1,          /* Add 1 to show edge of box at */
                    (gdouble) x2 - x1 + 1, /*  right-hand/bottom edge of   */
					(gdouble) y2 - y1 + 1, /*  right-most/bottom pixel.    */
					"stroke-color", "blue",
					"line-width", 1.0 / aug->canv.zoom,
					NULL);
		
		cviDot = goo_canvas_ellipse_new (cgpImage,
					(gdouble) h,
					(gdouble) v,
					1.0,
					1.0,
					"line-width", 1.0 / aug->canv.zoom,
					"stroke-color", !saturated ? blue : red,
					NULL);
	}
}

void ui_set_aug_window_controls (enum HWDevice dev, gboolean Binning)
{
	/* A scrappy little function to set the state of various controls on the
	 * autoguider image window.
	 */
	 
	 struct cam_img *aug = get_aug_image_struct ();
	 
	 GtkWidget *w;
	 
	 switch (dev) {
		 case UNICAP:
			set_entry_int ("txtImgSatLevel", 
						   R_config_d ("AugImage/Unicap/SatLimit", 250));
			aug->imdisp.satlevel = 250;					   
			w = xml_get_widget (xml_img, "optImgBin1x1");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
			gtk_widget_set_sensitive (w, FALSE);
			w = xml_get_widget (xml_img, "optImgBin2x2");
			gtk_widget_set_sensitive (w, FALSE);
			break;
		case V4L:
			set_entry_int ("txtImgSatLevel", 
						   R_config_d ("AugImage/V4L/SatLimit", 250));
			aug->imdisp.satlevel = 250;					   
			w = xml_get_widget (xml_img, "optImgBin1x1");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
			gtk_widget_set_sensitive (w, FALSE);
			w = xml_get_widget (xml_img, "optImgBin2x2");
			gtk_widget_set_sensitive (w, FALSE);
			break;
		case SX:
			set_entry_int ("txtImgSatLevel", 
						   R_config_d ("AugImage/SX/SatLimit", 65000));
			aug->imdisp.satlevel = 65000;					   
			set_entry_float ("txtImgExpLength", aug->exd.req_len);
			w = xml_get_widget (xml_img, "optImgBin1x1");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
			gtk_widget_set_sensitive (w, TRUE);
			w = xml_get_widget (xml_img, "optImgBin2x2");
			gtk_widget_set_sensitive (w, TRUE);
			break;
		case SX_GH:
			set_entry_int ("txtImgSatLevel", 
						   R_config_d ("AugImage/SX_GH/SatLimit", 65000));
			aug->imdisp.satlevel = 65000;					   
			set_entry_float ("txtImgExpLength", aug->exd.req_len);
			w = xml_get_widget (xml_img, "optImgBin1x1");
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), TRUE);
			gtk_widget_set_sensitive (w, TRUE);
			w = xml_get_widget (xml_img, "optImgBin2x2");
			gtk_widget_set_sensitive (w, TRUE);
			break;
		default:
			break;
	}
}

void ui_show_video_frame (guchar *frame, gchar *timestamp, guint num, 
					      gushort h, gushort v)
{
	/* Display a frame of video data from a recorded file */

	struct cam_img *vid = get_vid_image_struct ();
	
	static GdkPixbuf *image = NULL;
    
    /* Remove any photometric detection circles by removing their root group */
    
    if (cgpPhotom) {
        goo_canvas_item_remove (cgpPhotom);
        cgpPhotom = NULL;
    }
	
	/* Destroy previous pixbuf */
    
	if (image != NULL) {	
		g_object_unref (image);
		image = NULL;
	}
    
	/* Create new pixbuf */

	if (!(image = gdk_pixbuf_new_from_data (vid->disp083,
		                                    GDK_COLORSPACE_RGB, 
	                                        FALSE,
	                                        8,
	                                        h,
	                                        v,
	                                        h * 3,
	                                        NULL,
	                                        NULL))) {
        L_print ("{r}%s: Unable to create image from data\n", __func__);
        return;
    }
	
	/* Display image on canvas */
	
	if (vid->canv.cviImage)	/* Canvas image already exists */
		g_object_set (vid->canv.cviImage, "pixbuf", image, NULL);
	else                    /* Create new canvas image item */	
		vid->canv.cviImage = goo_canvas_image_new (cgpPlayback,
						                           image,
	                                               0.0,
	                                               0.0,
	                                               NULL);
						   
	/* Set the image to just below the selection rectangle */
	
	goo_canvas_item_lower (vid->canv.cviImage, vid->canv.cviRect);
	
	/* Update timestamp display */
	
	set_entry_int ("txtPBFrameNum", num);
	set_entry_string ("txtPBTimeStamp", timestamp);
}

void ui_show_photom_points (gchar *filename, gfloat aperture)
{
	/* Display the detected points on the current video frame */
	
	struct cam_img *vid = get_vid_image_struct ();
	
	gint i, n, f;
	gdouble x, y, c, e, s;
	gchar *buffer, **strings, *s1, *s2, *s3, *s4, *s5, *s6;
    
    /* Remove previous points by removing the group that contains them.  Then
     * create new empty group.
     */
    
    if (cgpPhotom)
        goo_canvas_item_remove (cgpPhotom);
    cgpPhotom = goo_canvas_group_new (cgpPlayback,
                                      "line-width", 1.0 / vid->canv.zoom,
                                      "stroke-color", "yellow",
                                      NULL);
                                      
    /* Get locations of new points from SExtractor file */
    
	if (!g_file_get_contents (filename, &buffer, NULL, NULL)) {
		P_print ("Unable to read SExtractor output file\n");
		L_print ("{r}Unable to read SExtractor output file\n");
		return;
	}
	strings = g_strsplit (buffer, "\n", -1);
    
	i = -1;
	while (strings[++i]) {
		n = (gint) strtol (strings[i], &s1, 10);
		x = strtod (++s1, &s2);
		y = (gdouble) vid->exd.v_pix - strtod (++s2, &s3);
		c = strtod (++s3, &s4);
		e = strtod (++s4, &s5);
		s = strtod (++s5, &s6);
		f = (gint) strtol (++s6, (gchar **) NULL, 10);
	
		if (n) {
			P_print ("{.}x: %7.2f, y: %7.2f,  counts: %.2f,  R:  %.2f,  "
					 "sky: %.4f,  flags: %d\n", x + vid->canv.r.htl, 
					 y + vid->canv.r.vtl, c, c/e, s, f);
			
            goo_canvas_ellipse_new (cgpPhotom,
                           (gdouble) (x + vid->canv.r.htl),
                           (gdouble) (y + vid->canv.r.vtl),
                           (gdouble) aperture / 2.0,
                           (gdouble) aperture / 2.0,
                           NULL);
		}
	}
	g_strfreev (strings);
}

void set_video_range_adjustment (guint num_frames)
{
	/* Set the parameters for the slider control */
	
	GtkAdjustment *adjustment;
	
	adjustment = GTK_ADJUSTMENT (gtk_adjustment_new (1.0,
                                                     1.0, 
                                                     (gdouble) num_frames, 
                                                     1.0, 
                                                     50.0,
                                                     0));
	gtk_range_set_adjustment (GTK_RANGE (xml_get_widget 
										 (xml_pbw, "hscPBFrames")), adjustment);
}

void set_video_range_value (guint frame_num)
{
	/* Set the video frame slider to the given value */
	
	gtk_range_set_value (GTK_RANGE (xml_get_widget 
							    (xml_pbw, "hscPBFrames")), (gdouble) frame_num);
}

gushort get_video_framebufsize (void)
{
	/* Return the video frame buffer size for video recording from the 
	 * configuration database (the user may wish to tune this parameter).
	 */
	
	return R_config_d ("Video/FrameBufSize", 50);
}

void ui_show_status_bar_info (void)
{
	/* Display image information on the status bar.
	 * Note that we display the pixel values as beginning at (1, 1) in the 
	 * bottom left corner of the underlying image, even though the canvas
	 * coordinates begin at (0, 0) in the top left corner of the window.
	 */
	
	struct cam_img *aug = get_aug_image_struct (); 	
	
	guint e, w, n, s;
	gint x = aug->canv.cursor_x;
	gint y = aug->canv.cursor_y;
	gint val, i;
	gfloat ratio;
	gchar *info;
	
	if (aug->Open) {
		if (x >= aug->exd.h_top_l && (x < aug->exd.h_top_l + aug->exd.h_pix) && 
			y >= aug->exd.v_top_l && (y < aug->exd.v_top_l + aug->exd.v_pix)) {
                
			/* Get the (possibly dark-subtracted and background-adjusted) value 
			 * in the photocell.
			 */
		
			i =aug->exd.h_pix * (y - aug->exd.v_top_l) + (x - aug->exd.h_top_l);
			val = aug->r161[i];
			if (aug->dark.Subtract)
				val = MAX (val - aug->dark.dk161[i], 0);
			val = MAX (val - aug->imdisp.black, 0);
			
		} else {      /* Cursor lies outside image area */
            x = aug->exd.h_top_l - 1;
            y = aug->exd.v_top_l + aug->exd.v_pix;
			val = 0;
		}
	
		if (aug->autog.Guide)
			telescope_get_guide_corrs (&e, &w, &n, &s, &ratio);
		else {
			e = w = n = s = 0;
			ratio = 0.0;
		}
	
		info = g_strdup_printf ("(%03i, %03i)   "
						"Min: %05i,  Max: %05i    Value: %05i    "
						"Zoom: %3.2f    "
	                    "N: %04d, S: %04d, E: %04d, W: %04d, R = %5.2f",
						x - aug->exd.h_top_l + 1, 
                        aug->exd.v_top_l + aug->exd.v_pix - y,
						/* If selection rect. lies outside image, make sure */
						/* min value is less than max value!                */
						MIN (aug->rect.min[GREY].val, aug->rect.max[GREY].val),
						aug->rect.max[GREY].val,
	                    val, aug->canv.zoom,
	                    n, s, e, w, ratio);
	
		set_status_bar (stsImageStatus, info, FALSE);
 		g_free (info);
	
	} else {
		info = g_strdup_printf ("(%03i-%03i, %03i-%03i)   "
								"Min: %05i,  Max: %05i    Value: %05i    "
								"Zoom: %3.2f    "
	           		            "N: %04d, S: %04d, E: %04d, W: %04d, R = %5.2f",
								 0, 0, 0, 0,	
							  	 0, 0, 0, 1.0,
								 0, 0, 0, 0, 0.0);
		
		set_status_bar (stsImageStatus, info, FALSE);
 		g_free (info);		
	}
	
	return;	
}

static gchar *get_open_filename (GtkWindow *window, gchar *filename)
{
	/* Return a valid filename to open */
	
	GtkWidget *dialog;
    
	dialog = gtk_file_chooser_dialog_new ("Open File",
					 					 window,
										 GTK_FILE_CHOOSER_ACTION_OPEN,
										 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										 NULL);
	gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), TRUE);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    }
	gtk_widget_destroy (dialog);
	
	return filename;
}

static gchar *get_save_filename (GtkWindow *window, gchar *filename)
{
	/* Return a valid filename to save */

	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Save File",
					 					 window,
										 GTK_FILE_CHOOSER_ACTION_SAVE,
										 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										 NULL);
	gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), TRUE);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		
		/* Check it's valid, and whether or not file already exists */
		
		if (!check_file (filename, FALSE)) {
			g_free (filename);
			filename = NULL;
			gtk_widget_destroy (dialog);
			return filename;
		}
	}
	gtk_widget_destroy (dialog);
	
	return filename;
}

gboolean save_file (struct cam_img *img, enum Colour colour, gboolean display)
{
	/* Save the image to a file.  If display == TRUE then we save a copy for
	 * display in a default location; otherwise we save a named copy in a 
	 * user-defined location.  This routine is called three times for a
	 * colour image; once each for the R, G and B components.
	 */

	gint num;
	gchar *fname, *uri = NULL, *dirname = NULL, *basename, *savefile = NULL;
	gchar *HomeDisplay = NULL, *entry_field = NULL;
	gboolean val;
	
	if (img->id != CCD && img->id != AUG)
		return show_error (__func__, "Unrecognised image type");
	
	/* Save default display copy... */
	
	if (display && (PrivatePath != NULL)) {
		if (img->id == CCD) {
			if (colour == GREY)
				HomeDisplay = g_build_filename (
							              PrivatePath, "ccd_display.fit", NULL);
			else {
				gchar *c;
				if (colour == R)
					c = "R";						
				else if (colour == G)
					c = "G";						
				else if (colour == B)
					c = "B";						
				else 
					c = "X";						
				fname = g_strdup_printf ("ccd_display_%s.fit", c);
				HomeDisplay = g_build_filename (PrivatePath, fname, NULL);
				g_free (fname);
			}
		} else
			HomeDisplay = g_build_filename (
							              PrivatePath, "aug_display.fit", NULL);
		val = image_save_as_fits (img, HomeDisplay, colour, display);		
		if (val) {
			G_print ("{b}Written %s for display in DS9\n", HomeDisplay);
			if (img->ds9.display != NULL)
				g_free (img->ds9.display);
			img->ds9.display = g_strdup (HomeDisplay);
		}
		g_free (HomeDisplay);
		return val;
	}
	
	/* Saved named copy... */
	
	if (img->id == CCD) {
		uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER 
		                            (xml_get_widget (xml_app, "flcCCDFolder")));
		entry_field = g_strdup_printf ("txtCCDFile");
	} else if (img->id == AUG) {
		uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER 
		                            (xml_get_widget (xml_app, "flcAUGFolder")));
		entry_field = g_strdup_printf ("txtAUGFile");
	}
	dirname = g_filename_from_uri (uri, NULL, NULL);
	g_free (uri);		
	basename = get_entry_string (entry_field);
	g_free (entry_field);
	
	if (img->id == CCD) {
		gboolean LongNames;
		LongNames = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
			                     xml_get_widget (xml_app, "chkLongFileNames"))); 
		get_entry_int ("txtFileNumCCD", 1, 9999, 0, FIL_PAGE, &num);
		if (colour == GREY) {
			if (!strcmp (img->exd.filter, "-")) {
				if (LongNames)
					savefile = g_strdup_printf (
							   "%s/%s_%s_%05i_%06.2f_%09.3f.fit",
							   dirname, img->exd.ExpType, basename, num, 
							   img->exd.ccdtemp, img->exd.req_len);
				else
					savefile = g_strdup_printf (
							   "%s/%s_%s_%05i.fit", 
							   dirname, img->exd.ExpType, basename, num);
			} else {
				if (LongNames)
					savefile = g_strdup_printf (
							   "%s/%s_%s_%05i_%s_%06.2f_%09.3f.fit", 
							   dirname, img->exd.ExpType, basename, num, 
							   img->exd.filter, img->exd.ccdtemp, 
							   img->exd.req_len);
				else
					savefile = g_strdup_printf (
							   "%s/%s_%s_%05i_%s.fit", 
							   dirname, img->exd.ExpType, basename, num, 
							   img->exd.filter);
			}
		} else {
			gchar *c;
			if (colour == R)
				c = "R";						
			else if (colour == G)
				c = "G";						
			else if (colour == B)
				c = "B";						
			else 
				c = "X";
			if (LongNames)
				savefile = g_strdup_printf (
						   "%s/%s_%s_%05i_%s_%06.2f_%09.3f.fit", 
						   dirname, img->exd.ExpType, basename, num, c, 
						   img->exd.ccdtemp, img->exd.req_len);
			else
				savefile = g_strdup_printf (
						   "%s/%s_%s_%05i_%s.fit", 
						   dirname, img->exd.ExpType, basename, num, c);
		}
		strcpy (img->exd.filename, savefile);
	} else if (img->id == AUG) {
		get_entry_int ("txtFileNumAUG", 1, 9999, 0, FIL_PAGE, &num);
		savefile = g_strdup_printf ("%s/%s_%05i.fit", dirname, basename, num);
	}
	
	g_free (basename);	
	g_free (dirname);	
	
	/* Check whether the file name is valid */
	
	if (!check_file (savefile, (img->AutoSave || img->SavePeriodic))) {
		g_free (savefile);
		return FALSE;
	}
	
	/* Write the FITS file */
	
	if (img->id == CCD) {
		if (image_save_as_fits (img, savefile, colour, display)) {
			if (colour == GREY || colour == B)
				set_entry_int ("txtFileNumCCD", num + 1);
			L_print ("{b}Saved %s\n", savefile);
		} else
			return FALSE;
	} else if (img->id == AUG) {
		if (image_save_as_fits (img, savefile, colour, display)) {
			set_entry_int ("txtFileNumAUG", num + 1);
			L_print ("{b}Saved %s\n", savefile);
		} else
			return FALSE;
	}
	g_free (savefile);
	
	/* Set 'file saved' condition to TRUE */
	
	file_saved (img, TRUE);
	
	return TRUE;
}	

void file_saved (struct cam_img *img, gboolean saved)
{
	/* Enable the 'File|Save As...' menu option and the 'Save' toolbar button
	 * when a new image is obtained.  Disable them when it has been saved.
	 * Set the FileSaved flag appropriately - this value is examined when the
	 * user attempts to close the application.
	 */

	GtkWidget *button, *mnuitem;
	gchar *btn_name = NULL, *menu_name = NULL;
	
	if (img->id == CCD) {
		btn_name = g_strdup_printf ("btnSaveCCD");
		menu_name = g_strdup_printf ("save_ccd_image");
	}
	else if (img->id == AUG) {
		btn_name = g_strdup_printf ("btnSaveAUG");
		menu_name = g_strdup_printf ("save_aug_image");
	}
	
	button = xml_get_widget (xml_app, btn_name);
	gtk_widget_set_sensitive (button, !saved);
	mnuitem = xml_get_widget (xml_app, menu_name);			
	gtk_widget_set_sensitive (mnuitem, !saved);
	g_free (btn_name);
	g_free (menu_name);
	
	img->FileSaved = saved;
}

static gboolean query_file_not_saved (void)
{
	/* Query the user if the most recent CCD image has not been saved */	

	struct cam_img *ccd = get_ccd_image_struct ();
		
	GtkWidget *dlgFileNotSaved;
	gint ret;
	
	if (!ccd->FileSaved) {
		dlgFileNotSaved = gtk_message_dialog_new (ccdApp,
										   	     GTK_DIALOG_DESTROY_WITH_PARENT,
											     GTK_MESSAGE_QUESTION,
											     GTK_BUTTONS_YES_NO,
		                                         "The CCD image has not been "
		                                         "saved.  Are you sure you want"
		                                         " to quit?");
		ret = gtk_dialog_run (GTK_DIALOG (dlgFileNotSaved));
		gtk_widget_destroy (dlgFileNotSaved);
		
        switch (ret) {
			case GTK_RESPONSE_YES:       /* User wants to quit */
				return FALSE;
                break;
            case GTK_RESPONSE_NO:        /* User doesn't want to quit */
				return TRUE;
				break;
            default:
			    return FALSE;
                break;
        }
	}
	
	return FALSE;
}

static gboolean check_file (gchar *filename, gboolean AutoSave)
{
	/* Check to see whether or not a file of this name already exists,
	 * and whether the filename is valid.
	 * If AutoSave is TRUE, don't bother with the file existence check; just
	 * overwrite it anyway.
	 */
	
	GtkWidget *dlgFileExists, *dlgMsg;
	gint ret;
	gchar *message;
	
	FILE *fp;
	
	if (!AutoSave) {
		if ((fp = fopen (filename, "rb"))) {
			fclose (fp);
			
			dlgFileExists = gtk_message_dialog_new (
											     ccdApp,
												 GTK_DIALOG_DESTROY_WITH_PARENT,
												 GTK_MESSAGE_QUESTION,
												 GTK_BUTTONS_YES_NO,
												 "A file of this name already "
												 "exists.  Do you want to "
												 "overwrite it?");
			ret = gtk_dialog_run (GTK_DIALOG (dlgFileExists));
			gtk_widget_destroy (dlgFileExists);
			
			switch (ret) {
				case GTK_RESPONSE_YES:      /* Overwrite */
					break;
				case GTK_RESPONSE_NO:       /* Don't overwrite */
					return FALSE;
					break;
				default:
					return FALSE;
					break;
			}
		}
	}
	
	/* Warn the user if the entered file name is invalid */
	
	if (!(fp = fopen (filename, "w"))) {
		message = g_strdup_printf ("Could not save %s - not a writeable file!", 
								   filename);
		if (!AutoSave) {
			dlgMsg = gtk_message_dialog_new (ccdApp,
											 GTK_DIALOG_DESTROY_WITH_PARENT,
											 GTK_MESSAGE_INFO,
											 GTK_BUTTONS_OK,
											 message);
			gtk_dialog_run (GTK_DIALOG (dlgMsg));
			gtk_widget_destroy (dlgMsg);
		} else
			L_print ("{r}%s\n", message);
		g_free (message);
		return FALSE;
	} else {
		fclose (fp);
		remove (filename);
	}

	return TRUE;	
}

void ui_show_aug_window (void)
{
	/* Show the autoguider image window.  (It exists for the lifetime of the
	 * application and is shown/hidden when opened and closed).  This routine
     * should be called after opening the autoguider camera.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	/* Reset canvas scroll region, zoom level and selection rectangle */
		
    goo_canvas_scroll_to (GOO_CANVAS (cnvImage), (gdouble) aug->exd.h_top_l, 
                                                 (gdouble) aug->exd.v_top_l);
	gtk_widget_activate (xml_get_widget (xml_img, "btnResetArea"));
    gtk_widget_activate (xml_get_widget (xml_img, "btnZoom1to1"));
    
    /* Set crosshair to correct location */
    
    ui_set_augcanv_crosshair (aug->exd.h_top_l + aug->exd.h_pix / 2.0, 
                              aug->exd.v_top_l + aug->exd.v_pix / 2.0);
	
	/* Show the window */

	gtk_widget_show (aug->aug_window);
}

void ui_hide_aug_window (void)
{
	/* Hide the autoguider window, if shown, and close the V4L configuration
	 * window, if it is open.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gtk_widget_hide (aug->aug_window);
	if (xml_V4L)
		gtk_widget_activate (xml_get_widget (xml_V4L, "btnV4LConfigClose"));
}

void select_device (void)
{
	/* Pop up a window asking the user to select an available device */
	
	static GtkComboBox *cmbDevSelect;
	gint i;
	gchar *str;
	
	gchar *objects[] = {"wndDevSelect", NULL};
	xml_dev = xml_load_new (xml_dev, GLADE_INTERFACE, objects);
	
    /* Create device selection combo box */
	
    cmbDevSelect = create_text_combo_box (
                      GTK_TABLE (xml_get_widget (xml_dev, "tblDevSelect")),
                      0, 1, 0, 1, cmbDevSelect, "ListDummyComboBox", 
                      0, NULL);
    g_hash_table_insert (hshCombo, "cmbDevSelect", cmbDevSelect);
	
    /* Fill combo box with an entry for each device found */
    
	for (i = 0; i < ds.num; i++) {
		str = g_strdup_printf ("%s : %s", ds.desc[i], ds.id[i]);
	    gtk_combo_box_append_text (cmbDevSelect, str);
	    g_free (str);
	}
    gtk_combo_box_set_active (cmbDevSelect, 0);  /* Set box to first entry */
}

void get_ccd_image_params (struct exposure_data *exd)
{
	/* Get the ccd image parameters from the text entry fields.  Don't invert
	 * the v-coordinate at this stage because these values get displayed in the
	 * task list.  The user will be confused if different values appear in the
	 * task list from the ones entered.
	 */

	struct cam_img *ccd = get_ccd_image_struct ();                              
	
	gint h_size, v_size, hbin, vbin;
	gint val;
	
	h_size = ccd->cam_cap.max_h;
	v_size = ccd->cam_cap.max_v;
	hbin = ccd->cam_cap.max_binh;
	vbin = ccd->cam_cap.max_binv;	
	
    get_entry_int ("txtH1", 1, h_size - 1, 1, CCD_PAGE, &val);
	exd->h_top_l = (gushort) val;
	get_entry_int ("txtH2", exd->h_top_l + 1, h_size, h_size, CCD_PAGE, &val);
	exd->h_bot_r = (gushort) val;
    get_entry_int ("txtHBin", 1, MIN (exd->h_bot_r - exd->h_top_l + 1, hbin), 
				   1, CCD_PAGE, &val);
	exd->h_bin = (gushort) val;
    get_entry_int ("txtV1", 1, v_size - 1, 1, CCD_PAGE, &val);
	exd->v_top_l = (gushort) val;
    get_entry_int ("txtV2", exd->v_top_l + 1, v_size, v_size, CCD_PAGE, &val);
	exd->v_bot_r = (gushort) val;
    get_entry_int ("txtVBin", 1, MIN (exd->v_bot_r - exd->v_top_l + 1, vbin), 
				   1, CCD_PAGE, &val);
	exd->v_bin = (gushort) val;
	
    get_entry_float ("txtExposure", ccd->cam_cap.min_exp, ccd->cam_cap.max_exp,
					 1.0, CCD_PAGE, &exd->req_len);
	
	exd->ExpType = gtk_combo_box_get_active_text (
                                  g_hash_table_lookup (hshCombo, "cmbExpType"));
	
	exd->filter = gtk_combo_box_get_active_text (
                                  g_hash_table_lookup (hshCombo, "cmbFilType"));
	
	get_entry_float ("txtChipTemp", TPR_MIN, TPR_MAX, TPR_DEF, 
					 CCD_PAGE, &exd->ccdtemp);	
}

void set_camera_state (struct cam_img *img)
{
	/* Set the initial camera state based on the stored configuration data.  
	 * We don't check here whether the camera actually supports any of these 
	 * functions; we don't necessarily know.
	 */
	
	gint i;
	
	img->state.d_ccd = R_config_f (CCDConfigKey (img, "DefTemp"), TPR_DEF);
	img->set_state (S_TEMP, 0, img->state.d_ccd, img->id);
	
	img->state.c_tol = R_config_f (CCDConfigKey (img, "TempTol"), 0.1);
	
	img->state.CoolOnConnect = R_config_d (
									CCDConfigKey (img, "CoolOnConnect"), FALSE);
	if (img->cam_cap.CanSetCCDTemp && img->state.CoolOnConnect) {
		img->set_state (S_COOL, img->state.CoolOnConnect, 0.0, img->id);
		L_print ("Requested CCD cooling at %5.1fC\n", img->state.d_ccd);
	}
	
	img->state.d_fans = R_config_d (CCDConfigKey (img, "Fans"), CCD_FAN_AUTO);
	img->set_state (S_FANS, img->state.d_fans, 0.0);
	
	if ((img->state.shut_prior = 
	    R_config_d (CCDConfigKey (img, "ShutterPriority"), -1)) > -1)
		img->set_state (S_PRIORITY, img->state.shut_prior, 0.0);
	
	if ((img->state.shut_mode = 
		R_config_d (CCDConfigKey (img, "ShutterMode"), -1)) > -1)
		img->set_state (S_MODE, img->state.shut_mode, 0.0);

	if ((img->state.shut_open = 
		R_config_d (CCDConfigKey (img, "ShutterOpen"), -1)) > -1)
		img->set_state (S_OPEN, img->state.shut_open, 0.0);

	if ((img->state.pre_flush = 
		R_config_d (CCDConfigKey (img, "PreFlush"), -1)) > -1)
		img->set_state (S_FLUSH, img->state.pre_flush, 0.0);
	
	if ((img->state.host_timed = 
		R_config_d (CCDConfigKey (img, "HostTimed"), -1)) > -1)
		img->set_state (S_HOST, img->state.host_timed, 0.0);
	
	if ((img->state.cam_gain = 
		R_config_d (CCDConfigKey (img, "CameraGain"), -1)) > -1)
		img->set_state (S_GAIN, img->state.cam_gain, 0.0);
	
	if ((img->state.read_speed = 
		R_config_d (CCDConfigKey (img, "ReadoutSpeed"), -1)) > -1)
		img->set_state (S_SPEED, img->state.read_speed, 0.0);

	if ((img->state.anti_bloom = 
		R_config_d (CCDConfigKey (img, "AntiBloom"), -1)) > -1)
		img->set_state (S_ABLOOM, img->state.anti_bloom, 0.0);
		
	img->state.invert = R_config_d (CCDConfigKey (img, "InvertImage"), 0);
	img->set_state (S_INVERT, img->state.invert, 0.0, img->id);
	
	img->Debayer = R_config_d (CCDConfigKey (img, "Debayer"), 0);
	i = R_config_d (CCDConfigKey (img, "BayerPattern"), 0);
	img->bayer_pattern = (i < 3 ? --i : i);
	
	if (img->id == CCD)
		img->state.IgnoreCooling = R_config_d ("Misc/IgnoreCCDCooling", FALSE);	
	
	/* The following items aren't really part of the camera state, but this is 
	 * the best place to set them.
	 */
	img->ds9.Invert_h = R_config_d (CCDConfigKey (img, "InvertDS9h"), 0);
	img->ds9.Invert_v = R_config_d (CCDConfigKey (img, "InvertDS9v"), 0);
}

void set_ccd_deftemp (void)
{
	/* Set the default CCD chip temperature on the CCD camera tab according
	 * to the setting in the CCD camera configuration window, when the camera
	 * is opened.
	 */
	
	gdouble val;
	
	val = R_config_f (CCDConfigKey (get_ccd_image_struct (),"DefTemp"),TPR_DEF);
	set_entry_float ("txtChipTemp", val);
}

gboolean show_camera_status (gboolean show)
{
	/* Display the CCD camera status in the status window if 'show' is TRUE, 
	 * otherwise delete the status display.  It is assumed that the camera
	 * status is queried elsewhere before calling this function.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	
	GtkTextIter start, end;
	gchar *line;
	
	/* Delete current window contents */
	
	gtk_text_buffer_get_iter_at_offset (txtStatusBuffer, &start, 0);
	gtk_text_buffer_get_iter_at_offset (txtStatusBuffer, &end, -1);
	gtk_text_buffer_delete (txtStatusBuffer, &start, &end);
	
	if (show && ccd->Open) {
		
		/* Insert the text in the status window */
		
		gtk_text_buffer_get_iter_at_offset (txtStatusBuffer, &end, -1);	
		
		if (ccd->device == QSI)
			line = g_strdup_printf ("Cooling:               %s\n"
									"Fan mode:              %s\n"
									"Heatsink temperature:  %+5.1fC\n"
									"CCD temperature:       %+5.1fC\n"
									"Cooler power:          %5.1f%%\n"
									"Filter position:       %d\n"
									"Camera status:       %s",
									(ccd->state.CoolState == 1) ? "On" : "Off",
									(ccd->state.c_fans == 0) ? "Off" : 
									(ccd->state.c_fans == 1) ? "Auto" : "High",
									ccd->state.c_amb,
									ccd->state.c_ccd,
									ccd->state.c_power,
									ccd->state.c_filter,
									ccd->state.status);
		else if (ccd->device == SX)
			line = g_strdup_printf ("Cooling:               %s\n"
									"CCD temperature:       %+5.1fC\n"
									"Camera status:       %s",
									(ccd->state.CoolState == 1) ? "On" : "Off",
									ccd->state.c_ccd,
									ccd->state.status);
		else
			return show_error (__func__, "Unknown camera type");
		
		gtk_text_buffer_insert_with_tags (txtStatusBuffer, &end, line, -1, 
										                         courier, NULL);
		g_free (line);
	}
	return TRUE;
}

gboolean get_V4L_settings (gchar *device)
{
	/* Return the V4L settings from the configuration file for the given 
	 * device.
	 */
	 
	struct cam_img *aug = get_aug_image_struct ();
	
	gint val;
	gchar key[128];
	
	snprintf (key, 128, "V4L/%s/4CC", device);
	if ((aug->vid_dat.pixfmt = R_config_d (key, -1)) == -1)
		return FALSE;  /* No config. data for this device */
	
	snprintf (key, 128, "V4L/%s/VideoStandard", device);
	if ((val = R_config_d (key, -1)) == -1)
		aug->vid_dat.HasVideoStandard = FALSE;
	else {
		aug->vid_dat.vid_std.id[0] = val;
		aug->vid_dat.HasVideoStandard = TRUE;
	}

	snprintf (key, 128, "V4L/%s/VideoInput", device);
	aug->vid_dat.vid_input.selected = R_config_d (key, 0);	
	
	snprintf (key, 128, "V4L/%s/Width", device);
	aug->vid_dat.width = R_config_d (key, 640);
	
	snprintf (key, 128, "V4L/%s/Height", device);
	aug->vid_dat.height = R_config_d (key, 480);

	snprintf (key, 128, "V4L/%s/fps", device);
	aug->vid_dat.fps = (gfloat) R_config_f (key, 25);
	
	return TRUE;
}

gboolean set_filter (gboolean ForceInternal, gchar *filter, gint *fo_diff)
{
	/* Set the filter position, based on the given name and the stored
	 * location of a filter of that name in the filter wheel.  Return the
	 * difference between the new filter focus offset and the old one as
	 * fo_diff.  If ForceInternal == TRUE, then use the camera's internal
	 * filter wheel (if there is one) irrespective of menu settings.
	 */
	
	struct cam_img *ccd = get_ccd_image_struct ();
	gint i, off_old, off_new;
	gchar f[MCSL];
	
	*fo_diff = 0;
	
	/* If filter is NULL, use the currently set filter in the exposure 
	 * data.
	 */
	
	if (!filter)
		filter = ccd->exd.filter;
	
	/* Return if no filter requested */
	
	if (!strcmp (filter, "-"))
		return TRUE;
	
	if (ForceInternal || !strcmp (menu.filterwheel, "filterwheel_int")) {
		if (ccd->Open && ccd->cam_cap.HasFilterWheel) {/* Camera incorporates */
												       /*  filter wheel       */
			/* Get the current focus offset */
			
			ccd->get_state (&ccd->state, FALSE);  /* Gets current filter pos. */
			strcpy (f, "-");
			get_filter_info (ccd, f, (gint *) &ccd->state.c_filter, &off_old);
			
			/* Now set the filter position */
			
			if (get_filter_info (ccd, filter, &i, &off_new)) {
				L_print ("{b}Setting filter wheel to \"%s\" at position %d\n", 
																	 filter, i);	
				if (!ccd->set_state (S_FILTER, i, 0.0))
					return show_error (__func__, "Error setting filter wheel "
														 "to desired position");
				*fo_diff = off_new - off_old;
			} else {
				L_print ("{o}Can't find \"%s\" in the current filter wheel\n", 
																	    filter);
				*fo_diff = 0;
				return FALSE;
			}
		} else
			return show_error (__func__, "CCD camera not open or does not have "
			                   "internal filter wheel");
	} else if (filter_is_open ()) {          /* External filter wheel is open */
	
		/* Get the current focus offset */
		
		i = filter_get_filter_pos ();
		strcpy (f, "-");
		get_filter_info (NULL, f, &i, &off_old);
		
		/* Now set the filter position */
		
		if (get_filter_info (NULL, filter, &i, &off_new)) {
			L_print ("{b}Setting filter wheel to \"%s\" at position %d\n", 
																 filter, i);			
			if (!filter_set_filter_pos (i))
				return show_error (__func__, "Error setting filter wheel "
													 "to desired position");
			*fo_diff = off_new - off_old;
		} else {
			L_print ("{o}Can't find \"%s\" in the current filter wheel\n", 
					                                                    filter);
			*fo_diff = 0;
			return FALSE;
		}
	} else {
			L_print ("{o}Can't find filter wheel!\n");
	}

	return TRUE;
}

gboolean get_filter_info (struct cam_img *img, gchar *filter, gint *pos, 
	                      gint *offset)
{
	/* If the filter name is "-", return the filter name and focus offset for  
	 * the given position.  If a filter name is given, return the filter 
	 * position and focus offset for the given filter name.  
	 * Do this irrespective of whether or not the filter wheel is actually open.
	 */
	
	gushort wheel;
	gchar f[MCSL], *key;
	
	if (img) {  /* Internal CCD camera/autoguider camera filter wheel */
		if (!strcmp (filter, "-")) { /* Get filter name and offset */
			wheel = R_config_d (CCDConfigKey (img, "Filter/ActiveWheel"), 1);
			key = g_strdup_printf ("%s/%d/%d",
								   CCDConfigKey (img, "Filter"), wheel, *pos);
			R_config_s (key, filter);
			g_free (key);
			if (!strcmp (filter, "-")) /* Filter not defined for given pos. */
				return FALSE;
		} else {	                 /* Get filter position and offset */
			wheel = R_config_d (CCDConfigKey (img, "Filter/ActiveWheel"), 1);
			*pos = 0;
			while (TRUE) {
				key = g_strdup_printf ("%s/%d/%d",
								   CCDConfigKey (img, "Filter"), wheel, *pos);
				strcpy (f, "-");
				R_config_s (key, f);
				g_free (key);
				if (!strcmp (filter, f))
					break;
				if (!strcmp (f, "-") && *pos > 0) /* Filter not found */
					return FALSE;
				*pos += 1;
			}
		}
		key = g_strdup_printf ("%s/%d/FO%d", 
							   CCDConfigKey (img, "Filter"), wheel, *pos);
		*offset = R_config_d (key, 0);
		g_free (key);
	} else {  /* External filter wheel */
		if (!strcmp (filter, "-")) { /* Get filter name and offset */
			wheel = R_config_d ("FilterWheel/ActiveWheel", 1);
			key = g_strdup_printf ("%s/%d/%d", "FilterWheel", wheel, *pos);
			R_config_s (key, filter);
			g_free (key);
			if (!strcmp (filter, "-")) /* Filter not defined for given pos. */
				return FALSE;
		} else {	                 /* Get filter position and offset */
			wheel = R_config_d ("FilterWheel/ActiveWheel", 1);
			*pos = 0;
			while (TRUE) {
				key = g_strdup_printf ("%s/%d/%d", "FilterWheel", wheel, *pos);
				strcpy (f, "-");
				R_config_s (key, f);
				g_free (key);
				if (!strcmp (filter, f))
					break;
				if (!strcmp (f, "-") && *pos > 0) /* Filter not found */
					return FALSE;
				*pos += 1;
			}
		}
		key = g_strdup_printf ("%s/%d/FO%d", "FilterWheel", wheel, *pos);
		*offset = R_config_d (key, 0);
		g_free (key);
	}
	return TRUE;
}

gboolean get_apply_filter_offset (void)
{
	/* Return TRUE if the filter focus offset is to be applied */
	
	return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
			           xml_get_widget (xml_app, "chkFocusApplyFilterOffsets")));
}

void apply_filter_focus_offset (gint offset)
{
	/* Apply the focus offset corresponding to the selected filter */
	
	struct focus f;
	
	if (menu.OpenFocusPort) {
		f.cmd = FC_MOVE_BY;
		f.move_by = offset;
		focus_comms->focus (&f);
		loop_focus_check_done ();
    } else
	    L_print ("{o}Focuser not available\n");
}

void check_focuser_temp (void)
{
	/* Query the focuser temperature and display on Focus tab.  Make focus
	 * adjustments if the user has requested it.
	 */
	
	struct focus f;
	gint pos, steps;
	gdouble deltaT, coeff;
	gdouble temp_diff;
	
	f.cmd = FC_VERSION;
	focus_comms->focus (&f);
	if (f.version < 3.0)
		return;
	
	f.cmd = FC_CUR_POS_GET | FC_TEMP_GET;
	focus_comms->focus (&f);
	if (!f.Error) {
		set_entry_float ("txtFocusTemperature", f.temp);
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
						       xml_get_widget (xml_app, "chkFocusTempComp")))) {
			focus_get_temp_diff_pos (&temp_diff, &pos);
			get_entry_float ("txtFocusDeltaT", 0.1, 100.0, 5.0,NO_PAGE,&deltaT);
			if (ABS (temp_diff) >= deltaT) {
				get_entry_float ("txtFocusTempCoeff", -9999.0, 9999.0, 0.0,
				                                               NO_PAGE, &coeff);
				steps = rint (coeff * temp_diff);
				if ((pos + steps) != f.cur_pos) {
					L_print ("{b}Applying temperature focus compensation; "
					         "moving to position %d\n", pos + steps);
					loop_focus_apply_temp_comp (pos + steps);
				}
			}
		}
	} else
		L_print ("{r}Error reading focuser temperature\n");
}

static void select_entry_region (const gchar *name)
{
	/* Simple helper function to select the contents of an entry widget */
	
	GtkEntry *entry;
	
	if ((entry = get_entry_widget (name)))
		gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
	else
		G_print ("{r}select_entry_region: Not a valid text field: %s\n", name);				
}

static void common_keyboard_shortcuts (GdkEventKey *event)
{
	/* Implement the keyboard shortcuts that are common to the main application
	 * window and the image display window.
	 */

	struct cam_img *aug = get_aug_image_struct (); 

	switch (event->keyval) {

		case GDK_F1:  /* Open/close autoguider camera */
			if (aug->autog.Guide)
				return;
			gtk_widget_activate (xml_get_widget (xml_app,"chkAutogOpen"));
			break;
			
		case GDK_F2:  /* Average and subtract dark frames */
			if (gtk_widget_get_visible (aug->aug_window))
				gtk_widget_activate (xml_get_widget
												  (xml_app, "btnAutogCapDark"));
			break;
		
		case GDK_F3:  /* Toggle autoguiding on/off */
			if (gtk_widget_get_visible (aug->aug_window)) {
				if (!aug->autog.Pause)
					gtk_widget_activate (xml_get_widget 
			                                        (xml_app, "tglAutogStart"));
			}
			break;
			
		case GDK_F4:  /* Pause/continue autoguiding */
			if (gtk_widget_get_visible (aug->aug_window)) {
				if (aug->autog.Guide)
					gtk_widget_activate (xml_get_widget 
				                                    (xml_app, "tglAutogPause"));
			}
			break;
		
		case GDK_F12:  /* Cancel CCD exposure */
			gtk_widget_activate (xml_get_widget (xml_app, "btnCancel"));
			break;
		
		default:
			return;
	}

	return;
}

static void comms_menus_update_ports (void)
{
	/* Dynamically rebuilds the relevant submenus of the Communications menu,
	 * adding USB serial ports where they exist.
	 */
	 
	GtkWidget *menuitem;
	gushort i;
	gchar name[15], t_name[12], a_name[12], w_name[12], f_name[12];
	
	/* Remove existing menu items */
	
	gtk_container_foreach (
			GTK_CONTAINER (xml_get_widget (xml_app, "telescope_link_menu")),
			comms_menus_remove_ports,
			NULL);
	gtk_container_foreach (
			GTK_CONTAINER (xml_get_widget (xml_app,"autoguider_link_menu")),
			comms_menus_remove_ports,
			NULL);
	gtk_container_foreach (
			GTK_CONTAINER (xml_get_widget (xml_app,"filter_wheel_link_menu")),
			comms_menus_remove_ports,
			NULL);
	gtk_container_foreach (
			GTK_CONTAINER (xml_get_widget (xml_app, "focuser_link_menu")),
			comms_menus_remove_ports,
			NULL);
				
	/* Add back the unique items for the guide signal ports */
				
	menuitem = gtk_menu_item_new_with_label ("Parallel port");
	gtk_widget_set_name (menuitem, "a_ParallelPort");
	gtk_menu_shell_append (GTK_MENU_SHELL (
				   xml_get_widget (xml_app, "autoguider_link_menu")), menuitem);
	g_signal_connect (menuitem, "activate", 
					  G_CALLBACK (comms_ports_activate_cb), NULL);
	if (!strcmp (menu.autoguider_port, "a_ParallelPort"))
			gtk_widget_set_sensitive (menuitem, FALSE);
	gtk_widget_show (menuitem);
	
	menuitem = gtk_menu_item_new_with_label ("Via autoguider camera");
	gtk_widget_set_name (menuitem, "a_GuideCam");
	gtk_menu_shell_append (GTK_MENU_SHELL (
				   xml_get_widget (xml_app, "autoguider_link_menu")), menuitem);
	g_signal_connect (menuitem, "activate", 
					  G_CALLBACK (comms_ports_activate_cb), NULL);
	if (!strcmp (menu.autoguider_port, "a_GuideCam"))
			gtk_widget_set_sensitive (menuitem, FALSE);
	gtk_widget_show (menuitem);
	
	menuitem = gtk_menu_item_new_with_label ("Via CCD camera");
	gtk_widget_set_name (menuitem, "a_CCDCam");
	gtk_menu_shell_append (GTK_MENU_SHELL (
				   xml_get_widget (xml_app, "autoguider_link_menu")), menuitem);
	g_signal_connect (menuitem, "activate", 
					  G_CALLBACK (comms_ports_activate_cb), NULL);
	if (!strcmp (menu.autoguider_port, "a_CCDCam"))
			gtk_widget_set_sensitive (menuitem, FALSE);
	gtk_widget_show (menuitem);
	
	/* Add back the unique items for the filter wheel ports */
	
	menuitem = gtk_menu_item_new_with_label ("Via CCD camera");
	gtk_widget_set_name (menuitem, "w_CCDCam");
	gtk_menu_shell_append (GTK_MENU_SHELL (
				 xml_get_widget (xml_app, "filter_wheel_link_menu")), menuitem);
	g_signal_connect (menuitem, "activate", 
					  G_CALLBACK (comms_ports_activate_cb), NULL);
	if (!strcmp (menu.filterwheel_port, "w_CCDCam"))
			gtk_widget_set_sensitive (menuitem, FALSE);
	gtk_widget_show (menuitem);
	
	menuitem = gtk_menu_item_new_with_label ("USB direct");
	gtk_widget_set_name (menuitem, "w_USBdirect");
	gtk_menu_shell_append (GTK_MENU_SHELL (
				 xml_get_widget (xml_app, "filter_wheel_link_menu")), menuitem);
	g_signal_connect (menuitem, "activate", 
					  G_CALLBACK (comms_ports_activate_cb), NULL);
	if (!strcmp (menu.filterwheel_port, "w_USBdirect"))
			gtk_widget_set_sensitive (menuitem, FALSE);
	gtk_widget_show (menuitem);
	
	/* Add four standard serial ports */
				
	for (i = 0; i < 4; i++) {
		sprintf (name, "/dev/ttyS%d", i);
		sprintf (t_name, "t_ttyS%d", i);
		sprintf (a_name, "a_ttyS%d", i);
		sprintf (w_name, "w_ttyS%d", i);
		sprintf (f_name, "f_ttyS%d", i);
		 
		menuitem = gtk_menu_item_new_with_label (name);
		gtk_widget_set_name (menuitem, t_name);
		gtk_menu_shell_append (GTK_MENU_SHELL (
				xml_get_widget (xml_app, "telescope_link_menu")), menuitem);
		g_signal_connect (menuitem, "activate", 
						  G_CALLBACK (comms_ports_activate_cb), NULL);
		if (!strcmp (menu.telescope_port, t_name))
				gtk_widget_set_sensitive (menuitem, FALSE);
		gtk_widget_show (menuitem);
		
		menuitem = gtk_menu_item_new_with_label (name);
		gtk_widget_set_name (menuitem, a_name);
		gtk_menu_shell_append (GTK_MENU_SHELL (
				xml_get_widget (xml_app, "autoguider_link_menu")), menuitem);
		g_signal_connect (menuitem, "activate", 
						  G_CALLBACK (comms_ports_activate_cb), NULL);
		if (!strcmp (menu.autoguider_port, a_name))
				gtk_widget_set_sensitive (menuitem, FALSE);
		gtk_widget_show (menuitem);
		
		menuitem = gtk_menu_item_new_with_label (name);
		gtk_widget_set_name (menuitem, w_name);
		gtk_menu_shell_append (GTK_MENU_SHELL (
				xml_get_widget (xml_app, "filter_wheel_link_menu")), menuitem);
		g_signal_connect (menuitem, "activate", 
						  G_CALLBACK (comms_ports_activate_cb), NULL);
		if (!strcmp (menu.filterwheel_port, w_name))
				gtk_widget_set_sensitive (menuitem, FALSE);
		gtk_widget_show (menuitem);
		
		menuitem = gtk_menu_item_new_with_label (name);
		gtk_widget_set_name (menuitem, f_name);
		gtk_menu_shell_append (GTK_MENU_SHELL (
				xml_get_widget (xml_app, "focuser_link_menu")), menuitem);
		g_signal_connect (menuitem, "activate", 
						  G_CALLBACK (comms_ports_activate_cb), NULL);
		if (!strcmp (menu.focus_port, f_name))
				gtk_widget_set_sensitive (menuitem, FALSE);
		gtk_widget_show (menuitem);
	}		 
	
	/* Find which USB-serial ports are available and add them to the menu.
	 * Activate the callback for that menu item if it corresponds to the 
	 * previously selected port.  (This will already have been done for the 
	 * other ports when the application initialised, but the USB-serial ports
	 * might not have been available then).
	 */
	 
	for (i = 0; i < 256; i++) {
		sprintf (name, "/dev/ttyUSB%d", i);
		sprintf (t_name, "t_ttyUSB%d", i);
		sprintf (a_name, "a_ttyUSB%d", i);
		sprintf (a_name, "w_ttyUSB%d", i);
		sprintf (f_name, "f_ttyUSB%d", i);
		 
		if (g_file_test (name, G_FILE_TEST_EXISTS)) {
			
			strcpy (ports[BASE_PORTS + i].name, name);
			
			menuitem = gtk_menu_item_new_with_label (name);
			gtk_widget_set_name (menuitem, t_name);
			gtk_menu_shell_append (GTK_MENU_SHELL (
			      xml_get_widget (xml_app, "telescope_link_menu")), menuitem);
			g_signal_connect (menuitem, "activate", 
			                  G_CALLBACK (comms_ports_activate_cb), NULL);
			if (!strcmp (menu.telescope_port, t_name)) {
				gtk_widget_set_sensitive (menuitem, FALSE);
				gtk_widget_activate (menuitem);
			}
			gtk_widget_show (menuitem);
			
			menuitem = gtk_menu_item_new_with_label (name);
			gtk_widget_set_name (menuitem, a_name);
			gtk_menu_shell_append (GTK_MENU_SHELL (
			      xml_get_widget (xml_app, "autoguider_link_menu")),menuitem);
			g_signal_connect (menuitem, "activate", 
			                  G_CALLBACK (comms_ports_activate_cb), NULL);
			if (!strcmp (menu.autoguider_port, a_name)) {
				gtk_widget_set_sensitive (menuitem, FALSE);
				gtk_widget_activate (menuitem);
			}
			gtk_widget_show (menuitem);
			
			menuitem = gtk_menu_item_new_with_label (name);
			gtk_widget_set_name (menuitem, w_name);
			gtk_menu_shell_append (GTK_MENU_SHELL (
			      xml_get_widget (xml_app, "filter_wheel_link_menu")),menuitem);
			g_signal_connect (menuitem, "activate", 
			                  G_CALLBACK (comms_ports_activate_cb), NULL);
			if (!strcmp (menu.filterwheel_port, w_name)) {
				gtk_widget_set_sensitive (menuitem, FALSE);
				gtk_widget_activate (menuitem);
			}
			gtk_widget_show (menuitem);
			
			menuitem = gtk_menu_item_new_with_label (name);
			gtk_widget_set_name (menuitem, f_name);
			gtk_menu_shell_append (GTK_MENU_SHELL (
			      xml_get_widget (xml_app, "focuser_link_menu")), menuitem);
			g_signal_connect (menuitem, "activate", 
			                  G_CALLBACK (comms_ports_activate_cb), NULL);
			if (!strcmp (menu.focus_port, f_name)) {
				gtk_widget_set_sensitive (menuitem, FALSE);
				gtk_widget_activate (menuitem);
			}
			gtk_widget_show (menuitem);
		}
	}		 
}

static void comms_ports_set_active (GtkWidget *widget, gpointer data)
{
	/* Activate the given menu item */
	
	if (!strcmp (gtk_widget_get_name (widget), menu.telescope_port))
		gtk_menu_item_activate (GTK_MENU_ITEM (widget));
	else if (!strcmp (gtk_widget_get_name (widget), menu.autoguider_port))
		gtk_menu_item_activate (GTK_MENU_ITEM (widget));
	else if (!strcmp (gtk_widget_get_name (widget), menu.filterwheel_port))
		gtk_menu_item_activate (GTK_MENU_ITEM (widget));
	else if (!strcmp (gtk_widget_get_name (widget), menu.focus_port))
		gtk_menu_item_activate (GTK_MENU_ITEM (widget));
}

static void comms_ports_activate_cb (GtkWidget* widget, gpointer data)
{
	/* Set the port for communications and add the name of the port to the
	 * parent menu item.
	 */
	
	GtkWidget *parent;
	const gchar *name;
	gchar *l;
	
	
	name = gtk_widget_get_name (widget);
	if (!strncmp (name, "t_", 2)) {
		strcpy (menu.telescope_port, name);
		serial_set_comms_port (name);
		l = g_strdup_printf ("Telescope comms (%s)", name+2);
		gtk_menu_item_set_label (GTK_MENU_ITEM (xml_get_widget (
											xml_app, "telescope_link")), l);
		g_free (l);
	} else if (!strncmp (name, "a_", 2)) {
		strcpy (menu.autoguider_port, name);
		if (serial_set_comms_port (name))
			gtk_widget_set_sensitive (
				xml_get_widget (xml_app, "open_autoguider_link"),
				autog_comms->pnum <= USBDIR ? FALSE : TRUE);
		l = g_strdup_printf ("Guide comms (%s)", name+2);
		gtk_menu_item_set_label (GTK_MENU_ITEM (xml_get_widget (
											xml_app, "autoguider_link")),l);
		g_free (l);
	} else if (!strncmp (name, "w_", 2)) {
		strcpy (menu.filterwheel_port, name);
		if (serial_set_comms_port (name))
			gtk_widget_set_sensitive (
				xml_get_widget (xml_app, "open_filter_wheel_link"),
				filter_comms->pnum <= USBCCD ? FALSE : TRUE);
		l = g_strdup_printf ("Filter wheel comms (%s)", name+2);
		gtk_menu_item_set_label (GTK_MENU_ITEM (xml_get_widget (
											xml_app, "filter_wheel_link")),l);
		g_free (l);
	} else if (!strncmp (name, "f_", 2)) {
		strcpy (menu.focus_port, name);
		serial_set_comms_port (name);
		l = g_strdup_printf ("Focuser comms (%s)", name+2);
		gtk_menu_item_set_label (GTK_MENU_ITEM (xml_get_widget (
											xml_app, "focuser_link")), l);
		g_free (l);
	} else
		L_print ("{r}%s: Invalid port name: %s\n", __func__, name);
	
	parent = gtk_widget_get_parent (widget);
	gtk_container_foreach (GTK_CONTAINER (parent), widget_set_sensitive, NULL);
	gtk_widget_set_sensitive (widget, FALSE);
}

static void comms_menus_remove_ports (GtkWidget *widget, gpointer data)
{
	/* Remove the passed menu item */
	
	gtk_widget_destroy (widget);
}

static void widget_set_sensitive (GtkWidget *widget, gpointer data)
{
	/* Sensitise the passed widget (intended for calling from a 'foreach'
	 * function).
	 */

	gtk_widget_set_sensitive (widget, TRUE);
}

static void widget_set_insensitive (GtkWidget *widget, gpointer data)
{
	/* De-sensitise the passed widget (intended for calling from a 'foreach'
	 * function).
	 */

	gtk_widget_set_sensitive (widget, FALSE);
}

static void restore_config_data (void)
{
	/* Set values from configuration file */

	struct cam_img *aug = get_aug_image_struct ();
	struct autog_config *s; 

	GtkToggleButton *button;
	gchar *m;
	gboolean RemoteTiming;
	
	/* Menu items... */
	
	/* Hide/set camera menu items insensitive if no libraries available */
	
	#ifndef HAVE_QSI
	gtk_widget_hide (xml_get_widget (xml_app, "ccd_qsi"));
	gtk_widget_hide (xml_get_widget (xml_app, "filterwheel_int"));
	#endif
	#ifndef HAVE_SX_CAM
	gtk_widget_hide (xml_get_widget (xml_app, "ccd_sx"));
	gtk_widget_hide (xml_get_widget (xml_app, "SX"));
	gtk_widget_hide (xml_get_widget (xml_app, "SX_GuideHead"));
	gtk_widget_hide (xml_get_widget (xml_app, "select_sx_camera"));
	gtk_widget_hide (xml_get_widget (xml_app, "configure_sx_camera"));
	#endif
	#ifndef HAVE_SX_FILTERWHEEL
	gtk_widget_hide (xml_get_widget (xml_app, "filterwheel_sx"));
	gtk_widget_hide (xml_get_widget (xml_app, "select_filter_wheel"));
	gtk_widget_hide (xml_get_widget (xml_app, "configure_filters"));
	#endif
	
	#if !defined (HAVE_QSI) && !defined (HAVE_SX_CAM)
	gtk_widget_hide (xml_get_widget (xml_app, "select_ccd_camera"));
	gtk_widget_hide (xml_get_widget (xml_app, "open_ccd_link"));
	gtk_widget_hide (xml_get_widget (xml_app, "configure_ccd_camera"));
	gtk_widget_hide (xml_get_widget (xml_app, "show_full_frame"));
	gtk_widget_hide (xml_get_widget (xml_app, "debayer"));
	#endif
	
	#if !defined (HAVE_QSI) && !defined (HAVE_SX_FILTERWHEEL)
	gtk_widget_hide (xml_get_widget (xml_app, "sep_comms_filterwheel"));
	gtk_widget_hide (xml_get_widget (xml_app, "filter_wheel_link"));
	gtk_widget_hide (xml_get_widget (xml_app, "open_filter_wheel_link"));
	#endif
	
	/* Hide the Unicap items if libunicap is not available */
	
	#ifndef HAVE_UNICAP
	gtk_widget_hide (xml_get_widget (xml_app, "unicap"));
	gtk_widget_hide (xml_get_widget (xml_app, "unicap_device"));
	gtk_widget_hide (xml_get_widget (xml_app, "unicap_properties")); 
	gtk_widget_hide (xml_get_widget (xml_app, "live_view"));
	#endif
	
    /* Hide the V4L items if libv4l2 is not available */
    
    #ifndef HAVE_LIBV4L2
    gtk_widget_hide (xml_get_widget (xml_app, "V4L_(/dev/video0)"));
    gtk_widget_hide (xml_get_widget (xml_app, "V4L_(/dev/video1)"));
    gtk_widget_hide (xml_get_widget (xml_app, "V4L_(/dev/video2)"));
    gtk_widget_hide (xml_get_widget (xml_app, "V4L_(/dev/video3)"));
    gtk_widget_hide (xml_get_widget (xml_app, "v4l_properties"));
    #endif
    
    #if !defined (HAVE_UNICAP) && !defined (HAVE_LIBV4L2)
    gtk_widget_hide (xml_get_widget (xml_app, "greyscale_conversion"));
    #endif
	
	/* Hide the Grace items if libgrace is not available */
	
	#ifndef HAVE_LIBGRACE_NP
	gtk_widget_hide (xml_get_widget (xml_app, "sep_windows_grace"));
	gtk_widget_hide (xml_get_widget (xml_app, "ccd_temperatures"));
	gtk_widget_hide (xml_get_widget (xml_app, "autoguider_trace"));
	#endif
	
	/* Hide parallel port items if libparapin is not available */
	
	#ifndef HAVE_LIBPARAPIN
	gtk_widget_hide (xml_get_widget (xml_app, "sep_comms_parallelport"));
	gtk_widget_hide (xml_get_widget (xml_app, "parallel_port"));
	gtk_widget_hide (xml_get_widget (xml_app, "open_parallel_port"));
	//gtk_widget_hide (xml_get_widget (xml_app, "V4L_(/dev/video0)_SC1"));
	#endif
	
	/* Get previous values of menu settings */
	
	strcpy (menu.telescope_port, "t_ttyS0");
	R_config_s ("Menu/Settings/TelescopePort", menu.telescope_port);
	/* Catch error if config. database contains old value without preceding   */
	/* "t_".  This was added in version 0.9.7                                 */
	if (strncmp (menu.telescope_port, "t_", 2)) {
		m = g_strconcat ("t_", menu.telescope_port, NULL);
		strcpy (menu.telescope_port, m);
		g_free (m);
	}
	menu.OpenTelPort = R_config_d ("Menu/Settings/OpenTelescopePort", FALSE);
	menu.Gemini = R_config_d ("Menu/Settings/UseGeminiCommands", FALSE);
	
	strcpy (menu.autoguider_port, "a_ttyS1");
	R_config_s ("Menu/Settings/AutoguiderPort", menu.autoguider_port);
	/* Catch error if config. database contains old value without preceding   */
	/* "a_".  This was added in version 0.9.7                                 */
	if (strncmp (menu.autoguider_port, "a_", 2)) {
		m = g_strconcat ("a_", menu.autoguider_port, NULL);
		strcpy (menu.autoguider_port, m);
		g_free (m);
	}
	menu.OpenAutogPort = R_config_d("Menu/Settings/OpenAutoguiderPort", FALSE);
	
	strcpy (menu.filterwheel_port, "w_ttyS3");
	R_config_s ("Menu/Settings/FilterwheelPort", menu.filterwheel_port);
	/* Introduced after version 0.9.7                                         */
	menu.OpenFilterwheelPort = R_config_d (
	                                "Menu/Settings/OpenFilterwheelPort", FALSE);
	
	strcpy (menu.focus_port, "f_ttyS2");
	R_config_s ("Menu/Settings/FocusPort", menu.focus_port);
	/* Catch error if config. database contains old value without preceding   */
	/* "f_".  This was added in version 0.9.7                                 */
	if (strncmp (menu.focus_port, "f_", 2)) {
		m = g_strconcat ("f_", menu.focus_port, NULL);
		strcpy (menu.focus_port, m);
		g_free (m);
	}
	menu.OpenFocusPort = R_config_d("Menu/Settings/OpenFocusPort", FALSE);
	
	#ifdef HAVE_QSI
	strcpy (menu.ccd_camera, "ccd_qsi");
	#elif defined (HAVE_SX_CAM)
	strcpy (menu.ccd_camera, "ccd_sx");
	#else
	strcpy (menu.ccd_camera, "");
	#endif
	R_config_s ("Menu/Settings/CCDCamera", menu.ccd_camera);	
	menu.FullFrame = R_config_d ("Menu/Settings/FullFrame", FALSE);
	#ifdef HAVE_LIBV4L_2
	strcpy (menu.aug_camera, "V4L_(/dev/video0)");
	#elif defined (HAVE_SX_CAM)
	strcpy (menu.aug_camera, "SX");
	#elif defined (HAVE_UNICAP)
	strcpy (menu.aug_camera, "unicap");
	#else
	strcpy (menu.aug_camera, "");
	#endif
	R_config_s ("Menu/Settings/AutoguiderCamera", menu.aug_camera);
	strcpy (menu.debayer, "simple");
	R_config_s ("Menu/Settings/Debayer", menu.debayer);
	strcpy (menu.greyscale, "maximum");
	R_config_s ("Menu/Settings/Greyscale", menu.greyscale);
	#ifdef HAVE_QSI
	strcpy (menu.filterwheel, "filterwheel_int");
	#elif defined (HAVE_SX_FILTERWHEEL)
	strcpy (menu.filterwheel, "filterwheel_sx");
	#else
	strcpy (menu.filterwheel, "");
	#endif
	R_config_s ("Menu/Settings/Filterwheel", menu.filterwheel);
	strcpy (menu.focuser, "focuser_robofocus");
	R_config_s ("Menu/Settings/Focuser", menu.focuser);	
	menu.Precess = R_config_d ("Menu/Settings/Precess", FALSE);
	menu.UTC = R_config_d ("Menu/Settings/UTC", FALSE);
	menu.ShowToolbar = R_config_d ("Menu/Settings/ShowToolbar", FALSE);
	
	/* Activate menu items */
	
	if (menu.Gemini)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                       "gemini_commands")));
	if (strcmp (menu.ccd_camera, ""))
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                         menu.ccd_camera)));
	if (menu.FullFrame)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                       "show_full_frame")));
	gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                            menu.debayer)));
	if (strcmp (menu.aug_camera, ""))
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                         menu.aug_camera)));
	gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                          menu.greyscale)));
	gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                            menu.focuser)));
	if (strcmp (menu.filterwheel, ""))
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                        menu.filterwheel)));
	if (menu.Precess)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                        "precess_coords")));
	if (menu.UTC)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app, 
	                                                               "use_utc")));
	gtk_check_menu_item_set_active (
				GTK_CHECK_MENU_ITEM (xml_get_widget (xml_app, "show_toolbar")), 
				                     menu.ShowToolbar);
	                                                               
	/* Create Communications menu entries for comms ports and set pointers to
	 * the currently selected ports.
	 */
	
	comms_menus_update_ports ();
	
	gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			xml_app, "telescope_link_menu")), comms_ports_set_active, NULL);
	gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			xml_app, "autoguider_link_menu")), comms_ports_set_active, NULL);
	gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			xml_app, "filter_wheel_link_menu")), comms_ports_set_active, NULL);
	gtk_container_foreach (GTK_CONTAINER (xml_get_widget (
			xml_app, "focuser_link_menu")), comms_ports_set_active, NULL);
	
	/* Re-open any previously open ports */
	
	if (menu.OpenTelPort)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app,
												       "open_telescope_link")));
	if (menu.OpenAutogPort)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app,
												      "open_autoguider_link")));
	if (menu.OpenFilterwheelPort)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app,
												    "open_filter_wheel_link")));
	if (menu.OpenFocusPort)
		gtk_menu_item_activate (GTK_MENU_ITEM (xml_get_widget (xml_app,
												         "open_focuser_link")));

	/* Autoguider settings... */	
	
	RemoteTiming = R_config_d ("LastConfig/RemoteTiming", FALSE);	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app,
	                                                   "chkAutogRemoteTiming"));
	gtk_toggle_button_set_active (button, RemoteTiming);	
	
	s = (struct autog_config *) g_malloc0 (sizeof (struct autog_config));
	s->Telescope = (gchar *) g_malloc0 (MCSL * sizeof (gchar));
	s->Instrument = (gchar *) g_malloc0 (MCSL * sizeof (gchar));	
	
	R_config_s ("LastConfig/Telescope", s->Telescope);
	R_config_s ("LastConfig/Instrument", s->Instrument);
	s->NSCalibDuration = R_config_f ("LastConfig/NSCalibDuration", 5.0);
	s->EWCalibDuration = R_config_f ("LastConfig/EWCalibDuration", 5.0);
	s->GuideSpeed = R_config_f ("LastConfig/GuideSpeed", 0.5);
	s->MaxShift = R_config_f ("LastConfig/MaxShift", 1.0);
	s->MaxDrift = R_config_f ("LastConfig/MaxDrift", 1.0);
	s->MaxMove = R_config_f ("LastConfig/MaxMove", 1.0);
	s->MaxOffset = R_config_f ("LastConfig/MaxOffset", 1.0);
	s->DriftSample = R_config_f ("LastConfig/DriftSample", 10.0);
	s->CorrFac = R_config_f ("LastConfig/CorrFac", 1.0);
	s->Update = R_config_f ("LastConfig/Update", 1.0);
	s->GuideDirn = R_config_d ("LastConfig/GuideDirn", TM_ALL);
	s->SimulGuide = R_config_d ("LastConfig/SimulGuide", 0);
	s->DriftNSOnly = R_config_d ("LastConfig/DriftNSOnly", 0);
	aug->imdisp.stdev = R_config_f ("LastConfig/SkyStdev", 3.0);
	aug->canv.csize = R_config_d ("LastConfig/CentroidSize", 17);
	
	set_entry_string ("txtTelescop", s->Telescope);
	set_entry_string ("txtInstrume", s->Instrument);
	set_entry_float ("txtAutogNSCalibDuration", s->NSCalibDuration);
	set_entry_float ("txtAutogEWCalibDuration", s->EWCalibDuration);
	set_spin_float ("spbAutogGuideSpeed", s->GuideSpeed);
	set_spin_float ("spbAutogMaxShift", s->MaxShift);
	set_spin_float ("spbAutogMaxDrift", s->MaxDrift);
	set_spin_float ("spbAutogMaxMove", s->MaxMove);
	set_spin_float ("spbAutogMaxOffset", s->MaxOffset);
	set_spin_int ("spbAutogDriftSample", s->DriftSample);	
	set_spin_float ("spbAutogCorrFac", s->CorrFac);
	set_entry_float ("txtAutogUpdate", s->Update);
	set_entry_float ("txtAutogSkySigma", aug->imdisp.stdev);
	set_entry_int ("txtAutogCentroidSize", aug->canv.csize);
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideNorth"));	
	gtk_toggle_button_set_active (button, s->GuideDirn & TM_NORTH);	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideSouth"));	
	gtk_toggle_button_set_active (button, s->GuideDirn & TM_SOUTH);	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideEast"));	
	gtk_toggle_button_set_active (button, s->GuideDirn & TM_EAST);	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogGuideWest"));	
	gtk_toggle_button_set_active (button, s->GuideDirn & TM_WEST);		
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app, "chkAutogSimulGuide"));
	gtk_toggle_button_set_active (button, s->SimulGuide);		
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_app,"chkAutogDriftNSOnly"));
	gtk_toggle_button_set_active (button, s->DriftNSOnly);		
	
	g_free (s->Telescope);
	g_free (s->Instrument);
	g_free (s);
	
	/* Focus settings */
	
	set_entry_float ("txtFocusLHSlope", R_config_f ("Focus/0/LHSlope", -1.0));
	set_entry_float ("txtFocusRHSlope", R_config_f ("Focus/0/RHSlope", 1.0));
	set_entry_float ("txtFocusPID", R_config_f ("Focus/0/PID", 0.0));
	set_entry_int ("txtFocusAFStart", R_config_d ("Focus/0/AFStart", 0));
	set_entry_float ("txtFocusNearHFD", R_config_f ("Focus/0/NearHFD", 10.0));
	set_entry_float ("txtFocusTempCoeff", R_config_f("Focus/0/TempCoeff",0.1));
	set_entry_float ("txtFocusDeltaT", R_config_f ("Focus/0/DeltaT", 5.0));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
				  xml_get_widget (xml_app, "chkFocusApplyFilterOffsets")),
				  R_config_d ("Focus/ApplyFilterOffsets", FALSE));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
				  xml_get_widget (xml_app, "chkFocusFastReadout")),
				  R_config_d ("Focus/FastReadout", FALSE));
				  
	/* Parallel port settings */
	
	#ifdef HAVE_LIBPARAPIN
	strcpy (ports[LPT].address, "378");
	R_config_s ("Ports/Parallel/Address", ports[LPT].address);
	ports[LPT].RAp = R_config_d ("Ports/Parallel/RAp", 6);
	ports[LPT].RAm = R_config_d ("Ports/Parallel/RAm", 7);
	ports[LPT].Decp = R_config_d ("Ports/Parallel/Decp", 8);
	ports[LPT].Decm = R_config_d ("Ports/Parallel/Decm", 9);
	//ports[LPT].SC1LongExp = R_config_d ("Ports/Parallel/SC1LongExp", 2);
	//ports[LPT].SC1Pause = R_config_d ("Ports/Parallel/SC1Pause", 10);
	
	menu.OpenParPort = R_config_d ("Menu/Settings/OpenParPort", FALSE);
	if (menu.OpenParPort)
		menu.OpenParPort = telescope_open_parallel_port ();
	if (menu.OpenParPort)
		reset_checkbox_state (RCS_OPEN_PARPORT, TRUE);
	else
		reset_checkbox_state (RCS_OPEN_PARPORT, FALSE);
	#endif
	
	/* General CCD camera setup */
	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
				               xml_get_widget (xml_app, "chkIgnoreCCDCooling")),
                               R_config_d ("Misc/IgnoreCCDCooling", FALSE));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
				               xml_get_widget (xml_app, "chkDisplayCCDImage")),
                               R_config_d ("Misc/DisplayCCDImage", TRUE));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
				               xml_get_widget (xml_app, "chkBeepExposure")),
                               R_config_d ("Misc/Beep", FALSE));
}

static void save_config_data (void)
{
	/* Save values to configuration file */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	gchar *s;
	
	aug->autog.s.Telescope = get_entry_string ("txtTelescop");
	aug->autog.s.Instrument = get_entry_string ("txtInstrume");			
	get_autog_guide_params ();
	
	W_config_d ("LastConfig/RemoteTiming", aug->autog.s.RemoteTiming);	
	W_config_s ("LastConfig/Telescope", aug->autog.s.Telescope);
	W_config_s ("LastConfig/Instrument", aug->autog.s.Instrument);
	W_config_f ("LastConfig/NSCalibDuration", aug->autog.s.NSCalibDuration);
	W_config_f ("LastConfig/EWCalibDuration", aug->autog.s.EWCalibDuration);
	W_config_f ("LastConfig/GuideSpeed", aug->autog.s.GuideSpeed);
	W_config_f ("LastConfig/MaxShift", aug->autog.s.MaxShift);
	W_config_f ("LastConfig/MaxDrift", aug->autog.s.MaxDrift);
	W_config_f ("LastConfig/MaxMove", aug->autog.s.MaxMove);
	W_config_f ("LastConfig/MaxOffset", aug->autog.s.MaxOffset);
	W_config_f ("LastConfig/DriftSample", aug->autog.s.DriftSample);	
	W_config_f ("LastConfig/CorrFac", aug->autog.s.CorrFac);
	W_config_f ("LastConfig/Update", aug->autog.s.Update);	
	W_config_d ("LastConfig/GuideDirn", aug->autog.s.GuideDirn);
	W_config_d ("LastConfig/SimulGuide", aug->autog.s.SimulGuide);
	W_config_d ("LastConfig/DriftNSOnly", aug->autog.s.DriftNSOnly);
	W_config_f ("LastConfig/SkyStdev", aug->imdisp.stdev);
	W_config_d ("LastConfig/CentroidSize", aug->canv.csize);
	
	g_free (aug->autog.s.Telescope);
	g_free (aug->autog.s.Instrument);
	
	s = get_entry_string ("txtFocusLHSlope");
	W_config_s ("Focus/0/LHSlope", s);
	g_free (s);
	s = get_entry_string ("txtFocusRHSlope");
	W_config_s ("Focus/0/RHSlope", s);
	g_free (s);
	s = get_entry_string ("txtFocusPID");
	W_config_s ("Focus/0/PID", s);
	g_free (s);
	s = get_entry_string ("txtFocusAFStart");
	W_config_s ("Focus/0/AFStart", s);
	g_free (s);
	s = get_entry_string ("txtFocusNearHFD");
	W_config_s ("Focus/0/NearHFD", s);
	g_free (s);
	s = get_entry_string ("txtFocusTempCoeff");
	W_config_s ("Focus/0/TempCoeff", s);
	g_free (s);
	s = get_entry_string ("txtFocusDeltaT");
	W_config_s ("Focus/0/DeltaT", s);
	g_free (s);
	
	W_config_s ("Menu/Settings/TelescopePort", menu.telescope_port);
	W_config_d ("Menu/Settings/OpenTelescopePort", menu.OpenTelPort);
	W_config_d ("Menu/Settings/UseGeminiCommands", menu.Gemini);
	W_config_s ("Menu/Settings/AutoguiderPort", menu.autoguider_port);	
	W_config_d ("Menu/Settings/OpenAutoguiderPort", menu.OpenAutogPort);
	W_config_s ("Menu/Settings/FilterwheelPort", menu.filterwheel_port);	
	W_config_d ("Menu/Settings/OpenFilterwheelPort", menu.OpenFilterwheelPort);
	W_config_s ("Menu/Settings/FocusPort", menu.focus_port);	
	W_config_d ("Menu/Settings/OpenFocusPort", menu.OpenFocusPort);
	W_config_d ("Menu/Settings/OpenParPort", menu.OpenParPort);
	W_config_s ("Menu/Settings/CCDCamera", menu.ccd_camera);
	W_config_d ("Menu/Settings/FullFrame", menu.FullFrame);
	W_config_s ("Menu/Settings/Debayer", menu.debayer);
	W_config_s ("Menu/Settings/AutoguiderCamera", menu.aug_camera);
	W_config_s ("Menu/Settings/Greyscale", menu.greyscale);
	W_config_s ("Menu/Settings/Filterwheel", menu.filterwheel);
	W_config_s ("Menu/Settings/Focuser", menu.focuser);
	W_config_d ("Menu/Settings/Precess", menu.Precess);
	W_config_d ("Menu/Settings/UTC", menu.UTC);
	W_config_d ("Menu/Settings/ShowToolbar", menu.ShowToolbar);
	
	W_config_s ("File/WatchFile/URI", gtk_file_chooser_get_uri (
				GTK_FILE_CHOOSER (xml_get_widget (xml_app, "flcWatchFolder"))));
	s = get_entry_string ("txtWatchFile");
	W_config_s ("File/WatchFile/Name", s);
	g_free (s);
	W_config_d ("File/WatchFile/Watch", gtk_toggle_button_get_active (
			    GTK_TOGGLE_BUTTON (xml_get_widget (xml_app,"chkWatchActive"))));
	
	#ifdef HAVE_LIBPARAPIN
	W_config_s ("Ports/Parallel/Address", ports[LPT].address);
	W_config_d ("Ports/Parallel/RAp", ports[LPT].RAp);
	W_config_d ("Ports/Parallel/RAm", ports[LPT].RAm);
	W_config_d ("Ports/Parallel/Decp", ports[LPT].Decp);
	W_config_d ("Ports/Parallel/Decm", ports[LPT].Decm);
	//W_config_d ("Ports/Parallel/SC1LongExp", ports[LPT].SC1LongExp);
	//W_config_d ("Ports/Parallel/SC1Pause", ports[LPT].SC1Pause);
	#endif
}

static void restore_watch_file (void)
{
	/* Restore the "watch file" settings */
	
	gchar *s, *w, *w1;
	gboolean Active;
	
	w = (gchar *) g_malloc0 (MCSL * sizeof (gchar));
	
	s = g_strconcat ("file://", UserDir, NULL);
	strcpy (w, s);
	g_free (s);
	R_config_s ("File/WatchFile/URI", w);
	gtk_file_chooser_set_uri (GTK_FILE_CHOOSER (
								xml_get_widget (xml_app, "flcWatchFolder")), w);
	s = g_filename_from_uri (w, NULL, NULL);
	w1 = g_strconcat (s, "/", NULL);
	g_free (s);
	
	strcpy (w, "");
	R_config_s ("File/WatchFile/Name", w);
	set_entry_string ("txtWatchFile", w);
	WatchFile = g_strconcat (w1, w, NULL);
	
	g_free (w1);
	g_free (w);
	
	Active = R_config_d ("File/WatchFile/Watch", FALSE);
	
	/* Normally, just activating the "watch file" toggle button would be 
	 * sufficient to get the watch file folder and name and grey out the
	 * controls via on_chkWatchActive_toggled.  But if you query the uri at this
	 * point it still seems to be null, even though it has apparently been set 
	 * above (possibly because this function is called before the gtk main loop 
	 * has started).  So we set ResetChkState to TRUE to have 
	 * on_chkWatchActive_toggled return immediately after setting the 'tick' 
	 * marker and then activate the file watch from here using the "watch file" 
	 * name returned from the configuration file.
	 */
	 
	if (Active) {
		ResetChkState = TRUE;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
							 xml_get_widget (xml_app, "chkWatchActive")), TRUE);
		gtk_widget_set_sensitive (
							 xml_get_widget (xml_app,"flcWatchFolder"), FALSE);
		gtk_widget_set_sensitive (
							 xml_get_widget (xml_app, "txtWatchFile"), FALSE);
		loop_watch_activate (TRUE);
	}				  
}

void save_PEC_guide_speed (gfloat GuideSpeed)
{
	/* Save the value of guide speed associated with the loaded PEC data to the
	 * configuration database.
	 */
	
	W_config_f ("PEC/GuideSpeed", GuideSpeed);
}

void save_RA_worm_pos (gushort WormPos)
{
	/* Save the value of the RA worm position */
	
	W_config_d ("PEC/WormPos", WormPos);
	L_print ("{b}Saved RA worm position: %d\n", WormPos);
}

gfloat get_goto_motion_limit (void)
{
	/* Get the motion limit for detecting end of GoTo operation for non-Gemini
	 * mounts.  (Default is 15 arcsec).
	 */
	
	return R_config_f ("Misc/GoToMotionLimit", 15.0);
}

void reset_checkbox_state (enum CheckBox chkbox, gboolean active)
{
	/* Reset a check box or toggle button to the desired state, to make the 
	 * visual appearance consistent with the state of the application (e.g. if 
	 * opening a window has failed to work then the check box needs to be 
	 * 'unticked').
	 */
	
	gchar *chk;

	ResetChkState = TRUE;
	
	switch (chkbox) {
		case RCS_OPEN_COMMS_LINK:
			chk = "open_telescope_link";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_CCD_LINK:
			chk = "open_ccd_link";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_AUTOG_LINK:
			chk = "open_autoguider_link";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_FILTER_LINK:
			chk = "open_filter_wheel_link";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_FOCUS_LINK:
			chk = "open_focuser_link";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_PARPORT:
			chk = "open_parallel_port";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_PEC_ON:
			chk = "periodic_error_correction";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_ACTIVATE_WATCH:
			chk = "chkWatchActive";
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_LIVEVIEW_WINDOW:
			chk = "live_view";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_PLAYBACK_WINDOW:
			chk = "playback";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_TRACE_WINDOW:
			chk = "autoguider_trace";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_OPEN_TEMPS_WINDOW:
			chk = "ccd_temperatures";
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_USE_AUTOGUIDER:
			chk = "chkAutogOpen";
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
								 (xml_get_widget (xml_app, chk)), active);
		    break;
		case RCS_LV_RECORD:
			chk = "tglLVRecord";
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
								 (xml_get_widget (xml_lvw, chk)), active);
		    break;
		default:
			break;
	}
}

gboolean check_format (gboolean is_RA, gchar s[])
{
	/* Check a RA/Time or Dec string for the correct format i.e. (+/-)nn.nn.nn 
	 * where n is a decimal digit and '.' is any non-decimal digit 
	 * (i.e. the separator can be anything apart from a number).
	 * Note that all six digits must be present; leading or trailing blanks
	 * are not allowed.  The routine inserts a colon as the separator.
	 * If the format is valid, check that it is numerically appropriate.
	 */
	
	gint i, n, dd, hh, mm, ss;
	gchar *endm, *ends;
	
	/* If the entry text string is apparently a task parameter (i.e. begins
	 * with '%'), then if the task list is running in TestOnly mode, return TRUE
	 * if the task parameter is valid or FALSE if it is invalid.  We return at 
	 * this point if the task list is running in TestOnly mode because the value
	 * corresponding to the task parameter may not yet have been set.
	 */
	
	s = get_task_param_value (FALSE, s, &n);
	if (n) {
		if (tasks_get_status () & TSK_TESTONLY) {
			return n > 0 ? TRUE : FALSE;
		}
	}
	
	if (is_RA) { /* i.e. a right ascension (or time) value */
		for (i = 0; i < 8; i++) {
			if (!isdigit (s[i])) {
				if (i != 2 && i != 5) {
					L_print ("{r}RA/Time not in hh:mm:ss format!\n");
					return FALSE;
				} else
				    s[i] = ':';
			}
		}

		hh = (gint) strtol (s, &endm, 10);
		mm = (gint) strtol (++endm, &ends, 10);
		ss = (gint) strtol (++ends, (gchar **) NULL, 10);
		if (hh > 24 || mm > 59 || ss > 59) {
			L_print ("{r}hh, mm or ss too large for RA/Time!\n");
			return FALSE;
		}
		if (hh == 24 && (mm > 0 || ss > 0)) {
			L_print("{r}Can't exceed 24 hours for RA/Time!\n");
			return FALSE;
		}
	}	
	
	if (!is_RA) { /* i.e. a declination value */
		if (strncmp (s, "+", 1) && strncmp (s, "-", 1)) {
			if (!isdigit (s[0])) {
				L_print ("{r}Dec not in (+/-)dd:mm:ss format!\n");
				return FALSE;
			}
			i = 9;
			s[i] = '\0';
			while (i--)
				s[i] = s[i - 1];
			s[0] = '+';  /* Missing "+", so insert one */
		}
				
		for (i = 1; i < 9; i++) {
			if (!isdigit (s[i])) {
				if (i != 3 && i != 6) {
					L_print ("{r}Dec not in (+/-)dd:mm:ss format!\n");
					return FALSE;
				} else
				    s[i] = ':';
			}
		}

		dd = (gint) strtol (s+1, &endm, 10);
		mm = (gint) strtol (++endm, &ends, 10);
		ss = (gint) strtol (++ends, (gchar **) NULL, 10);
		if (dd > 90 || mm > 59 || ss > 59) {
			L_print ("{r}dd, mm or ss too large for Dec!\n");
			return FALSE;
		}
		if (dd == 90 && (mm > 0 || ss > 0)) {
			L_print ("{r}Can't exceed 90 degrees north or south for Dec!\n");
			return FALSE;
		}
	}
	return TRUE;
}

struct tm *get_time (gboolean UTC)
{
	/* Return the current time to the nearest second */
	
	time_t tt;

	time (&tt);
	return UTC ? gmtime (&tt) : localtime (&tt);
}

gint get_UTC_offset (void)
{
	/* Get the offset between local time and UTC (integer hours only) */
	
	struct tm *dt;
    time_t t;
	gint l_d, l_h, g_d, g_h;
	
	time (&t);
	dt = gmtime (&t);     /* UTC */
	g_d = dt->tm_mday;
	g_h = dt->tm_hour;
	dt = localtime (&t);  /* Local time */
	l_d = dt->tm_mday;
	l_h = dt->tm_hour;
	
    /* Calculate UTC offset - allow for UTC and local time being on consecutive 
	 * days spanning end of month.  (Note this works for integer offsets only).
	 */

    l_d = abs (l_d - g_d) > 1 ? ((l_d > g_d) ? l_d : l_d + g_d) : l_d;
    g_d = abs (l_d - g_d) > 1 ? ((g_d > l_d) ? g_d : l_d + g_d) : g_d;
	return 24 * (l_d - g_d) + (l_h - g_h);
}

void finished_tasks (void)
{
	/* This routine is called by the tasks loop when the end of the task list
	 * has been reached.
	 */
	
	set_task_buttons (FALSE);
}

static void warn_PEC_guidespeed (gboolean MsgBox)
{
	/* Warn user if guide speed associated with most recently uploaded PEC data
	 * is different from presently set guide speed value.
	 */
	
	gfloat Autog_speed, PEC_speed;
	gdouble val;
	gchar *string;
	
	get_spin_float ("spbAutogGuideSpeed", &val);
	Autog_speed = (gfloat) val;
	PEC_speed = R_config_f ("PEC/GuideSpeed", 0.0);
	
	if (PEC_speed >= 0.2 && PEC_speed <= 0.8 && PEC_speed != Autog_speed) {
		if (MsgBox) { 
			string = g_strdup_printf ("Warning - Guide speed of %3.1f does not "
								 "match most recently set PEC speed of %3.1f\n", 
								 Autog_speed, PEC_speed);
			msg (string);
			g_free (string);
		}
		L_print ("{o}Warning - Guide speed of %3.1f does not match most "
				   "recently set PEC speed of %3.1f\n", Autog_speed, PEC_speed);
	}
}

void msg (const gchar *message)
{
	/* Originally this routine displayed a message (typically a warning message)
	 * to the user via a dialog box.  But this isn't much use for unattended
	 * operation, so now an L_print is used with the message coloured orange.
	 * Furthermore, L_print can be called from any thread, whereas the dialog
	 * box had to be displayed via the main gtk thread which was problematical 
	 * in some cases.
	 */
	
	//~ GtkWidget *dlgMsg;
	
	//~ if (!tasks_get_status ()) {
		//~ gdk_window_raise (xml_get_widget (xml_app, "ccdApp")->window);
    	//~ dlgMsg = gtk_message_dialog_new (ccdApp,
	    	                            //~ GTK_DIALOG_DESTROY_WITH_PARENT,
									    //~ GTK_MESSAGE_INFO,
										//~ GTK_BUTTONS_OK,
	                	                //~ message);
		//~ gtk_dialog_run (GTK_DIALOG (dlgMsg));
		//~ gtk_widget_destroy (dlgMsg);
	//~ } else
		//~ L_print ("%s\n", message);
	
	L_print ("{o}%s\n", message);
}

void err_msg (gchar *entry_text, const gchar *message)
{
	/* Originally this routine displayed an error message relating to incorrect
	 * data entry to the user via a dialog box.  But this isn't much use for 
	 * unattended operation, so now an L_print is used with the message coloured
	 * red.
	 * Furthermore, L_print can be called from any thread, whereas the dialog
	 * box had to be displayed via the main gtk thread which was problematical 
	 * in some cases.
	 */
	
	//~ GtkWidget *dlgError;
	//~ GString *text;
    
	//~ /* Display message */
	
    //~ text = g_string_new (entry_text);
	//~ g_string_append (text, message);
	
	//~ if (!tasks_get_status ()) {
    	//~ dlgError = gtk_message_dialog_new (ccdApp,
	    	                              //~ GTK_DIALOG_DESTROY_WITH_PARENT,
										  //~ GTK_MESSAGE_ERROR,
										  //~ GTK_BUTTONS_OK,
	                	                  //~ text->str);
		//~ gtk_dialog_run (GTK_DIALOG (dlgError));
		//~ gtk_widget_destroy (dlgError); 
	//~ } else
		//~ L_print ("{r}%s\n", text->str);
	
	//~ g_string_free (text, FALSE);
	
	L_print ("{r}'%s' %s\n", entry_text, message);
}

gboolean show_error (const gchar *routine, const gchar *message)
{
	/* Display error message to user */

	L_print ("{r}%s: %s\n", routine, message);
	return FALSE;
}

static void InitApp (void)
{
	/* Initialise the application */
	
    struct stat statbuf;
    GFile *source, *dest;
	GdkColor gdk_red = {0, 49151, 0, 0};
	GdkColor gdk_orange = {0, 49151, 49151, 0};	
	GdkColor gdk_blue = {0, 0, 0, 49151};
	GdkColor gdk_green = {0, 0, 49151, 0};
	GdkColor gdk_magenta = {0, 49151, 0, 49151};
	const gchar *f = "Courier";
	gchar *DefaultConfigFile, *LogPath, *string, *uri;
    gboolean OK, RefreshFile = FALSE;
	
	/* Set debugging messages off */
	
	Debug = FALSE;  /* Must match initial state in Glade gtkbuilder file */
	
    /* Create hash table for storing combo box pointers */
    
    hshCombo = g_hash_table_new (NULL, NULL);
    
	/* Make sure that the home directory exists */
	
	UserDir = g_get_home_dir ();
	PrivatePath = g_build_filename (UserDir, "GoQat", NULL);
	if (opendir (PrivatePath) == NULL)
		if (mkdir (PrivatePath, S_IRUSR | S_IWUSR | S_IXUSR) < 0) {
			L_print ("{r}Error - Can't create home directory!\n");
			PrivatePath = NULL;
			return;
		}
    
    /* Copy default configuration file from installation directory if it does 
     * not exist, or is empty.
     */
    
    ConfigFile = g_build_filename (PrivatePath, CONFIG_FILE, NULL);
    if (!g_file_test (ConfigFile, G_FILE_TEST_EXISTS)) /* File doesn't exist */
        RefreshFile = TRUE;
    else {
        g_stat (ConfigFile, &statbuf);
        if (!statbuf.st_size)                /* File does exist but is empty */
            RefreshFile = TRUE;
    }
    if (RefreshFile) {
        DefaultConfigFile = g_build_filename (GOQAT_DATADIR, CONFIG_FILE, NULL);
        source = g_file_new_for_path (DefaultConfigFile);
        dest = g_file_new_for_path (ConfigFile);
        OK = g_file_copy (source, dest, G_FILE_COPY_OVERWRITE, 
                          NULL, NULL, NULL, NULL);
        g_free (DefaultConfigFile);
        if (!OK)
            g_error ("Unable to create configuration file: %s\n", ConfigFile); 
    }
    
	/* Open the log file */
	
	LogPath = g_build_filename (PrivatePath, "log.txt", NULL);
	f_log = fopen (LogPath, "a");
	g_free (LogPath);
	
	/* Get a pointer to the log window */
	
	txtLog = GTK_TEXT_VIEW (xml_get_widget (xml_app, "txtLog"));
	
	/* Set up the text buffer for the log window */
	
	txtLogBuffer = gtk_text_view_get_buffer (txtLog); 
	
	/* Define tags for colours */
	
	red = gtk_text_buffer_create_tag (txtLogBuffer, "red", "foreground-gdk", 
	                                                            &gdk_red, NULL);
	orange = gtk_text_buffer_create_tag (txtLogBuffer, "orange","foreground-gdk", 
	                                                         &gdk_orange, NULL);	
	blue = gtk_text_buffer_create_tag (txtLogBuffer, "blue", "foreground-gdk", 
	                                                           &gdk_blue, NULL);
	green = gtk_text_buffer_create_tag (txtLogBuffer, "green", "foreground-gdk", 
	                                                          &gdk_green, NULL);
	magenta =gtk_text_buffer_create_tag(txtLogBuffer,"magenta","foreground-gdk", 
	                                                        &gdk_magenta, NULL);
	
	string = g_strdup_printf ("\n\n###### Opened new message logging session "
							  "for GoQat %s ######\n\n", VERSION);
	
	L_print ("%s", string);      /* Write message to log window and file */
	if (f_log)
		fprintf (f_log, "%s", string);
	g_free (string);
	
	/* Get a pointer to the CCD camera status window */
		
	txtStatus = GTK_TEXT_VIEW (xml_get_widget (xml_app, "txtStatus"));
		
	/* Set up the text buffer for the camera status window */

	txtStatusBuffer = gtk_text_view_get_buffer (txtStatus); 
		
	/* Define tag for font */
	
	courier = gtk_text_buffer_create_tag (txtStatusBuffer,"courier","font", f,
										                                  NULL);
										                                 
	/* Explicitly set the default folder paths for the file chooser controls.
	 * This goes against recommendations in the gtk documentation, but the
	 * controls are initialised with no path selected on some systems otherwise.
	 */
                                  
    uri = g_filename_to_uri (UserDir, NULL, NULL);
    gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (xml_get_widget (
											 xml_app, "flcCCDFolder")), uri);
    gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (xml_get_widget (
											 xml_app, "flcAUGFolder")), uri);
    gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (xml_get_widget (
											 xml_app, "flcWatchFolder")), uri);
	g_free (uri);
	
	/* Define the serial port structures */
	
	serial_define_comms_ports ();
	
	/* Make sure that the default camera selections correspond to cameras for
	 * which the relevant libraries are available, at the first invocation.
	 */
	 
	if (R_config_d ("App/FirstInvocation", TRUE)) {
		/* CCD cameras */
		#if defined (HAVE_QSI)
			W_config_s ("Menu/Settings/CCDCamera", "ccd_qsi");
		#elif defined (HAVE_SX_CAM)
			W_config_s ("Menu/Settings/CCDCamera", "ccd_sx");
		#endif
		/* Autoguider cameras */
		#if defined (HAVE_SX_CAM)
			W_config_s ("Menu/Settings/AutoguiderCamera", "SX");
		#elif defined (HAVE_LIBV4L2)
			W_config_s ("Menu/Settings/AutoguiderCamera", "V4L_(/dev/video0)");
		#elif defined (HAVE_UNICAP)
			W_config_s ("Menu/Settings/AutoguiderCamera", "unicap");
		#endif
		W_config_d ("App/FirstInvocation", FALSE);
	}
}

void WriteLog (char *string)
{
	/* Write message string to log buffer and flush to log window immediately
	 * if being called by the main gtk thread.  Otherwise (i.e. if called by
	 * other threads) leave messages to be flushed via the event loop code which
	 * runs in the main gtk thread.
	 * 
	 * This function is thread safe and can be called from anywhere.
	 */
	 
	struct tm *dt;
	gchar *s, *line;
	
	
	if (strncmp (string, "{.}", 3) != 0) {/*Insert time if doesn't begin "{.}"*/
		dt = get_time (UseUTC);
		s = g_strdup_printf ("%4i-%02i-%02i, %02i:%02i:%02i > ", 
							 1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday, 
							 dt->tm_hour, dt->tm_min, dt->tm_sec);
	} else {
		s = g_strdup_printf ("%s", "");
		string = string + 3;
	}
	
	if (strncmp (string, "{r}", 3) == 0)
		line = g_strconcat ("{r}", s, &string[3], NULL);
	else if (strncmp (string, "{o}", 3) == 0)
		line = g_strconcat ("{o}", s, &string[3], NULL);
	else if (strncmp (string, "{b}", 3) == 0)
		line = g_strconcat ("{b}", s, &string[3], NULL);
	else if (strncmp (string, "{g}", 3) == 0)
		line = g_strconcat ("{g}", s, &string[3], NULL);
	else
		line = g_strconcat (s, string, NULL);
	g_free (s);
	
	g_static_mutex_lock (&LogMutex);
	if (MsgNum < MAX_LOG_BUF_MSG)	
		LogBuf[MsgNum++] = line;
	else
		printf ("GoQat lost message: %s\n", line);
	g_static_mutex_unlock (&LogMutex);

	/* If the calling thread is the same as the main thread (i.e. the thread in
	 * which gtk is running), then flush the log to the log window immediately.
	 * Otherwise, exit and wait for the log to be flushed by the call from
	 * loop.c, which is also running in the main gtk thread.
	 */
	 
	if (main_tid == (pid_t) syscall (SYS_gettid))	
		FlushLog ();	
}

void FlushLog (void)
{
	/* Write pending messages to log window, and to log file if task list is 
	 * being executed.
	 *
	 * The string may have the following prefixes:
	 * {.} - Don't prefix date and time (see WriteLog)
	 * {r} - Colour string red
	 * {o} - Colour string orange
	 * {b} - Colour string blue
	 * {g} - Colour string green
	 *
	 * The {.} prefix may be used immediately before any of the colour prefixes.
	 * The colour of the string is magenta by default.
	 */
	 
	GtkTextIter end;
	GtkTextMark *mark = NULL;
	gint i;
	gchar *line;
	
	g_static_mutex_lock (&LogMutex);
	
	for (i = 0; i < MsgNum; i++) {
	
		gtk_text_buffer_get_iter_at_offset (txtLogBuffer, &end, -1);
		line = LogBuf[i];	
		
		if (strncmp (line, "{r}", 3) == 0)         /* Colour text red */
			gtk_text_buffer_insert_with_tags (txtLogBuffer, &end, 
											  &line[3], -1, red, NULL);
		else if (strncmp (line, "{o}", 3) == 0)  /* Colour text orange */
			gtk_text_buffer_insert_with_tags (txtLogBuffer, &end,
											  &line[3], -1, orange, NULL);
		else if (strncmp (line, "{b}", 3) == 0)  /* Colour text blue */
			gtk_text_buffer_insert_with_tags (txtLogBuffer, &end,
											  &line[3], -1, blue, NULL);
		else if (strncmp (line, "{g}", 3) == 0)  /* Colour text green */
			gtk_text_buffer_insert_with_tags (txtLogBuffer, &end,
											  &line[3], -1, green, NULL);
		else                                       /* Colour text magenta */
			gtk_text_buffer_insert_with_tags (txtLogBuffer, &end,
											  line, -1, magenta, NULL);
		
		/* Write to log file if task list running */
		
		if ((tasks_get_status () & TSK_ACTIVE) && f_log)
			fprintf (f_log, "%s", line);
		
		g_free (LogBuf[i]);
	}
	
	/* Scroll the log window to make added text visible */
	
	if (MsgNum) {
		gtk_text_buffer_get_iter_at_offset (txtLogBuffer, &end, -1);	
		mark = gtk_text_buffer_create_mark (txtLogBuffer, NULL, &end, FALSE);
		gtk_text_view_scroll_to_mark (txtLog, mark, 0.0, TRUE, 1.0, 1.0);
	}
	
	MsgNum = 0;
	g_static_mutex_unlock (&LogMutex);
}

void WritePhot (char *string)
{
	/* Write string to photometry window.
	 *
	 * The string may have the following prefixes:
	 * {.} - Don't prefix date and time
	 *
	 * The {.} prefix may be used immediately before any other prefixes.
	 * The colour of the string is magenta by default.
	 */

	GtkTextIter end;
	GtkTextMark *mark = NULL;
	struct tm *dt;
	gchar *s, *line;
	gboolean ShowTime;
	
	if (strncmp (string, "{.}", 3) != 0) {/*Insert time if doesn't begin "{.}"*/
		dt = get_time (UseUTC);
		s = g_strdup_printf ("%4i-%02i-%02i, %02i:%02i:%02i > ", 
							 1900 + dt->tm_year, 1 + dt->tm_mon, dt->tm_mday, 
							 dt->tm_hour, dt->tm_min, dt->tm_sec);
		ShowTime = TRUE;
	} else {
		s = g_strdup_printf ("%s", "");
		string = string + 3;
		ShowTime = FALSE;
	}
	
	/* Insert the text, with appropriate colours */
	
	gtk_text_buffer_get_iter_at_offset (txtPhotBuffer, &end, -1);	
	
	line = g_strconcat (s, string, NULL);  /* Colour text magenta */		
	gtk_text_buffer_insert_with_tags (txtPhotBuffer, &end, line, -1, 
	                                                           magenta_1, NULL);
	g_free (s);
	
	/* Scroll the log window to make added text visible */
	
	gtk_text_buffer_get_iter_at_offset (txtPhotBuffer, &end, -1);	
	mark = gtk_text_buffer_create_mark (txtPhotBuffer, NULL, &end, FALSE);
	gtk_text_view_scroll_to_mark (txtPhot, mark, 0.0, TRUE, 1.0, 1.0);
	
	/* Free the strings */
	
	g_free (line);
	if (ShowTime)
		g_free (string);
	else
		g_free (string - 3);
}

static gchar *ReadCString (gchar *string, gchar *fmt, ...)
{
	/* Attempt to read the specified configuration value.  The value is returned
     * as a string, which is set equal to the value passed into the routine if 
	 * the configuration key does not exist.
	 */

    GError *error = NULL;
	va_list ap;
	gchar *p;
	gchar *group, *key;
	gchar *s1 = NULL;
	static gchar s2[MCSL];
    
    /* Load configuration data */
    
    if (!g_key_file_load_from_file (KeyFileData, ConfigFile, 
               G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error))
        goto config_error;
    
    key = g_strrstr (string, "/")+1;  /* Pointer arithmetic */
    group = g_strndup (string, (key-string-1) / sizeof (gchar));
    
    GError *err = NULL;
    if (!(s1 = g_key_file_get_string  (KeyFileData, group, key, &err)))
        goto config_error;
        
    strncpy (s2, s1, MCSL);
    s2[MCSL - 1] = '\0';
    g_free (s1);
    
    return s2;
    
config_error:

	G_print ("{o}%s\n", err->message);

    /* Return passed-in value */

    va_start (ap, fmt);
    for (p = fmt; *p; p++) {
        switch (*p) {
            case 'd':
                sprintf (s2, fmt, va_arg (ap, int));
                break;
            case 'f':
                sprintf (s2, fmt, va_arg (ap, double));
                break;
            case 's':
                sprintf (s2, fmt, va_arg (ap, char *));
                break;
            default:
                break;
        }
    }
	va_end (ap);
    
    if (error) {
        L_print ("{r}Error reading configuration data: %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
	
	return s2;
}

static gchar **ReadCArray (gchar *string)
{
    /* Attempt to read all the configuration values for the given group from the
     * configuration file.
     */
    
    GError *error = NULL;
    guint num = 0, num_alloc = 0;
    gchar **keys, *s;
    static gchar **vals = NULL;
    
    const gint ALLOC_ELEMS = 100;
    
    if (!g_key_file_load_from_file (KeyFileData, ConfigFile, 
               G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error))
        goto config_error;
        
    if (!(keys = g_key_file_get_keys (KeyFileData, string, NULL, &error)))
        goto config_error;
    
    while (keys[num++]) {  /* Iterate over all keys... */
        if (num > num_alloc) {      /* Allocate pointer storage if required */
            num_alloc += ALLOC_ELEMS;
            vals = (gchar **)g_realloc(vals,(num_alloc + 1) * sizeof (gchar *));
        }
        s = g_key_file_get_string (KeyFileData, string, keys[num - 1], NULL);
        vals[num - 1] = g_malloc0 ((strlen (s) + 1) * sizeof (gchar));
        strcpy (vals[num - 1], s);  /* Store a copy of the configuration value*/
        g_free (s);
    }
    vals[num - 1] = NULL;  /* Terminate the array with a NULL */
    
    g_strfreev (keys);
    return vals;
    
config_error:
    L_print ("{r}Error reading configuration data: %s\n", error->message);
    g_error_free (error);
    error = NULL;
    g_strfreev (keys);
    return NULL;
}

static void WriteCString (gchar *string, gchar *val)
{
	/* Attempt to write the specified configuration value */
	
	gchar *group, *key;
    gchar *buffer;
    gsize buflen;
    
    /* Load configuration data */
    
    if (g_key_file_load_from_file (KeyFileData, ConfigFile,
               G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
    
        key = g_strrstr (string, "/")+1;  /* Pointer arithmetic */
        group = g_strndup (string, (key-string-1) / sizeof (gchar));
        
        g_key_file_set_string  (KeyFileData, group, key, val);
        buffer = g_key_file_to_data (KeyFileData, &buflen, NULL);
        g_file_set_contents  (ConfigFile, buffer, buflen, NULL);
        
        g_free (buffer);
        g_free (group);
        g_free (val);
    } else
        L_print ("{r}Error writing to configuration file\n");
}

static void DeleteCGroup (gchar *string)
{
    /* Delete the specified group from the configuration file */
    
    gchar *buffer;
    gsize buflen;

    /* Load configuration data */
    
    if (g_key_file_load_from_file (KeyFileData, ConfigFile,
               G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL)) {
    
        g_key_file_remove_group  (KeyFileData, string, NULL);
        buffer = g_key_file_to_data (KeyFileData, &buflen, NULL);
        g_file_set_contents  (ConfigFile, buffer, buflen, NULL);
        
        g_free (buffer);
    }
}

static gchar *CCDConfigKey (struct cam_img *img, const gchar *str)
{
	/* A helper function to construct the key for accessing CCD configuration
	 * data.
	 */
	
	gchar *s = NULL;
	static gchar *key = NULL;
	
	if (!key) {
		g_free (key);
		key = NULL;
	}
	
	if (img->device == QSI)
		s = img->cam_cap.camera_snum;
	else if (img->device == SX)
		s = img->cam_cap.camera_desc;
	
	key = g_strconcat ("CCDConfig/", img->cam_cap.camera_manf, "/",
				                     s, "/", str, NULL);
	
	return key;
}

void init_task_params (void)
{
	/* Initialise the task parameters */
	
	gushort i;
	
	for (i = 0; i < NUMTPARAMS; i++) {
		tp[i] = (gchar *) g_malloc (SIZETPARAMS * sizeof (gchar));
		strcpy (tp[i], " ");
	}	
}

void free_task_params (void)
{
	/* Free the task parameters */
	
	gushort i;
	
	for (i = 0; i < NUMTPARAMS; i++) {
		g_free (tp[i]);
		tp[i] = NULL;
	}
}

void get_task_params (gchar **tparams)
{
	/* Pass the task parameters */
	
	gushort i;
	
	for (i = 0; i < NUMTPARAMS; i++)
		tparams[i] = tp[i];
}

void set_task_params (gchar **tparams)
{
	/* Receive the task parameters */
	
    gushort i;

	for (i = 0; i < NUMTPARAMS; i++)
		if (tp[i] && tparams[i])
			strcpy (tp[i], tparams[i]);
}

gchar *get_task_param_value (gboolean FreeString, gchar *string, gint *status)
{
	/* If 'string' is of the form '%XXX' where XXX is an integer, then replace
	 * 'string' with the task parameter value corresponding to %XXX.
	 * If FreeString is TRUE, the passed string should be freed and then set
	 * to point to a copy of the parameter value; otherwise the existing content
	 * of the passed string should be overwritten with the parameter value.
	 * If 'string' is a valid task parameter, set status to 1.
	 * If 'string' is an invalid task parameter, set status to -1.
	 * If 'string' is not a task parameter, set status to 0.
	 * If the task list is not active, set status to 0.
	 */
	
	gushort i, n;
	gchar c, *message;
	
	if (tasks_get_status () == TSK_INACTIVE) {
		*status = 0;
		return string;
	}
	
	if (!strncmp (string, "%", 1)) {
		for (i = 1; i < strlen (string); i++) {
		    if (!isdigit (c = string[i])) {
				message = g_strdup_printf (" is not a valid task parameter.");
				err_msg (string, message);
				g_free (message);
				*status = -1;
				return string;
			}
		}
		
		if ((n = (gushort) strtol (&string[1], NULL, 10)) < NUMTPARAMS) {
			G_print ("Task parameter %s", string);
			if (FreeString) {
				g_free (string);
				string = g_strdup (tp[n]);
			} else {
			    strcpy (string, tp[n]);
			}
			G_print ("{.}, value: %s\n", string);
			*status = 1;
			return string;
		} else {
			message = g_strdup_printf(" exceeds the maximum permissible number "
									  "of task parameters [%d]", NUMTPARAMS);
			err_msg (string, message);
			g_free (message);
			*status = -1;
			return string;
		}
	}
	
	*status = 0;
	return string;
}

void exit_and_shutdown (void)
{
	/* Schedule a system shutdown after 1 minute and exit the application */
	
	gchar *s;
	
	s = g_strdup_printf ("sudo /sbin/shutdown -h +1");
    popen (s, "r");
	g_free (s);
	
	loop_stop_loop ();
}

#ifdef HAVE_UNICAP
static void unicap_device_change_cb (UnicapgtkDeviceSelection *selection,
									 gchar *device_id, GtkWidget *format)
{
	/* Device change callback - called when the user selects a unicap device */
	 
	struct cam_img *aug = get_aug_image_struct ();

	unicap_void_device (&aug->ucp_device);
	strcpy (aug->ucp_device.identifier, device_id);
  
	if (!SUCCESS (unicap_enumerate_devices (
	                           &aug->ucp_device, &aug->ucp_device, 0)) ||
        !SUCCESS (unicap_open (&aug->ucp_handle, &aug->ucp_device))) {
		L_print ("{r}%s: Device '%s' not available!\n", __func__, device_id);
		return;
     }
 
	 unicapgtk_video_format_selection_set_handle (
					UNICAPGTK_VIDEO_FORMAT_SELECTION (format), aug->ucp_handle);
}
#endif

#ifdef HAVE_UNICAP 
static void unicap_format_change_cb (GtkWidget *ugtk, unicap_format_t *format,
                                     gpointer *p)
{
	/* Format change callback - called when user changes unicap format */
	   
	struct cam_img *aug = get_aug_image_struct ();
	   
	aug->ucp_format_spec = *format;
}
#endif

#ifdef HAVE_UNICAP
void set_record_on (gboolean on)
{
	/* Turn video recording on/off via the 'Record' button */
	
	GtkToggleButton *button;
	
	if (!menu.LiveView) {
		L_print ("{o}Can't %s recording - LiveView window not open!\n", 
				                                         on ? "start" : "stop");
		tasks_task_done (on ? T_RST : T_RSP);
	    return;
	}
	
	button = GTK_TOGGLE_BUTTON (xml_get_widget (xml_lvw, "tglLVRecord"));
	
	if (on == gtk_toggle_button_get_active (button)) {
		L_print ("{o}Warning - recording is already %s\n", (on ? "on" : "off"));
		tasks_task_done (on ? T_RST : T_RSP);
		return;
	}
	
	if (on) {
		
		/* Start recording */
		
	    if (!gtk_toggle_button_get_active (button))
			gtk_toggle_button_set_active (button, TRUE);
		tasks_task_done (T_RST);
	} else {
		
		/* Stop recording */
		
	    if (gtk_toggle_button_get_active (button))
			gtk_toggle_button_set_active (button, FALSE);
		tasks_task_done (T_RSP);
	}
}
#endif

#ifdef HAVE_UNICAP
gboolean liveview_record_is_writeable (void)
{
	/* Return TRUE if directory for recording file is writeable, FALSE if not */
	
	gchar *uri = NULL, *dirname = NULL;
	gboolean IsWriteable;
	
	uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER 
						            (xml_get_widget (xml_lvw, "flcVideoFile")));
	dirname = g_filename_from_uri (uri, NULL, NULL);
	g_free (uri);
	IsWriteable = !access (dirname, W_OK) ? TRUE : FALSE;
	if (dirname) {
		g_free (dirname);
		dirname = NULL;
	}
	
	return IsWriteable;
}
#endif

#ifdef HAVE_UNICAP
gboolean open_liveview_window (void)
{
	/* Open the live view window and connect the signal handlers, provided that
	 * the selected camera device is Unicap.
	 */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	if (aug->device == UNICAP) {
		gchar *objects[] = {"wndLiveView", NULL};
		xml_lvw = xml_load_new (xml_lvw, GLADE_INTERFACE, objects);
	} else
	    return show_error (__func__, "Can't open live view window unless "
					      "a Unicap device is the selected autoguider camera!");
	
	return TRUE;
}
#endif

#ifdef HAVE_UNICAP
void show_liveview_window (void)
{
	/* Show the live view window, if hidden */
	
	struct cam_img *aug = get_aug_image_struct ();
	
	gtk_widget_show (aug->ugtk_window);
}
#endif

#ifdef HAVE_UNICAP
void hide_liveview_window (void)
{
	/* Hide the live view window, if shown */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	gtk_widget_hide (aug->ugtk_window);
}
#endif

#ifdef HAVE_UNICAP
void close_liveview_window (void)
{
	/* Close the live view window, if open */
	
	struct cam_img *aug = get_aug_image_struct ();
		
	if (aug->ugtk_window != NULL) {
		gtk_widget_destroy (aug->ugtk_window);
		aug->ugtk_window = NULL;
	}
}
#endif

static GtkBuilder *xml_load_new (GtkBuilder *xml, gchar *file, gchar *objects[])
{
    /* Load new builder object(s) from file */
    
    GError *error = NULL;
    
    xml = gtk_builder_new ();
    if (!gtk_builder_add_objects_from_file (xml, file, objects, &error)) {
        g_warning ("Couldn't load builder file: %s", error->message);
        g_error_free (error);
        xml = NULL;
    } else
        gtk_builder_connect_signals (xml, NULL);
    
    return xml;
}

GtkWidget *xml_get_widget (GtkBuilder *xml, const gchar *name)
{
    /* Return requested widget from builder object */
    
    GObject *object;
    
    if ((object = gtk_builder_get_object (xml, name)) != NULL)
		return GTK_WIDGET (object);
	else
		return NULL;
}

static GtkComboBox *create_text_combo_box (GtkTable *tblTable, guint l, guint r, 
                                           guint t, guint b, 
                                           GtkComboBox *cmbBox,
                                           const gchar *list, guint index,
                                           GCallback func)
{
    /* Create and fill a text-only combo box.  Easier than leaping through
     * countless hoops with Glade.
     */
    
    GError *error;
    gint i = 0;
    gchar **keys;

    /* Create combo box */
    
    cmbBox = GTK_COMBO_BOX (gtk_combo_box_new_text ());
    gtk_widget_show (GTK_WIDGET (cmbBox));
    gtk_table_attach_defaults (tblTable, GTK_WIDGET (cmbBox), l, r, t, b);
    
    /* Fill with text strings from given list in key (configuration) file */
    
    error = NULL;
    if (!g_key_file_load_from_file (KeyFileData, ConfigFile, 
             G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error))
        goto combo_error;
    
    error = NULL;
    if (!(keys = g_key_file_get_keys (KeyFileData, list, NULL, &error)))
        goto combo_error;
    
    while (keys[i])
        gtk_combo_box_append_text (cmbBox, 
                                   g_key_file_get_string (KeyFileData,
                                   list, keys[i++], NULL));
    g_strfreev (keys);
    
    /* Optionally connect callback function when combo box selection changed */
    
    if (func)
        g_signal_connect (cmbBox, "changed", *func, NULL);
        
    /* Select the given entry */
        
    gtk_combo_box_set_active (cmbBox, index);
    
    return cmbBox;
    
combo_error:
    gtk_combo_box_append_text (cmbBox, "****");
    gtk_combo_box_set_active (cmbBox, 0);
    L_print ("{r} Error filling combo box: %s\n", error->message);
    return cmbBox;
}


/******************************************************************************/
/*                              MAIN ROUTINE                                  */
/******************************************************************************/

int main (int argc, char *argv[])
{
	struct cam_img *aug;
	FILE *fp;
	
    /* Initialise gtk */

	g_thread_init (NULL);
    gdk_threads_init ();
	gdk_threads_enter ();  /* Try to remove this in future release */
    gtk_init (&argc, &argv);
    
	/* Get main (gtk) thread id */
	
	main_tid = (pid_t) syscall (SYS_gettid);
	
	/* Check that the Glade gtkbuilder file is available */
	
	if (!(fp = fopen (GLADE_INTERFACE, "rb"))) {
		L_print ("{r}Error - Can't seem to find the Glade gtkbuilder file!\n");
		g_error ("main: Can't open %s\n", GLADE_INTERFACE);
	}
	fclose (fp);
	
	/* Draw the main window and connect its signal handlers.  The adjustment
     * objects for spin buttons aren't children of the spin buttons (in the xml
     * written by Glade), so they have to be pre-loaded separately.
     */

    gchar *app_objects[] = {"spbBin_adj",
                            "spbAutogGuideSpeed_adj",
                            "spbAutogMaxShift_adj",
                            "spbAutogMaxMove_adj",
                            "spbAutogCorrFac_adj",
                            "spbAutogMaxDrift_adj",
                            "spbAutogDriftSample_adj",
                            "spbAutogMaxOffset_adj",
                            "spbAutogCenterSpeed_adj",
                            "ccdApp",
                            NULL};
    xml_app = xml_load_new (xml_app, GLADE_INTERFACE, app_objects);
	
	stsAppStatus = GTK_STATUSBAR (xml_get_widget (xml_app, "stsAppStatus"));
	prgAppBar = GTK_PROGRESS_BAR (xml_get_widget (xml_app, "prgAppBar"));
	ccdApp = GTK_WINDOW (xml_get_widget (xml_app, "ccdApp"));
    
    /* Create new configuration data structure */
    
    KeyFileData = g_key_file_new ();
    
	/* Initialise libusb if it's available (needed by SX hardware) */
	
	#ifdef HAVE_LIBUSB
	gqusb_init ();
	#endif
	
	/* Initialise the CCD camera and autoguider image structures */
	
	ccdcam_init ();
	augcam_init ();
		
	/* Initialise telescope data */
	
	telescope_init ();
	
	/* Initialise filter wheel data */
	
	filter_init ();
	
	/* Initialise XPA mechanism for image display via ds9 */
	
	xpa_open ();
	
	/* Open the image window and connect the signal handlers.  The adjustment
     * objects for sliders aren't children of the sliders (in the xml
     * written by Glade), so they have to be pre-loaded separately.
     */	
    
    gchar *img_objects[] = {"hscBackground_adj",
                            "hscBrightness_adj",
                            "hscContrast_adj",
                            "hscGamma_adj",
                            "hscGain_adj",
                            "wndImage",
                            NULL};
	xml_img = xml_load_new (xml_img, GLADE_INTERFACE, img_objects);
    stsImageStatus = GTK_STATUSBAR (xml_get_widget (xml_img, "stsImageStatus"));
	
	aug = get_aug_image_struct ();
	aug->aug_window = xml_get_widget (xml_img, "wndImage");
	ui_hide_aug_window ();
	
	/* Initialise the application */
	
	InitApp ();

	/* Restore configuration data */
	
	restore_config_data ();
	
	ResetChkState = FALSE;
    
	/* Initialise the task queue */
		
	tasks_init (xml_app);
	
	/* Start the event loop */
	
	loop_start_loop ();
	
	/* Restore the "watch file" settings (must be called after event loop has
	 * started).
	 */
	
	restore_watch_file ();
	
    /* Enter the main gtk loop */
    
	gtk_main();   
	
	/* Close comms links */
	
	telescope_close_guide_port ();
	telescope_close_comms_port ();
	filter_close_comms_port ();
	focus_close_comms_port ();
	
	/* Save configuration data */
	
	save_config_data ();
	
	/* Close the CCD and autoguider cameras if still open */
	
	augcam_close ();
	ccdcam_close ();
	
	/* Tidy up libusb */
	
	#ifdef HAVE_LIBUSB
	gqusb_exit ();
	#endif
	
	/* Close the XPA mechanism */
	
	xpa_close ();
	
	/* Close the log file */
	
	if (f_log)
		fclose (f_log);
	
    /* Free the configuration data structure */
    
	g_key_file_free (KeyFileData);
	
	gdk_threads_leave ();  /* Try to remove this in future release */
    return 0;
}
