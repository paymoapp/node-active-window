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

	void ActiveWindow::buildAppCache() {
		std::string userHome = std::getenv("HOME");
		std::string xdgDataDirs = std::getenv("XDG_DATA_DIRS");

		// fetch applications from every data dir
		this->loadDesktopEntriesFromDirectory(userHome + "/.local/share/applications");
		size_t startPos = 0;
		size_t delimPos = xdgDataDirs.find(":");
		while (delimPos != std::string::npos) {
			this->loadDesktopEntriesFromDirectory(xdgDataDirs.substr(startPos, delimPos - startPos) + "/applications");
			startPos = delimPos + 1;
			delimPos = xdgDataDirs.find(":", startPos);
		}
		this->loadDesktopEntriesFromDirectory(xdgDataDirs.substr(startPos, delimPos - startPos) + "/applications");
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

	void ActiveWindow::loadDesktopEntriesFromDirectory(std::string dir) {
		DIR* d = opendir(dir.c_str());

		if (d == NULL) {
			return;
		}

		struct dirent* ent;
		while ((ent = readdir(d)) != NULL) {
			std::string file = ent->d_name;

			if (file == "." || file == ".." || file.find_last_of(".") == std::string::npos || file.substr(file.find_last_of(".")) != ".desktop") {
				continue;
			}

			CSimpleIni ini;
			ini.SetUnicode();
			if (ini.LoadFile((dir + "/" + file).c_str()) < 0) {
				continue;
			}

			std::string wmClass = ini.GetValue("Desktop Entry", "StartupWMClass", "");
			std::string appName = ini.GetValue("Desktop Entry", "Name", "");
			std::string appIcon = ini.GetValue("Desktop Entry", "Icon", "");
			
			if (appIcon == "") {
				// we only need apps that have icons
				continue;
			}

			std::string iconPath = this->resolveIconPath(appIcon);

			if (iconPath == "") {
				// we couldn't resolve the icon path
				continue;
			}

			// WM class should take precedence
			std::string indexBy = wmClass != "" ? wmClass : this->processStringForIndex(appName);

			if (this->apps.find(indexBy) != this->apps.end()) {
				// already indexed
				continue;
			}

			this->appToIcon[indexBy] = iconPath;
			this->apps.insert(indexBy);
		}

		closedir(d);
	}

	std::string ActiveWindow::processStringForIndex(std::string str) {
		icu::UnicodeString ustr(str.c_str());
		ustr.toLower();

		// replace 
		ustr.findAndReplace(u8" ", u8"-");
		
		std::string processed;
		ustr.toUTF8String(processed);

		return processed;
	}

	std::string ActiveWindow::resolveIconPath(std::string icon) {
		if (icon[0] == '/') {
			// we already have a path
			if (access(icon.c_str(), F_OK) != -1) {
				return icon;
			}
			
			return "";
		}

		std::string userHome = std::getenv("HOME");
		std::string xdgDataDirs = std::getenv("XDG_DATA_DIRS");
		std::string iconPath = "";

		// legacy icon folder
		iconPath = this->tryResolveIconWithDirectory(icon, userHome + "/.icons");
		if (iconPath != "") {
			return iconPath;
		}

		// user icon folder
		iconPath = this->tryResolveIconWithDirectory(icon, userHome + "/.local/share/icons");
		if (iconPath != "") {
			return iconPath;
		}

		// icon folders in XDG_DATA_DIRS
		size_t startPos = 0;
		size_t delimPos = xdgDataDirs.find(":");
		while (delimPos != std::string::npos) {
			iconPath = this->tryResolveIconWithDirectory(icon, xdgDataDirs.substr(startPos, delimPos - startPos) + "/icons");
			if (iconPath != "") {
				return iconPath;
			}

			startPos = delimPos + 1;
			delimPos = xdgDataDirs.find(":", startPos);
		}
		iconPath = this->tryResolveIconWithDirectory(icon, xdgDataDirs.substr(startPos, delimPos - startPos) + "/icons");
		if (iconPath != "") {
			return iconPath;
		}

		// pixmaps folder
		iconPath = this->tryResolveIconWithDirectory(icon, "/usr/share/pixmaps");

		return iconPath;
	}

	std::string ActiveWindow::tryResolveIconWithDirectory(std::string icon, std::string dir) {
		if (!this->dirExists(dir)) {
			return "";
		}

		std::string iconPath = "";

		// check for file directly in folder
		iconPath = this->buildIconPath(icon, dir);
		if (access(iconPath.c_str(), F_OK) != -1) {
			return iconPath;
		}

		// try to fetch icons for popular themes
		iconPath = this->tryResolveIconWithTheme(icon, dir, "Adwaita");
		if (iconPath != "") {
			return iconPath;
		}

		iconPath = this->tryResolveIconWithTheme(icon, dir, "breeze");
		if (iconPath != "") {
			return iconPath;
		}

		iconPath = this->tryResolveIconWithTheme(icon, dir, "oxygen");
		if (iconPath != "") {
			return iconPath;
		}

		iconPath = this->tryResolveIconWithTheme(icon, dir, "gnome");
		if (iconPath != "") {
			return iconPath;
		}

		iconPath = this->tryResolveIconWithTheme(icon, dir, "hicolor");

		return iconPath;
	}

	std::string ActiveWindow::tryResolveIconWithTheme(std::string icon, std::string dir, std::string theme) {
		if (!this->dirExists(dir + "/" + theme)) {
			return "";
		}

		// loop through different scales
		std::string iconPath = "";
		for (std::vector<std::string>::const_iterator it = this->iconScalesPreference.begin(); it != this->iconScalesPreference.end(); it++) {
			iconPath = this->buildIconPath(icon, dir + "/" + theme + "/" + *it + "/apps");
			if (access(iconPath.c_str(), F_OK) != -1) {
				return iconPath;
			}
		}

		return "";
	}

	std::string ActiveWindow::buildIconPath(std::string icon, std::string dir) {
		if (icon.find_last_of(".") != std::string::npos && icon.substr(icon.find_last_of(".")) == ".png") {
			return dir + "/" + icon;
		}

		return dir + "/" + icon + ".png";
	}

	bool ActiveWindow::dirExists(std::string dir) {
		DIR* d = opendir(dir.c_str());

		if (d == NULL) {
			return false;
		}

		closedir(d);
		return true;
	}
}
