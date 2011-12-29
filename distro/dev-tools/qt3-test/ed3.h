#include <qapplication.h>
#include <qgroupbox.h>
#include <qlineedit.h>

class QLineEdit;

class LineEdits:public QGroupBox
{
public:
  LineEdits(QWidget *parent = 0, const char *name = 0);
  QLineEdit *lined1;
};
