#ifndef H_FRAME
#define H_FRAME

#include "base.h"

#include "gwavetool.h"

#define FRAME_LRU_NUM 5

struct TFrame {

  struct TApp       *pApp;
  struct TWaveView  *pWaveView;
  struct TStatusBar *pStatusBar;

  GtkWidget      *pwndTop;
  GtkWidget      *pVbox;
  GtkWidget      *pmenu;
  GtkAccelGroup  *pag;

  gboolean        bDead;
  GladeXML       *pGlade;

  GString        *pstrLastFile,*apstrLRU[FRAME_LRU_NUM];
  GtkWidget      *aMenuLRU[FRAME_LRU_NUM];

};

void         frame_init(struct TFrame *me, struct TApp *papp);
void         frame_destroy(struct TFrame *me);

/* event handlers */
void     frame_repaint(struct TFrame *me);
TResult  frame_activate_LRU(struct TFrame *me, int iLRUfile);

GtkWindow*   frame_window(struct TFrame *me);

/* Helpers for message boxes */
gint         frame_message_box(struct TFrame *me,
			       GtkMessageType idType,
			       GtkButtonsType idButtons,
			       const char *szContent, ...);
void         frame_message_error(struct TFrame *me, const char *szContent, ...);
gboolean     frame_message_confirm(struct TFrame *me, const char *szContent, ...);
void         frame_message_notimplemented(struct TFrame *me);

void         frame_sync_state(struct TFrame *me);
  
const gchar  *frame_get_last_filename(struct TFrame *me);
void         frame_set_last_filename(struct TFrame *me, const gchar *szFile);
void         frame_set_busy_cursor(struct TFrame *me, gboolean bBusy);
void         frame_idle_task(struct TFrame *me);

#endif
