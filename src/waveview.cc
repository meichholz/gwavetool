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
 * Index conversion functions
 * ====================================================================== */

#define WAVEVIEW_TIME_LEFT  (aZoomLevel[iZoom].dLeft)
#define WAVEVIEW_TIME_RIGHT (aZoomLevel[iZoom].dRight)

double TWaveView::PointToTime(long lx)
{
  int cx,cy;
  gdk_drawable_get_size(pCanvasLeft->window,&cx,&cy);
  double dLeft,dRight;
  dRight=WAVEVIEW_TIME_RIGHT;
  dLeft=WAVEVIEW_TIME_LEFT;
  return dLeft+((double)lx-1.0)/(double)cx*(dRight-dLeft);
}

double TWaveView::SampleToTime(long li)
{
  class TWave *pWave=App()->Wave();
  if (!pWave->IsValid()) return 0.0;
  return (double)li/(double)(pWave->GetSampleRate());
}

long TWaveView::TimeToPoint(double dTime)
{
  double dLeft,dRight;
  int cx,cy;
  gdk_drawable_get_size(pCanvasLeft->window,&cx,&cy);
  dRight=WAVEVIEW_TIME_RIGHT;
  dLeft=WAVEVIEW_TIME_LEFT;
  return long((dTime-dLeft)/(dRight-dLeft)*(double)(cx-1.0));
}

long TWaveView::TimeToSample(double dTime)
{
  return long(dTime*(double)(App()->Wave()->GetSampleRate()));
}

/* ======================================================================
 * Zoom control layer
 * ====================================================================== */

void TWaveView::OnRangeValue(GtkRange *pScroller)
{
  double dWidth=WAVEVIEW_TIME_RIGHT-WAVEVIEW_TIME_LEFT;
  aZoomLevel[iZoom].dLeft=gtk_range_get_value(pScroller);
  aZoomLevel[iZoom].dRight=WAVEVIEW_TIME_LEFT+dWidth;
  SetupScroller();
  Repaint();
  return;
}


void TWaveView::SetupScroller(void)
{
  GtkObject *pad;
  double dMin=WAVEVIEW_TIME_LEFT;
  double dMax=WAVEVIEW_TIME_RIGHT;
  double dFrame=dMax-dMin;
  double dTotalTime=SampleToTime(App()->Wave()->GetSampleCount());

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
  // gtk_range_set_update_policy(GTK_RANGE(pScrollbar),GTK_UPDATE_DELAYED);
  gtk_range_set_adjustment(GTK_RANGE(pScrollbar),GTK_ADJUSTMENT(pad));
  // !!! pad MUST NOT be unreffed
}

void TWaveView::ZoomReset(gboolean bRepaint)
{
  iZoom=0;
  aZoomLevel[iZoom].dLeft=0.0;
  aZoomLevel[iZoom].dRight=SampleToTime(App()->Wave()->GetSampleCount()-1);
  SetupScroller();
  if (bRepaint)
    Repaint();
}

void TWaveView::ZoomIn(void)
{
  if (!CanZoomIn()) return;
  double dMid=(WAVEVIEW_TIME_RIGHT+WAVEVIEW_TIME_LEFT)/2.0;
  double dWidth=(WAVEVIEW_TIME_RIGHT-WAVEVIEW_TIME_LEFT)/5.0;
  iZoom++;
  aZoomLevel[iZoom].dLeft=dMid-dWidth/2.0;
  aZoomLevel[iZoom].dRight=dMid+dWidth/2.0;
  SetupScroller();
  Repaint();
}

void TWaveView::ZoomOut(void)
{
  if (!CanZoomOut()) return;
  iZoom--;
  SetupScroller();
  Repaint();
}

gboolean TWaveView::CanZoomIn(void)
{
  return (App()->Wave()->IsValid() && iZoom<WAVEVIEW_ZOOM_MAX-1);
}

gboolean TWaveView::CanZoomOut(void)
{
  return (App()->Wave()->IsValid() && iZoom>0);
}

/* ======================================================================
 * Mouse handler
 * ====================================================================== */

gboolean TWaveView::OnMouseMove(GdkEventMotion *event)
{
  /* get time code from coord */
  double dTime=PointToTime((long)event->x);
  gchar szBuffer[WAVEVIEW_TIMECODE_LENGTH];
  GetTimeCode(szBuffer,sizeof(szBuffer),dTime);
  pFrame->pStatusBar->SetPos(szBuffer);
  return true; /* true=!propagate */
}

