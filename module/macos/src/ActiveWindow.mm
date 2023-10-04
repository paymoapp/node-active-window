#include "ActiveWindow.h"

namespace PaymoActiveWindow {
	ActiveWindow::ActiveWindow(unsigned int iconCacheSize) {
		this->registerObservers();

		if (iconCacheSize > 0) {
			this->iconCache = new IconCache(iconCacheSize);
		}
	}

	ActiveWindow::~ActiveWindow() {
		// remove observers
		NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
		NSNotificationCenter* notificationCenter = [workspace notificationCenter];

		for (std::vector<id>::iterator it = this->observers.begin(); it != this->observers.end(); it++) {
			[notificationCenter removeObserver:*it];
		}

		delete this->iconCache;
		this->iconCache = NULL;
	}

	WindowInfo* ActiveWindow::getActiveWindow() {
		NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
		NSRunningApplication* frontApp = [workspace frontmostApplication];

		if (frontApp == nil) {
			return NULL;
		}

		WindowInfo* info = new WindowInfo();

		info->application = [frontApp.localizedName UTF8String];
		info->path = [frontApp.executableURL.path UTF8String];
		info->pid = frontApp.processIdentifier;
		info->icon = this->getAppIcon(frontApp.icon, &info->path);

		info->title = this->getWindowTitle(info->pid);

		return info;
	}

	bool ActiveWindow::requestScreenCaptureAccess() {
		if (@available(macOS 11, *)) {
			// this api SHOULD work on 10.15 as well,
			// but it doesn't: https://developer.apple.com/forums/thread/683860
			this->hasScreenCaptureAccess = CGPreflightScreenCaptureAccess();

			if (!this->hasScreenCaptureAccess) {
				// request
				this->hasScreenCaptureAccess = CGRequestScreenCaptureAccess();
			}
		}
		else if (@available(macOS 10.15, *)) {
			// this is a hack to check if screen capture access is granted on catalina.
			// Source: https://stackoverflow.com/questions/56597221/detecting-screen-recording-settings-on-macos-catalina/58985069#58985069
			this->hasScreenCaptureAccess = false;

			NSRunningApplication* currentApp = NSRunningApplication.currentApplication;
			int pid = currentApp.processIdentifier;

			CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
			for (NSDictionary* window in (NSArray*)windows) {
				NSNumber* windowPid = window[(id)kCGWindowOwnerPID];
				int wPid = [windowPid intValue];

				if (wPid == pid) {
					// we can always access our own process
					continue;
				}

				NSString* windowTitle = window[(id)kCGWindowName];

				if (windowTitle != nil) {
					this->hasScreenCaptureAccess = true;
					break;
				}
			}
		}
		else {
			// on older versions there's no permission for this
			this->hasScreenCaptureAccess = true;
		}

		return this->hasScreenCaptureAccess;
	}

	watch_t ActiveWindow::watchActiveWindow(watch_callback cb) {
		watch_t watchId = this->nextWatchId++;

		this->watches[watchId] = cb;

		return watchId;
	}

	void ActiveWindow::unwatchActiveWindow(watch_t watch) {
		this->watches.erase(watch);
	}

	void ActiveWindow::runLoop() {
		// run RunLoop for 0.1 ms or until first event is handled
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.0001, true);
	}

	std::string ActiveWindow::getAppIcon(NSImage* img, const std::string* path) {
		if (this->iconCache != NULL && this->iconCache->has(path)) {
			return this->iconCache->get(path);
		}

		std::string icon = this->encodeNSImage(img);

		if (this->iconCache != NULL) {
			this->iconCache->set(path, &icon);
		}

		return icon;
	}

	std::string ActiveWindow::encodeNSImage(NSImage* img) {
		CGImageRef cgRef = [img CGImageForProposedRect:NULL context:nil hints:nil];
		NSBitmapImageRep* imgRep = [[NSBitmapImageRep alloc] initWithCGImage:cgRef];
		[imgRep setSize:[img size]];
		NSData* pngData = [imgRep representationUsingType:NSBitmapImageFileTypePNG properties:[NSDictionary dictionary]];

		unsigned int len = [pngData length];
		std::vector<char> buf(len);
		memcpy(buf.data(), [pngData bytes], len);

		std::string pngImg(buf.begin(), buf.end());

		return "data:image/png;base64," + base64_encode(pngImg);
	}

	std::string ActiveWindow::getWindowTitle(int pid) {
		if (!this->hasScreenCaptureAccess) {
			// no title information without screen recording permission
			return "";
		}

		CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
		NSString* windowTitle = nil;

		for (NSDictionary* window in (NSArray*)windows) {
			NSNumber* windowPid = window[(id)kCGWindowOwnerPID];
			int wPid = [windowPid intValue];

			if (wPid != pid) {
				// not our window
				continue;
			}

			if (window[(id)kCGWindowName]) {
				// we have a title and since we iterate from top to bottom
				// this is the title we're looking for
				windowTitle = window[(id)kCGWindowName];

				if (windowTitle.length == 0) {
					// sometimes a popup is open and it does not have title
					// try to get title for the window under it
					continue;
				}

				break;
			}
		}

		std::string title(windowTitle != nil ? [windowTitle UTF8String] : "");
		CFRelease(windows);

		return title;
	}

	void ActiveWindow::registerObservers() {
		NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
		NSNotificationCenter* notificationCenter = [workspace notificationCenter];

		this->observers.push_back([notificationCenter addObserverForName:NSWorkspaceDidActivateApplicationNotification object:nil queue:nil usingBlock:^(NSNotification* notification) {
			this->handleNotification();
		}]);
	}

	void ActiveWindow::handleNotification() {
		WindowInfo* info = this->getActiveWindow();

		for (std::map<watch_t, watch_callback>::iterator it = this->watches.begin(); it != this->watches.end(); it++) {
			try {
				it->second(info);
			}
			catch (...) {
				// doing nothing
			}
		}

		delete info;
	}
}
