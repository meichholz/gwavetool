#ifndef H_STATUS_BAR
#define H_STATUS_BAR

#include "base.h"

#define STATUSBAR_NUM_ELEMENTS 3

class TStatusBar : public TBase {

 protected:
  class TFrame *pFrame;

  GtkWidget *pBox,*apFrames[STATUSBAR_NUM_ELEMENTS];
  GtkWidget *pStatusbar,*pLabelXY,*pProgress;

 public:

  /* constructors */
           TStatusBar(class TFrame *pParent,GtkWidget *pContainer);
  virtual ~TStatusBar();

  void     SetPercentage(double dPercent);
  void     SetPos(const gchar *szTimeCode);

};

#endif
