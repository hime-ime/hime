/* To get GETTEXT_PACKAGE */
#include "../config.h"

#include <libintl.h>

/* TODO: Should support build-time configuration */
#define GTK_LOCALEDIR "/usr/share/locale"

/* TODO: Should support #if HIME_i18n_message here */
#define _(String) dgettext(GETTEXT_PACKAGE,String)
#define N_(String) (String)
