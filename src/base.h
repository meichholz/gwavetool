#ifndef H_BASE
#define H_BASE

#include "main.h"

#define EVENT_TYPE_EXPOSE "expose_event"
#define EVENT_TYPE_CONFIGURE "configure_event"

#define EVENT_TYPE_MOUSEMOVE "motion_notify_event"
#define EVENT_TYPE_BUTTONDOWN "button_press_event"
#define EVENT_TYPE_BUTTONUP "button_release_event"

#define EVENT_TYPE_DELETE "delete_event"
#define EVENT_TYPE_DESTROY "destroy"

#define SIGNAL_TYPE_VALUECHANGED "value-changed"

/*
  Glue class for connecting every object to the application providing
  for navigation withing the application.

  Base function is App() giving a reference
*/

void procPaint(GObject *pobject, GdkEventExpose *pEvent, class TBase *pOwner);
void procConfigure(GObject *pobject, GdkEventConfigure *pEvent, class TBase *pOwner);
void procDelete(GObject *pobject, GdkEventAny *pEvent, class TBase *pOwner);
void procDestroy(GObject *pobject, GdkEventAny *pEvent, class TBase *pOwner);
gboolean procMouseMove(GObject *pobject, GdkEventMotion *pEvent, class TBase *pOwner);
gboolean procMouseButton(GObject *pobject, GdkEventButton *pEvent, class TBase *pOwner);
void procRangeValue(GObject *pobject, class TBase *pOwner);
void procMenu(class TBase *p,guint uID, GtkWidget *pMenu);

class TBase {

 private:
  class TApp *papp;

 public:
              TBase(class TApp *pappIn) { papp=pappIn; };
  virtual    ~TBase() {};
  class TApp *App(void) { return papp; };

  virtual void OnPaint(GdkEventExpose *pEvent) { };
  virtual void OnConfigure(GdkEventConfigure *pEvent) { };
  virtual void OnDelete(GdkEventAny *pEvent) { };
  virtual void OnDestroy(GdkEventAny *pEvent) { };
  virtual void OnRangeValue(GtkRange *pRange) { };
  virtual gboolean OnMenu(int idMenu) { return false; };
  virtual gboolean OnMouseMove(GdkEventMotion *pEvent) { return false; };
  virtual gboolean OnMouseButton(GdkEventButton *pEvent) { return false; };
};

#endif
