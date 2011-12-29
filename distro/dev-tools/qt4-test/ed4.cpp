#include "ed4.h"

int main()
{
  QApplication a(0, 0);
  linedit *l=new linedit;
  l->gui_init();
  return a.exec();
}
