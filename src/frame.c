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

#define USE_GLADE 1

#include <glade/glade-build.h>

#define W(s) glade_xml_get_widget(me->pGlade,s)
#define WCONNECT(f) glade_xml_signal_connect_data(me->pGlade,#f,G_CALLBACK(f),me)

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
  statusbar_set_percentage(me->pStatusBar,f);
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

void frame_on_new(GtkMenuItem *pItem, struct TFrame *me)
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

void frame_on_load(GtkMenuItem *pItem, struct TFrame *me)
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
  waveview_zoom_reset(me->pWaveView,true);
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
  waveview_zoom_reset(me->pWaveView,true);
  frame_sync_state(me);
  frame_repaint(me);
  return bOk ? rcOk : rcIO;
}

void frame_on_lastfile(GtkMenuItem *pItem, struct TFrame *me)
{
  int i;
  const gchar *szName=gtk_widget_get_name(GTK_WIDGET(pItem));
  fprintf(stderr,"DEBUG:reactivating %s\n",szName);
  i=szName[8]-'0';
  if (i>=1 && i<=5)
    frame_activate_LRU(me,i-1);
  else
    fprintf(stderr,"TODO: FATAL: lastfile out of range\n");
}

/* ======================================================================
 * Quit: Leave everything behind
 * ====================================================================== */

static void frame_do_quit(struct TFrame *me)
{
  if (app_can_close(me->pApp) || 
      frame_message_confirm(me,
			    "Thus this great application *really* be shut down, my dear?"))
    gtk_main_quit();
}

/* ======================================================================
 * Menu callbacks
 * ====================================================================== */

static void frame_on_quit(GtkMenuItem *pItem, struct TFrame *me)
{
  frame_do_quit(me);
}

static void frame_on_save(GtkMenuItem *pItem, struct TFrame *me)
{
     frame_message_notimplemented(me);
}

static void frame_on_save_as(GtkMenuItem *pItem, struct TFrame *me)
{
     frame_message_notimplemented(me);
}

static void frame_on_demo_init(GtkMenuItem *pItem, struct TFrame *me)
{
  wave_create_samples(me->pApp->pWave);
  waveview_zoom_reset(me->pWaveView,true);
  frame_repaint(me);
}

static void frame_on_record(GtkMenuItem *pItem, struct TFrame *me)
{
  frame_message_notimplemented(me); 
}

static void frame_on_play(GtkMenuItem *pItem, struct TFrame *me)
{
  waveview_start_play(me->pWaveView); 
  frame_sync_state(me);
}

static void frame_on_stop(GtkMenuItem *pItem, struct TFrame *me)
{
  waveview_abort_recorder(me->pWaveView); 
  frame_sync_state(me);
}

static void frame_on_zoomin(GtkMenuItem *pItem, struct TFrame *me)
{
  waveview_zoom_in(me->pWaveView); 
  frame_sync_state(me);
}

static void frame_on_zoomout(GtkMenuItem *pItem, struct TFrame *me)
{
  waveview_zoom_out(me->pWaveView); 
  frame_sync_state(me);
}

static void frame_on_zoom0(GtkMenuItem *pItem, struct TFrame *me)
{
  waveview_zoom_reset(me->pWaveView,true); 
  frame_sync_state(me);
}

static void frame_on_about(GtkMenuItem *pItem, struct TFrame *me)
{
  struct TAboutDialog dlg;
  aboutdialog_init(&dlg,me->pApp);
  aboutdialog_do_modal(&dlg);
  aboutdialog_destroy(&dlg);
}

static void frame_on_preferences(GtkMenuItem *pItem, struct TFrame *me)
{
  frame_message_notimplemented(me); 
}

void frame_on_delete(struct TFrame *me, GdkEventAny *pev)
{
  me->bDead=true;
  frame_do_quit(me);
}

// ======================================================================
// Constructor stuff
// ======================================================================

/* NOTE: all callbacks have now to be LOCAL since we have no longer virtual stuff at hand!!! */

void frame_procDelete(GObject *pobject, GdkEventAny *pEvent, struct TFrame *me) { frame_on_delete(me,pEvent); }

