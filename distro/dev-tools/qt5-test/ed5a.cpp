#include <QtWidgets>
#include "ed5.h"

Window::Window()
{
  edit2 = new QLineEdit;
  QGridLayout *echoLayout = new QGridLayout;
  echoLayout->addWidget(edit2, 1, 0, 1, 2);
  setLayout(echoLayout);
}
