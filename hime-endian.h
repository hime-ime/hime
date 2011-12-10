#define swap_ch(a, b) do { char t; t = *(a); *(a) = *(b); *(b) = t; } while (0)

#define swap_byte_2(pp) do { char *p=(char *)pp;  swap_ch(p, p+1); } while (0)
#define swap_byte_4(pp) do { char *p=(char *)pp;  swap_ch(p, p+3); swap_ch(p+1, p+2); } while (0)
#define swap_byte_8(pp) do { char *p=(char *)pp;  swap_ch(p, p+7); swap_ch(p+1, p+6); swap_ch(p+2, p+5); swap_ch(p+3, p+4);} while (0)

#if __BYTE_ORDER == __BIG_ENDIAN
//#warning "big endian"
#define to_hime_endian_2(pp) swap_byte_2(pp)
#define to_hime_endian_4(pp) swap_byte_4(pp)
#define to_hime_endian_8(pp) swap_byte_8(pp)
#else
#define to_hime_endian_2(pp) do { } while (0)
#define to_hime_endian_4(pp) do { } while (0)
#define to_hime_endian_8(pp) do { } while (0)
#endif
