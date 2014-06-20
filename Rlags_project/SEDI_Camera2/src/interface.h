/******************************************************************************/
/*                        MAIN HEADER FILE FOR GOQAT                          */
/*                                                                            */
/* Main header file for GoQat.                                                */
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

#include <stdio.h>
#include <sys/types.h>
#include <goocanvas.h>
#include "ccd.h"
#include "telescope.h"
#include "focus.h"
#include "ports.h"

#ifdef HAVE_LIBQSIAPI
#define HAVE_QSI 1
#endif

#ifdef HAVE_LIBUDEV
#define HAVE_SX_FILTERWHEEL 1
#endif

#ifdef HAVE_LIBUSB
#define HAVE_SX_CAM 1
#endif

#ifdef HAVE_LIBUNICAP
#define HAVE_UNICAP 1
#include <unicap.h>
#include <unicapgtk.h>
#include <ucil.h>
#endif

#define AUGCANV_H 5000           /* Horizontal size of autoguider image canvas*/
#define AUGCANV_V 5000           /* Vertical size of autoguider image canvas  */
#define VIDCANV_H 5000           /* Horizontal size of video playback canvas  */
#define VIDCANV_V 5000           /* Vertical size of video playback canvas    */

#define HWID    135              /* Width of histogram canvas                 */
#define BOXSIZE 128              /* Size of histogram box etc                 */
#define XPLOT     3              /* Top left X coord for histogram box        */
#define YHIST    25              /* Top left Y coord for histogram box        */
#define YGAP     60              /* Spacing between histog. & flux plots      */
#define TGAP      7              /* Text vertical spacing                     */

#define AUTOGT_H  750.0			 /* Horizontal and vertical size of autoguider*/
#define AUTOGT_V  300.0			 /*  trace window							  */

#define CCD_PAGE 0               /* UI notebook pages                         */
#define AUG_PAGE 1
#define TEL_PAGE 2
#define FOC_PAGE 3
#define FIL_PAGE 4
#define TAS_PAGE 5
#define NO_PAGE  999             /* Set to any value > no. notebook pages.    */
#define TSK_PAGE 1000            /* String is from task list,not notebook page*/

#define CHP_MIN 2                /* Min/max/default for CCD size              */
#define CHP_MAX 4096
#define CHP_DEF 1000

#define EXP_MIN 0.0              /* Min/max/default for CCD exposure times    */
#define EXP_MAX 14400.0
#define EXP_DEF 1.0

#define TPR_MIN -100.0           /* Min/max/default for CCD chip temperature  */
#define TPR_MAX 50.0
#define TPR_DEF -5.0

#define WAI_MIN 0.0              /* Min/max/default for task pause/wait times */
#define WAI_MAX 9999.0
#define WAI_DEF 0.0

#define REP_MIN 1                /* Min/max/default for BeginLoop repeats     */
#define REP_MAX 99999
#define REP_DEF 1

#define MOV_MIN -21600.0         /* Min/max/default for Move command          */
#define MOV_MAX 21600.0
#define MOV_DEF 1.0

#define MAX_DEVICES 128          /* Maximum number of devices                 */
                                 /* (cameras, filter wheels etc)              */
                                 
#define MAX_VIDITEMS 50          /* Maximum no. video standards/inputs        */

#define NUMTPARAMS 10            /* Number of task params (%1, %2 etc)        */
#define SIZETPARAMS 128          /* Storage (gchar) for each task parameter   */

#define MCSL 80         /* Maximum configuration string length for R_config_s */

								 /* Writing to log                            */
#define L_print(string, args...) WriteLog(g_strdup_printf(string, ##args))
								 /* Writing debug messages to log             */
#define G_print(string, args...) if (Debug) WriteLog(g_strdup_printf(string, ##args));
								 /* Writing to photometry window              */
#define P_print(string, args...) WritePhot(g_strdup_printf(string, ##args))
	

enum CamType {                   /* Camera/image data ID's                    */
	CCD,                         /* Main imaging CCD camera & data            */
	AUG,                         /* Autoguider camera & data                  */
    VID                          /* Video data captured via live view window  */
};

enum HWDevice {                  /* Hardware device types                     */
    NO_DEV,
	QSI, 
	SX,
	SX_GH,
	OTHER, 
	UNICAP, 
	V4L,
	INTERNAL
};

enum ZoomType {                  /* Possible autoguider canvas zoom modes     */
	ZOOM_IN, 
	ZOOM_OUT,
    ZOOM_1x1,
    ZOOM_2x2
};

enum Debayer {                   /* Debayering methods                        */
	DB_SIMP,
	DB_NEAR,
	DB_BILIN,
	DB_QUAL,
	DB_DOWN,
	DB_GRADS
};

enum GreyScale {                 /* Greyscale conversion for autoguider       */
	LUMIN, 
	DESAT, 
	MAXIM,
	MONO
};

enum ColsPix {                   /* Number of colours per pixel               */
	C_GREY = 1,
	C_RGB = 3
};

enum Colour {                    /* Colour/greyscale options                  */
	R, 
	G, 
	B, 
	GREY
};

enum Range {                     /* Slider ranges for autoguider image window */
	AIW_BACKGROUND,
	AIW_BRIGHTNESS, 
	AIW_CONTRAST, 
	AIW_GAMMA,
	AIW_GAIN
};

enum Guide {                     /* Autoguider operations                     */
	INIT, 
	GUIDE, 
	PAUSE, 
	CONT, 
	QUIT
};

enum TaskTypes {                 /* Task types                                */
	T_OBJ, 
	T_BSQ, 
	T_WTU, 
	T_PSF, 
	T_EAT, 
	T_EXP, 
	T_FOC,
	T_AGN, 
	T_AGF, 
	T_GST, 
	T_GSP, 
	T_RST, 
	T_RSP, 
	T_YBT, 
	T_GTO,
	T_SCR,
	T_PMT, 
	T_SDN, 
	T_WRT
};

