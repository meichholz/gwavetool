#ifndef H_FRAME
#define H_FRAME

#include "base.h"

#include "gwavetool.h"

#define FRAME_LRU_NUM 5

class TFrame : public TBase {

 public:

  class TWaveView  *pWaveView;
  class TStatusBar *pStatusBar;
  class TMenu      *pMainMenu;

 private:
  GtkItemFactory *pmif;
  GtkAccelGroup  *pag;
  GtkWidget      *pwndTop;
  GtkWidget      *pMenu;
  GtkWidget      *pVbox;

  gboolean        bDead;

  GString        *pstrLastFile,*apstrLRU[FRAME_LRU_NUM];
  GtkWidget      *aMenuLRU[FRAME_LRU_NUM];

  /* menu handlers */
  void         OnMenuLoad(void);
  void         OnMenuSave(void);
  void         OnMenuSaveAs(void);
  void         OnMenuNew(void);

 public:
  /* constructors */
           TFrame(class TApp *papp);
  virtual ~TFrame();

  /* event handlers */
  virtual void     Repaint(void);
  virtual void     OnDelete(GdkEventAny *pev);
  virtual gboolean OnMenu(guint idMenu);
 
  TResult      ActivateLRU(int iLRUfile);

  GtkWindow*   Window(void) { return GTK_WINDOW(pwndTop); };

  // Helpers for message boxes
  gint         MessageBox(GtkMessageType idType, GtkButtonsType idButtons,
			  const char *szContent, ...);
  void         MessageError(const char *szContent, ...);
  gboolean     MessageConfirm(const char *szContent, ...);
  void         MessageNotimplemented(void);

  void         SyncState(void);
  
  const gchar  *GetLastFilename(void);
  void         SetLastFilename(const gchar *szFile);
  void         SetBusyCursor(gboolean bBusy);
  void         IdleTask(void);
};

#endif
