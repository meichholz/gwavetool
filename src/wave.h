#ifndef H_WAVE
#define H_WAVE

#include "base.h"

#define WAVE_CHANNEL_LEFT   0
#define WAVE_CHANNEL_RIGHT  1

#define WAVE_FRAME_SAMPLES  1024

struct TWave {
  struct TApp *pApp;
  short    *pSamples;
  long      lcSamples;
  int       nRate;
  int       cChannels;
  gboolean  bDirty;
};

struct TWave *wave_new(struct TApp *pApp);
struct TWave *wave_new_from_file(struct TApp *pApp, const gchar *szFile);
void          wave_free(struct TWave *me);

TResult wave_create_samples(struct TWave *me);
TResult wave_get_frame(struct TWave *me, short *psSamples, long liFrame);
int     wave_get_frame_size(struct TWave *me);
long    wave_get_frame_count(struct TWave *me);

long    wave_get_sample_count(struct TWave *me);
int     wave_get_channel_count(struct TWave *me);
int     wave_get_sample_rate(struct TWave *me);
short   wave_peek_sample(struct TWave *me, int iChannel, long li);

gboolean wave_is_valid(struct TWave *me);
gboolean wave_is_dirty(struct TWave *me);
void     wave_set_dirty(struct TWave *me, gboolean b);

#endif
