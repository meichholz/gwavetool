#include "base.h"
#include "app.h"
#include "frame.h"

#include "statusbar.h"


// ======================================================================
// Constructor stuff
// ======================================================================

TStatusBar::TStatusBar(class TFrame *pParent,GtkWidget *pContainer) : TBase(pParent->pApp)
{
  pFrame=pParent;
  int i;
  pBox=gtk_hbox_new(false,2);
  gtk_box_pack_end(GTK_BOX(pContainer),pBox,false,false,2);

  for (i=0; i<STATUSBAR_NUM_ELEMENTS; i++)
    apFrames[i]=gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(pBox),apFrames[0],false,false,0);
  gtk_box_pack_start(GTK_BOX(pBox),apFrames[1],false,false,0);
  // gtk_box_pack_start(GTK_BOX(pBox),apFrames[2],true,true,0);
  for (i=0; i<STATUSBAR_NUM_ELEMENTS; i++)
    gtk_widget_show(apFrames[i]);
  gtk_widget_show(pBox);
  pLabelXY=gtk_label_new("00:00:00:000");
  pProgress=gtk_progress_bar_new();
  pStatusbar=gtk_statusbar_new();
  gtk_container_add(GTK_CONTAINER(apFrames[0]),pLabelXY);
  gtk_container_add(GTK_CONTAINER(apFrames[1]),pProgress);
  // gtk_container_add(GTK_CONTAINER(apFrames[2]),pStatusbar);
  gtk_box_pack_start(GTK_BOX(pBox),pStatusbar,true,true,0);
  gtk_widget_show(pLabelXY);
  gtk_widget_show(pProgress);
  gtk_widget_show(pStatusbar);

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pProgress),0.0);
  gtk_statusbar_push(GTK_STATUSBAR(pStatusbar),1," ready.");

}

TStatusBar::~TStatusBar()
{
}

void TStatusBar::SetPercentage(double dPercent)
{
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pProgress),dPercent);
}

void TStatusBar::SetPos(const gchar *szTimeCode)
{
  gtk_label_set_text(GTK_LABEL(pLabelXY),szTimeCode);
}
