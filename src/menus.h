#ifndef H_MENUS
#define H_MENUS

#include "base.h"

struct TMenuTemplateItem {
  gchar *szName;
  gchar *szAccel;
  int   id;
  int   iDepth;
  int   iParent;
};

struct TMenuTemplate {
  gchar                    *szName;
  struct TMenuTemplateItem *pItems;
  int                       cItems;
};


class TMenu : public TBase {

 protected:

  GtkWidget    *pMenubar;

 public:

  /* constructors */
           TMenu(const gchar *szMenuID, class TFrame *pParent);
  virtual ~TMenu();

  GtkWidget *Menu(void) { return pMenubar; }

};

#endif
