#ifndef H_DLG_NEW
#define H_DLG_NEW

#include "base.h"

#include "gwavetool.h"

struct TDlgNew {

  struct TApp    *pApp;
  struct TFrame  *pFrame;

  GtkWidget      *pdlg;
  GladeXML       *pGlade;

  int cChannels;
  int cBits;
  int nSamplerate;

};

struct TDlgNew *dlgnew_create(struct TFrame *pParent);
void            dlgnew_destroy(struct TDlgNew *me);
gint            dlgnew_run(struct TDlgNew *me);

void            dlgnew_set(struct TDlgNew *me, int cBitsIn, int cChannelsIn, int nSamplerateIn);
void            dlgnew_get(struct TDlgNew *me, int *pcBitsOut, int *pcChannelsOut, int *pnSamplerateOut);

#endif
