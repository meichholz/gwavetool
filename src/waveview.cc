/*
  waveview.cc
*/

#include "app.h"
#include "frame.h"
#include "wave.h"
#include "waveview.h"
#include "statusbar.h"

#include <values.h>
#include <math.h>

#define CONFIG_USE_OSS

#ifdef CONFIG_USE_ESD
#include <esd.h>
#endif

#ifdef CONFIG_USE_OSS
#include <fcntl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#endif

#define STANDARD_FONT_NAME   "-*-lucida-*-r-*-*-10-*-*-*-*-*-*-*"

#define WAVEVIEW_TIMECODE_LENGTH 14

/*
 * Ok, the scaleParams structure is a bit denaturated for now, but who knows,
 * if there will be some need for new members later...
 */

#define WAVEVIEW_MIN_INTERVAL 0.000010

struct {
  double dInterval;
} scaleParams[] = {
  { 600.0 },
  { 300.0 },
  { 150.0 },
  { 90.0 },
  { 60.0 },
  { 25.0 },
  { 10.0 },
  { 5.0 },
  { 2.5 },
  { 1.0 },
  { 0.5 },
  { 0.25 },
  { 0.2 },
  { 0.1 },
  { 0.05 },
  { 0.025 },
  { 0.02 },
  { 0.01 },
  { 0.005 },
  { 0.0025 },
  { 0.002 },
  { 0.001 },
  { 0.0005 },
  { 0.00025 },
  { 0.0001 },
  { 0.00005 },
  { 0.000025 },
  { WAVEVIEW_MIN_INTERVAL },
  { 0.0 }
};

/* ======================================================================
 * GetTimeCode() terminates safely and gives H:MM:SS:mmm
 * ====================================================================== */

void   get_time_code(gchar *szBuffer,
		     int cchBufferMax,
		     double dTime)
{
  int nHour,nMin,nSec,nMilli;
  nHour=int(dTime/3600.0);
  nMin=int((dTime-3600.0*nHour)/60.0);
  nSec=int((dTime-3600.0*nHour-60.0*nMin));
  nMilli=int((dTime-floor(dTime))*1000.0);
  snprintf(szBuffer,cchBufferMax,"%d:%02d:%02d:%03d",
	   nHour,nMin,nSec,nMilli);
  szBuffer[cchBufferMax-1]='\0';
}

/* ====================================================================== */

gboolean waveview_is_busy(struct TWaveView *me)
 {
  return me->stateRecorder==idle;
 }

/* ======================================================================
 * Index conversion functions
 * ====================================================================== */

#define WAVEVIEW_TIME_LEFT  (me->aZoomLevel[me->iZoom].dLeft)
#define WAVEVIEW_TIME_RIGHT (me->aZoomLevel[me->iZoom].dRight)

double waveview_point_to_time(struct TWaveView *me, long lx)
{
  int cx,cy;
  gdk_drawable_get_size(me->pCanvasLeft->window,&cx,&cy);
  double dLeft,dRight;
  dRight=WAVEVIEW_TIME_RIGHT;
  dLeft=WAVEVIEW_TIME_LEFT;
  return dLeft+((double)lx-1.0)/(double)cx*(dRight-dLeft);
}

double waveview_sample_to_time(struct TWaveView *me, long li)
{
  class TWave *pWave=me->pApp->pWave;
  if (!pWave->IsValid()) return 0.0;
  return (double)li/(double)(pWave->GetSampleRate());
}

long   waveview_time_to_point(struct TWaveView *me, double dTime)
{
  double dLeft,dRight;
  int cx,cy;
  gdk_drawable_get_size(me->pCanvasLeft->window,&cx,&cy);
  dRight=WAVEVIEW_TIME_RIGHT;
  dLeft=WAVEVIEW_TIME_LEFT;
  return long((dTime-dLeft)/(dRight-dLeft)*(double)(cx-1.0));
}

long   waveview_time_to_sample(struct TWaveView *me, double dTime)
{
  return long(dTime*(double)(me->pApp->pWave->GetSampleRate()));
}

/* ======================================================================
 * Zoom control layer
 * ====================================================================== */

void     waveview_on_range_value(struct TWaveView *me, GtkRange *pScroller)
{
  double dWidth=WAVEVIEW_TIME_RIGHT-WAVEVIEW_TIME_LEFT;
  me->aZoomLevel[me->iZoom].dLeft=gtk_range_get_value(pScroller);
  me->aZoomLevel[me->iZoom].dRight=WAVEVIEW_TIME_LEFT+dWidth;
  waveview_setup_scroller(me);
  waveview_repaint(me);
  return;
}


