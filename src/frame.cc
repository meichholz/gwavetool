/*
  frame.cc
*/

#include "app.h"
#include "frame.h"
#include "wave.h"
#include "waveview.h"
#include "dlgabout.h"
#include "statusbar.h"
#include "menus.h"

#include <stdarg.h>

/* ======================================================================
 * Idle Task Mimic
 * ====================================================================== */

void TFrame::IdleTask(void)
{
  static double f=0.0;
  static double fInc=0.01;
  f+=fInc;
  if (f>=1.0) fInc=-0.01;
  if (f<=0.0) fInc=0.01;
  pStatusBar->SetPercentage(f);
}

/* ======================================================================
 * Application Repaint
 * ====================================================================== */

void TFrame::Repaint(void)
{
  GdkRectangle rect;
  gdk_window_get_frame_extents(pwndTop->window,&rect);
  // gtk_window_get_position(GTK_WINDOW(pwndTop),&rect.x,&rect.y);
  rect.x=rect.y=0;
  gdk_window_invalidate_rect(pwndTop->window,&rect,true);
}

/* ======================================================================
 * general message box
 * ====================================================================== */

gint TFrame::MessageBox(GtkMessageType idType, GtkButtonsType idButtons,
			 const char *szContent, ...)
{
  char szBuffer[1024];
  va_list ap;
  va_start(ap,szContent);
  vsnprintf(szBuffer,1024,szContent,ap);
  szBuffer[sizeof(szBuffer)-1]='\0';
  GtkWidget *dlg=gtk_message_dialog_new(GTK_WINDOW(pwndTop),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					idType,idButtons,
					"%s",szBuffer);
  gint rc=gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
  va_end(ap);
  return rc;
}

/* ======================================================================
 * The stupid Yes-No-ShootMe question
 * ====================================================================== */

gboolean TFrame::MessageConfirm(const char *szContent, ...)
{
  char szBuffer[1024];
  va_list ap;
  va_start(ap,szContent);
  vsnprintf(szBuffer,1024,szContent,ap);
  szBuffer[sizeof(szBuffer)-1]='\0';
  GtkWidget *dlg=gtk_message_dialog_new(GTK_WINDOW(pwndTop),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					"%s",szBuffer);
  gint rc=gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
  va_end(ap);
  return rc==GTK_RESPONSE_YES; /* -8 */
}

/* ======================================================================
 * A convenient error message box
 * ====================================================================== */

