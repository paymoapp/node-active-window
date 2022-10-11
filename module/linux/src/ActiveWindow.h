#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include "SimpleIni/SimpleIni.h"
#include "base64/base64.h"
#include "lodepng/lodepng.h"

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
	const int targetIconSize = 128;

	class ActiveWindow {
	public:
		ActiveWindow();
		~ActiveWindow();
		WindowInfo* getActiveWindow();
		void buildAppCache();
		watch_t watchActiveWindow(watch_callback cb);
		void unwatchActiveWindow(watch_t watch);
	private:
		Display* display = NULL;
		std::map<std::string, std::string> appToIcon;
		std::set<std::string> apps;

		const std::vector<std::string> iconScalesPreference = { "256x256", "48x48", "22x22" };

		watch_t nextWatchId = 1;

		std::thread* watchThread = NULL;
		std::mutex mutex;
		std::atomic<bool> threadShouldExit;
		std::map<watch_t, watch_callback> watches;

		Window getFocusedWindow();
		std::string getWindowTitle(Window win);
		std::string getApplicationName(Window win);
		pid_t getWindowPid(Window win);
		std::string getProcessPath(pid_t pid);
		std::string getApplicationIcon(std::string app);
		std::string getWindowIcon(Window win);
		void loadDesktopEntriesFromDirectory(std::string dir);
		std::string processStringForIndex(std::string str);
		std::string resolveIconPath(std::string icon);
		std::string tryResolveIconWithDirectory(std::string icon, std::string dir);
		std::string tryResolveIconWithTheme(std::string icon, std::string dir, std::string theme);
		std::string buildIconPath(std::string icon, std::string dir);
		bool dirExists(std::string dir);
		std::string encodePngIcon(std::string iconPath);
		void runWatchThread();
		static int xErrorHandler(Display* display, XErrorEvent* error);
	};
}

#endif
