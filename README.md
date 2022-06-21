# node-active-win

NodeJS library using native modules to get the active window and some metadata (including the application icon) on Windows, MacOS and Linux.

### Table of Contents

- [Native libraries](#native-libraries)
  - [Linux (`module/linux`)](#linux-modulelinux)
    - [Data structures](#data-structures)
    - [Public functions](#public-functions)
    - [Example](#example)
    - [Building](#building)

## Native libraries

You can import the each platform dependent library as a standalone C++ / Objective-C++ library. You can find the library itself in the module's `src` directory. The `napi` directory contains the Node-API bindings and the `demo` directory contains a small demo program that can be built using a Makefile.

You can build the demo by navigating to the `module/<platform>/demo` folder and executing `make`. You can run the demo using `make run` and clean the build artifacts using `make clean`.

The demo has 3 running modes:
- _default_: `make run` - in this mode the library is used to fetch the current window details, then there's a 3 second delay after which the current window is fetched again
- _loop_: `make run MODE=loop` - in this mode the library is used the poll the current window in every 3 seconds until SIGINT (Ctrl+C) is received
- _watch_: `make run MODE=watch` - in this mode the library is used to watch the current window and it's title. There's no polling involved in this mode

### Linux (`module/linux`)

#### Data structures

###### 🗃 &nbsp;&nbsp; WindowInfo

```c++
struct WindowInfo {
	std::string title; // UTF8 encoded string of window title. Empty string if couldn't fetch
	std::string application; // UTF8 encoded string of application name. Empty string if couldn't fetch
	std::string path; // UTF8 encoded string of application path. Empty string if couldn't fetch
	int pid; // PID of process. -1 if couldn't fetch
	std::string icon; // base64 encoded PNG icon. Empty string if couldn't fetch
}
```

###### 🗃 &nbsp;&nbsp; watch_t

```c++
typedef unsigned int watch_t;
```

###### 🗃 &nbsp;&nbsp; watch_callback

```c++
typedef std::function<void(WindowInfo*)> watch_callback;
```

#### Public functions

###### 𝑓 &nbsp;&nbsp; Constructor

```c++
ActiveWindow();
```

###### 𝑓 &nbsp;&nbsp; getActiveWindow

```c++
WindowInfo* getActiveWindow();
```

Returns pointer to WindowInfo containing the gathered information about the current window. The pointer can be `NULL` in the case of an error or if there is no active window (ex: all the windows are minified). You should free up the allocated WindowInfo object using `delete`.

###### 𝑓 &nbsp;&nbsp; buildAppCache

```c++
void buildAppCache();
```

Gathers all the `.desktop` entries available on the system (starting from `~/.local/share/applications` through each `$XDG_DATA_DIRS/applications`) and resolves the icon path for them. This cache is used to get the icon for a given window. If this function is not called, then no icons will be resolved.

###### 𝑓 &nbsp;&nbsp; watchActiveWindow

```c++
watch_t watchActiveWindow(watch_callback cb);
```

Sets up a watch for the active window. If there's a change in the current active window, or the title of the active window, the callback will be fired with the current active window. You don't need to call `getActiveWindow()` in the callback, you can use the supplied parameter.

This method will also start a background watch thread if it's not already running. Please note that the callbacks will be executed on this thread, so you should assure thread safety.

You __MUST NOT__ free up the WindowInfo object received in the parameter. If you need to store the active window you __SHOULD__ make a copy of it, since the WindowInfo object will be freed after calling all the callbacks.

You should save the returned watch ID to unsubscribe later.

###### 𝑓 &nbsp;&nbsp; unwatchActiveWindow

```c++
void unwatchActiveWindow(watch_t watch);
```

Removes the watch associated with the supplied watch ID.

The background watch thread will not be closed, even if there're no more watches left. It will only be closed when the class's destructor is called.

#### Example

See `module/linux/demo.cpp` for an example.

```c++
#include <iostream>
#include "ActiveWindow.h"

using namespace std;
using namespace PaymoActiveWindow;

int main() {
	ActiveWindow* activeWindow = new ActiveWindow();

	WindowInfo* windowInfo = activeWindow->getActiveWindow();

	if (windowInfo == NULL) {
		cout<<"Could not get active window\n";
	}
	else {
		cout<<"Title: "<<windowInfo->title<<"\n";
		cout<<"Application: "<<windowInfo->application<<"\n";
		cout<<"Executable path: "<<windowInfo->path<<"\n";
		cout<<"PID: "<<windowInfo->pid<<"\n";
		cout<<"Icon: "<<windowInfo->icon<<"\n";
	}

	delete windowInfo;
	delete activeWindow;

	return 0;
}
```

#### Building

See `module/linux/Makefile` for a sample makefile. You need to check the `lib` target.

You should use C++20 for building and you need to link the following libraries:
- X11 (`-lX11`) - libx11-dev
- ICU-UC (`-licuuc`) - libicu-dev
- PThread (`-lpthread`) - libpthread-stubs0-dev