void     waveview_setup_scroller(struct TWaveView *me)
{
  GtkObject *pad;
  double dMin=WAVEVIEW_TIME_LEFT;
  double dMax=WAVEVIEW_TIME_RIGHT;
  double dFrame=dMax-dMin;
  double dTotalTime=waveview_sample_to_time(me,me->pApp->pWave->GetSampleCount());

  // enshure full page in nonvalid waveforms, just cosmetic...
  if (dFrame<=WAVEVIEW_MIN_INTERVAL) dFrame=WAVEVIEW_MIN_INTERVAL;
  if (dTotalTime<=WAVEVIEW_MIN_INTERVAL) dTotalTime=WAVEVIEW_MIN_INTERVAL;
  pad=gtk_adjustment_new(dMin,
			 0.0,dTotalTime,
			 dFrame/20.0,
			 dFrame/1.2,
			 dFrame
			 );
  if (!pad) g_print("!! adjustment failed!!!\n");
  // gtk_range_set_update_policy(GTK_RANGE(me->pScrollbar),GTK_UPDATE_DELAYED);
  gtk_range_set_adjustment(GTK_RANGE(me->pScrollbar),GTK_ADJUSTMENT(pad));
  // !!! pad MUST NOT be unreffed
}

void     waveview_zoom_reset(struct TWaveView *me, gboolean bRepaint)
{
  me->iZoom=0;
  me->aZoomLevel[me->iZoom].dLeft=0.0;
  me->aZoomLevel[me->iZoom].dRight=waveview_sample_to_time(me,me->pApp->pWave->GetSampleCount()-1);
  waveview_setup_scroller(me);
  if (bRepaint)
    waveview_repaint(me);
}

void     waveview_zoom_in(struct TWaveView *me)
{
  if (!waveview_can_zoom_in(me)) return;
  double dMid=(WAVEVIEW_TIME_RIGHT+WAVEVIEW_TIME_LEFT)/2.0;
  double dWidth=(WAVEVIEW_TIME_RIGHT-WAVEVIEW_TIME_LEFT)/5.0;
  me->iZoom++;
  me->aZoomLevel[me->iZoom].dLeft=dMid-dWidth/2.0;
  me->aZoomLevel[me->iZoom].dRight=dMid+dWidth/2.0;
  waveview_setup_scroller(me);
  waveview_repaint(me);
}

void     waveview_zoom_out(struct TWaveView *me)
{
  if (!waveview_can_zoom_out(me)) return;
  me->iZoom--;
  waveview_setup_scroller(me);
  waveview_repaint(me);
}

gboolean waveview_can_zoom_in(struct TWaveView *me)
{
  return (me->pApp->pWave->IsValid() && me->iZoom<WAVEVIEW_ZOOM_MAX-1);
}

gboolean waveview_can_zoom_out(struct TWaveView *me)
{
  return (me->pApp->pWave->IsValid() && me->iZoom>0);
}

/* ======================================================================
 * Mouse handler
 * ====================================================================== */

gboolean waveview_on_mouse_move(struct TWaveView *me, GdkEventMotion *pEvent)
{
  /* get time code from coord */
  double dTime=waveview_point_to_time(me,(long)pEvent->x);
  gchar szBuffer[WAVEVIEW_TIMECODE_LENGTH];
  get_time_code(szBuffer,sizeof(szBuffer),dTime);
  me->pFrame->pStatusBar->SetPos(szBuffer);
  return true; /* true=!propagate */
}

gboolean waveview_on_mouse_button(struct TWaveView *me, GdkEventButton *pEvent)
{
  long x,y;
  x=int(pEvent->x);
  y=int(pEvent->y);
  debug_printf(DEBUG_SELECTION,"view: %ld/%ld:%04x/%d",x,y,pEvent->state,pEvent->type);
  return true; /* true=!propagate */
}

/* ======================================================================
 * waveview_repaint(me) seems not to be elegant.
 * It just invalidates the known area of the canvas window...
 * ====================================================================== */

void     waveview_repaint(struct TWaveView *me)
{
  GdkRectangle rect;
  rect.x=rect.y=0;
  gdk_drawable_get_size(me->pCanvasLeft->window,&rect.width,&rect.height);
  gdk_window_invalidate_rect(me->pCanvasLeft->window,&rect,TRUE);
  gdk_drawable_get_size(me->pCanvasRight->window,&rect.width,&rect.height);
  gdk_window_invalidate_rect(me->pCanvasRight->window,&rect,TRUE);
}

