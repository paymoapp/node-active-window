#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include <string>
#include <vector>
#include "base64.h"

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

	class ActiveWindow {
	public:
		WindowInfo* getActiveWindow();
		bool requestScreenCaptureAccess();
	private:
		bool hasScreenCaptureAccess = false;

		std::string encodeNSImage(NSImage* img);
		std::string getWindowTitle(int pid);
	};
}

#endif
