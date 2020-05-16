void p_err (char *fmt, ...);

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
void __hime_dbg_ (char *fmt, ...);
#define dbg(fmt, ...) __hime_dbg_ ("%s:%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define dbg(...) \
    do {         \
    } while (0)
#endif