/* ======================================================================
 * OnPaint : Draw the whole shit
 * ====================================================================== */

#define SUPER_EXPONENT  3

void     waveview_on_paint(struct TWaveView *me, GdkEventExpose *pEvent)
{
  // TODO: clipping und fallweise Bearbeitung
  waveview_on_paint_canvas(me,pEvent,me->pCanvasRight,WAVE_CHANNEL_RIGHT);
  waveview_on_paint_canvas(me,pEvent,me->pCanvasLeft,WAVE_CHANNEL_LEFT);
}

void   waveview_on_paint_canvas(struct TWaveView *me, GdkEventExpose *pEvent,
				GtkWidget *pCanvas,
				int iChannel)
{
  /* g_print("Wave exposure\n"); */
  static GdkColor rgb_colors[] = {
    { 0,0,0,0 },
    { 0,0,0x8000,0 },
    { 0,0,0xFFFF,0 },
    { 0,0xC000,0xC000,0xC000, },
    { 0,0xFFFF,0xFFFF,0xFFFF, },
    
  };
  typedef enum { black,green,bgreen,dkgray,white } TColorIndex;
  gint cx,cy,yScale,cyWave;
  int cColors=sizeof(rgb_colors)/sizeof(rgb_colors[0]);
  GdkDrawable *pDrawable=pCanvas->window;
  GdkGC *pgc=gdk_gc_new(pDrawable);
  gdk_drawable_get_size(pDrawable,&cx,&cy);
  GdkColormap *pColormap=gdk_colormap_get_system();
  gboolean aSuccess[cColors];
  gdk_colormap_alloc_colors(pColormap,rgb_colors,
			    cColors,FALSE,TRUE,aSuccess);
  gdk_gc_set_background(pgc,rgb_colors+black);
  gdk_gc_set_foreground(pgc,rgb_colors+black);
  gdk_draw_rectangle(pDrawable,pgc,TRUE,0,0,cx,cy);
  cyWave=cy/2;
  yScale=cyWave;
  class TWave *pWave=me->pApp->pWave;
  if (pWave && pWave->IsValid() && iChannel<pWave->GetChannelCount())
    {
      long li;
      gint x0,y0,x,y;
      double dLeft=WAVEVIEW_TIME_LEFT;
      double dRight=WAVEVIEW_TIME_RIGHT;
      double dTime;
      x0=y0=0;
      gdk_gc_set_foreground(pgc,rgb_colors+green);
      double dStep=(dRight-dLeft)/(cx<<SUPER_EXPONENT);

      for (dTime=dLeft; dTime<dRight; dTime+=dStep)
	{
	  x=waveview_time_to_point(me,dTime);
	  li=waveview_time_to_sample(me,dTime);
	  y=yScale+cyWave*pWave->PeekSample(iChannel,li)/MAXSHORT;
	  if (!li) { x0=x; y0=y; }
	  gdk_draw_line(pDrawable,pgc,x0,y0,x,y);
	  x0=x; y0=y;
	}
      gdk_gc_set_foreground(pgc,rgb_colors+white);
      int cyTick=cy/50;
      GdkFont *pFont=gdk_font_load(STANDARD_FONT_NAME);
      gdk_font_ref(pFont);
      double dInterval=0.0;
      // choose the biggest suitable interval
      int i;
      for (i=0; dInterval==0.0 && scaleParams[i].dInterval; i++)
	{
	  if ((dRight-dLeft)/6.0 >= scaleParams[i].dInterval)
	    dInterval=scaleParams[i].dInterval; // not too efficiently...
	}
      if (dInterval==0.0) dInterval=WAVEVIEW_MIN_INTERVAL;
      // TODO: choose better left scale border
      double dFirst;
      dFirst=((long)(dLeft/dInterval)+1)*dInterval; /* overflowable? */
      double dLast=dRight;
      for (dTime=dFirst; dTime<dLast; dTime+=dInterval)
	{
	  long x=waveview_time_to_point(me,dTime);
	  char szBuffer[WAVEVIEW_TIMECODE_LENGTH];
	  gdk_draw_line(pDrawable,pgc,x,yScale-cyTick,x,yScale+cyTick);
	  get_time_code(szBuffer,sizeof(szBuffer),dTime);
	  gdk_draw_string(pDrawable,pFont,pgc,x-30,yScale+7*cyTick/3,szBuffer);
	}
      gdk_font_unref(pFont);
    }
  gdk_gc_set_foreground(pgc,rgb_colors+bgreen);
  gdk_draw_line(pDrawable,pgc,0,yScale,cx-1,yScale);
  gdk_colormap_free_colors(pColormap,rgb_colors,cColors);
  gdk_gc_unref(pgc);
}

