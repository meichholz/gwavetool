#ifndef H_WAVEVIEW
#define H_WAVEVIEW

#include "base.h"

#define WAVEVIEW_ZOOM_MAX 8

typedef enum { idle, aborted, play, record } TRecorderState;

struct TWaveView {

  struct TFrame  *pFrame;
  struct TApp    *pApp;

  GtkWidget      *pCanvasLeft;
  GtkWidget      *pCanvasRight;
  GtkWidget      *pSeparator;
  GtkWidget      *pScrollbar;

  long		          liRecorder;
  volatile int            idOutputDevice; /* esd/oss file handle */
  volatile pthread_t      thRecorder;
  
  volatile TRecorderState  stateRecorder;

  int            iZoom;
  struct TZoomLevel {
    double dLeft,dRight; /* in seconds */
  } aZoomLevel[WAVEVIEW_ZOOM_MAX];
};

void   get_time_code(gchar *szBuffer, int cchBufferMax, double dTime);

void   waveview_init(struct TWaveView *me, class TFrame *pParentIn, GtkWidget *pParentBoxIn);
void   waveview_destroy(struct TWaveView *me);

double waveview_point_to_time(struct TWaveView *me, long lx);
double waveview_sample_to_time(struct TWaveView *me, long li);
long   waveview_time_to_sample(struct TWaveView *me, double dTime);
long   waveview_time_to_point(struct TWaveView *me, double dTime);

void   waveview_on_paint_canvas(struct TWaveView *me, GdkEventExpose *pEvent,
				GtkWidget *pCanvas,
				int iChannel);
void     waveview_on_paint(struct TWaveView *me, GdkEventExpose *pEvent);
void     waveview_repaint(struct TWaveView *me);
gboolean waveview_on_mouse_move(struct TWaveView *me, GdkEventMotion *pEvent);
gboolean waveview_on_mouse_button(struct TWaveView *me, GdkEventButton *pEvent);
void     waveview_on_range_value(struct TWaveView *me, GtkRange *pScroller);

gboolean waveview_can_zoom_out(struct TWaveView *me);
gboolean waveview_can_zoom_in(struct TWaveView *me);
void     waveview_zoom_in(struct TWaveView *me);
void     waveview_zoom_out(struct TWaveView *me);
void     waveview_zoom_reset(struct TWaveView *me, gboolean bRepaint);
void     waveview_setup_scroller(struct TWaveView *me);
  
void     waveview_abort_recorder(struct TWaveView *me);
void     waveview_close_recorder(struct TWaveView *me);
void     waveview_start_play(struct TWaveView *me);
void*    waveview_recorder_thread(struct TWaveView *me); /* must be public, called from wrapper */
gboolean waveview_is_busy(struct TWaveView *me);
  
#endif
