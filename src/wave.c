#include "base.h"
#include "wave.h"
#include "app.h"
#include "frame.h"

#include <math.h>
#include <values.h>

#include <sndfile.h>

#define WAVE_RATE_CDAUDIO    44100

/* ----------------------------------------------------------------------
 * CreateSamples()
 * ---------------------------------------------------------------------- */

TResult wave_create_samples(struct TWave *me)
{
  long i;
  me->nRate        = 50;
  me->lcSamples    = 500;
  me->pSamples     = (short*)calloc(me->lcSamples, sizeof(short));
  me->cChannels    = 1;
  if (!me->pSamples) return rcNomem;

  me->bDirty=true;

  for (i=0; i<me->lcSamples; i++)
    {
      me->pSamples[i]=(short)(32767*sin(2.0*M_PI*i/me->nRate));
    }
  return rcOk;
}

/* ======================================================================
 * GetFrame()
 * reads a whole frame of me->cChannels * WAVE_FRAME_SAMPLES shorts
 * ====================================================================== */

TResult wave_get_frame(struct TWave *me, short *psSamples, long liFrame)
{
  if (!me->pSamples)
    {
      debug_printf(DEBUG_WAVE,"GetFrame() with no samples");
      return rcBounds;
    }
  long li=me->cChannels*WAVE_FRAME_SAMPLES*liFrame;
  if (li<0 || li>me->lcSamples*me->cChannels)
    {
      debug_printf(DEBUG_WAVE,"GetFrame() out of bounds: li=%ld(%ld)",li,me->lcSamples);
      return rcBounds;
    }
  long lcToMove=WAVE_FRAME_SAMPLES*me->cChannels; /* in shorts!!! */
  memset(psSamples,0,lcToMove); /* trailing bytes are initialised */
  if (li+lcToMove>me->lcSamples*me->cChannels)
    lcToMove=me->lcSamples*me->cChannels-li;
  debug_printf(DEBUG_WAVE,"giving frame %ld size %ld at %lx",liFrame,lcToMove,li);
  memcpy(psSamples,me->pSamples+li,lcToMove*sizeof(short));
  return rcOk;
}

/* ======================================================================
 * PeekSample()
 * ====================================================================== */

short   wave_peek_sample(struct TWave *me, int iChannel, long li)
{
  if (!me->pSamples)
    {
      debug_printf(DEBUG_WAVE,"peek with no samples");
      return 0;
    }
  if (iChannel>=me->cChannels || iChannel<0 || li>=me->lcSamples || li<0)
    {
      debug_printf(DEBUG_WAVE,"bounds: ch:%d, i:%ld(%ld)",iChannel,li,me->lcSamples);
      return 0;
    }
  return me->pSamples ? me->pSamples[me->cChannels*li+iChannel] : 0;
}

/* ======================================================================
 * Constructor stuff
 * ====================================================================== */

struct TWave *wave_new(struct TApp *pApp)
{
  struct TWave *me=(struct TWave *)malloc(sizeof(struct TWave));
  me->pSamples=NULL; me->bDirty=false;
  me->pApp=pApp;
  return me;
}

/* ======================================================================
 * Construction from WAV (e.g.) file
 * return NULL on errors
 * ====================================================================== */

struct TWave *wave_new_from_file(struct TApp *pApp, const gchar *szFile)
{
  SF_INFO sfi;
  struct TWave *me=wave_new(pApp);
  memset(&sfi,0,sizeof(sfi));
  sfi.format=0; /* just for the dumb */
  SNDFILE *file=sf_open(szFile,SFM_READ,&sfi);
  if (file==NULL)
    {
      g_print("SNDFILE open error\n");
      free(me);
      return NULL; /* object keeps being !IsValid() */
    }
  me->lcSamples=(long)sfi.frames;
  me->nRate=sfi.samplerate;
  me->cChannels=sfi.channels;
  long lcTotalSamples=me->lcSamples*me->cChannels;
  me->pSamples=(short*)calloc(lcTotalSamples,sizeof(short));
  if (!me->pSamples) {
    frame_message_error(me->pApp->pFrame,"not enough virtual memory");
    me->lcSamples=0;
    free(me);
    return NULL;
  }
  long lcSamplesLeft=lcTotalSamples;
  short *pNext=me->pSamples;
  while (lcSamplesLeft>0)
    {
      app_poll_queue(me->pApp);
      long lcSamplesInChunk=lcSamplesLeft;
      if (lcSamplesInChunk>WAVE_FRAME_SAMPLES*me->cChannels)
	lcSamplesInChunk=WAVE_FRAME_SAMPLES*me->cChannels;
      lcSamplesLeft-=lcSamplesInChunk;
      if (lcSamplesInChunk!=sf_read_short(file,pNext,lcSamplesInChunk))
	{
	  debug_printf(DEBUG_WAVE,"read error: %s\n",sf_strerror(file));
	  me->lcSamples=0;
	  free(me->pSamples);
	  me->pSamples=NULL;
	  sf_close(file);
	  free(me);
	  return NULL;
	}
      pNext+=lcSamplesInChunk;
    }
  sf_close(file);
  return me;
}

void wave_free(struct TWave *me)
{
  if (me)
    {
      if (me->pSamples)
	free(me->pSamples);
      free(me);
    }
}

/* **********************************************************************
 * Interface functions
 * ********************************************************************** */

int     wave_get_frame_size(struct TWave *me)
  { return WAVE_FRAME_SAMPLES*me->cChannels; }
long    wave_get_frame_count(struct TWave *me)
 { return (WAVE_FRAME_SAMPLES-1+me->lcSamples)/WAVE_FRAME_SAMPLES; }

long    wave_get_sample_count(struct TWave *me)
  { return me->lcSamples; }
int     wave_get_channel_count(struct TWave *me)
  { return me->cChannels; }
int     wave_get_sample_rate(struct TWave *me)
  { return me->nRate; }

gboolean wave_is_valid(struct TWave *me)
  { return (me!=NULL && me->pSamples!=NULL) ? true : false ; }
gboolean wave_is_dirty(struct TWave *me)
  { return me->bDirty; }
void     wave_set_dirty(struct TWave *me, gboolean b)
  { me->bDirty=b; }
