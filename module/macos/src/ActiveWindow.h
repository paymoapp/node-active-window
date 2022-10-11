#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include <string>
#include <vector>
#include <functional>
#include <map>
#include "base64/base64.h"

#ifndef _PAYMO_ACTIVEWINDOW_H
#define _PAYMO_ACTIVEWINDOW_H

namespace PaymoActiveWindow {
	struct WindowInfo {
		std::string title = "";
		std::string application = "";
		std::string path = "";
		int pid = 0;
		std::string icon = "";
	};

	typedef unsigned int watch_t;
	typedef std::function<void(WindowInfo*)> watch_callback;

	class ActiveWindow {
	public:
		ActiveWindow();
		~ActiveWindow();
		WindowInfo* getActiveWindow();
		bool requestScreenCaptureAccess();
		watch_t watchActiveWindow(watch_callback cb);
		void unwatchActiveWindow(watch_t watch);
		void runLoop();
	private:
		bool hasScreenCaptureAccess = false;

		std::vector<id> observers;

		watch_t nextWatchId = 1;

		std::map<watch_t, watch_callback> watches;

		std::string encodeNSImage(NSImage* img);
		std::string getWindowTitle(int pid);
		void registerObservers();
		void handleNotification();
	};
}

#endif
