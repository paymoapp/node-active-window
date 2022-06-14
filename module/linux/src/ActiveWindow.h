#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>

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
		ActiveWindow();
		~ActiveWindow();
		WindowInfo* getActiveWindow();
	private:
		Display* display = NULL;

		Window getFocusedWindow();
		std::string getWindowTitle(Window win);
		std::string getApplicationName(Window win);
		pid_t getWindowPid(Window win);
		std::string getProcessPath(pid_t pid);
	};
}

#endif
