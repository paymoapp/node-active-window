#include "../src/ActiveWindow.h"
#include <iostream>

void printWindowInfo(PaymoActiveWindow::ActiveWindow* aw) {
	std::cout<<"Window currently in foreground:"<<std::endl;

	PaymoActiveWindow::WindowInfo* inf = aw->getActiveWindow();

	if (inf == null) {
		std::cout<<"Error: Could not get window info"<<std::endl;
		return;
	}

	std::wcout<<L"Title: \""<<inf->title<<L"\""<<std::endl;
	std::wcout<<L"Application: \""<<inf->application<<L"\""<<std::endl;
	std::wcout<<L"Path: \""<<inf->path<<L"\""<<std::endl;
	std::cout<<"PID: \""<<inf->pid<<"\""<<std::endl;
	std::cout<<"Is UWP App: "<<(inf->isUWPApp ? "true" : "false")<<std::endl;
	std::wcout<<L"UWP Package name: \""<<inf->uwpPackage<<L"\""<<std::endl;
	std::cout<<"Icon (base64 with viewer): https://systemtest.tk/uploads/d8120932c898c1191bbda1cb6250c3bb#"<<inf->icon<<std::endl;

	delete inf;
}

int main() {
	PaymoActiveWindow::ActiveWindow aw;

	printWindowInfo(&aw);

	std::cout<<"Now sleeping 3 seconds for you to move to another window\n\n\n";
	Sleep(3000);

	printWindowInfo(&aw);

	return 0;
}
