#ifndef H_DIALOG_ABOUT
#define H_DIALOG_ABOUT

#include "base.h"
#include "app.h"

struct TAboutDialog {
  GtkDialog   *pdlg;
  struct TApp *papp;
};

void aboutdialog_init(struct TAboutDialog *me, struct TApp *papp);
void aboutdialog_destroy(struct TAboutDialog *me);
gint aboutdialog_do_modal(struct TAboutDialog *me);

#endif