void frame_init(struct TFrame *me, struct TApp *papp)
{
  int i;
  SETZERO(me);
  me->pApp=papp;
  me->pwndTop=gtk_window_new(GTK_WINDOW_TOPLEVEL);

  me->pstrLastFile=g_string_new("untitled.wav");


  /* create menu from GLADE */
  me->pGlade=glade_xml_new("gwavetool.glade", "menubar", NULL);
  if (me->pGlade==NULL)
    {
      /* TODO: Fatal exception equivalent */
      fprintf(stderr,"FATAL:frame_init: cannot construct GLADE\n");
    }

  me->pmenu=glade_xml_get_widget(me->pGlade, "menubar");

  WCONNECT(frame_on_new);
  WCONNECT(frame_on_load);
  WCONNECT(frame_on_save);
  WCONNECT(frame_on_save_as);
  WCONNECT(frame_on_new);
  WCONNECT(frame_on_quit);
  WCONNECT(frame_on_lastfile);
  WCONNECT(frame_on_play);
  WCONNECT(frame_on_record);
  WCONNECT(frame_on_stop);
  WCONNECT(frame_on_zoomin);
  WCONNECT(frame_on_zoomout);
  WCONNECT(frame_on_zoom0);
  WCONNECT(frame_on_preferences);
  WCONNECT(frame_on_about);
  WCONNECT(frame_on_demo_init);

  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      char szPath[100];
      sprintf(szPath,"unknown.%d.wav",i);
      me->apstrLRU[i]=g_string_new(szPath);
      // g_string_assign(apstrLRU[i],szPath);
    }
  g_string_assign(me->apstrLRU[0],"../testfiles/sax.wav");
  g_string_assign(me->apstrLRU[1],"1test.wav");
  g_string_assign(me->apstrLRU[2],"test2.wav");
  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      char szPath[100];
      sprintf(szPath,"lastfile%d",i+1);
      me->aMenuLRU[i]=glade_xml_get_widget(me->pGlade,szPath);
    }
  for (i=0; i<FRAME_LRU_NUM; i++)
    {
      GtkWidget *label=gtk_bin_get_child(GTK_BIN(me->aMenuLRU[i]));
      gtk_label_set_text(GTK_LABEL(label),me->apstrLRU[i]->str);
    }

  me->pVbox=gtk_vbox_new(FALSE,FALSE);

  gtk_container_add(GTK_CONTAINER(me->pwndTop),me->pVbox);
  gtk_box_pack_start(GTK_BOX(me->pVbox),me->pmenu,FALSE,FALSE,0);

  me->pWaveView  = (struct TWaveView*) malloc(sizeof(struct TWaveView));
  me->pStatusBar = (struct TStatusBar*)malloc(sizeof(struct TStatusBar));
  waveview_init(me->pWaveView,me,me->pVbox);
  statusbar_init(me->pStatusBar,me,me->pVbox);

  gtk_widget_show(me->pVbox);

  g_signal_connect(G_OBJECT(me->pwndTop), EVENT_TYPE_DELETE, G_CALLBACK(frame_procDelete),me);
  g_signal_connect(G_OBJECT(me->pwndTop), EVENT_TYPE_DELETE, G_CALLBACK(frame_procDelete),me);

  /* --- geometry and arrangement */
  /* gtk_window_parse_geometry       (GTK_WINDOW(me->pwndTop), szOptGeometry); */

  gtk_window_set_default_size(GTK_WINDOW(me->pwndTop),640,480);

  gtk_widget_show(me->pwndTop);
  /* gdk_window_move(me->pwndTop->window,10,40); */

  /* omitting this accelerator magic leaves the CTRL-* keys unused */
  me->pag=glade_xml_ensure_accel(me->pGlade);
  gtk_window_add_accel_group(GTK_WINDOW(me->pwndTop),me->pag);

  me->bDead=false;
}

// ======================================================================

void frame_destroy(struct TFrame *me)
{
  int i;
  me->bDead=true;
  waveview_destroy(me->pWaveView); free(me->pWaveView);
  statusbar_destroy(me->pStatusBar); free(me->pStatusBar);
  g_object_unref(me->pGlade);
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
		      && wave_is_valid(me->pApp->pWave)
		      && wave_get_sample_count(me->pApp->pWave));
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
  gtk_widget_set_sensitive(W("save"),bHaveFile);
  gtk_widget_set_sensitive(W("saveas"),bHaveFile);
  gtk_widget_set_sensitive(W("zoomin"),bHaveFile && waveview_can_zoom_in(me->pWaveView));
  gtk_widget_set_sensitive(W("zoomout"),bHaveFile && waveview_can_zoom_out(me->pWaveView));
  gtk_widget_set_sensitive(W("play"),bHaveFile && waveview_is_busy(me->pWaveView));
  gtk_widget_set_sensitive(W("record"),bHaveFile && waveview_is_busy(me->pWaveView));
  gtk_widget_set_sensitive(W("stop"),bHaveFile && !waveview_is_busy(me->pWaveView));
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

