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

TApp::TApp(int argc, char *argv[]) : TBase(this)
{
  g_thread_init(NULL);
  gdk_threads_init();
  gtk_init(&argc,&argv);
  
  pWave = new TWave(this);
  pFrame= new TFrame(this);
  pOptions= new TOptions(this);
}

TApp::~TApp()
{
  debug_printf(DEBUG_FRAMEWORK,"deleting pFrame.");
  delete pFrame;
  debug_printf(DEBUG_FRAMEWORK,"deleting pWave.");
  delete pWave;
  debug_printf(DEBUG_FRAMEWORK,"deleting pOptions.");
  delete pOptions;
}

// ----------------------------------------------------------------------
// Run()
// ----------------------------------------------------------------------

void TApp::PollQueue(void) 
{
  while (bActive and gtk_events_pending ())
    if (gtk_main_iteration ())
      bActive=false;
}

static gboolean idle_proc(void *obj)
{
  ((TApp*)obj)->IdleTask();
  return false;
}

void TApp::IdleTask(void)
{
  pFrame->IdleTask();
}

TResult TApp::Run(void)
{
  bActive=true;
  pFrame->SyncState();
  gdk_threads_enter();
  int idIdle=gtk_idle_add(idle_proc,this);
  gtk_main();
  gtk_idle_remove(idIdle);
  gdk_threads_leave();
  debug_printf(DEBUG_FRAMEWORK,"existing gtk_main()");
  return rcOk;
}

/* ======================================================================
 * NewWaveFromFile(szFile)
 * ====================================================================== */

gboolean TApp::NewWaveFromFile(const gchar *szFile)
{
  delete pWave;
  pWave=new TWave(this,szFile);
  return pWave->IsValid();
}


/* ======================================================================
 * CanClose()
 * ====================================================================== */

gboolean TApp::CanClose()
{
  if (Wave() && Wave()->IsValid()) return ! Wave()->IsDirty();
  return true;
}