enum TaskStatus {                /* Task list status                          */
	TSK_INACTIVE = 0,
	TSK_ACTIVE,
	TSK_TESTONLY
};

enum VideoFileTypes {            /* File types for saving frames from VID file*/
	SVF_FITS,
	SVF_VID
};

enum CheckBox {				     /* Application check boxes/toggle buttons    */
	RCS_OPEN_COMMS_LINK, 
	RCS_OPEN_CCD_LINK, 
	RCS_OPEN_AUTOG_LINK,
	RCS_OPEN_FILTER_LINK,
	RCS_OPEN_FOCUS_LINK,
	RCS_OPEN_PARPORT,
	RCS_PEC_ON,
	RCS_ACTIVATE_WATCH, 
	RCS_OPEN_PLAYBACK_WINDOW, 
	RCS_OPEN_LIVEVIEW_WINDOW, 
	RCS_OPEN_TRACE_WINDOW, 
	RCS_OPEN_TEMPS_WINDOW, 
	RCS_USE_AUTOGUIDER,
	RCS_LV_RECORD
};

struct main_menu {               /* State of various main menu items          */
	gchar telescope_port[MCSL];
	gboolean OpenTelPort;
	gboolean Gemini;
	gchar autoguider_port[MCSL];
	gboolean OpenAutogPort;
	gchar filterwheel_port[MCSL];
	gboolean OpenFilterwheelPort;
	gchar focus_port[MCSL];
	gboolean OpenFocusPort;
	gboolean OpenParPort;
	gchar ccd_camera[MCSL];
	gboolean OpenCCDCam;
	gboolean FullFrame;
	gchar debayer[MCSL];
	gchar aug_camera[MCSL];
	gchar greyscale[MCSL];
	gchar filterwheel[MCSL];
	gchar focuser[MCSL];
	gboolean PEC;
	gboolean Precess;
	gboolean UTC;
	gboolean LiveView;
	gboolean ShowToolbar;
};

struct main_menu menu;           /* State of various main menu items          */

struct device_selection {        /* Info. for devices selected via menu       */
	const char *id[MAX_DEVICES];    /* Serial no. or other identifier         */
	const char *desc[MAX_DEVICES];  /* Device description                     */
	char lid[MAX_DEVICES][256];     /* Local copy of serial/identifier        */
	char ldesc[MAX_DEVICES][256];   /* Local copy of device description       */
	gshort *idx;/* The index of the selected device in the id and desc arrays */
	gint num;                       /* Number of devices found                */
};

struct device_selection ds;

struct exposure_data {           /* Camera exposure data                      */
    gdouble req_len;             /* Requested length of exposure (s)          */
    gdouble act_len;             /* Actual length of exposure (s)             */
	gdouble ccdtemp;             /* CCD chip temperature                      */
    unsigned short  h_top_l;     /* h top left pixel for exposure             */
    unsigned short  v_top_l;     /* v top left pixel for exposure             */
    unsigned short  h_bot_r;     /* h bottom right pixel for exposure         */
    unsigned short  v_bot_r;     /* v bottom right pixel for exposure         */
    unsigned short  h_bin;       /* h bin value                               */
    unsigned short  v_bin;       /* v bin value                               */
	unsigned short  h_pix;       /* Number of pixels in h direction           */
	unsigned short  v_pix;       /* Number of pixels in v direction           */
	guint exp_start;             /* Time (ms) of start of most recent exposure*/
	                             /*  or time of start of video sequence after */
	                             /*  re-sizing selection rectangle            */
	guint exp_end;               /* Time (ms) of end of most recent exposure  */
	gint focus_pos;              /* Focus position for this exposure          */
	gfloat frame_rate;           /* No. video frames per second               */
	gchar *ExpType;              /* Exposure type                             */
	gchar *filter;               /* Filter used for exposure                  */
	gchar filename[256];         /* Name of most recently saved exposure      */
	gboolean FreeRunning ;       /* TRUE if we can't control start and end    */
};

struct fits_data {               /* FITS keyword data                         */
	gint    focus_pos;
	gfloat  focus_temp;
	gdouble tm_start;
	gdouble epoch;
	gchar   date_obs[24];
	gchar   utstart[13];
	gchar	RA[9];
	gchar	Dec[10];
};

struct data_val {                /* Minimum, maximum, mean, mode, stdev, shift*/
	struct {
		gushort val;
	} min[4];
	struct {
		gushort val;
		gushort h;
		gushort v;
	} max[4];
	struct {
		gfloat val;
		gfloat h;
		gfloat v;
	} mean[4];
	struct {
		gushort peakbin;
		guint peakcount;
		guint *hist;
	} mode[4];
	struct {
		gushort median;
		gfloat val;
	} stdev[4];
	struct {
		gfloat shift_ns;         /* Pixels shift N/S and E/W since selection  */
		gfloat shift_ew;         /*  rectangle last re-drawn.                 */
	};
};

struct display_limits {          /* Display limits and greyscale/debayering   */
	gushort B, W;                /* Fixed limits for black and white values   */
	gushort black, white;        /* User-adjustable black and and white values*/
	gushort satlevel;            /* Saturation level for autoguider display   */
	gfloat stdev;                /* Background subtraction stdev (centroiding)*/
	gfloat gamma;                /* Value for gamma adj. of displayed image   */
	enum GreyScale greyscale;    /* Type of greyscale conversion              */
	enum Debayer debayer;        /* Type of debayering                        */
};
								 
struct sel_rect {                /* Selection rectangle                       */
	gint htl;                    /* horz top left                             */
	gint vtl;                    /* vert top left                             */
	gint hbr;                    /* horz bot right                            */
	gint vbr;                    /* vert bot right                            */
	gint new_htl;                /* new value of htl                          */
	gint new_vtl;                /* new value of vbr                          */
};

