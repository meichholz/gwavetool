#ifndef H_STATUS_BAR
#define H_STATUS_BAR

#include "base.h"

#define STATUSBAR_NUM_ELEMENTS 3

struct TStatusBar {
  struct     TFrame *pFrame;
  GtkWidget *pBox,*apFrames[STATUSBAR_NUM_ELEMENTS];
  GtkWidget *pStatusBar,*pLabelXY,*pProgress;
};

void statusbar_init(struct TStatusBar *me, struct TFrame *pParent, GtkWidget *pContainer);
void statusbar_destroy(struct TStatusBar *me);
void statusbar_set_percentage(struct TStatusBar *me, double dPercent);
void statusbar_set_pos(struct TStatusBar *me, const gchar *szTimeCode);
#endif