void TFrame::MessageError(const char *szContent, ...)
{
  char szBuffer[1024];
  va_list ap;
  va_start(ap,szContent);
  vsnprintf(szBuffer,1024,szContent,ap);
  szBuffer[sizeof(szBuffer)-1]='\0';
  GtkWidget *dlg=gtk_message_dialog_new(GTK_WINDOW(pwndTop),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s",szBuffer);
  gtk_dialog_run(GTK_DIALOG(dlg));
  gtk_widget_destroy(dlg);
  va_end(ap);
}

/* ======================================================================
 * Not implemented dummy funktion notificator
 * ====================================================================== */

void TFrame::MessageNotimplemented(void)
{
  GtkWidget *pdlg=gtk_message_dialog_new(GTK_WINDOW(pwndTop),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					"This function has not (yet) been implemented");
  gtk_dialog_run(GTK_DIALOG(pdlg));
  gtk_widget_destroy(pdlg);
}

void TFrame::OnMenuNew(void)
{
     MessageNotimplemented();
}

/* ======================================================================
 * Load new File with File selector
 * ====================================================================== */

void TFrame::SetBusyCursor(gboolean bBusy)
{
   gdk_window_set_cursor(pwndTop->window,gdk_cursor_new(bBusy ? GDK_WATCH : GDK_LEFT_PTR));
   App()->PollQueue();
}

void TFrame::OnMenuLoad(void)
{
  GtkFileSelection *psel;
  GtkWidget *pdlg;
  if (!App()->CanClose()) return;
  pdlg=gtk_file_selection_new(_("Open Waveform file"));
  psel=GTK_FILE_SELECTION(pdlg);
  gtk_widget_show(pdlg);
  /* ignored: gtk_file_selection_set_filename(psel,GetLastFilename()); */
  gtk_file_selection_complete(psel,"*.wav"); /* must be done AFTER show() */

  gint rc=gtk_dialog_run(GTK_DIALOG(pdlg));
  
  if (rc!=GTK_RESPONSE_OK)
    {
      gtk_widget_destroy(pdlg);
      SyncState();
      return;
    }
  const gchar *szFile=gtk_file_selection_get_filename(psel);
  gtk_widget_destroy(pdlg);
  SetBusyCursor(true);
  gboolean bOk=App()->NewWaveFromFile(szFile);
  SetBusyCursor(false);
  if (bOk)
    SetLastFilename(szFile);
  else
    MessageError("cannot open %s",szFile);
  Repaint();
  pWaveView->ZoomReset();
}

/* ======================================================================
 * ActicateLRU() : Re-Activate fiel from LRU Menu. Fix Menu accordingly.
 * ====================================================================== */

TResult TFrame::ActivateLRU(int iLRUfile)
{
  char *szFile=apstrLRU[iLRUfile]->str;
  SetBusyCursor(true);
  gboolean bOk=App()->NewWaveFromFile(szFile);
  SetBusyCursor(false);
  if (bOk)
    SetLastFilename(szFile);
  else
    MessageError("cannot open %s",szFile);
  pWaveView->ZoomReset();
  SyncState();
  Repaint();
  return bOk ? rcOk : rcIO;
}

/* ======================================================================
 * other menu stuff
 * ====================================================================== */

void TFrame::OnMenuSave(void)
{
     MessageNotimplemented();
}

void TFrame::OnMenuSaveAs(void)
{
     MessageNotimplemented();
}

gboolean TFrame::OnMenu(guint id)
{
  switch (id)
    {
      // file menu
    case ID_NEW:    OnMenuSaveAs(); break;
    case ID_OPEN:   OnMenuLoad(); break;
    case ID_SAVE:   OnMenuSave(); break;
    case ID_SAVEAS: OnMenuSaveAs(); break;
    case ID_QUIT:
      // TODO: depends on the state (aka CanClose())
      if (App()->CanClose() || 
	  MessageConfirm("Thus this great application *really* be shut down, my dear?"))
	gtk_main_quit();
      break;
      // demo menu
    case ID_DEMO_INIT:
      App()->Wave()->CreateSamples();
      pWaveView->ZoomReset();
      Repaint();
      break;
      // Help Menu
    case ID_VIEW_STOP:     pWaveView->AbortRecorder(); break;
    case ID_VIEW_PLAY:     pWaveView->StartPlay(); return true; // no repaint!
    case ID_VIEW_RECORD:   MessageNotimplemented(); break;
    case ID_VIEW_ZOOMIN:   pWaveView->ZoomIn(); break;
    case ID_VIEW_ZOOMOUT:  pWaveView->ZoomOut(); break;
    case ID_VIEW_ZOOM0:    pWaveView->ZoomReset(); break;
    case ID_LASTFILE1:
    case ID_LASTFILE2:
    case ID_LASTFILE3:
    case ID_LASTFILE4:
    case ID_LASTFILE5:
      ActivateLRU(id-ID_LASTFILE1);
      break;
    case ID_ABOUT:
      { TAboutDialog dlg(App());
        dlg.DoModal();
      }
      break;
    default:
      MessageNotimplemented();
    }
  SyncState();
  return true; /* no further processing required */
}

void TFrame::OnDelete(GdkEventAny *pev)
{
  bDead=true;
  OnMenu(ID_QUIT);
}

// ======================================================================
// Constructor stuff
// ======================================================================

#define BRANCH "<Branch>"
#define SEPARATOR "<Separator>"

static void procLocalMenu(class TFrame *p,guint uID, GtkWidget *pMenu)
{
  p->OnMenu(uID);
}

#define ITEMCB(p) ((GtkItemFactoryCallback)(p))


static GtkItemFactoryEntry menuTemplate[] = {
  {"/_File",NULL,NULL,0,BRANCH},
    {"/File/_New...","<control>n",ITEMCB(procLocalMenu),ID_NEW,NULL},
    {"/File/_Open...","<control>o",ITEMCB(procLocalMenu),ID_OPEN,NULL},
    {"/File/-",NULL,NULL,0,SEPARATOR},
    {"/File/_Save","<control>s",ITEMCB(procLocalMenu),ID_SAVE,NULL},
    {"/File/_Save as...","<control>s",ITEMCB(procLocalMenu),ID_SAVEAS,NULL},
    {"/File/-",NULL,NULL,0,SEPARATOR},
    {"/File/_Quit","<control>q",ITEMCB(procLocalMenu),ID_QUIT,NULL},
    {"/File/-",NULL,NULL,0,SEPARATOR},
    {"/File/Lastfile1","<control>1",ITEMCB(procLocalMenu),ID_LASTFILE1,NULL},
    {"/File/Lastfile2","<control>2",ITEMCB(procLocalMenu),ID_LASTFILE2,NULL},
    {"/File/Lastfile3","<control>3",ITEMCB(procLocalMenu),ID_LASTFILE3,NULL},
    {"/File/Lastfile4","<control>4",ITEMCB(procLocalMenu),ID_LASTFILE4,NULL},
    {"/File/Lastfile5","<control>5",ITEMCB(procLocalMenu),ID_LASTFILE5,NULL},

  {"/_Edit",NULL,NULL,0,BRANCH},
    {"/Edit/_Cut",NULL,ITEMCB(procLocalMenu),ID_EDIT_CUT,NULL},
    {"/Edit/C_opy",NULL,ITEMCB(procLocalMenu),ID_EDIT_COPY,NULL},
    {"/Edit/_Paste",NULL,ITEMCB(procLocalMenu),ID_EDIT_PASTE,NULL},
    {"/Edit/-",NULL,NULL,0,SEPARATOR},
    {"/Edit/Select _all",NULL,ITEMCB(procLocalMenu),ID_EDIT_SELECTALL,NULL},
    {"/Edit/Select _nothing",NULL,ITEMCB(procLocalMenu),ID_EDIT_SELECTNOTHING,NULL},
  {"/_View",NULL,NULL,0,BRANCH},
    {"/View/_Record",NULL,ITEMCB(procLocalMenu),ID_VIEW_RECORD,NULL},
    {"/View/_Play","<control>p",ITEMCB(procLocalMenu),ID_VIEW_PLAY,NULL},
    {"/View/_Stop",NULL,ITEMCB(procLocalMenu),ID_VIEW_STOP,NULL},
    {"/View/-1",NULL,NULL,0,SEPARATOR},
    {"/View/Zoom _in","<control>z",ITEMCB(procLocalMenu),ID_VIEW_ZOOMIN,NULL},
    {"/View/Zoom _out","<alt>z",ITEMCB(procLocalMenu),ID_VIEW_ZOOMOUT,NULL},
    {"/View/Zoom _back totally",NULL,ITEMCB(procLocalMenu),ID_VIEW_ZOOM0,NULL},
  {"/_Options",NULL,NULL,0,BRANCH},
    {"/Options/_Preferences...",NULL,ITEMCB(procLocalMenu),ID_OPTIONS,NULL},
  {"/_Help",NULL,NULL,0,"<LastBranch>"},
    {"/Help/_Create Demo Waveform","<control>d",ITEMCB(procLocalMenu),ID_DEMO_INIT,NULL},
    {"/Help/-",NULL,NULL,0,SEPARATOR},
    {"/Help/_About...",NULL,ITEMCB(procLocalMenu),ID_ABOUT,NULL},
  };

TFrame::TFrame(class TApp *papp) : TBase(papp)
{
  int i;

  pwndTop=gtk_window_new(GTK_WINDOW_TOPLEVEL);

  pstrLastFile=g_string_new("untitled.wav");

  pag = gtk_accel_group_new();
  pmif = gtk_item_factory_new(GTK_TYPE_MENU_BAR,"<main>",pag);
  gint cItems = sizeof(menuTemplate)/sizeof(menuTemplate[0]);
  gtk_item_factory_create_items(pmif,cItems,menuTemplate,this);
  gtk_window_add_accel_group(GTK_WINDOW(pwndTop),pag);
  pMenu=gtk_item_factory_get_widget(pmif,"<main>");
  pMainMenu = new TMenu("MainMenu",this);

  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      char szPath[100];
      sprintf(szPath,"unknown.%d.wav",i);
      apstrLRU[i]=g_string_new(szPath);
      // g_string_assign(apstrLRU[i],szPath);
    }
  g_string_assign(apstrLRU[0],"1sax.wav");
  g_string_assign(apstrLRU[1],"1test.wav");
  g_string_assign(apstrLRU[2],"test2.wav");
  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      char szPath[100];
      sprintf(szPath,"/File/Lastfile%d",i+1);
      aMenuLRU[i]=gtk_item_factory_get_item(pmif,szPath);
    }
  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      GtkWidget *label=gtk_bin_get_child(GTK_BIN(aMenuLRU[i]));
      gtk_label_set_text(GTK_LABEL(label),apstrLRU[i]->str);
    }

  pVbox=gtk_vbox_new(FALSE,FALSE);

  gtk_container_add(GTK_CONTAINER(pwndTop),pVbox);
  gtk_box_pack_start(GTK_BOX(pVbox),pMenu,FALSE,FALSE,0);
  gtk_widget_show(pMenu);

  pWaveView = new TWaveView(this,pVbox);
  pStatusBar = new TStatusBar(this,pVbox);

  gtk_widget_show(pVbox);

  g_signal_connect(G_OBJECT(pwndTop), EVENT_TYPE_DELETE, G_CALLBACK(procDelete),this);
  g_signal_connect(G_OBJECT(pwndTop), EVENT_TYPE_DELETE, G_CALLBACK(procDelete),this);

  /* --- geometry and arrangement */
  /* gtk_window_parse_geometry       (GTK_WINDOW(pwndTop), szOptGeometry); */

  gtk_window_set_default_size(GTK_WINDOW(pwndTop),640,480);

  gtk_widget_show(pwndTop);
  /* gdk_window_move(pwndTop->window,10,40); */

  bDead=false;
}

