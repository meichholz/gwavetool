#ifndef H_WAVEVIEW
#define H_WAVEVIEW

#include "base.h"

#define WAVEVIEW_ZOOM_MAX 8

typedef enum { idle, aborted, play, record } TRecorderState;

class TWaveView : public TBase {

 private:

  class TFrame   *pFrame;

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

 private:
  void     CloseRecorder(void);

 public:

           TWaveView(class TFrame *pParentIn, GtkWidget *pParentBoxIn);
  virtual ~TWaveView();

          void   GetTimeCode(gchar *szBuffer, int cchBufferMax, double dTime);
	  double PointToTime(long lx);
	  double SampleToTime(long li);
	  long   TimeToSample(double dTime);
	  long   TimeToPoint(double dTime);

  virtual void OnPaintCanvas(GdkEventExpose *pEvent,
			     GtkWidget *pCanvas,
			     int iChannel);
  virtual void OnPaint(GdkEventExpose *pEvent);
  virtual void Repaint(void);
  virtual gboolean OnMouseMove(GdkEventMotion *pEvent);
  virtual gboolean OnMouseButton(GdkEventButton *pEvent);
  virtual void OnRangeValue(GtkRange *pScroller);

  gboolean CanZoomOut(void);
  gboolean CanZoomIn(void);
  void     ZoomIn(void);
  void     ZoomOut(void);
  void     ZoomReset(gboolean bRepaint=true);
  void     SetupScroller(void);
  
  void     AbortRecorder(void) { stateRecorder=aborted; }
  void     StartPlay(void);
  void     *RecorderThread(void); /* must be public, called from wrapper */
  gboolean IsBusy(void) { return stateRecorder==idle; }
  
};

#endif
