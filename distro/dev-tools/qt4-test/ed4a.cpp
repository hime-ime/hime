#include "ed4.h"

void linedit::gui_init()
{
  edit2=new edit1;
  layout=new QHBoxLayout;
  layout->addWidget(edit2);
  this->setLayout(layout);
  this->show();
}
