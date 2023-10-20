#include "ActiveWindow.h"

namespace PaymoActiveWindow {
	ActiveWindow::ActiveWindow(unsigned int iconCacheSize) {
		XSetErrorHandler(ActiveWindow::xErrorHandler);
		this->display = XOpenDisplay(NULL);

		if (this->display == NULL) {
			throw std::logic_error("Failed to open display");
		}

		if (iconCacheSize > 0) {
			this->iconCache = new IconCache(iconCacheSize);
		}
	}

	ActiveWindow::~ActiveWindow() {
		// stop watch thread
		if (this->watchThread != NULL) {
			this->threadShouldExit.store(true, std::memory_order_relaxed);
			this->watchThread->join();
			delete this->watchThread;
			this->watchThread = NULL;
		}

		delete this->iconCache;
		this->iconCache = NULL;
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

		info->icon = this->getIcon(info->application, activeWin);

		return info;
	}

	void ActiveWindow::buildAppCache() {
		std::string userHome = this->safeGetEnv("HOME");
		std::string xdgDataDirs = this->safeGetEnv("XDG_DATA_DIRS");

		// fetch applications from every data dir
		if (userHome != "") {
			this->loadDesktopEntriesFromDirectory(userHome + "/.local/share/applications");
		}

		if (xdgDataDirs != "") {
			size_t startPos = 0;
			size_t delimPos = xdgDataDirs.find(":");
			while (delimPos != std::string::npos) {
				this->loadDesktopEntriesFromDirectory(xdgDataDirs.substr(startPos, delimPos - startPos) + "/applications");
				startPos = delimPos + 1;
				delimPos = xdgDataDirs.find(":", startPos);
			}
			this->loadDesktopEntriesFromDirectory(xdgDataDirs.substr(startPos, delimPos - startPos) + "/applications");
		}
	}

	watch_t ActiveWindow::watchActiveWindow(watch_callback cb) {
		watch_t watchId = this->nextWatchId++;

		this->mutex.lock();
		this->watches[watchId] = cb;
		this->mutex.unlock();

		// start thread if not started
		if (this->watchThread == NULL) {
			this->threadShouldExit.store(false, std::memory_order_relaxed);
			this->watchThread = new std::thread(&ActiveWindow::runWatchThread, this);
		}

		return watchId;
	}

	void ActiveWindow::unwatchActiveWindow(watch_t watch) {
		this->mutex.lock();
		this->watches.erase(watch);
		this->mutex.unlock();
	}

