#include "menus.h"
#include "frame.h"


#include "gwavetool.h"
#define ID_SEPARATOR -1

#include "menus.hc"

static struct TMenuTemplate amtDirectory[] = {
  { "main_menu", amiMainMenu, sizeof(amiMainMenu)/sizeof(TMenuTemplateItem) },
  { NULL, NULL, 0 } };

// ======================================================================
// Constructor stuff
// ======================================================================

TMenu::TMenu(const gchar *szMenuID, class TFrame *pParent) : TBase(pParent->App())
{
}

TMenu::~TMenu()
{
}
