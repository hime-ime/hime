#include "hime-qt.h"

#ifdef QT4
using namespace Qt;
#endif

/* Static variables */
static HIMEQt *client = NULL;


/* Bindings */
void hime_client_messenger_opened ()
{
    client->messenger_opened ();
}


void hime_client_messenger_closed ()
{
    client->messenger_closed ();
}


/* Implementations */
HIMEQt::HIMEQt (): socket_notifier (NULL) {
    client = this;
}


HIMEQt::~HIMEQt () {
    client = NULL;
}


void HIMEQt::messenger_opened ()
{
}


void HIMEQt::messenger_closed ()
{
}


void HIMEQt::handle_message ()
{
}

