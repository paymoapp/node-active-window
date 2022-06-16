#include "ActiveWindow.h"

namespace PaymoActiveWindow {
	WindowInfo* ActiveWindow::getActiveWindow() {
		NSRunningApplication* frontApp = [NSWorkspace sharedWorkspace].frontmostApplication;

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

	std::string ActiveWindow::encodeNSImage(NSImage* img) {
		CGImageRef cgRef = [img CGImageForProposedRect:NULL context:nil hints:nil];
		NSBitmapImageRep* imgRep = [[NSBitmapImageRep alloc] initWithCGImage:cgRef];
		[imgRep setSize:[img size]];
		NSData* pngData = [imgRep representationUsingType:NSBitmapImageFileTypePNG properties:nil];

		unsigned int len = [pngData length];
		std::vector<char> buf(len);
		memcpy(buf.data(), [pngData bytes], len);

		[imgRep release];
		[pngData release];

		std::string pngImg(buf.begin(), buf.end());

		return "data:image/png;base64," + base64_encode(pngImg);
	}

	std::string ActiveWindow::getWindowTitle(int pid) {
		if (!this->hasScreenCaptureAccess) {
			// no title information without screen recording permission
			return "";
		}

		CFArrayRef windows = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);

		for (NSDictionary* window in (NSArray*)windows) {
			NSNumber* windowPid = window[(id)kCGWindowOwnerPID];
			int wPid = [windowPid intValue];

			if (wPid != pid) {
				// not our window
				continue;
			}

			NSString* windowTitle = window[(id)kCGWindowName];

			if (windowTitle == nil) {
				return "";
			}

			std::string title([windowTitle UTF8String]);

			return title;
		}

		return "";
	}
}
