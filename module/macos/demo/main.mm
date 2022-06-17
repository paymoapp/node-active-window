#import <Foundation/Foundation.h>
#include "../src/ActiveWindow.h"
#include <iostream>
#include <csignal>

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

bool loop = true;
void signalHandler(int signum) {
	std::cout<<"Got Ctrl+C"<<std::endl;
	loop = false;
}

int main(int argc, char* argv[]) {
	PaymoActiveWindow::ActiveWindow* aw = new PaymoActiveWindow::ActiveWindow();

	std::cout<<"Requesting screen capture access\n";
	bool scaResp = aw->requestScreenCaptureAccess();
	std::cout<<"The permission was: "<<(scaResp ? "granted" : "denied")<<"\n";

	if (argc > 1) {
		std::cout<<"Printing window info in infinite loop. Press Ctrl+C to break."<<std::endl;
		signal(SIGINT, signalHandler);
		while (loop) {
			printWindowInfo(aw);
			[NSThread sleepForTimeInterval:3.0f];
		}
	}

	printWindowInfo(aw);

	std::cout<<"Now sleeping 3 seconds for you to move to an other window\n\n\n";
	[NSThread sleepForTimeInterval:3.0f];

	printWindowInfo(aw);

	delete aw;

	return 0;
}
