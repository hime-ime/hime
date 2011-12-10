typedef enum {
  Connection_type_unix = 1,
  Connection_type_tcp = 2
} Connection_type;

typedef struct {
  ClientState *cs;
  int tag;
  u_int seed;
  Connection_type type;
#if	UNIX
  int fd;
#else
  HANDLE fd;
#endif
} HIME_ENT;

extern HIME_ENT *hime_clients;
extern int hime_clientsN;
extern Server_IP_port srv_ip_port;