gboolean TWaveView::OnMouseButton(GdkEventButton *event)
{
  long x,y;
  x=int(event->x);
  y=int(event->y);
  debug_printf(DEBUG_SELECTION,"view: %ld/%ld:%04x/%d",x,y,event->state,event->type);
  return true; /* true=!propagate */
}

/* ======================================================================
 * Repaint() seems not to be elegant.
 * It just invalidates the known area of the canvas window...
 * ====================================================================== */

void TWaveView::Repaint(void)
{
  GdkRectangle rect;
  rect.x=rect.y=0;
  gdk_drawable_get_size(pCanvasLeft->window,&rect.width,&rect.height);
  gdk_window_invalidate_rect(pCanvasLeft->window,&rect,TRUE);
  gdk_drawable_get_size(pCanvasRight->window,&rect.width,&rect.height);
  gdk_window_invalidate_rect(pCanvasRight->window,&rect,TRUE);
}

/* ======================================================================
 * OnPaint : Draw the whole shit
 * ====================================================================== */

#define SUPER_EXPONENT  3

void TWaveView::OnPaint(GdkEventExpose *pEvent)
{
  // TODO: clipping und fallweise Bearbeitung
  OnPaintCanvas(pEvent,pCanvasRight,WAVE_CHANNEL_RIGHT);
  OnPaintCanvas(pEvent,pCanvasLeft,WAVE_CHANNEL_LEFT);
}

