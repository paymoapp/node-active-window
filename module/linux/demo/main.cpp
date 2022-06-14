#include "../src/ActiveWindow.h"
#include <iostream>
#include <unistd.h>
#include <X11/Xlib.h>

int handle_xerror(Display* display, XErrorEvent* error) {
	std::cout<<"XERROR!\ntype: "<<error->type<<"\nerror_code: "<<error->error_code<<std::endl;

	return 1;
}

void printWindowInfo(PaymoActiveWindow::ActiveWindow* aw) {
	std::cout<<"Window currently in foreground:"<<std::endl;

	PaymoActiveWindow::WindowInfo* inf = aw->getActiveWindow();

	if (inf == NULL) {
		std::cout<<"Error: Could not get window info"<<std::endl;
		return;
	}

	std::cout<<"Title: \""<<inf->title<<"\""<<std::endl;
	std::cout<<"Application: \""<<inf->application<<"\""<<std::endl;
	std::cout<<"Path: \""<<inf->path<<"\""<<std::endl;
	std::cout<<"PID: \""<<inf->pid<<"\""<<std::endl;
	std::cout<<"Icon (base64 with viewer): https://systemtest.tk/uploads/d8120932c898c1191bbda1cb6250c3bb#"<<inf->icon<<std::endl;

	delete inf;
}

int main(int argc, char* argv[]) {
	XSetErrorHandler(handle_xerror);
	PaymoActiveWindow::ActiveWindow aw;

	if (argc > 1) {
		std::cout<<"Printing window info in infinite loop"<<std::endl;
		for (;;) {
			printWindowInfo(&aw);
			sleep(1);
		}
	}

	printWindowInfo(&aw);

	std::cout<<"Now sleeping 3 seconds for you to move to another window\n\n\n";
	sleep(3);

	printWindowInfo(&aw);

	return 0;
}
