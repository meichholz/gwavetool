//
// dlgabout.cc
//

#include "dlgabout.h"
#include "app.h"
#include "frame.h"

gint TAboutDialog::DoModal(gint idDefault)
{
  gint rc=gtk_dialog_run(pdlg);
  if (rc==GTK_RESPONSE_ACCEPT) return TRUE;
  return FALSE;
}

TAboutDialog::TAboutDialog(class TApp *papp) : TBase(papp)
{
  pdlg=GTK_DIALOG(gtk_dialog_new_with_buttons("About GWaveTool",
				   pApp->pFrame->Window(),
				   GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
				   GTK_STOCK_OK,
				   GTK_RESPONSE_ACCEPT,
				   NULL));
  gchar *buf= g_strdup_printf(_("\nGWaveTool %s\n\nCopyright (c) 2003 by Marian Eichholz\n\nReleased under the GNU General Public License\n\n"),::szVersion);
  GtkWidget *label = gtk_label_new(buf);
  g_free(buf);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
  gtk_widget_show(label);
  gtk_container_add(GTK_CONTAINER(pdlg->vbox),label);

}

TAboutDialog::~TAboutDialog()
{
  gtk_widget_destroy(GTK_WIDGET(pdlg));
}
