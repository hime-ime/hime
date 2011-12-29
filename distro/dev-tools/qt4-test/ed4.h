#include <QtGui>

class edit1;

class linedit:public QFrame {
public:
  void gui_init();
  edit1 *edit2;
  QHBoxLayout *layout;
  QFrame *frame;
};

class edit1:public QLineEdit {
};
