#ifndef H_DIALOG_ABOUT
#define H_DIALOG_ABOUT

#include "base.h"

class TAboutDialog : public TBase {

 protected:
  GtkDialog *pdlg;
  
 public:
  /* constructors */
           TAboutDialog(class TApp *papp);
  virtual ~TAboutDialog();
  virtual  gint DoModal(gint idDefault=-1);
};

#endif

