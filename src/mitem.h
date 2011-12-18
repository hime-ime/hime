typedef struct {
  unich_t *name;
  char *stock_id;
  void (*cb)(GtkCheckMenuItem *checkmenuitem, gpointer dat);
  int *check_dat;
  GtkWidget *item;
  gulong handler;
} MITEM;