struct canvas {                  /* Image display canvas features             */
	GooCanvasItem *cviImage;     /* Pointer to displayed image on canvas      */
	GooCanvasItem *cviRect;      /* Pointer to selection rectangle            */
	GooCanvasItem *chx_line;     /* Pointer to cross-hair x line              */
	GooCanvasItem *chy_line;     /* Pointer to cross-hair y line              */		
	struct sel_rect r;           /* Selection rectangle                       */
	gushort csize;               /* Centroid size                             */
    gint cursor_x, cursor_y;     /* x and y positions of cursor               */
	gint scroll_x, scroll_y;     /* Scroll offsets of canvas                  */
	gdouble zoom;                /* Current zoom factor                       */
	gboolean NewRect;            /* Set flag when new selection rect. defined */
	gboolean Centroid;           /* Set flag to show centroid marker          */	
};	

struct dark_frame {              /* Dark frame data                           */
	 /* dk161 is 16-bit greyscale data to be used as a dark frame.  It points
	  * to allocated memory.
	  */
	gushort *dk161;              /* Dark exposure (16 bit grey)               */
	gushort num;                 /* Number of exposures for average dark frame*/
	gboolean Capture;            /* Set flag to capture dark exposure         */	
	gboolean Subtract;           /* TRUE to subtract dark frame               */
};

struct autog_config {            /* Autoguider configuration data             */
	gushort GuideDirn;           /* Permitted guiding directions              */
	gdouble Rate_NS;             /* Rate of star motion north/south (pixels/s)*/ 
	gdouble Rate_EW;             /* Rate of star motion east/west (pixels/s)  */
	gdouble Uvec_N[2];           /* Unit vector north (x,y)                   */
	gdouble Uvec_E[2];           /* Unit vector east (x,y)                    */
	gdouble NSCalibDuration;     /* Duration of north/south calib. time (s)   */
	gdouble EWCalibDuration;     /* Duration of east/west calib. time (s)     */
	gfloat CalibDec;             /* Declination at which calibration was done */
	gfloat CalibGuideSpeed;      /* Guide speed used for autog. calibration   */
	gfloat GuideSpeed;           /* Guide speed relative to sidereal rate     */
	gfloat MaxShift;             /* Maximum shift before making a correction  */
	gfloat MaxDrift;             /* Maximum drift before making a correction  */
	gfloat MaxMove;              /* Maximum permitted move per correction     */
	gfloat MaxOffset;            /* Ignore offsets greater than this          */
	gfloat DriftSample;          /* Drift sample duration for average drift   */
	gfloat CorrFac;              /* Proportion of total correction to make    */
	gfloat Update;               /* Min. elapsed time between guider cor'ction*/
	gchar *Telescope;            /* Telescope identification string           */
	gchar *Instrument;           /* Instrument identification string          */
	gboolean SimulGuide;         /* Set TRUE if simultaneous N/S & E/W guiding*/
	gboolean DriftNSOnly;        /* Set TRUE if drift correct only for N/S    */
	gboolean RemoteTiming;       /* Set TRUE if guide timing done remotely    */
	gboolean DecCorr;            /* Set flag to apply dec.corr'n. to E/W speed*/
};

struct autog_params {            /* Autoguider parameters                     */
	struct autog_config s;       /* Autoguider configuration                  */
	gushort worm_pos;            /* Telescope drive RA worm position          */
	gboolean Write;              /* TRUE writes guide star posn. data to file */
	gboolean Worm;               /* TRUE writes RA worm pos. as well as above */
	gboolean Guide;				 /* Set flag if autoguiding is active         */
	gboolean Pause;              /* Set flag if autoguiding is paused         */
};

struct graph_info {			     /* Graph drawing information                 */
	gdouble zoom;
	gboolean showh;
	gboolean showv;
	gboolean draw;
	gboolean reset;
};

struct SAO_DS9 {                 /* DS9 image display data                    */
    FILE *stream;                /* File pipe stream for DS9 process          */
	gchar *window;               /* DS9 window name                           */
	gchar *display;              /* Path for image file for display in DS9    */
	gboolean Invert_h;           /* Invert DS9 -> camera h coordinate mapping */
	gboolean Invert_v;           /* Invert DS9 -> camera h coordinate mapping */
};
                                 /* Video data, mostly for V4L autoguider     */
struct video_data {              /*  image capture, but some Unicap stuff too */  
	struct {
		gint selected;           /* Selected item                             */
		gint num;                /* Total number of available items           */
		guint64 id[MAX_VIDITEMS];/* IDs of available items                    */
		gchar name[MAX_VIDITEMS][128];/* Names of available items             */
	} vid_std;                   /* Video standards                           */
	struct {
		gint selected;
		gint num;
		gchar name[MAX_VIDITEMS][128];
	} vid_input;                 /* Video inputs                              */
	struct {
		void *start;
		size_t length;
	} *buffers;                  /* Video buffers                             */
	struct {
		GStaticMutex mutex;
		gint size;
		guchar *buffer[2];
		guchar *alloc;
	} fifo;                      /* FIFO for buffered V4L frames              */
	gushort byte_incr;           /* Number of bytes per pixel                 */
	gushort byte_offset;         /* Offset to first 'Y' data in image         */
	gint type;                   /* Type of data: 1-2 byte grey, 3 byte colour*/             
	gint pixfmt;                 /* Selected pixel format (fourcc)            */
	gint width;                  /* Image width                               */
	gint height;                 /* Image height                              */
	gint size;                   /* Image size (bytes)                        */
	gint bufnum;                 /* Number of video buffers / buffer number   */
	guint frames_tot;            /* Total number of frames received           */
	guint frames_drop;           /* Total number of frames dropped            */
	gfloat fps;                  /* Frames per second                         */
	gchar card[128];             /* Card/device name                          */
	gboolean HasVideoStandard;   /* TRUE if the device can set video standards*/
};

