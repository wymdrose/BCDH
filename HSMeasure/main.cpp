#include "hsmeasure.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HSMeasure w;
	w.show();
	return a.exec();
}
