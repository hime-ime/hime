#include <QApplication>
#include "ed5.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Window l;
  l.show();
  return a.exec();
}
