#ifndef H_WAVE
#define H_WAVE

#include "base.h"

#define WAVE_CHANNEL_LEFT   0
#define WAVE_CHANNEL_RIGHT  1

#define WAVE_FRAME_SAMPLES  1024

class TWave : public TBase {

 protected:
  short    *pSamples;
  long      lcSamples;
  int       nRate;
  int       cChannels;
  gboolean  bDirty;

 public:

  /* constructors */
           TWave(class TApp *papp);
           TWave(class TApp *papp,const gchar *szFile);
  virtual ~TWave();

  TResult  CreateSamples(void);

  int      GetFrameSize(void) { return WAVE_FRAME_SAMPLES*cChannels; }
  long     GetFrameCount(void) { return (WAVE_FRAME_SAMPLES-1+lcSamples)/WAVE_FRAME_SAMPLES; }

  long     GetSampleCount(void) { return lcSamples; }
  int      GetChannelCount(void) { return cChannels; }
  int      GetSampleRate(void) { return nRate; }
  short    PeekSample(int iChannel, long li);
  TResult  GetFrame(short *psSamples, long liFrame);

  gboolean IsValid(void) { return (pSamples!=NULL) ? true : false ; }
  gboolean IsDirty(void) { return bDirty; }
  void     SetDirty(gboolean b) { bDirty=b; }
};

#endif
