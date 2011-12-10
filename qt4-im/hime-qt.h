#include <QObject>
#include <QSocketNotifier>

//#include "hime-imcontext-qt.h"
#include "hime-common-qt.h"

class HIMEQt: public QObject
{

    Q_OBJECT

    public slots:

        void handle_message ();

    public:

        /**
         * Constructor.
         */
        HIMEQt ();


        /**
         * Destructor.
         */
        ~HIMEQt ();


        /**
         * A messenger is opened.
         */
        void messenger_opened ();

        /**
         * A messenger is closed.
         */
        void messenger_closed ();


    private:


        /**
         * The notifier for the messenger socket.
         */
        QSocketNotifier *socket_notifier;

};