struct cam_img {                 /* Camera/image data                         */
	#ifdef HAVE_UNICAP
	unicap_handle_t ucp_handle;  /* Unicap video data (autoguider)            */
	unicap_device_t ucp_device;  /* Unicap video data                         */
	unicap_format_t ucp_format;  /* Unicap video data                         */
	unicap_format_t ucp_format_spec;/* Unicap video data                      */
    GtkWidget *ugtk_display;     /* Unicap video data                         */
	GtkWidget *ugtk_window;      /* Unicap video data                         */
	gboolean Record;             /* TRUE if recording video stream to disk    */
	#endif
	struct video_data vid_dat;  /* Mostly V4L2 video data                     */
	struct ccd_capability cam_cap;    /* CCD camera capability (see ccd.h)    */	
	struct ccd_state state;      /* CCD camera state and status               */
	struct exposure_data exd;    /* Camera exposure data                      */
	struct fits_data fits;       /* FITS data                                 */
	/* Min., max., mean, mode, stdev: img is the underlying (raw) image data for 
	 * main CCD camera or autoguider camera, pic is the displayed autoguider 
	 * image data and rect is the displayed autoguider image data within the 
	 * selection rectangle.
     */
	struct data_val img, pic, rect;    
	struct display_limits imdisp;/* Image display limits and greyscale/debayer*/	
	struct canvas canv;          /* Autoguider image canvas features          */
	struct dark_frame dark;      /* Autoguider dark frame data                */
	struct autog_params autog;   /* Autoguider parameters                     */		
	struct graph_info graph;     /* Graph drawing information                 */
	struct SAO_DS9 ds9;          /* DS9 image viewer information              */
	 /* Function to be called to set state of this camera                     */
	int (*set_state) (enum CamState state, int ival, double dval, ...);
	 /* Function to be called to get state of this camera                     */
	int (*get_state) (struct ccd_state *state, gboolean AllSettings, ...);
	GtkWidget *aug_window;       /* Pointer to autoguider image window        */
	/*                     
	 * CCD camera as main camera:
	 * 
	 * r161  -   RAW greyscale data up to 16-bit depth.  This is displayed in 
	 *           DS9 unless FullFrame is TRUE, but it is always saved as the 
	 *           named FITS file if requested, irrespective of the FullFrame 
	 *           setting.
	 * db163 -   A debayered version of r161 in RGB format up to 16-bit depth.  
	 *           This is displayed in DS9 unless FullFrame is TRUE, but it is 
	 *           always saved as the named FITS file if requested, irrespective 
	 *           of the FullFrame setting.  (In practice, for a colour image, 
	 *           this is saved as three separate FITS files representing the R, 
	 *           G and B components).
	 * ff161 -   A full frame version of r161.  If r161 is a subframe, then 
	 *           r161 is embedded in ff161 if FullFrame is TRUE.  In this case,
	 *           ff161 is displayed in DS9 (via an intermediate FITS file) 
	 *           rather than r161.
	 * ff163 -   A full frame version of db163 containing db163 embedded in it
	 *           (if db163 is a subframe) and displayed in DS9 via an 
	 *           intermediate FITS file instead of db163, if FullFrame is TRUE.
	 * 
	 * CCD camera as autoguider camera:
	 * 
	 * r161  -   RAW greyscale data up to 16-bit depth.  This is not saved as a 
	 *           named FITS file.
	 * ff161 -   A full frame version of r161 that may have been dark 
	 *           subtracted or had values below the specified background level 
	 *           set to zero.  This is not saved as a named FITS file.
	 * 
	 * Other autoguider devices (webcams, frame-grabbers etc):
	 * 
	 * r083  -   8-bit image data containing up to 3 bytes per pixel, obtained
	 *           from the device driver/library (and hence may not be truly 
	 *           raw).  This is not saved as a named FITS file.
	 * ff161 -   A full frame version of r083 that has been converted to 16-bit
	 *           greyscale and may have been dark subtracted or had values below
	 *           the specified background level set to zero.  This is not saved 
	 *           as a named FITS file.
	 * 
	 * All autoguider devices:
	 * 
	 * disp083 - 8-bit image data containing 3 bytes per pixel.  This is a 
	 *           greyscale conversion of ff161, with each group of three bytes 
	 *           the same value.  This is required for plotting on the image 
	 *           canvas which only accepts images in RGB format.  For CCD 
	 *           cameras used as autoguider cameras, disp083 may be gamma 
	 *           corrected in addition to any modifications already in ff161.
	 *           This greyscale data is saved as the named FITS file if 
	 *           requested.
	 */
	gushort *r161;               /* Raw 16-bit grey image data                */
	gushort *db163;              /* 16-bit debayered RGB image data           */
	gushort *ff161;              /* Full frame (possibly modified) 16-bit grey*/
	gushort *ff163;              /* Full frame 16-bit debayered RGB data      */
	enum CamType id;             /* Identifier for this data structure        */
	gshort devnum;               /* Device number in list of possible devices */
	gint fd;                     /* File descriptor for driver file           */
	enum HWDevice device;        /* Type of camera or video input device      */
	gint bayer_pattern;          /* The bayer pattern (see debayer_get_tile)  */
	guchar *r083;                /* Raw data for (non-CCD) autoguider devices */
	guchar *disp083;             /* Manipulated 8-bit RGB (grey) display data */
	gboolean Open;               /* Set flag if camera open                   */
	gboolean Expose;             /* Set flag if CCD/autog exposure in progress*/
	gboolean Debayer;            /* Set flag to debayer the raw data          */
	gboolean FullFrame;          /* Set flag to embed data in full chip area  */
	gboolean FastFocus;          /* Set flag to use fast readout for focusing */
	gboolean AutoSave;           /* Flag to indicate if image to be autosaved */
	gboolean SavePeriodic;       /* Flag to indicate image periodically saved */
	gboolean FileSaved;          /* Flag to indicate current image is saved   */
	gboolean Display;            /* Set flag to display image                 */
	gboolean Beep;               /* Set flag to beep at end of exposure       */
	gboolean Error;              /* Set flag to indicate an error condition in*/
	                             /*  cases where it is not possible to do this*/
	                             /*  in a returning function call.            */
};

