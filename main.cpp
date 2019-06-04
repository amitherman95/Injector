#include "DLLInjector.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DLLInjector mainForm;
	mainForm.show();
	return a.exec();
}
