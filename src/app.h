#ifndef H_APP
#define H_APP

#include "base.h"

struct TApp {
  struct TFrame  *pFrame;
  struct TWave   *pWave;
  gboolean       bActive;
};

void     app_init (struct TApp *pApp, int argc, char *argv[]);
void     app_destroy(struct TApp *pApp);
TResult  app_run(struct TApp *pApp);
gboolean app_new_wave_from_file(struct TApp *pApp, const gchar *szFile);
gboolean app_can_close(struct TApp *pApp);
void     app_poll_queue(struct TApp *pApp);
void     app_idle_task(struct TApp *pApp);

#endif