struct GemModel {                /* Gemini pointing model parameters          */
	gint A;                      /* Azimuth misalignment from pole            */
	gint E;                      /* Elevation misalignment from pole          */
	gint NP;                     /* Axis non-perpendicularity at pole         */
	gint NE;                     /* Axis non-perpendicularity at equator      */
	gint IH;                     /* Index error in hour angle                 */
	gint ID;                     /* Index error in declination                */
	gint FR;                     /* Mirror flop / Gear play (RA)              */
	gint FD;                     /* Mirror flop / Gear play (Dec)             */
	gint CF;                     /* Counterweight and RA axis flexure         */
	gint TF;                     /* Tube flexure                              */
};

/* interface */

#ifndef GOQAT_INTERFACE
#define GOQAT_INTERFACE

extern gboolean get_entry_int (const gchar *name, gint minval, gint maxval, 
							   gint defval, gint page, gint *val);
extern gboolean get_entry_float (const gchar *name, gdouble minval, 
								 gdouble maxval, gdouble defval, 
								 gint page, gdouble *val);
extern gchar *get_entry_string (const gchar *name);
extern void set_entry_float (const gchar *name, gdouble val);
extern void set_entry_string (const gchar *name, gchar *string);
extern void set_ccd_gui (gboolean set);
extern void set_exposure_buttons (gboolean active);
extern void set_task_buttons (gboolean active);
extern void set_focus_done (void);
extern void set_progress_bar (gboolean zeroise, gint elapsed);
extern void set_status_bar (GtkStatusbar *bar, gchar *message, gboolean Clear);
extern void set_range_minmaxstep (enum Range range, gdouble min, gdouble max, 
								  gdouble step, gushort dp);
extern void set_range_value (enum Range range, gboolean Sensitive, 
							 gdouble value);
extern void ui_set_aug_window_controls (enum HWDevice dev, gboolean Binning);
extern void set_elapsed_time (guint elapsed);
extern void ui_set_augcanv_rect_colour (gchar *colour);
extern void set_fits_data (struct cam_img *img, struct timeval *time, 
	                       gboolean UseDateobs, gboolean QueryHardware);
extern void set_autog_on (gboolean on);
extern void set_guide_on (gboolean on);
extern void get_autog_guide_params (void);
extern void get_autog_movespeed (gboolean *CenterSpeed, gfloat *speed);
extern void set_autog_sensitive (gboolean sensitive, gboolean autogopened);
extern void set_autog_calibrate_done (void);
extern gboolean ui_control_action (gchar *cmd, gchar *token);
extern gboolean ui_show_augcanv_image (void);
extern void ui_show_augcanv_rect (gboolean Show);
extern void ui_set_augcanv_rect_colour (gchar *colour);
extern void ui_set_augcanv_rect_full_area (void);
extern void ui_set_augcanv_crosshair (gdouble x, gdouble y);
extern GooCanvasItem *ui_show_augcanv_plot (GooCanvasPoints *points,
                                            GooCanvasItem *plot);
extern GooCanvasItem *ui_show_augcanv_text (gdouble x, gdouble y, gchar *string, 
                                            gdouble val, gushort type, 
                                            gint sigfig, gchar *colour, 
                                            GooCanvasItem *text);
extern void ui_show_augcanv_centroid (gboolean show, gboolean saturated, 
                                      gfloat h, gfloat v,
                                      gushort x1, gushort x2, 
                                      gushort y1, gushort y2);
extern void ui_show_video_frame (guchar *frame, gchar *timestamp, guint num, 
					             gushort h, gushort v);
extern void ui_show_photom_points (gchar *filename, gfloat aperture);
extern void set_video_range_adjustment (guint num_frames);
extern void set_video_range_value (guint frame_num);
extern gushort get_video_framebufsize (void);
extern gboolean save_file (struct cam_img *img, enum Colour colour, 
	                       gboolean display);
extern void file_saved (struct cam_img *img, gboolean saved);
extern void ui_show_aug_window (void);
extern void ui_hide_aug_window (void);
extern void select_device (void);
extern void get_ccd_image_params (struct exposure_data *exd);
extern void set_camera_state (struct cam_img *img);
extern gboolean get_V4L_settings (gchar *device);
extern gboolean set_filter (gboolean ForceInternal,gchar *filter,gint *fo_diff);
extern gboolean get_apply_filter_offset (void);
extern void apply_filter_focus_offset (gint offset);
extern void set_ccd_deftemp (void);
extern gboolean show_camera_status (gboolean show);
extern gboolean get_filter_info (struct cam_img *img, gchar *filter, gint *pos, 
	                             gint *offset);
extern void check_focuser_temp (void);
extern void save_PEC_guide_speed (gfloat GuideSpeed);
extern void save_RA_worm_pos (gushort WormPos);
extern gfloat get_goto_motion_limit (void);
extern void reset_checkbox_state (enum CheckBox chkbox, gboolean active);
extern gboolean check_format (gboolean is_RA, gchar s[]);
extern guint get_pos_minnum (void);
extern struct tm *get_time (gboolean UTC);
extern gint get_UTC_offset (void);
extern void finished_tasks (void);
extern void msg (const gchar *message);
extern void err_msg (gchar *entry_text, const gchar *message);
extern gboolean show_error (const gchar *routine, const gchar *message);
extern void WriteLog (char *string);
extern void FlushLog (void);
extern void WritePhot (char *string);
extern void init_task_params (void);
extern void free_task_params (void);
extern void get_task_params (gchar **tparams);
extern void set_task_params (gchar **tparams);
extern gchar *get_task_param_value (gboolean FreeString, gchar *string, 
									gint *status);