// ======================================================================

TFrame::~TFrame()
{
  int i;
  bDead=true;
  delete pWaveView;
  delete pStatusBar;
  delete pMainMenu;
  g_object_unref(pmif);
  g_string_free(pstrLastFile,true);
  for (i=0; i<FRAME_LRU_NUM; i++)
    g_string_free(apstrLRU[i],true);
  gtk_widget_destroy(pwndTop);
}

// ======================================================================
// State synchronisator
// ======================================================================

// TODO: Hier herrscht noch totale Konzeptlosigkeit!!!

void TFrame::SyncState(void)
{
  if (bDead) return;

  GString *pstrCaption=g_string_new("");
  g_string_sprintf(pstrCaption,"%s - %s",szAppTitle,pstrLastFile->str);
  gtk_window_set_title(GTK_WINDOW(pwndTop),pstrCaption->str);
  g_string_free(pstrCaption,true);
  gboolean bHaveFile=(NULL != App()->Wave()
		      && App()->Wave()->IsValid()
		      && App()->Wave()->GetSampleCount());
  enum TState { empty, normal, selection, playing, recording, busy } newstate;
  /* funny stuff to detect running recording or playing code */
  if (false)
    newstate=selection;
  else {
    newstate=empty;
    if (bHaveFile)
      newstate=normal;
  }
  debug_printf(DEBUG_FRAMEWORK,"frame newstate: %d",newstate);
  /*
   * Now rearrange the menu tree according to the new state
   */

  gtk_widget_set_sensitive(gtk_item_factory_get_item(pmif,"/File/Save"),bHaveFile);
  gtk_widget_set_sensitive(gtk_item_factory_get_item(pmif,"/File/Save as..."),bHaveFile);
  gtk_widget_set_sensitive(gtk_item_factory_get_item(pmif,"/View/Zoom in"),bHaveFile && pWaveView->CanZoomIn());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(pmif,"/View/Zoom out"),bHaveFile && pWaveView->CanZoomOut());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(pmif,"/View/Play"),bHaveFile && pWaveView->IsBusy());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(pmif,"/View/Record"),bHaveFile && pWaveView->IsBusy());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(pmif,"/View/Stop"),bHaveFile && !(pWaveView->IsBusy()));

}

/* ======================================================================
 * Last file name management
 * ====================================================================== */

const gchar *TFrame::GetLastFilename(void)
{
  return pstrLastFile->str;
}

void TFrame::SetLastFilename(const gchar *szFile)
{
  g_string_assign(pstrLastFile,szFile);
  SyncState();
}

