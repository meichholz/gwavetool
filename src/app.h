#ifndef H_APP
#define H_APP

#include "base.h"

struct TApp {
  class TFrame   *pFrame;
  class TWave    *pWave;
  class TOptions *pOptions;
  gboolean       bActive;
};

void     app_init (struct TApp *papp, int argc, char *argv[]);
void     app_destroy(struct TApp *papp);
TResult  app_run(struct TApp *papp);
gboolean app_new_wave_from_file(struct TApp *papp, const gchar *szFile);
gboolean app_can_close(struct TApp *papp);
void     app_poll_queue(struct TApp *papp);
void     app_idle_task(struct TApp *papp);

#endif