	Window ActiveWindow::getFocusedWindow() {
		// we're going to use the _NET_ACTIVE_WINDOW property of the root window
		Atom property = XInternAtom(this->display, "_NET_ACTIVE_WINDOW", true);
		Window root = DefaultRootWindow(this->display);

		Atom actualType;
		int actualFormat;
		unsigned long itemsCount;
		unsigned long bytes;

		unsigned char* prop;

		if (XGetWindowProperty(this->display, root, property, 0, sizeof(Window), false, XA_WINDOW, &actualType, &actualFormat, &itemsCount, &bytes, &prop) != Success) {
			return None;
		}

		if (actualFormat == 0) {
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

		Atom actualType;
		int actualFormat;
		unsigned long itemsCount;
		unsigned long bytes;

		unsigned char* prop;

		if (XGetWindowProperty(this->display, win, property, 0, sizeof(pid_t), false, XA_CARDINAL, &actualType, &actualFormat, &itemsCount, &bytes, &prop) != Success) {
			return -1;
		}

		if (actualFormat == 0) {
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
		if (path.length() > 0) {
			path.pop_back();
		}

		close(fd);

		return path;
	}

	std::string ActiveWindow::getIcon(std::string app, Window win) {
		if (this->iconCache != NULL && this->iconCache->has(&app)) {
			return this->iconCache->get(&app);
		}

		std::string icon = this->getApplicationIcon(app);

		if (icon == "") {
			icon = this->getWindowIcon(win);
		}

		if (this->iconCache != NULL) {
			this->iconCache->set(&app, &icon);
		}

		return icon;
	}

	std::string ActiveWindow::getApplicationIcon(std::string app) {
		// check if there's a direct match (using WM_CLASS)
		if (this->appToIcon.find(app) != this->appToIcon.end()) {
			// we have an icon
			return this->encodeIcon(this->appToIcon[app]);
		}

		// find using string match
		std::string needle = this->processStringForIndex(app);
		for (std::unordered_map<std::string, std::string>::iterator it = this->appToIcon.begin(); it != this->appToIcon.end(); it++) {
			if (it->first.find(needle) != std::string::npos) {
				// we have a likely match
				return this->encodeIcon(it->second);
			}
		}

		return "";
	}

	std::string ActiveWindow::getWindowIcon(Window win) {
		Atom property = XInternAtom(this->display, "_NET_WM_ICON", true);

		Atom actualType;
		int actualFormat;
		unsigned long itemsCount;
		unsigned long bytes;
		unsigned char* prop;

		int maxSize = 0;
		int maxSizeOffset = 0;
		int offset = 0;

		do {
			if (XGetWindowProperty(this->display, win, property, offset, 1, 0, XA_CARDINAL, &actualType, &actualFormat, &itemsCount, &bytes, &prop) != Success) {
				return "";
			}
			if (actualFormat == 0) {
				return "";
			}
			int width = *reinterpret_cast<int*>(prop);
			XFree(prop);

			if (XGetWindowProperty(this->display, win, property, offset + 1, 1, 0, XA_CARDINAL, &actualType, &actualFormat, &itemsCount, &bytes, &prop) != Success) {
				return "";
			}
			int height = *reinterpret_cast<int*>(prop);
			XFree(prop);

			int size = width * height;

			if (size > maxSize) {
				maxSize = size;
				maxSizeOffset = offset;
			}

			if (width >= targetIconSize) {
				break;
			}

			offset += size + 2;
		} while(maxSize * 4 < bytes);

		if (XGetWindowProperty(this->display, win, property, maxSizeOffset, 1, 0, XA_CARDINAL, &actualType, &actualFormat, &itemsCount, &bytes, &prop) != Success) {
			return "";
		}
		int width = *reinterpret_cast<int*>(prop);
		XFree(prop);

		if (XGetWindowProperty(this->display, win, property, maxSizeOffset + 1, 1, 0, XA_CARDINAL, &actualType, &actualFormat, &itemsCount, &bytes, &prop) != Success) {
			return "";
		}
		int height = *reinterpret_cast<int*>(prop);
		XFree(prop);

		int size = width * height;

		if (XGetWindowProperty(this->display, win, property, maxSizeOffset + 2, size, 0, XA_CARDINAL, &actualType, &actualFormat, &itemsCount, &bytes, &prop) != Success) {
			return "";
		}

		unsigned long* pixmap = reinterpret_cast<unsigned long*>(prop);
		std::vector<unsigned char> imgData;
		imgData.reserve(size * 4);

		for (int i = 0; i < size; i++) {
			unsigned long pixel = *pixmap++;
			imgData.push_back((pixel & 0x00ff0000) >> 16); // r
			imgData.push_back((pixel & 0x0000ff00) >> 8); // g
			imgData.push_back((pixel & 0x000000ff)); // b
			imgData.push_back((pixel & 0xff000000) >> 24); // a
		}

		XFree(prop);

		std::vector<unsigned char> png;
		if (lodepng::encode(png, imgData, width, height, LCT_RGBA, 8)) {
			return "";
		}

		std::string pngStr(png.begin(), png.end());

		return "data:image/png;base64," + base64_encode(pngStr);
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

			if (this->appToIcon.find(indexBy) != this->appToIcon.end()) {
				// already indexed
				continue;
			}

			this->appToIcon[indexBy] = iconPath;
		}

		closedir(d);
	}

	std::string ActiveWindow::processStringForIndex(std::string str) {
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
			return std::tolower(c);
		});

		std::replace(result.begin(), result.end(), ' ', '-');
		std::replace(result.begin(), result.end(), '_', '-');