/* ======================================================================
 * CloseRecorder(void)
 * ====================================================================== */

void     waveview_close_recorder(struct TWaveView *me)
{
#ifdef CONFIG_USE_OSS
  close(me->idOutputDevice);
#endif
#ifdef CONFIG_USE_ESD
  esd_audio_close(me->idOutputDevice);
#endif
}

/* ======================================================================
 * StartPlay()
 * ====================================================================== */

void*    waveview_recorder_thread(struct TWaveView *me)
{
  class TWave      *pWave=me->pApp->pWave;
  class TStatusBar *pStatusBar=me->pFrame->pStatusBar;
  if (!pWave || !pWave->IsValid())
    {
      me->stateRecorder=idle;
      waveview_close_recorder(me);
      return NULL;
    }
  /* TODO: setting channels, samplerate and so on */
  int li;
  short *psBuffer=new short[pWave->GetFrameSize()];
  if (!psBuffer)
    {
      me->stateRecorder=idle;
      waveview_close_recorder(me);
      return NULL;
      // TODO: Panic()
    }
  gdk_threads_enter();
  frame_sync_state(me->pFrame);
  gdk_threads_leave();
  long liStart=waveview_time_to_sample(me,WAVEVIEW_TIME_LEFT)/WAVE_FRAME_SAMPLES;
  long liEnd=waveview_time_to_sample(me,WAVEVIEW_TIME_RIGHT)/WAVE_FRAME_SAMPLES;

#ifdef CONFIG_USE_OSS
  int param;
  // ioctl(me->idOutputDevice,SNDCTL_DSP_RESET,NULL);
  param=AFMT_S16_LE;
  ioctl(me->idOutputDevice,SNDCTL_DSP_SETFMT,&param);
  param=me->pApp->pWave->GetChannelCount();
  ioctl(me->idOutputDevice,SNDCTL_DSP_CHANNELS,&param);
  param=me->pApp->pWave->GetSampleRate();
  ioctl(me->idOutputDevice,SNDCTL_DSP_SPEED,&param);
  printf("Set SPEED to %d\n",param);
#endif
  switch (me->stateRecorder)
    {
    case play:
      for (li=liStart;
	   liEnd>liStart && li<liEnd && me->stateRecorder!=aborted;
	   li++)
	{
	  // should be done by the main loops idle task
	  gdk_threads_enter();
	  pStatusBar->SetPercentage((double)(li-liStart)/(double)(liEnd-liStart));
	  gdk_threads_leave();
	  pWave->GetFrame(psBuffer,li);
#ifdef CONFIG_USE_ESD
	  esd_audio_write(psBuffer,pWave->GetFrameSize()*sizeof(short));
#endif
#ifdef CONFIG_USE_OSS
	  write(me->idOutputDevice,psBuffer,pWave->GetFrameSize()*sizeof(short));
#endif
	}
      break;
    default:
      debug_printf(DEBUG_RECORDER,"unsupported recorder state: %d",me->stateRecorder);
    }
  waveview_close_recorder(me);
  delete [] psBuffer;
 
  me->stateRecorder=idle;
  gdk_threads_enter();
  pStatusBar->SetPercentage(0.0);
  frame_sync_state(me->pFrame);
  gdk_threads_leave();
  // THREAD EXIT
  pthread_exit(NULL);
}

static void *procRecorder(void *pView)
{
  return waveview_recorder_thread((struct TWaveView *)pView);
}

typedef void *(*TThreadProc)(void*);

void     waveview_start_play(struct TWaveView *me)
{
#ifdef CONFIG_USE_ESD
  me->idOutputDevice=esd_audio_open();
  if (!me->idOutputDevice)
    {
      frame_message_error(me->pFrame,"Cannot contact ESD");
      return;
    }
#endif
#ifdef CONFIG_USE_OSS
  me->idOutputDevice=open("/dev/dsp",O_WRONLY);
  if (!me->idOutputDevice)
    {
      frame_message_error(me->pFrame,"Cannot contact DSP device");
      return;
    }
#endif

  me->stateRecorder=play;
  int rc=pthread_create((pthread_t*)&(me->thRecorder),NULL,(TThreadProc)procRecorder,me);
  if (rc!=0)
    {
      me->stateRecorder=idle;
      waveview_close_recorder(me);
      frame_message_error(me->pFrame,"Cannot create recorder thread");
      return;
    }
  else
    pthread_detach(me->thRecorder);
}

