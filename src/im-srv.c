/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation version 2.1
 * of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <string.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include "hime.h"

#include "im-srv.h"

Server_IP_port srv_ip_port;

// initialized in setup_unix_domain_socket()
static int im_sockfd;

// initialized in setup_tcp_socket()
static int im_tcp_sockfd;

static Window xim_xwin;

static gboolean cb_read_hime_client_data (GIOChannel *source,
                                          GIOCondition condition,
                                          gpointer data) {

    const int fd = GPOINTER_TO_INT (data);

    process_client_req (fd);

    return TRUE;
}

static void gen_passwd_idx (void) {
    srv_ip_port.passwd.seed = (rand () >> 1) % __HIME_PASSWD_N_;
    to_hime_endian_4 (&srv_ip_port.passwd.seed);

    const Server_IP_port new_srv_ip_port = srv_ip_port;

    // update srv_ip_port to addr_atom X property
    Display *display = GDK_DISPLAY ();
    Atom addr_atom = get_hime_addr_atom (display);
    XChangeProperty (display,
                     xim_xwin,
                     addr_atom,
                     XA_STRING,
                     8,  // 8-bit, we are passing a char pointer
                     PropModeReplace,
                     (unsigned char *) &new_srv_ip_port,
                     sizeof (srv_ip_port));
    XSync (display, FALSE);
}

static int accept_sockfd (const Connection_type type) {
    if (type == Connection_type_unix) {
        // passing NULL to accept(3) means that we don't care the address info
        // of the connecting socket
        return accept (im_sockfd, NULL, NULL);
    }

    // otherwise, type == Connection_type_tcp
    return accept (im_tcp_sockfd, NULL, NULL);
}

static gboolean cb_new_hime_client (GIOChannel *source,
                                    GIOCondition condition,
                                    gpointer data) {

    const Connection_type type = (Connection_type) GPOINTER_TO_INT (data);

    const int newsockfd = accept_sockfd (type);
    if (newsockfd < 0) {
        perror ("Failed to accept socket fd");
        return FALSE;
    }

    if (newsockfd >= hime_clientsN - 1) {
        const int prev_hime_clientsN = hime_clientsN;
        hime_clientsN = newsockfd + 1;
        hime_clients = trealloc (hime_clients, HIME_ENT, hime_clientsN);

        // Initialize clientstate in useless hime_clients for recognition
        for (int c = prev_hime_clientsN; c < hime_clientsN; c++) {
            hime_clients[c].cs = NULL;
        }
    }

    memset (&hime_clients[newsockfd], 0, sizeof (hime_clients[0]));

    hime_clients[newsockfd].tag = g_io_add_watch (
        g_io_channel_unix_new (newsockfd),
        G_IO_IN,
        cb_read_hime_client_data,
        GINT_TO_POINTER (newsockfd));

    if (type == Connection_type_tcp) {
        hime_clients[newsockfd].seed = srv_ip_port.passwd.seed;
        gen_passwd_idx ();
    }

    hime_clients[newsockfd].type = type;

    return TRUE;
}

static void init_unix_socket (struct sockaddr_un *serv_addr,
                              const char *sock_path) {
    // initialize the unix domain socket structure with hime's socket path

    memset (serv_addr, 0, sizeof (*serv_addr));

    serv_addr->sun_family = AF_UNIX;
    strncpy (serv_addr->sun_path, sock_path, sizeof (serv_addr->sun_path));

    dbg ("-- %s\n", serv_addr->sun_path);
}

static void unlink_serv_addr_sock_path (const struct sockaddr_un *serv_addr) {
    // unlink (remove) the old socket path if exists

    struct stat st;
    if (!stat (serv_addr->sun_path, &st)) {
        // serv_addr->sun_path exists, try to unlink it
        if (unlink (serv_addr->sun_path) < 0) {
            char buf[UNIX_PATH_MAX + 128];
            snprintf (buf, sizeof (buf), "unlink error %s", serv_addr->sun_path);
            perror (buf);
        }
    }
}

static void setup_xproperty (Display *display, const char *sock_path) {
    // update sock_path to sockpath_atom X property

    Server_sock_path srv_sockpath;
    strncpy (srv_sockpath.sock_path, sock_path, sizeof (srv_sockpath.sock_path));

    Atom sockpath_atom = get_hime_sockpath_atom (display);
    XChangeProperty (display,
                     xim_xwin,
                     sockpath_atom,
                     XA_STRING,
                     8,  // 8-bit, we are passing a char pointer
                     PropModeReplace,
                     (unsigned char *) &srv_sockpath,
                     sizeof (srv_sockpath));
}

static void setup_xselection (Display *display) {
    // set xim_xwin to be the selection (clipboard) owner of addr_atom
    Atom addr_atom = get_hime_addr_atom (display);
    XSetSelectionOwner (display, addr_atom, xim_xwin, CurrentTime);
}

