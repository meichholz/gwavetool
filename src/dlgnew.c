/*
  frame.cc
*/

#include "app.h"
#include "frame.h"
#include "dlgnew.h"

#include <stdarg.h>

#define TB(s) GTK_TOGGLE_BUTTON(glade_xml_get_widget(me->pGlade,s))
#define WCONNECT(f) glade_xml_signal_connect_data(me->pGlade,#f,G_CALLBACK(f),me)

static void on_sps_toggled(GtkButton *p, struct TDlgNew *me)
{
  const gchar *sz;
  sz=gtk_button_get_label(p);
  fprintf(stderr,"PRESSED: %s\n",sz);
}


/* ======================================================================
 * Constructor stuff
 * ====================================================================== */

struct TDlgNew *dlgnew_create(struct TFrame *pParent)
{
  struct TDlgNew *me;
  me=(struct TDlgNew*)calloc(1,sizeof(struct TDlgNew));
  if (!me) return NULL;
  
  me->pFrame=pParent;
  me->pApp=pParent->pApp;

  me->pGlade=glade_xml_new("gwavetool.glade", "dlg-new", NULL);
  if (me->pGlade==NULL)
    {
      /* TODO: HELPER!!! Fatal exception equivalent */
      fprintf(stderr,"FATAL:frame_init: cannot construct GLADE\n");
    }

  me->pdlg=glade_xml_get_widget(me->pGlade, "dlg-new");

  WCONNECT(on_sps_toggled);
  return me;
}

void dlgnew_destroy(struct TDlgNew *me)
{
  g_object_unref(me->pGlade);
  gtk_widget_destroy(me->pdlg);
}

/* ====================================================================== */

void dlgnew_set(struct TDlgNew *me, int cBitsIn, int cChannelsIn, int nSamplerateIn)
{
  me->cBits=cBitsIn;
  me->cChannels=cChannelsIn;
  me->nSamplerate=nSamplerateIn;
}

void dlgnew_get(struct TDlgNew *me, int *pcBitsOut, int *pcChannelsOut, int *pnSamplerateOut)
{
  *pcBitsOut=me->cBits;
  *pcChannelsOut=me->cChannels;
  *pnSamplerateOut=me->nSamplerate;
}

gint dlgnew_run(struct TDlgNew *me)
{
  gchar szName[20];
  /* gtk_widget_show(me->pdlg); */
  snprintf(szName,sizeof(szName),"%dbit",me->cBits);
  gtk_toggle_button_set_active(TB(szName),true);
  snprintf(szName,sizeof(szName),"%dsps",me->nSamplerate);
  gtk_toggle_button_set_active(TB(szName),true);
  gint rc=gtk_dialog_run(GTK_DIALOG(me->pdlg));
  return rc;
 }

/* ====================================================================== */

