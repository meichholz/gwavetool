/*
  app.cc

  giving some application glue for interconnecting the various parts, such as
  - preferences
  - windows
  - file stuff

*/

#include "app.h"
#include "frame.h"
#include "wave.h"
#include "options.h"

//
// Constructor/Destructor
//

void app_init(struct TApp *me,int argc, char *argv[])
{
  g_thread_init(NULL);
  gdk_threads_init();
  gtk_init(&argc,&argv);
  me->pWave = new TWave(me);
  me->pFrame= (struct TFrame *)malloc(sizeof(TFrame)); frame_init(me->pFrame,me);
  me->pOptions= new TOptions(me);
}

void app_destroy(struct TApp *me)
{
  debug_printf(DEBUG_FRAMEWORK,"deleting pFrame.");
  frame_destroy(me->pFrame); free(me->pFrame);
  debug_printf(DEBUG_FRAMEWORK,"deleting pWave.");
  delete me->pWave;
  debug_printf(DEBUG_FRAMEWORK,"deleting pOptions.");
  delete me->pOptions;
}

// ----------------------------------------------------------------------
// Run()
// ----------------------------------------------------------------------

void app_poll_queue(struct TApp *me)
{
  while (me->bActive and gtk_events_pending ())
    if (gtk_main_iteration ())
      me->bActive=false;
}

static gboolean _idle_proc(void *obj)
{
  app_idle_task((struct TApp *)obj);
  return false;
}

void app_idle_task(struct TApp *me)
{
  frame_idle_task(me->pFrame);
}

TResult app_run(struct TApp *me)
{
  me->bActive=true;
  frame_sync_state(me->pFrame);
  gdk_threads_enter();
  int idIdle=gtk_idle_add(_idle_proc,me);
  gtk_main();
  gtk_idle_remove(idIdle);
  gdk_threads_leave();
  debug_printf(DEBUG_FRAMEWORK,"existing gtk_main()");
  return rcOk;
}

/* ======================================================================
 * NewWaveFromFile(szFile)
 * ====================================================================== */

gboolean app_new_wave_from_file(struct TApp *me, const gchar *szFile)
{
  delete me->pWave;
  me->pWave=new TWave(me,szFile);
  return me->pWave->IsValid();
}


/* ======================================================================
 * CanClose()
 * ====================================================================== */

gboolean app_can_close(struct TApp *me)
{
  if (me->pWave && me->pWave->IsValid())
     return ! me->pWave->IsDirty();
  return true;
}
