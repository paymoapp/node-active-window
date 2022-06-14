#include "ActiveWindow.h"

namespace PaymoActiveWindow {
	ActiveWindow::ActiveWindow() {
		this->display = XOpenDisplay(NULL);

		if (this->display == NULL) {
			throw std::logic_error("Failed to open display");
		}
	}

	ActiveWindow::~ActiveWindow() {
		XCloseDisplay(this->display);
		this->display = NULL;
	}

	WindowInfo* ActiveWindow::getActiveWindow() {
		if (this->display == NULL) {
			return NULL;
		}

		Window activeWin = this->getFocusedWindow();

		if (activeWin == None) {
			return NULL;
		}

		WindowInfo* info = new WindowInfo();

		info->title = this->getWindowTitle(activeWin);
		info->application = this->getApplicationName(activeWin);
		info->pid = this->getWindowPid(activeWin);

		if (info->pid >= 0) {
			info->path = this->getProcessPath(info->pid);
		}

		return info;
	}

	Window ActiveWindow::getFocusedWindow() {
		// we're going to use the _NET_ACTIVE_WINDOW property of the root window
		Atom property = XInternAtom(this->display, "_NET_ACTIVE_WINDOW", true);
		Window root = DefaultRootWindow(this->display);

		Atom _actualType;
		int _actualFormat;
		unsigned long _itemsCount;
		unsigned long _bytes;

		unsigned char* prop;

		if (XGetWindowProperty(this->display, root, property, 0, sizeof(Window), false, XA_WINDOW, &_actualType, &_actualFormat, &_itemsCount, &_bytes, &prop) != Success) {
			return None;
		}

		Window focused = *reinterpret_cast<Window*>(prop);
		XFree(prop);

		return focused;
	}

	std::string ActiveWindow::getWindowTitle(Window win) {
		XTextProperty textProp;
		if (!XGetWMName(this->display, win, &textProp)) {
			return "";
		}

		if (!textProp.value || !textProp.nitems) {
			return "";
		}

		if (textProp.encoding == XA_STRING) {
			// ASCII window title
			std::string title(reinterpret_cast<char*>(textProp.value));
			XFree(textProp.value);
			return title;
		}

		// UTF-8 window title
		char** list;
		int listCount;
		if (Xutf8TextPropertyToTextList(this->display, &textProp, &list, &listCount) < Success || listCount < 1 || !list) {
			XFree(textProp.value);
			return "";
		}

		XFree(textProp.value);
		std::string title(reinterpret_cast<char*>(list[0]));
		XFreeStringList(list);

		return title;
	}

	std::string ActiveWindow::getApplicationName(Window win) {
		XClassHint classHint;

		if (!XGetClassHint(this->display, win, &classHint)) {
			return "";
		}

		std::string application(classHint.res_class);
		XFree(classHint.res_name);
		XFree(classHint.res_class);

		return application;
	}

	pid_t ActiveWindow::getWindowPid(Window win) {
		Atom property = XInternAtom(this->display, "_NET_WM_PID", true);

		Atom _actualType;
		int _actualFormat;
		unsigned long _itemsCount;
		unsigned long _bytes;

		unsigned char* prop;

		if (XGetWindowProperty(this->display, win, property, 0, sizeof(pid_t), false, XA_CARDINAL, &_actualType, &_actualFormat, &_itemsCount, &_bytes, &prop) != Success) {
			return -1;
		}

		pid_t pid = *reinterpret_cast<pid_t*>(prop);
		XFree(prop);

		return pid;
	}

	std::string ActiveWindow::getProcessPath(pid_t pid) {
		std::stringstream procPath;
		procPath<<"/proc/"<<pid<<"/cmdline";

		int fd = open(procPath.str().c_str(), O_RDONLY);
		if (fd < 0) {
			return "";
		}

		std::string path = "";

		int bytesRead = 0;
		do {
			char buf[256];
			bytesRead = read(fd, buf, 255);

			// convert inline nulls to spaces
			for (int i = 0; i < bytesRead; i++) {
				if (buf[i] == 0) {
					buf[i] = ' ';
				}
			}

			// add terminating null
			buf[bytesRead] = 0;

			// concat to existing path
			path += buf;
		} while (bytesRead > 0);

		// remove last null which was converted to space
		path.pop_back();

		close(fd);

		return path;
	}
}
