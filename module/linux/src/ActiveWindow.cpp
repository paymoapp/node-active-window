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

		if (XGetWindowProperty(this->display, root, property, 0, 4, false, XA_WINDOW, &_actualType, &_actualFormat, &_itemsCount, &_bytes, &prop) != Success) {
			return None;
		}

		Window focused = static_cast<unsigned long>(prop[0] + (prop[1] << 8) + (prop[2] << 16) + (prop[3] << 24));
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
}
