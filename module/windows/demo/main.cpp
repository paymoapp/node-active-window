#include "../src/ActiveWindow.h"
#include <iostream>

void printWindowInfo(PaymoActiveWindow::ActiveWindow* aw) {
	std::cout<<"Window currently in foreground:"<<std::endl;

	PaymoActiveWindow::WindowInfo* inf = aw->getActiveWindow();

	if (inf == null) {
		std::cout<<"Error: Could not get window info"<<std::endl;
	}

	std::wcout<<L"Title: \""<<inf->title<<L"\""<<std::endl;
	std::wcout<<L"Application: \""<<inf->application<<L"\""<<std::endl;
	std::wcout<<L"Path: \""<<inf->path<<L"\""<<std::endl;
	std::cout<<"PID: \""<<inf->pid<<"\""<<std::endl;

	delete inf;
}

int main() {
	PaymoActiveWindow::ActiveWindow aw;

	printWindowInfo(&aw);

	std::cout<<"Now sleeping 10 seconds for you to move to another window\n\n\n";
	Sleep(10000);

	printWindowInfo(&aw);

	return 0;
}
