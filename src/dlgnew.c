/*
  frame.cc
*/

#include "app.h"
#include "frame.h"
#include "dlgnew.h"

#include <stdarg.h>

#define W(s) glade_xml_get_widget(me->pGlade,s)
#define WCONNECT(f) glade_xml_signal_connect_data(me->pGlade,#f,G_CALLBACK(f),me)

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

  /* WCONNECT(dlgnew_on_new); */
  return me;
}

void dlgnew_destroy(struct TDlgNew *me)
{
  g_object_unref(me->pGlade);
  gtk_widget_destroy(me->pdlg);
}

/* ====================================================================== */

gint dlgnew_run(struct TDlgNew *me)
{
  /* gtk_widget_show(me->pdlg); */
  gint rc=gtk_dialog_run(GTK_DIALOG(me->pdlg));
  return rc;
 }

/* ====================================================================== */