extern void exit_and_shutdown (void);
#ifdef HAVE_UNICAP
extern void set_record_on (gboolean on);
extern gboolean liveview_record_is_writeable (void);
extern gboolean open_liveview_window (void);
extern void show_liveview_window (void);
extern void hide_liveview_window (void);
extern void close_liveview_window (void);
#endif
GtkWidget *xml_get_widget (GtkBuilder *xml, const gchar *name);
	
#endif /* GOQAT_INTERFACE */

/* ccdcam */

#ifndef GOQAT_CCDCAM
#define GOQAT_CCDCAM

extern void ccdcam_init (void);
extern gboolean ccdcam_get_cameras (void);
extern gboolean ccdcam_open (void);
extern gboolean ccdcam_close (void);
extern void ccdcam_set_exposure_data (struct exposure_data *exd);
extern gboolean ccdcam_start_exposure (void);
extern gboolean ccdcam_cancel_exposure (void);
extern gboolean ccdcam_interrupt_exposure (void);
extern gboolean ccdcam_image_ready (void);
extern gboolean ccdcam_download_image (void);
extern gboolean ccdcam_process_image (void);
extern gboolean ccdcam_debayer (void);
extern gboolean ccdcam_set_temperature (gboolean *AtTemperature);
extern void ccdcam_set_fast_readspeed (gboolean Set);
extern gboolean ccdcam_measure_HFD (gboolean Initialise, gboolean Plot, 
									gint box, struct exposure_data *exd,
									gdouble *hfd);
extern gboolean ccdcam_plot_temperatures (void);
extern gboolean ccdcam_get_status (void);
extern void ccdcam_sx_error_func (int *err, const char *func, char *msg);
extern struct cam_img *get_ccd_image_struct (void);

#endif /* GOQAT_CCDCAM */

/* augcam */

#ifndef GOQAT_AUGCAM
#define GOQAT_AUGCAM

extern void augcam_init (void);
extern gboolean augcam_open (void);
extern gboolean augcam_close (void);
extern gboolean augcam_start_exposure (void);
extern gboolean augcam_image_ready (void);
extern gboolean augcam_capture_exposure (void);
extern gboolean augcam_process_image (void);
extern void augcam_set_camera_binning (gushort h_bin, gushort v_bin);
extern gboolean augcam_set_vidval (enum Range range, gdouble value);
extern gboolean augcam_write_starpos_time (void);
extern gboolean augcam_write_starpos (void);
extern void augcam_read_dark_frame (void);
extern void augcam_flush_dark_frame (void);
#ifdef HAVE_UNICAP
extern void augcam_unicap_new_frame_cb (unicap_event_t event, 
		  unicap_handle_t handle, unicap_data_buffer_t *buffer, gpointer *data);
#endif
#ifdef HAVE_LIBV4L2
gboolean augcam_grab_v4l_buffer (void);
#endif
#ifdef HAVE_SX_CAM
extern gboolean augcam_get_sx_cameras (void);
extern struct sx_cam *augcam_get_sx_cam_struct (void);
#endif
extern struct cam_img *get_aug_image_struct (void);

#endif /* GOQAT_AUGCAM */

/* image */

#ifndef GOQAT_IMAGE
#define GOQAT_IMAGE

extern void xpa_open (void);
extern void xpa_close (void);
extern gboolean xpa_display_image (struct cam_img *img, enum Colour colour);
extern gboolean xpa_get_rect_coords(gfloat *x, gfloat *y, gfloat *w, gfloat *h);
extern gboolean image_calc_hist_and_flux (void);
extern gboolean is_in_image (struct cam_img *img, gushort *xoff1,gushort *yoff1,
	                         gushort *xoff2, gushort *yoff2);
extern void image_get_stats (struct cam_img *img, enum Colour col);
extern gboolean image_embed_data (struct cam_img *img);
extern gboolean image_save_as_fits (struct cam_img *img, gchar *savefile, 
	                                enum Colour colour, gboolean display);
#ifdef HAVE_LIBGRACE_NP
extern gboolean Grace_Open (gchar *plot, gboolean *AlreadyOpen);
extern void Grace_SetXAxis (gshort graph, gfloat xmin, gfloat xmax);
extern void Grace_SetYAxis (gshort graph, gfloat ymin, gfloat ymax);
extern void Grace_XAxisMajorTick (gushort graph, gint tick);
extern void Grace_YAxisMajorTick (gushort graph, gint tick);
extern void Grace_PlotPoints (gushort graph, gushort set, gfloat x, gfloat y);
extern void Grace_ErasePlot (gushort graph, gushort set);
extern void Grace_SaveToFile (gchar *FileName);
extern gboolean Grace_Update (void);
#endif

#endif /* GOQAT_IMAGE */

/* loop */

#ifndef GOQAT_LOOP
#define GOQAT_LOOP

extern void loop_start_loop (void);
extern void loop_ccd_open (gboolean Open);
extern void loop_ccd_start (void);
extern void loop_ccd_interrupt (void);
extern void loop_ccd_cancel (void);
extern void loop_ccd_display_image (void);
extern void loop_ccd_calibrate_autofocus (gboolean Start, gint start_pos,
								          gint end_pos, gint step, gint repeat,
										  gdouble exp_len, gint box);
extern void loop_ccd_autofocus (gboolean Start, gdouble LHSlope, 
								gdouble RHSlope, gdouble PID, gint start_pos, 
								gboolean Inside, gdouble near_HFD, 
								gdouble exp_len, gint box);
