#include "base.h"

void procPaint(GObject *pobject, GdkEventExpose *pEvent, class TBase *pOwner)
{  pOwner->OnPaint(pEvent); }

void procConfigure(GObject *pobject, GdkEventConfigure *pEvent, class TBase *pOwner)
{ pOwner->OnConfigure(pEvent); }

void procDelete(GObject *pobject, GdkEventAny *pEvent, class TBase *pOwner)
{ pOwner->OnDelete(pEvent); }

void procDestroy(GObject *pobject, GdkEventAny *pEvent, class TBase *pOwner)
{ pOwner->OnDestroy(pEvent); }

gboolean procMouseMove(GObject *pobject, GdkEventMotion *event, class TBase *pOwner)
{
  return pOwner->OnMouseMove(event);
}

gboolean procMouseButton(GObject *pobject, GdkEventButton *event, class TBase *pOwner)
{
  return pOwner->OnMouseButton(event);
}

void  procRangeValue(GObject *pobject, class TBase *pOwner)
{
  pOwner->OnRangeValue(GTK_RANGE(pobject));
}

void procMenu(class TBase *p,guint uID, GtkWidget *pMenu)
{
  p->OnMenu(uID);
}
