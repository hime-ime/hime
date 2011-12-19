void p_err(char *fmt,...);
#if DEBUG
void __hime_dbg_(char *fmt,...);
#define dbg(...) __hime_dbg_(__VA_ARGS__)
#else
#define dbg(...) do {} while (0)
#endif