extern void loop_ccd_temps (gboolean display, guint period);
extern void loop_autog_open (gboolean Open);
extern void loop_autog_restart (void);
extern void loop_autog_calibrate (gboolean Calibrate);
extern void loop_autog_exposure_wait (gboolean Wait, enum MotionDirection dirn);
extern void loop_autog_guide (gboolean guide);
extern void loop_autog_pause (gboolean pause);
extern void loop_autog_DS9 (void);
extern void loop_focus_open (gboolean Open);
extern void loop_focus_stop (void);
extern gboolean loop_focus_is_focusing (void);
extern void loop_focus_apply_filter_offset (gint offset);
extern void loop_focus_apply_temp_comp (gint pos);
extern void loop_focus_check_done (void);
extern void loop_LiveView_open (gboolean open);
extern void loop_LiveView_record (gboolean record, gchar *dirname);
extern void loop_LiveView_flush_to_disk (gushort buf);
extern void loop_video_iter_frames (gboolean Iter);
extern void loop_telescope_goto (gchar *sRA, gchar *sDec);
extern void loop_telescope_move (gdouble RA, gdouble Dec);
extern void loop_telescope_restart (void);
extern void loop_telescope_park (void);
extern void loop_telescope_yellow (void);
extern void loop_save_image (gint id);
extern void loop_save_periodic (gboolean save, guint period);
extern void loop_stop_loop (void);
extern void loop_watch_activate (gboolean Activate);
extern void loop_tasks_wait (void);
extern void loop_tasks_pause (void);
extern void loop_tasks_at (void);
extern void loop_tasks_script (void);
extern void loop_display_blinkrect (gboolean Blink);
extern guint loop_elapsed_since_first_iteration (void);

#endif /* GOQAT_LOOP */

/* telescope */

#ifndef GOQAT_TELESCOPE
#define GOQAT_TELESCOPE

extern void telescope_init (void);
extern gboolean telescope_open_comms_port (void);
extern void telescope_close_comms_port (void);
extern gboolean telescope_open_guide_port (void);
extern void telescope_close_guide_port (void);
#ifdef HAVE_LIBPARAPIN
extern gboolean telescope_open_parallel_port (void);
#endif
extern gboolean telescope_close_parallel_port (void);
extern void telescope_move_start (enum TelMotion direction);
extern void telescope_move_stop (enum TelMotion direction);
extern void telescope_guide_start (enum TelMotion direction);
extern void telescope_guide_stop (enum TelMotion direction);
void telescope_guide_pulse (enum TelMotion direction, gint duration);								   
extern void telescope_d_start (enum TelMotion direction);
extern void telescope_d_stop (enum TelMotion direction);
extern void telescope_d_pulse (enum TelMotion direction, gint duration);
extern void telescope_s_start (enum TelMotion direction);
extern void telescope_s_stop (enum TelMotion direction);
extern void telescope_s_pulse (enum TelMotion direction, gint duration);
#ifdef HAVE_LIBPARAPIN
extern void telescope_p_start (enum TelMotion direction);
extern void telescope_p_stop (enum TelMotion direction);
#endif
extern void telescope_goto (gchar *sRA, gchar *sDec);
extern gboolean telescope_goto_done (void);
extern void telescope_move_by (gdouble RA, gdouble Dec);
extern void telescope_warm_restart (void);
extern gboolean telescope_warm_restart_done (void);
extern void telescope_park_mount (void);
extern gboolean telescope_park_mount_done (void);
extern gboolean telescope_yellow_button (void);
extern void telescope_set_center_speed (gfloat speed);
extern void telescope_set_guide_speed (gfloat speed);
extern gboolean telescope_autog_calib (gfloat *calib_coords);
extern gboolean telescope_guide (enum Guide status, guint elapsed);
extern gboolean telescope_write_guidecorr_time (void);
extern gboolean telescope_get_RA_Dec (gboolean precess, gdouble *epoch, 
									  gchar *sRA, gchar *sDec,
	                                  gdouble *fRA, gdouble *fDec, 
									  gboolean *OK_RA, gboolean *OK_Dec);
extern gushort telescope_get_RA_worm_pos (void);
extern void telescope_get_guide_corrs (guint *l, guint *r, guint *u, guint *d, 
									   gfloat *percent);
extern gboolean telescope_get_gemini_model (struct GemModel *gp);
extern gboolean telescope_set_gemini_model (struct GemModel *gp);
extern gboolean telescope_get_gemini_PEC (gchar *filename);
extern gboolean telescope_set_gemini_PEC (gchar *filename);
extern gboolean telescope_set_gemini_defaults (gchar *filename);
extern gboolean telescope_PEC_on (gboolean On);
extern void telescope_set_time (void);
extern guint telescope_query_status (gboolean list);
extern void telescope_save_motion_rates (void);
extern void telescope_restore_motion_rates (void);
extern gdouble stof_RA (gchar *RA);

#endif /* GOQAT_TELESCOPE */

/* serial ports */

#ifndef GOQAT_SERIAL
#define GOQAT_SERIAL

extern void serial_define_comms_ports (void);
extern gboolean serial_set_comms_port (const gchar *name);
extern gboolean serial_open_port (struct port *p, enum PortUsers user);
extern void serial_close_port (struct port *p);
extern gint s_read (gint file, gchar string[], gint bytes);
extern gint s_write (gint file, gchar string[], gint bytes);
	
#endif /* GOQAT_SERIAL */

/* filter wheels */

#ifndef GOQAT_FILTER
#define GOQAT_FILTER

extern void filter_init (void);
extern void filter_set_device (enum HWDevice dev);
extern gboolean filter_get_filterwheels (void);
extern gshort *get_filter_devnumptr (void);
extern gboolean filter_open_comms_port (void);
extern gboolean filter_close_comms_port (void);
extern gboolean filter_is_open (void);
extern gboolean filter_set_filter_pos (gushort pos);
extern gushort filter_get_filter_pos (void);

#endif /* GOQAT_FILTER */
	
/* focussing */
	
