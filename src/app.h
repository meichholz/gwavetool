#ifndef H_APP
#define H_APP

#include "base.h"

class TApp : public TBase {

 protected:
  class TFrame   *pFrame;
  class TWave    *pWave;
  class TOptions *pOptions;
  gboolean       bActive;

 public:

  /* constructors */
           TApp(int argc, char *argv[]);
  virtual ~TApp();
  class TWave  *Wave(void) { return pWave; }
  class TFrame *Frame(void) { return pFrame; }
  TResult Run();
  gboolean NewWaveFromFile(const gchar *szFile);

  gboolean CanClose(void);

  void    PollQueue(void);
  void    IdleTask(void);

};

#endif
