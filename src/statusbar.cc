#include "base.h"
#include "app.h"
#include "frame.h"

#include "statusbar.h"


// ======================================================================
// Constructor stuff
// ======================================================================

void statusbar_init(struct TStatusBar *me, struct TFrame *pParent, GtkWidget *pContainer)
{
  me->pFrame=pParent;
  int i;
  me->pBox=gtk_hbox_new(false,2);
  gtk_box_pack_end(GTK_BOX(pContainer),me->pBox,false,false,2);

  for (i=0; i<STATUSBAR_NUM_ELEMENTS; i++)
    me->apFrames[i]=gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(me->pBox),me->apFrames[0],false,false,0);
  gtk_box_pack_start(GTK_BOX(me->pBox),me->apFrames[1],false,false,0);
  // gtk_box_pack_start(GTK_BOX(me->pBox),me->apFrames[2],true,true,0);
  for (i=0; i<STATUSBAR_NUM_ELEMENTS; i++)
    gtk_widget_show(me->apFrames[i]);
  gtk_widget_show(me->pBox);
  me->pLabelXY=gtk_label_new("00:00:00:000");
  me->pProgress=gtk_progress_bar_new();
  me->pStatusBar=gtk_statusbar_new();
  gtk_container_add(GTK_CONTAINER(me->apFrames[0]),me->pLabelXY);
  gtk_container_add(GTK_CONTAINER(me->apFrames[1]),me->pProgress);
  // gtk_container_add(GTK_CONTAINER(me->apFrames[2]),me->pStatusBar);
  gtk_box_pack_start(GTK_BOX(me->pBox),me->pStatusBar,true,true,0);
  gtk_widget_show(me->pLabelXY);
  gtk_widget_show(me->pProgress);
  gtk_widget_show(me->pStatusBar);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(me->pProgress),0.0);
  gtk_statusbar_push(GTK_STATUSBAR(me->pStatusBar),1," ready.");

}

void statusbar_destroy(struct TStatusBar *me)
{
  /* TODO: pBox vernichten? Container auflösen? */
}

void statusbar_set_percentage(struct TStatusBar *me, double dPercent)
{
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(me->pProgress),dPercent);
}

void statusbar_set_pos(struct TStatusBar *me, const gchar *szTimeCode)
{
  gtk_label_set_text(GTK_LABEL(me->pLabelXY),szTimeCode);
}
