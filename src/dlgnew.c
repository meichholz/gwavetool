/*
  frame.cc
*/

#include "app.h"
#include "frame.h"
#include "dlgnew.h"

#include <stdarg.h>

#define TB(s) GTK_TOGGLE_BUTTON(glade_xml_get_widget(me->pGlade,s))
#define WCONNECT(f) glade_xml_signal_connect_data(me->pGlade,#f,G_CALLBACK(f),me)

static int anLabelSampleRates[]={48000,44100,24000,22050,16000,11025,8000,0};
static int anLabelBits[]={16,8,0};
static const gchar *aszLabelChannels[]={"mono","stereo",NULL};

static void on_sps_toggled(GtkButton *p, struct TDlgNew *me)
{
  const gchar *sz;
  sz=gtk_button_get_label(p);
  fprintf(stderr,"PRESSED: %s\n",sz);
}

static void on_samplerate_changed(GtkComboBox *p, struct TDlgNew *me)
{
  int i=gtk_combo_box_get_active(p);
  me->nSamplerate=anLabelSampleRates[i];
  fprintf(stderr,"found: %d/sec\n",me->nSamplerate);
}
static void on_resolution_changed(GtkComboBox *p, struct TDlgNew *me)
{
  int i=gtk_combo_box_get_active(p);
  me->cBits=anLabelBits[i];
  fprintf(stderr,"found: %d bits\n",me->cBits);
}
static void on_channels_changed(GtkComboBox *p, struct TDlgNew *me)
{
  int i=gtk_combo_box_get_active(p);
  me->cChannels=i+1;
  fprintf(stderr,"found: %d channels\n",me->cChannels);
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

  WCONNECT(on_samplerate_changed);
  WCONNECT(on_resolution_changed);
  WCONNECT(on_channels_changed);

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
  gint rc,iSel,i;
  GtkComboBox *box;
  /* set sample rate */
  box=GTK_COMBO_BOX(glade_xml_get_widget(me->pGlade,"samplerate"));
  iSel=0;
  for (i=0; anLabelSampleRates[i]; i++)
    {
      snprintf(szName,sizeof(szName),"%d",anLabelSampleRates[i]);
      gtk_combo_box_append_text(box,szName);
      if (anLabelSampleRates[i] == me->nSamplerate) iSel=i;
    }
  gtk_combo_box_set_active(box,iSel);
  /* word size */
  box=GTK_COMBO_BOX(glade_xml_get_widget(me->pGlade,"resolution"));
  iSel=0;
  for (i=0; anLabelBits[i]; i++)
    {
      snprintf(szName,sizeof(szName),"%d",anLabelBits[i]);
      gtk_combo_box_append_text(box,szName);
      if (anLabelBits[i] == me->cBits) iSel=i;
    }
  gtk_combo_box_set_active(box,iSel);
  /* channel count */
  box=GTK_COMBO_BOX(glade_xml_get_widget(me->pGlade,"channels"));
  gtk_combo_box_set_active(box,me->cChannels-1);
  /* ok, now run the shit */
  rc=gtk_dialog_run(GTK_DIALOG(me->pdlg));
  return rc;
 }

/* ====================================================================== */

