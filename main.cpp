/**
 * @file
 * @brief Qt application event loop
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */

#include <QApplication>

#include "cmainwindow.h"

int main (int argc, char* argv[])
{
	QApplication app (argc, argv);
	QGitRepoViewer::CMainWindow wnd;
	wnd.show ();

	return app.exec();
}
