typedef void (*cb_selec_by_idx_t)(int);
typedef void (*cb_page_ud_t)();
void set_win1_cb(cb_selec_by_idx_t selc_by_idx, cb_page_ud_t cb_page_up, cb_page_ud_t cb_page_down);
