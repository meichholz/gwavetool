#include "base.h"
#include "wave.h"
#include "app.h"
#include "frame.h"

#include <math.h>
#include <values.h>

#include <sndfile.h>

#define WAVE_RATE_CDAUDIO    44100

// ----------------------------------------------------------------------
// CreateSamples()
// ----------------------------------------------------------------------

TResult TWave::CreateSamples(void) /* demo function */
{
  long i;
  nRate        = 50;
  lcSamples    = 500;
  pSamples     = new short[lcSamples];
  cChannels    = 1;
  if (!pSamples) return rcNomem;

  bDirty=true;

  for (i=0; i<lcSamples; i++)
    {
      pSamples[i]=short(32767*sin(2.0*M_PI*i/nRate));
    }
  return rcOk;
}

/* ======================================================================
 * GetFrame()
 * reads a whole frame of cChannels * WAVE_FRAME_SAMPLES shorts
 * ====================================================================== */

TResult TWave::GetFrame(short *pBuffer, long liFrame)
{
  if (!pSamples)
    {
      debug_printf(DEBUG_WAVE,"GetFrame() with no samples");
      return rcBounds;
    }
  long li=cChannels*WAVE_FRAME_SAMPLES*liFrame;
  if (li<0 || li>lcSamples*cChannels)
    {
      debug_printf(DEBUG_WAVE,"GetFrame() out of bounds: li=%ld(%ld)",li,lcSamples);
      return rcBounds;
    }
  long lcToMove=WAVE_FRAME_SAMPLES*cChannels; // in shorts!!!
  memset(pBuffer,0,lcToMove); /* trailing bytes are initialised */
  if (li+lcToMove>lcSamples*cChannels)
    lcToMove=lcSamples*cChannels-li;
  debug_printf(DEBUG_WAVE,"giving frame %ld size %ld at %lx",liFrame,lcToMove,li);
  memcpy(pBuffer,pSamples+li,lcToMove*sizeof(short));
  return rcOk;
}

/* ======================================================================
 * PeekSample()
 * ====================================================================== */

short TWave::PeekSample(int iChannel, long li)
{
  if (!pSamples)
    {
      debug_printf(DEBUG_WAVE,"peek with no samples");
      return 0;
    }
  if (iChannel>=cChannels || iChannel<0 || li>=lcSamples || li<0)
    {
      debug_printf(DEBUG_WAVE,"bounds: ch:%d, i:%ld(%ld)",iChannel,li,lcSamples);
      return 0;
    }
  return pSamples ? pSamples[cChannels*li+iChannel] : 0;
}

// ======================================================================
// Constructor stuff
// ======================================================================

TWave::TWave(class TApp *papp) : TBase(papp)
{
  pSamples=NULL; bDirty=false;
}

/* ======================================================================
 * Construction from WAV (e.g.) file
 * ====================================================================== */

TWave::TWave(class TApp *papp, const gchar *szFile) : TBase(papp)
{
  SF_INFO sfi;
  pSamples=NULL; bDirty=false;
  memset(&sfi,0,sizeof(sfi));
  sfi.format=0; // just for the dumb
  SNDFILE *file=sf_open(szFile,SFM_READ,&sfi);
  if (file==NULL)
    {
      g_print("SNDFILE open error\n");
      return; // object keeps being !IsValid()
    }
  lcSamples=(long)sfi.frames;
  nRate=sfi.samplerate;
  cChannels=sfi.channels;
  long lcTotalSamples=lcSamples*cChannels;
  pSamples=new short[lcTotalSamples];
  if (!pSamples) {
    App()->Frame()->MessageError("not enough virtual memory");
    lcSamples=0;
    return;
  }
  long lcSamplesLeft=lcTotalSamples;
  short *pNext=pSamples;
  while (lcSamplesLeft>0)
    {
      App()->PollQueue();
      long lcSamplesInChunk=lcSamplesLeft;
      if (lcSamplesInChunk>WAVE_FRAME_SAMPLES*cChannels)
	lcSamplesInChunk=WAVE_FRAME_SAMPLES*cChannels;
      lcSamplesLeft-=lcSamplesInChunk;
      if (lcSamplesInChunk!=sf_read_short(file,pNext,lcSamplesInChunk))
	{
	  debug_printf(DEBUG_WAVE,"read error: %s\n",sf_strerror(file));
	  lcSamples=0;
	  delete [] pSamples;
	  pSamples=NULL;
	  sf_close(file);
	  return;
	}
      pNext+=lcSamplesInChunk;
    }
  sf_close(file);
}

TWave::~TWave()
{
  delete [] pSamples;
}
