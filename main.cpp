#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setOrganizationName("antifin");
	app.setApplicationName("remembrance");

	QTranslator translator;
	translator.load("remembrance_ru");
	app.installTranslator(&translator);

	MainWindow wnd;
	wnd.show();
	return app.exec();
}