#ifndef GOQAT_FOCUS
#define GOQAT_FOCUS
	
extern gboolean focus_open_comms_port (void);
extern void focus_close_comms_port (void);
extern gboolean focus_is_moving (void);
extern void focus_store_temp_and_pos (void);
extern void focus_get_temp_diff_pos (gdouble *temp_diff, gint *pos);
	
#endif /* GOQAT_FOCUS */

/* tasks */

#ifndef GOQAT_TASKS
#define GOQAT_TASKS

extern void tasks_init (GtkBuilder *xml);
extern void tasks_start (void);
extern void tasks_pause (gboolean pause);
extern void tasks_stop (void);
extern void tasks_task_done (enum TaskTypes Task);
extern void tasks_move_up (void);
extern void tasks_move_down (void);
extern void tasks_delete (void);
extern void tasks_clear (void);
extern void tasks_add_Object (gchar *str);
extern void tasks_add_BeginSequence (void);
extern void tasks_add_WaitUntil (gchar *str);
extern void tasks_add_PauseFor (gchar *str);
extern void tasks_add_At (gchar *str);
extern void tasks_add_Exposure (gchar **stra);
extern void tasks_add_BeginLoop (gchar *str);
extern void tasks_add_EndLoop (void);
extern void tasks_add_FocusTo (gchar *str);
extern void tasks_add_FocusMove (gchar *str);
extern void tasks_add_IfTrue (gchar *str);
extern void tasks_add_IfFalse (gchar *str);
extern void tasks_add_EndIf (void);
extern void tasks_add_While (gchar *str);
extern void tasks_add_EndWhile (void);
extern void tasks_add_Aug (gboolean on);
extern void tasks_add_Guide (gboolean start);
extern void tasks_add_GoTo (gchar *str1, gchar *str2);
extern void tasks_add_Move (gchar *str1, gchar *str2);
extern void tasks_add_Exec (gchar *filename, gboolean Exec);
extern void tasks_add_SetParam (gchar *param, gchar *str);
extern void tasks_add_WarmRestart (void);
extern void tasks_add_ParkMount (void);
extern void tasks_add_Record (gboolean record);
extern void tasks_add_YellowButton (void);
extern void tasks_add_Shutdown (void);
extern void tasks_add_Exit (void);
extern void tasks_activate_watch (void);
extern gboolean tasks_execute_tasks (gboolean TestOnly, gboolean *error);
extern void tasks_watch_file (void);
extern guint tasks_get_status (void);
extern gboolean tasks_task_wait (void);
extern gboolean tasks_task_pause (void);
extern gboolean tasks_task_at (void);
extern gboolean tasks_load_file (gchar *filename);
extern gboolean tasks_write_file (gchar *filename);
extern gboolean tasks_script_done (void);

#endif /* GOQAT_TASKS */
								
/* video */

#ifndef GOQAT_VIDEO
#define GOQAT_VIDEO

extern void video_init (void);
#ifdef HAVE_UNICAP
extern gboolean video_record_start (gchar *dirname);
extern void video_record_stop (void);
extern void video_buffer_frame (guchar *buffer, struct timeval *fill_time, 
	                            guint now);
extern void video_flush_buffer (gushort buf);
#endif
extern gboolean video_open_file (gchar *file);
extern gint video_get_frame_number (void);
extern void video_set_start_time (void);
extern void video_set_frame_rate (gushort fps);
extern gboolean video_show_frame (guint frame_num);
extern gboolean video_update_timestamp (gchar *stamp);
extern void video_set_timestamps (void);
extern void video_play_frames (gboolean Play);
extern void video_show_prev (void);
extern void video_show_next (void);
extern void video_close_file (void);
extern gboolean video_save_frames (gchar *file, gint filetype, gboolean Range);
extern gboolean video_iter_frames (gboolean Final);
extern struct cam_img *get_vid_image_struct (void);
extern void video_photom_frames (gfloat aperture, gfloat minarea, 
							     gfloat thresh, gfloat shift, gboolean Range);
#endif /* GOQAT_VIDEO */

#ifndef GOQAT_DEBAYER
#define GOQAT_DEBAYER

#define restrict __restrict
extern gint dc1394_bayer_NearestNeighbor_uint16 (
					     const gushort *restrict bayer, gushort *restrict rgb, 
						 gint sx, gint sy, gint tile, gint bits);
extern gint dc1394_bayer_Bilinear_uint16 (
						 const gushort *restrict bayer, gushort *restrict rgb,
						 gint sx, gint sy, gint tile, gint bits);
extern gint dc1394_bayer_HQLinear_uint16 (
						 const gushort *restrict bayer, gushort *restrict rgb,
						 gint sx, gint sy, gint tile, gint bits);
extern gint dc1394_bayer_Downsample_uint16 (
						 const gushort *restrict bayer, gushort *restrict rgb,
						 gint sx, gint sy, gint tile, gint bits);
extern gint dc1394_bayer_Simple_uint16 (
						 const gushort *restrict bayer, gushort *restrict rgb,
						 gint sx, gint sy, gint tile, gint bits);	
extern gint dc1394_bayer_VNG_uint16 (
						 const gushort *restrict bayer, gushort *restrict dst,
						 gint sx, gint sy, gint pattern, gint bits);
extern gint dc1394_bayer_AHD_uint16 (
						 const gushort *restrict bayer, gushort *restrict dst,
						 gint sx, gint sy, gint pattern, gint bits);
extern gint debayer_get_tile (gint pattern, gshort h_offset, gshort v_offset);

#endif /* GOQAT_DEBAYER */

/* --variables-- */
extern gchar *PrivatePath;        /* Path to folder for private files         */
extern gboolean GeminiCmds;       /* TRUE if Losmandy Gemini commands used    */
extern gboolean UseUTC;           /* Controls use of UTC/localtime            */
extern gboolean Debug;            /* Controls writing of debug messages to log*/