static void setup_unix_domain_socket (void) {
    // setup the HIME unix domain socket on get_hime_im_srv_sock_path()
    //
    // invoke cb_new_hime_client callback function when there are data to read
    // from im_sockfd.

    char sock_path[UNIX_PATH_MAX];
    get_hime_im_srv_sock_path (sock_path, sizeof (sock_path));

    struct sockaddr_un serv_addr;
    init_unix_socket (&serv_addr, sock_path);
    unlink_serv_addr_sock_path (&serv_addr);

    if ((im_sockfd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror ("cannot create unix socket");
        exit (-1);
    }

    if (bind (im_sockfd, (struct sockaddr *) &serv_addr, SUN_LEN (&serv_addr)) < 0) {
        perror ("Failed to bind unix socket on hime socket path");
        exit (-1);
    }

    // size of the connection queue == 2
    if (listen (im_sockfd, 2) < 0) {
        perror ("Failed to listen to im_sockfd");
        exit (1);
    }

    dbg ("im_sockfd:%d\n", im_sockfd);

    g_io_add_watch (g_io_channel_unix_new (im_sockfd),
                    G_IO_IN,
                    cb_new_hime_client,
                    GINT_TO_POINTER (Connection_type_unix));

    Display *display = GDK_DISPLAY ();
    setup_xproperty (display, sock_path);
    setup_xselection (display);
}

static void get_ip_address (uint32_t *ip) {

    struct ifaddrs *ifaddr = NULL;
    if (getifaddrs (&ifaddr) == -1) {
        perror ("Failed to retrieve network interface information with getifaddrs");
        exit (EXIT_FAILURE);
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }

        // IPv4 interfaces
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *peer_addr = (struct sockaddr_in *) ifa->ifa_addr;
            const char *ipaddr = inet_ntoa (peer_addr->sin_addr);
            if (!strcmp (ipaddr, "127.0.0.1")) {
                continue;
            }
            dbg ("ip addr %s\n", ipaddr);
            memcpy (ip, &peer_addr->sin_addr.s_addr, INET_ADDRSTRLEN);
            break;
        }
    }

    freeifaddrs (ifaddr);
}

static void setup_hime_passwd (void) {
    srand (time (NULL));
    for (int i = 0; i < __HIME_PASSWD_N_; i++) {
        srv_ip_port.passwd.passwd[i] = (rand () >> 2) & 0xff;
    }
}

static void init_tcp_socket (struct sockaddr_in *serv_addr_tcp) {
    // initialize the tcp socket structure

    memset (serv_addr_tcp, 0, sizeof (*serv_addr_tcp));

    serv_addr_tcp->sin_family = AF_INET;
    serv_addr_tcp->sin_addr.s_addr = htonl (INADDR_ANY);
}

static void bind_tcp_socket (struct sockaddr_in *serv_addr_tcp) {
    // attempt to bind the socket on port 9999~19999

    unsigned short port = 9999;
    for (; port < 20000; port++) {
        serv_addr_tcp->sin_port = htons (port);
        if (bind (im_tcp_sockfd,
                  (struct sockaddr *) serv_addr_tcp,
                  sizeof (*serv_addr_tcp)) == 0) {
            break;
        }
    }
}

static void setup_tcp_socket (void) {
    // setup the HIME tcp socket for remote client
    //
    // invoke cb_new_hime_client callback function when there are data to read
    // from im_tcp_sockfd

    struct sockaddr_in serv_addr_tcp;
    init_tcp_socket (&serv_addr_tcp);

    if ((im_tcp_sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("cannot create tcp socket");
        exit (-1);
    }

    bind_tcp_socket (&serv_addr_tcp);

    memset (&srv_ip_port, 0, sizeof (srv_ip_port));
    get_ip_address (&srv_ip_port.ip);
    srv_ip_port.port = serv_addr_tcp.sin_port;

    dbg ("server port bind to %s:%d\n",
         inet_ntoa (serv_addr_tcp.sin_addr),
         serv_addr_tcp.sin_port);

    setup_hime_passwd ();

    // size of the connection queue == 5
    if (listen (im_tcp_sockfd, 5) < 0) {
        perror ("Failed to listen to im_tcp_sockfd");
        exit (1);
    }

    dbg ("after listen:%d\n", im_tcp_sockfd);

    gen_passwd_idx ();

    g_io_add_watch (g_io_channel_unix_new (im_tcp_sockfd),
                    G_IO_IN,
                    cb_new_hime_client,
                    GINT_TO_POINTER (Connection_type_tcp));
}

void init_hime_im_serv (const Window window) {
    dbg ("init_hime_im_serv\n");

    xim_xwin = window;

    setup_unix_domain_socket ();

    if (!hime_remote_client) {
        dbg ("connection via TCP is disabled\n");
        return;
    }

    setup_tcp_socket ();
}
