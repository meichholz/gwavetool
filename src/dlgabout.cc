//
// dlgabout.cc
//

#include "dlgabout.h"
#include "app.h"
#include "frame.h"

gint aboutdialog_do_modal(struct TAboutDialog *me)
{
  gint rc=gtk_dialog_run(me->pdlg);
  if (rc==GTK_RESPONSE_ACCEPT) return TRUE;
  return FALSE;
}

void aboutdialog_init(struct TAboutDialog *me, struct TApp *papp)
{
  me->papp=papp;
  me->pdlg=GTK_DIALOG(gtk_dialog_new_with_buttons("About GWaveTool",
				   frame_window(papp->pFrame),
				   GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
				   GTK_STOCK_OK,
				   GTK_RESPONSE_ACCEPT,
				   NULL));
  gchar *buf= g_strdup_printf(_("\nGWaveTool %s\n\nCopyright (c) 2003 by Marian Eichholz\n\nReleased under the GNU General Public License\n\n"),::szVersion);
  GtkWidget *label = gtk_label_new(buf);
  g_free(buf);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
  gtk_widget_show(label);
  gtk_container_add(GTK_CONTAINER(me->pdlg->vbox),label);
}

void aboutdialog_destroy(struct TAboutDialog *me)
{
  gtk_widget_destroy(GTK_WIDGET(me->pdlg));
}
