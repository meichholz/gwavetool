/*
  frame.cc
*/

#include "app.h"
#include "frame.h"
#include "wave.h"
#include "waveview.h"
#include "dlgabout.h"
#include "statusbar.h"

#include <stdarg.h>

/* utility */

GtkWindow*   frame_window(struct TFrame *me)
 {
   return GTK_WINDOW(me->pwndTop);
 }

/* ======================================================================
 * Idle Task Mimic
 * ====================================================================== */

void         frame_idle_task(struct TFrame *me)
{
  static double f=0.0;
  static double fInc=0.01;
  f+=fInc;
  if (f>=1.0) fInc=-0.01;
  if (f<=0.0) fInc=0.01;
  me->pStatusBar->SetPercentage(f);
}

/* ======================================================================
 * Application Repaint
 * ====================================================================== */

void     frame_repaint(struct TFrame *me)
{
  GdkRectangle rect;
  gdk_window_get_frame_extents(me->pwndTop->window,&rect);
  // gtk_window_get_position(GTK_WINDOW(pwndTop),&rect.x,&rect.y);
  rect.x=rect.y=0;
  gdk_window_invalidate_rect(me->pwndTop->window,&rect,true);
}

/* ======================================================================
 * general message box
 * ====================================================================== */

gint         frame_message_box(struct TFrame *me,
			       GtkMessageType idType,
			       GtkButtonsType idButtons,
			       const char *szContent, ...)
{
  char szBuffer[1024];
  va_list ap;
  va_start(ap,szContent);
  vsnprintf(szBuffer,1024,szContent,ap);
  szBuffer[sizeof(szBuffer)-1]='\0';
  GtkWidget *dlg=gtk_message_dialog_new(GTK_WINDOW(me->pwndTop),
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

gboolean     frame_message_confirm(struct TFrame *me, const char *szContent, ...)
{
  char szBuffer[1024];
  va_list ap;
  va_start(ap,szContent);
  vsnprintf(szBuffer,1024,szContent,ap);
  szBuffer[sizeof(szBuffer)-1]='\0';
  GtkWidget *dlg=gtk_message_dialog_new(GTK_WINDOW(me->pwndTop),
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

void         frame_message_error(struct TFrame *me, const char *szContent, ...)
 {
  char szBuffer[1024];
  va_list ap;
  va_start(ap,szContent);
  vsnprintf(szBuffer,1024,szContent,ap);
  szBuffer[sizeof(szBuffer)-1]='\0';
  GtkWidget *dlg=gtk_message_dialog_new(GTK_WINDOW(me->pwndTop),
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

void         frame_message_notimplemented(struct TFrame *me)
{
  GtkWidget *pdlg=gtk_message_dialog_new(GTK_WINDOW(me->pwndTop),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					"This function has not (yet) been implemented");
  gtk_dialog_run(GTK_DIALOG(pdlg));
  gtk_widget_destroy(pdlg);
}

void frame_on_menu_new(struct TFrame *me)
{
     frame_message_notimplemented(me);
}

/* ======================================================================
 * Load new File with File selector
 * ====================================================================== */

void         frame_set_busy_cursor(struct TFrame *me, gboolean bBusy)
{
   gdk_window_set_cursor(me->pwndTop->window,
			 gdk_cursor_new(bBusy ? GDK_WATCH : GDK_LEFT_PTR));
   app_poll_queue(me->pApp);
}

void         frame_on_menu_load(struct TFrame *me)
{
  GtkFileSelection *psel;
  GtkWidget *pdlg;
  if (!app_can_close(me->pApp)) return;
  pdlg=gtk_file_selection_new(("Open Waveform file"));
  psel=GTK_FILE_SELECTION(pdlg);
  gtk_widget_show(pdlg);
  /* ignored: gtk_file_selection_set_filename(psel,GetLastFilename()); */
  gtk_file_selection_complete(psel,"*.wav"); /* must be done AFTER show() */

  gint rc=gtk_dialog_run(GTK_DIALOG(pdlg));
  
  if (rc!=GTK_RESPONSE_OK)
    {
      gtk_widget_destroy(pdlg);
      frame_sync_state(me);
      return;
    }
  const gchar *szFile=gtk_file_selection_get_filename(psel);
  gtk_widget_destroy(pdlg);
  frame_set_busy_cursor(me,true);
  gboolean bOk=app_new_wave_from_file(me->pApp,szFile);
  frame_set_busy_cursor(me,true);
  if (bOk)
    frame_set_last_filename(me,szFile);
  else
    frame_message_error(me,"cannot open %s",szFile);
  frame_repaint(me);
  me->pWaveView->ZoomReset();
}

/* ======================================================================
 * ActicateLRU() : Re-Activate fiel from LRU Menu. Fix Menu accordingly.
 * ====================================================================== */

TResult frame_activate_LRU(struct TFrame *me, int iLRUfile)
{
  char *szFile=me->apstrLRU[iLRUfile]->str;
  frame_set_busy_cursor(me,true);
  gboolean bOk=app_new_wave_from_file(me->pApp,szFile);
  frame_set_busy_cursor(me,false);
  if (bOk)
    frame_set_last_filename(me,szFile);
  else
    frame_message_error(me,"cannot open %s",szFile);
  me->pWaveView->ZoomReset();
  frame_sync_state(me);
  frame_repaint(me);
  return bOk ? rcOk : rcIO;
}

/* ======================================================================
 * other menu stuff
 * ====================================================================== */

void frame_on_menu_save(struct TFrame *me)
{
     frame_message_notimplemented(me);
}

void frame_on_menu_save_as(struct TFrame *me)
{
     frame_message_notimplemented(me);
}

gboolean frame_on_menu(struct TFrame *me, guint id)
{
  switch (id)
    {
      /* file menu */
    case ID_NEW:    frame_on_menu_new(me); break;
    case ID_OPEN:   frame_on_menu_load(me); break;
    case ID_SAVE:   frame_on_menu_save(me); break;
    case ID_SAVEAS: frame_on_menu_save_as(me); break;
    case ID_QUIT:
      /* TODO: depends on the state (aka CanClose()) */
      if (app_can_close(me->pApp) || 
	  frame_message_confirm(me,
				"Thus this great application *really* be shut down, my dear?"))
	gtk_main_quit();
      break;
      /* demo menu */
    case ID_DEMO_INIT:
      me->pApp->pWave->CreateSamples();
      me->pWaveView->ZoomReset();
      frame_repaint(me);
      break;
      /* Help Menu */
    case ID_VIEW_STOP:     me->pWaveView->AbortRecorder(); break;
    case ID_VIEW_PLAY:     me->pWaveView->StartPlay(); return true; /* no repaint! */
    case ID_VIEW_RECORD:   frame_message_notimplemented(me); break;
    case ID_VIEW_ZOOMIN:   me->pWaveView->ZoomIn(); break;
    case ID_VIEW_ZOOMOUT:  me->pWaveView->ZoomOut(); break;
    case ID_VIEW_ZOOM0:    me->pWaveView->ZoomReset(); break;
    case ID_LASTFILE1:
    case ID_LASTFILE2:
    case ID_LASTFILE3:
    case ID_LASTFILE4:
    case ID_LASTFILE5:
      frame_activate_LRU(me,id-ID_LASTFILE1);
      break;
    case ID_ABOUT:
      { TAboutDialog dlg;
        aboutdialog_init(&dlg,me->pApp);
        aboutdialog_do_modal(&dlg);
	aboutdialog_destroy(&dlg);
      }
      break;
    default:
      frame_message_notimplemented(me);
    }
  frame_sync_state(me);
  return true; /* no further processing required */
}

void frame_on_delete(struct TFrame *me, GdkEventAny *pev)
{
  me->bDead=true;
  frame_on_menu(me,ID_QUIT);
}

// ======================================================================
// Constructor stuff
// ======================================================================

#define BRANCH "<Branch>"
#define SEPARATOR "<Separator>"

static void procLocalMenu(struct TFrame *p,guint uID, GtkWidget *pMenu)
{
  frame_on_menu(p,uID);
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

void frame_init(struct TFrame *me, struct TApp *papp)
{
  int i;
  SETZERO(me);
  me->pApp=papp;
  me->pwndTop=gtk_window_new(GTK_WINDOW_TOPLEVEL);

  me->pstrLastFile=g_string_new("untitled.wav");

  me->pag = gtk_accel_group_new();
  me->pmif = gtk_item_factory_new(GTK_TYPE_MENU_BAR,"<main>",me->pag);
  gint cItems = sizeof(menuTemplate)/sizeof(menuTemplate[0]);
  gtk_item_factory_create_items(me->pmif,cItems,menuTemplate,me);
  gtk_window_add_accel_group(GTK_WINDOW(me->pwndTop),me->pag);
  me->pMenu=gtk_item_factory_get_widget(me->pmif,"<main>");

  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      char szPath[100];
      sprintf(szPath,"unknown.%d.wav",i);
      me->apstrLRU[i]=g_string_new(szPath);
      // g_string_assign(apstrLRU[i],szPath);
    }
  g_string_assign(me->apstrLRU[0],"../testfiles/1sax.wav");
  g_string_assign(me->apstrLRU[1],"1test.wav");
  g_string_assign(me->apstrLRU[2],"test2.wav");
  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      char szPath[100];
      sprintf(szPath,"/File/Lastfile%d",i+1);
      me->aMenuLRU[i]=gtk_item_factory_get_item(me->pmif,szPath);
    }
  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      GtkWidget *label=gtk_bin_get_child(GTK_BIN(me->aMenuLRU[i]));
      gtk_label_set_text(GTK_LABEL(label),me->apstrLRU[i]->str);
    }

  me->pVbox=gtk_vbox_new(FALSE,FALSE);

  gtk_container_add(GTK_CONTAINER(me->pwndTop),me->pVbox);
  gtk_box_pack_start(GTK_BOX(me->pVbox),me->pMenu,FALSE,FALSE,0);
  gtk_widget_show(me->pMenu);

  me->pWaveView = new TWaveView(me,me->pVbox);
  me->pStatusBar = new TStatusBar(me,me->pVbox);

  gtk_widget_show(me->pVbox);

  g_signal_connect(G_OBJECT(me->pwndTop), EVENT_TYPE_DELETE, G_CALLBACK(procDelete),me);
  g_signal_connect(G_OBJECT(me->pwndTop), EVENT_TYPE_DELETE, G_CALLBACK(procDelete),me);

  /* --- geometry and arrangement */
  /* gtk_window_parse_geometry       (GTK_WINDOW(me->pwndTop), szOptGeometry); */

  gtk_window_set_default_size(GTK_WINDOW(me->pwndTop),640,480);

  gtk_widget_show(me->pwndTop);
  /* gdk_window_move(me->pwndTop->window,10,40); */

  me->bDead=false;
}

// ======================================================================

void frame_destroy(struct TFrame *me)
{
  int i;
  me->bDead=true;
  delete me->pWaveView;
  delete me->pStatusBar;
  g_object_unref(me->pmif);
  g_string_free(me->pstrLastFile,true);
  for (i=0; i<FRAME_LRU_NUM; i++)
    g_string_free(me->apstrLRU[i],true);
  gtk_widget_destroy(me->pwndTop);
}

// ======================================================================
// State synchronisator
// ======================================================================

// TODO: Hier herrscht noch totale Konzeptlosigkeit!!!

void frame_sync_state(struct TFrame *me)
{
  if (me->bDead) return;

  GString *pstrCaption=g_string_new("");
  g_string_sprintf(pstrCaption,"%s - %s",szAppTitle,me->pstrLastFile->str);
  gtk_window_set_title(GTK_WINDOW(me->pwndTop),pstrCaption->str);
  g_string_free(pstrCaption,true);
  gboolean bHaveFile=(NULL != me->pApp->pWave
		      && me->pApp->pWave->IsValid()
		      && me->pApp->pWave->GetSampleCount());
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

  gtk_widget_set_sensitive(gtk_item_factory_get_item(me->pmif,"/File/Save"),bHaveFile);
  gtk_widget_set_sensitive(gtk_item_factory_get_item(me->pmif,"/File/Save as..."),bHaveFile);
  gtk_widget_set_sensitive(gtk_item_factory_get_item(me->pmif,"/View/Zoom in"),bHaveFile && me->pWaveView->CanZoomIn());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(me->pmif,"/View/Zoom out"),bHaveFile && me->pWaveView->CanZoomOut());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(me->pmif,"/View/Play"),bHaveFile && me->pWaveView->IsBusy());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(me->pmif,"/View/Record"),bHaveFile && me->pWaveView->IsBusy());
  gtk_widget_set_sensitive(gtk_item_factory_get_item(me->pmif,"/View/Stop"),bHaveFile && !(me->pWaveView->IsBusy()));
}

/* ======================================================================
 * Last file name management
 * ====================================================================== */

const gchar *frame_get_last_filename(struct TFrame *me)
{
  return me->pstrLastFile->str;
}

void frame_set_last_filename(struct TFrame *me, const gchar *szFile)
{
  g_string_assign(me->pstrLastFile,szFile);
  frame_sync_state(me);
}

