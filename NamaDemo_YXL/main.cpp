#define POINTER_64 __ptr64
#include "NamaDemo_YXL.h"
#include <QtWidgets/QApplication>
#include "YXL/YXLHelper.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	NamaDemo_YXL w;
	w.show();
	return a.exec();
}