void     waveview_abort_recorder(struct TWaveView *me)
 {
   me->stateRecorder=aborted;
   /* TODO: do something to sync */
 }

// ======================================================================
// Constructor stuff
// ======================================================================

/* copycat binding stubs: no virtual functions any more! */

void waveview_procPaint(GObject *pobject, GdkEventExpose *pEvent, struct TWaveView *me)
{  waveview_on_paint(me,pEvent); }

gboolean waveview_procMouseMove(GObject *pobject, GdkEventMotion *pEvent, struct TWaveView *me)
{
  return waveview_on_mouse_move(me,pEvent);
}

gboolean waveview_procMouseButton(GObject *pobject, GdkEventButton *event, struct TWaveView *me)
{
  return waveview_on_mouse_button(me,event);
}

void  waveview_procRangeValue(GObject *pobject, struct TWaveView *me)
{
  waveview_on_range_value(me,GTK_RANGE(pobject));
}

void   waveview_init(struct TWaveView *me,
		     class TFrame *pParent, GtkWidget *pParentBox)
{
  me->pFrame=pParent;
  me->pApp=pParent->pApp;
  me->pCanvasLeft=gtk_drawing_area_new();
  me->pCanvasRight=gtk_drawing_area_new();
  // TODO: entfällt: me->pSeparator=gtk_hseparator_new();
  me->pScrollbar=gtk_hscrollbar_new(NULL);
  gtk_box_pack_start(GTK_BOX(pParentBox),me->pCanvasLeft,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(pParentBox),me->pScrollbar,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(pParentBox),me->pCanvasRight,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(me->pCanvasLeft), EVENT_TYPE_EXPOSE, G_CALLBACK(waveview_procPaint),gpointer(me));
  g_signal_connect(G_OBJECT(me->pCanvasRight), EVENT_TYPE_EXPOSE, G_CALLBACK(waveview_procPaint),gpointer(me));
  gtk_widget_show(me->pCanvasLeft);
  gtk_widget_show(me->pScrollbar);
  gtk_widget_show(me->pCanvasRight);

  /*
   * enable mouse move notofication, connect signal handler
   */

  g_signal_connect(G_OBJECT(me->pCanvasRight), EVENT_TYPE_MOUSEMOVE, G_CALLBACK(waveview_procMouseMove),me);
  g_signal_connect(G_OBJECT(me->pCanvasLeft), EVENT_TYPE_MOUSEMOVE, G_CALLBACK(waveview_procMouseMove),me);
  g_signal_connect(G_OBJECT(me->pCanvasRight), EVENT_TYPE_BUTTONDOWN, G_CALLBACK(waveview_procMouseButton),me);
  g_signal_connect(G_OBJECT(me->pCanvasRight), EVENT_TYPE_BUTTONUP, G_CALLBACK(waveview_procMouseButton),me);
  g_signal_connect(G_OBJECT(me->pCanvasLeft), EVENT_TYPE_BUTTONDOWN, G_CALLBACK(procMouseButton),me);
  g_signal_connect(G_OBJECT(me->pCanvasLeft), EVENT_TYPE_BUTTONUP, G_CALLBACK(waveview_procMouseButton),me);
  gtk_widget_add_events(me->pCanvasRight,GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(me->pCanvasLeft,GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
 
  g_signal_connect(G_OBJECT(me->pScrollbar), SIGNAL_TYPE_VALUECHANGED, G_CALLBACK(waveview_procRangeValue),me);

  // some easy geometry hints
  GdkGeometry gg;
  memset(&gg,0,sizeof(gg));
  gg.min_width=10; gg.min_height=10;
  gtk_window_set_geometry_hints (frame_window(pParent),
				 me->pCanvasLeft,
				 &gg,
				 GdkWindowHints(GDK_HINT_MIN_SIZE));
  gtk_window_set_geometry_hints (frame_window(pParent),
				 me->pCanvasRight,
				 &gg,
				 GdkWindowHints(GDK_HINT_MIN_SIZE));
  waveview_zoom_reset(me,false);
  me->stateRecorder=idle;
}

void   waveview_destroy(struct TWaveView *me)
{
  waveview_abort_recorder(me);
  gtk_widget_destroy(me->pCanvasLeft);
  gtk_widget_destroy(me->pCanvasRight);
  // TODO: gtk_widget_destroy(me->pSeparator);
  gtk_widget_destroy(me->pScrollbar);
}

