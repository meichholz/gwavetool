#ifndef H_DLG_NEW
#define H_DLG_NEW

#include "base.h"

#include "gwavetool.h"

struct TDlgNew {

  struct TApp    *pApp;
  struct TFrame  *pFrame;

  GtkWidget      *pdlg;
  GladeXML       *pGlade;

};

struct TDlgNew *dlgnew_create(struct TFrame *pParent);
void            dlgnew_destroy(struct TDlgNew *me);
gint            dlgnew_run(struct TDlgNew *me);

#endif