void TWaveView::OnPaintCanvas(GdkEventExpose *pEvent,
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
  class TWave *pWave=App()->Wave();
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
	  x=TimeToPoint(dTime);
	  li=TimeToSample(dTime);
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
	  long x=TimeToPoint(dTime);
	  char szBuffer[WAVEVIEW_TIMECODE_LENGTH];
	  gdk_draw_line(pDrawable,pgc,x,yScale-cyTick,x,yScale+cyTick);
	  GetTimeCode(szBuffer,sizeof(szBuffer),dTime);
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
 * GetTimeCode() terminates safely and gives H:MM:SS:mmm
 * ====================================================================== */

void TWaveView::GetTimeCode(gchar *szBuffer, int cchBufferMax, double dTime)
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

/* ======================================================================
 * CloseRecorder(void)
 * ====================================================================== */

void TWaveView::CloseRecorder(void)
{
#ifdef CONFIG_USE_OSS
  close(idOutputDevice);
#endif
#ifdef CONFIG_USE_ESD
  esd_audio_close(idOutputDevice);
#endif
}

/* ======================================================================
 * StartPlay()
 * ====================================================================== */

void *TWaveView::RecorderThread(void)
{
  class TWave      *pWave=App()->Wave();
  class TStatusBar *pStatusBar=pFrame->pStatusBar;
  if (!pWave || !pWave->IsValid())
    {
      stateRecorder=idle;
      CloseRecorder();
      return NULL;
    }
  // TODO: setting channels, samplerate and so on
  int li;
  short *psBuffer=new short[pWave->GetFrameSize()];
  if (!psBuffer)
    {
      stateRecorder=idle;
      CloseRecorder();
      return NULL;
      // TODO: Panic()
    }
  gdk_threads_enter();
  pFrame->SyncState();
  gdk_threads_leave();
  long liStart=TimeToSample(WAVEVIEW_TIME_LEFT)/WAVE_FRAME_SAMPLES;
  long liEnd=TimeToSample(WAVEVIEW_TIME_RIGHT)/WAVE_FRAME_SAMPLES;

#ifdef CONFIG_USE_OSS
  int param;
  // ioctl(idOutputDevice,SNDCTL_DSP_RESET,NULL);
  param=AFMT_S16_LE;
  ioctl(idOutputDevice,SNDCTL_DSP_SETFMT,&param);
  param=App()->Wave()->GetChannelCount();
  ioctl(idOutputDevice,SNDCTL_DSP_CHANNELS,&param);
  param=App()->Wave()->GetSampleRate();
  ioctl(idOutputDevice,SNDCTL_DSP_SPEED,&param);
  printf("Set SPEED to %d\n",param);
#endif
  switch (stateRecorder)
    {
    case play:
      for (li=liStart;
	   liEnd>liStart && li<liEnd && stateRecorder!=aborted;
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
	  write(idOutputDevice,psBuffer,pWave->GetFrameSize()*sizeof(short));
#endif
	}
      break;
    default:
      debug_printf(DEBUG_RECORDER,"unsupported recorder state: %d",stateRecorder);
    }
  CloseRecorder();
  delete [] psBuffer;
 
  stateRecorder=idle;
  gdk_threads_enter();
  pStatusBar->SetPercentage(0.0);
  pFrame->SyncState();
  gdk_threads_leave();
  // THREAD EXIT
  pthread_exit(NULL);
}

static void *procRecorder(void *pView)
{
  return ((class TWaveView *)pView)->RecorderThread();
}

typedef void *(*TThreadProc)(void*);

void TWaveView::StartPlay(void)
{
#ifdef CONFIG_USE_ESD
  idOutputDevice=esd_audio_open();
  if (!idOutputDevice)
    {
      pFrame->MessageError("Cannot contact ESD");
      return;
    }
#endif
#ifdef CONFIG_USE_OSS
  idOutputDevice=open("/dev/dsp",O_WRONLY);
  if (!idOutputDevice)
    {
      pFrame->MessageError("Cannot contact DSP device");
      return;
    }
#endif

  stateRecorder=play;
  int rc=pthread_create((pthread_t*)&thRecorder,NULL,(TThreadProc)procRecorder,this);
  if (rc!=0)
    {
      CloseRecorder();
      stateRecorder=idle;
      pFrame->MessageError("Cannot create recorder thread");
      return;
    }
  else
    pthread_detach(thRecorder);
}
// ======================================================================
// Constructor stuff
// ======================================================================

TWaveView::TWaveView(class TFrame *pParent, GtkWidget *pParentBox) : TBase(pParent->App())
{
  pFrame=pParent;
  pCanvasLeft=gtk_drawing_area_new();
  pCanvasRight=gtk_drawing_area_new();
  // TODO: entfällt: pSeparator=gtk_hseparator_new();
  pScrollbar=gtk_hscrollbar_new(NULL);
  gtk_box_pack_start(GTK_BOX(pParentBox),pCanvasLeft,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(pParentBox),pScrollbar,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(pParentBox),pCanvasRight,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(pCanvasLeft), EVENT_TYPE_EXPOSE, G_CALLBACK(procPaint),gpointer(this));
  g_signal_connect(G_OBJECT(pCanvasRight), EVENT_TYPE_EXPOSE, G_CALLBACK(procPaint),gpointer(this));
  gtk_widget_show(pCanvasLeft);
  gtk_widget_show(pScrollbar);
  gtk_widget_show(pCanvasRight);

  /*
   * enable mouse move notofication, connect signal handler
   */

  g_signal_connect(G_OBJECT(pCanvasRight), EVENT_TYPE_MOUSEMOVE, G_CALLBACK(procMouseMove),this);
  g_signal_connect(G_OBJECT(pCanvasLeft), EVENT_TYPE_MOUSEMOVE, G_CALLBACK(procMouseMove),this);
  g_signal_connect(G_OBJECT(pCanvasRight), EVENT_TYPE_BUTTONDOWN, G_CALLBACK(procMouseButton),this);
  g_signal_connect(G_OBJECT(pCanvasRight), EVENT_TYPE_BUTTONUP, G_CALLBACK(procMouseButton),this);
  g_signal_connect(G_OBJECT(pCanvasLeft), EVENT_TYPE_BUTTONDOWN, G_CALLBACK(procMouseButton),this);
  g_signal_connect(G_OBJECT(pCanvasLeft), EVENT_TYPE_BUTTONUP, G_CALLBACK(procMouseButton),this);
  gtk_widget_add_events(pCanvasRight,GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events(pCanvasLeft,GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK);
 
  g_signal_connect(G_OBJECT(pScrollbar), SIGNAL_TYPE_VALUECHANGED, G_CALLBACK(procRangeValue),this);

  // some easy geometry hints
  GdkGeometry gg;
  memset(&gg,0,sizeof(gg));
  gg.min_width=10; gg.min_height=10;
  gtk_window_set_geometry_hints (GTK_WINDOW(pParent->Window()),
				 pCanvasLeft,
				 &gg,
				 GdkWindowHints(GDK_HINT_MIN_SIZE));
  gtk_window_set_geometry_hints (GTK_WINDOW(pParent->Window()),
				 pCanvasRight,
				 &gg,
				 GdkWindowHints(GDK_HINT_MIN_SIZE));
  ZoomReset(false);
  stateRecorder=idle;
}

TWaveView::~TWaveView()
{
  gtk_widget_destroy(pCanvasLeft);
  gtk_widget_destroy(pCanvasRight);
  // TODO: gtk_widget_destroy(pSeparator);
  gtk_widget_destroy(pScrollbar);
}

