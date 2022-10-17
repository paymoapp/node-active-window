#import <Foundation/Foundation.h>
#include "../src/ActiveWindow.h"
#include <iostream>
#include <csignal>
#include <cstring>
#include <ctime>

void printWindowInfo(PaymoActiveWindow::WindowInfo* inf) {
	std::cout<<"Title: \""<<inf->title<<"\""<<std::endl;
	std::cout<<"Application: \""<<inf->application<<"\""<<std::endl;
	std::cout<<"Path: \""<<inf->path<<"\""<<std::endl;
	std::cout<<"PID: \""<<inf->pid<<"\""<<std::endl;
	std::cout<<"Icon (base64 with viewer): https://systemtest.tk/uploads/d8120932c898c1191bbda1cb6250c3bb#"<<inf->icon<<std::endl;
}

void pollWindowInfo(PaymoActiveWindow::ActiveWindow* aw) {
	std::cout<<"Window currently in foreground:"<<std::endl;

	PaymoActiveWindow::WindowInfo* inf = aw->getActiveWindow();

	if (inf == NULL) {
		std::cout<<"Error: Could not get window info"<<std::endl;
		return;
	}

	printWindowInfo(inf);

	delete inf;
}

bool loop = true;
void signalHandler(int signum) {
	std::cout<<"Got Ctrl+C"<<std::endl;
	loop = false;
}

int main(int argc, char* argv[]) {
	PaymoActiveWindow::ActiveWindow* aw = new PaymoActiveWindow::ActiveWindow(10);

	std::cout<<"Requesting screen capture access\n";
	bool scaResp = aw->requestScreenCaptureAccess();
	std::cout<<"The permission was: "<<(scaResp ? "granted" : "denied")<<"\n";

	if (argc < 2) {
		// default mode
		pollWindowInfo(aw);

		std::cout<<"Now sleeping 3 seconds for you to move to an other window\n\n\n";
		[NSThread sleepForTimeInterval:3.0f];

		pollWindowInfo(aw);
	}
	else if (strcmp(argv[1], "loop") == 0) {
		// infinite loop mode
		std::cout<<"Printing window info in infinite loop. Ctrl+C to exit"<<std::endl;
		signal(SIGINT, signalHandler);
		while (loop) {
			pollWindowInfo(aw);
			std::cout<<"\n\n\n";
			[NSThread sleepForTimeInterval:3.0f];
		}
	}
	else if (strcmp(argv[1], "watch") == 0) {
		// watch mode
		std::cout<<"Running in watch mode. Ctrl+C to exit"<<std::endl;
		signal(SIGINT, signalHandler);
		std::cout<<"Please note: this is not a real watch mode. We have to manually run the runloop for it to work. "<<
			"If you implement this library into an application that already has an NSApplication and it has a running "<<
			"NSRunLoop, then you don't have to call activeWindow->runLoop() all the time."<<std::endl;

		PaymoActiveWindow::watch_t watchId = aw->watchActiveWindow([](PaymoActiveWindow::WindowInfo* inf) {
			std::cout<<"[Notif] Active window has changed!\n";
			
			if (inf == NULL) {
				std::cout<<"Empty"<<std::endl;
				return;
			}

			printWindowInfo(inf);
			std::cout<<"\n\n\n";
		});

		std::cout<<"Set up watch. Id = "<<watchId<<std::endl;

		while (loop) {
			aw->runLoop();
			[NSThread sleepForTimeInterval:0.1f];
		}

		std::cout<<"Removing watch"<<std::endl;
		aw->unwatchActiveWindow(watchId);
		std::cout<<"Watch removed"<<std::endl;
	}
	else if (strcmp(argv[1], "benchmark") == 0) {
		// benchmark mode
		std::cout<<"Benchmark mode. Will run 100000 iterations and print CPU time"<<std::endl;
		clock_t start = clock();
		for (int i = 0; i < 100000; i++) {
			PaymoActiveWindow::WindowInfo* inf = aw->getActiveWindow();
			delete inf;
		}
		clock_t end = clock();

		double elapsedTime = (double)(end - start) / CLOCKS_PER_SEC;

		std::cout<<"Elapsed clocks: "<<(end - start)<<"\nElapsed seconds: "<<elapsedTime<<std::endl;
	}
	else {
		std::cout<<"Error: Invalid mode"<<std::endl;
	}

	delete aw;

	return 0;
}
