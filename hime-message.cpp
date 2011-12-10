#include "hime.h"

void print_help()
{
  p_err("usage: hime-message -icon file_name -text string -duration milli_seconds\n");
}

int main(int argc, char **argv)
{
  int i;
  char text[128];
  char icon[128];
  int duration = 3000;

  gtk_init(&argc, &argv);

  if (argc < 3)
    print_help();

  strcpy(text, "-");
  strcpy(icon, "-");

  for(i=1; i < argc; i+=2) {
    if (!strcmp(argv[i], "-icon")) {
      strcpy(icon, argv[i+1]);
    } else
    if (!strcmp(argv[i], "-text")) {
      strcpy(text, argv[i+1]);
    } else
    if (!strcmp(argv[i], "-duration")) {
      duration = atoi(argv[i+1]);
    } else {
      dbg("unknown opt %s", argv[i]);
    }
  }

  char message[512];

  sprintf(message, "#hime_message %s %s %d", icon, text, duration);


  gdk_init(NULL, NULL);

  send_hime_message(GDK_DISPLAY(), message);

  return 0;
}
