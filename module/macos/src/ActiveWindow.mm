#include "ActiveWindow.h"

namespace PaymoActiveWindow {
	ActiveWindow::ActiveWindow() {
		this->registerObservers();
	}

	ActiveWindow::~ActiveWindow() {
		// remove observers
		NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
		NSNotificationCenter* notificationCenter = [workspace notificationCenter];

		for (std::vector<id>::iterator it = this->observers.begin(); it != this->observers.end(); it++) {
			[notificationCenter removeObserver:*it];
		}
	}

	WindowInfo* ActiveWindow::getActiveWindow() {
		this->runLoop();

		NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
		NSRunningApplication* frontApp = [workspace frontmostApplication];

		if (frontApp == nil) {
			return NULL;
		}

		WindowInfo* info = new WindowInfo();

		info->application = [frontApp.localizedName UTF8String];
		info->path = [frontApp.executableURL.path UTF8String];
		info->pid = frontApp.processIdentifier;
		info->icon = this->encodeNSImage(frontApp.icon);

		info->title = this->getWindowTitle(info->pid);

		return info;
	}

	bool ActiveWindow::requestScreenCaptureAccess() {
		if (@available(macOS 10.15, *)) {
			this->hasScreenCaptureAccess = CGPreflightScreenCaptureAccess();

			if (!this->hasScreenCaptureAccess) {
				// request
				this->hasScreenCaptureAccess = CGRequestScreenCaptureAccess();
			}
		}
		else {
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

			windowTitle = window[(id)kCGWindowName];
			break;
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
