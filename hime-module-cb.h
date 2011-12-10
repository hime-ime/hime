#ifdef __cplusplus
extern "C" {
#endif
  int module_init_win(HIME_module_main_functions *funcs);
  void module_win_geom();
  int module_reset();
  int module_get_preedit(char *str, HIME_PREEDIT_ATTR attr[], int *pcursor, int *compose_flag);
  gboolean module_feedkey(int kv, int kvstate);
  int module_feedkey_release(KeySym xkey, int kbstate);
  void module_move_win(int x, int y);
  void module_change_font_size();
  void module_show_win();
  void module_hide_win();
  int module_win_visible();
  int module_flush_input();
#ifdef __cplusplus
}
#endif

///////// for hime main() only
typedef struct _HIME_module_callback_functions {
  int (*module_init_win)(HIME_module_main_functions *funcs);
  void (*module_get_win_geom)();
  int (*module_reset)();
  int (*module_get_preedit)(char *str, HIME_PREEDIT_ATTR attr[], int *pcursor, int *compose_flag);
  gboolean (*module_feedkey)(int kv, int kvstate);
  int (*module_feedkey_release)(KeySym xkey, int kbstate);
  void (*module_move_win)(int x, int y);
  void (*module_change_font_size)();
  void (*module_show_win)();
  void (*module_hide_win)();
  int (*module_win_visible)();
  int (*module_flush_input)();
  void (*module_setup_window_create)();
} HIME_module_callback_functions;