		return result;
	}

	std::string ActiveWindow::resolveIconPath(std::string icon) {
		if (icon[0] == '/') {
			// we already have a path
			if (access(icon.c_str(), F_OK) != -1) {
				return icon;
			}
			
			return "";
		}

		std::string userHome = this->safeGetEnv("HOME");
		std::string xdgDataDirs = this->safeGetEnv("XDG_DATA_DIRS");
		std::string iconPath = "";

		if (userHome != "") {
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
		}

		if (xdgDataDirs != "") {
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

	std::string ActiveWindow::safeGetEnv(const char* envVar) {
		char* value = std::getenv(envVar);

		if (value == nullptr) {
			return "";
		}

		return value;
	}

	std::string ActiveWindow::encodeIcon(std::string iconPath) {
		std::string fileExtension = iconPath.substr(iconPath.find_last_of(".") + 1);

		if (fileExtension != "png" && fileExtension != "svg") {
			// not supported file extension (XPM)
			// only PNG, SVG and XPM are valid options, but we can't handle XPM yet
			// ALSO: extension must be lowercase, no need for case conversion
			// Ref: https://developer-old.gnome.org/icon-theme-spec/
			return "";
		}

		int fd = open(iconPath.c_str(), O_RDONLY);

		if (fd < 0) {
			return "";
		}

		struct stat st;
		fstat(fd, &st);

		std::vector<unsigned char> buf(st.st_size);

		int bytesRead = read(fd, buf.data(), st.st_size);

		if (bytesRead != st.st_size) {
			close(fd);
			return "";
		}

		close(fd);

		std::string icon(buf.begin(), buf.end());

		std::string contentType = fileExtension == "png" ? "data:image/png" : "data:image/svg+xml";

		return contentType + ";base64," + base64_encode(icon);
	}

	void ActiveWindow::runWatchThread() {
		Atom activeWindowProperty = XInternAtom(this->display, "_NET_ACTIVE_WINDOW", true);
		Atom windowTitleProperty = XInternAtom(this->display, "_NET_WM_NAME", true);
		Window root = DefaultRootWindow(this->display);

		XSetWindowAttributes setAttributes;
		setAttributes.event_mask = PropertyChangeMask;

		XChangeWindowAttributes(this->display, root, CWEventMask, &setAttributes);

		XEvent event;
		while (!this->threadShouldExit.load(std::memory_order_relaxed)) {
			if (XEventsQueued(this->display, QueuedAfterFlush) > 0) {
				// handle all queued events
				while (XEventsQueued(this->display, QueuedAlready) > 0) {
					// handle event
					XNextEvent(this->display, &event);

					if (event.type != PropertyNotify || (event.xproperty.atom != activeWindowProperty && event.xproperty.atom != windowTitleProperty)) {
						// not the event we're interested in
						continue;
					}

					Window currentWindow = this->getFocusedWindow();
					if (currentWindow != None) {
						XChangeWindowAttributes(this->display, currentWindow, CWEventMask, &setAttributes);
					}

					WindowInfo* info = this->getActiveWindow();

					// notify every callback
					this->mutex.lock();
					for (std::unordered_map<watch_t, watch_callback>::iterator it = this->watches.begin(); it != this->watches.end(); it++) {
						try {
							it->second(info);
						}
						catch (...) {
							// doing nothing
						}
					}
					this->mutex.unlock();

					delete info;
				}
			}

			// sleep 100ms
			usleep(100000);
		}
	}

	int ActiveWindow::xErrorHandler(Display* display, XErrorEvent* error) {
		char buff[BUFSIZ];
		
		XGetErrorText(display, error->error_code, buff, BUFSIZ);
		std::string errText(buff);

		XGetErrorDatabaseText(display, "XRequest", std::to_string((int)error->request_code).c_str(), "", buff, BUFSIZ);
		std::string majorCode(buff);

		XGetErrorDatabaseText(display, "XRequest", std::to_string((int)error->minor_code).c_str(), "", buff, BUFSIZ);
		std::string minorCode(buff);

		std::cerr<<"[WARN] PaymoActiveWindow::ActiveWindow: Cought XErrorEvent"<<std::endl;
		std::cerr<<"       Error type: "<<error->type<<std::endl;
		std::cerr<<"       Error code: "<<(int)error->error_code<<" ("<<errText<<")"<<std::endl;
		std::cerr<<"       Major op-code: "<<(int)error->request_code<<" ("<<majorCode<<")"<<std::endl;
		std::cerr<<"       Minor op-code: "<<(int)error->minor_code<<" ("<<minorCode<<")"<<std::endl;

		return 0;
	}
}
