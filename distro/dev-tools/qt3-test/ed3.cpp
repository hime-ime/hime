#include "ed3.h"

int main()
{
  QApplication a(0, 0);
  LineEdits l;
  a.setMainWidget(&l);
  l.show();
  return a.exec();
}
