#include <napi.h>
#include "../src/ActiveWindow.h"

#ifndef _PAYMO_MODULE_H
#define _PAYMO_MODULE_H

namespace module {
	Napi::Object Init(Napi::Env env, Napi::Object exports);

	Napi::Object getActiveWindow(const Napi::CallbackInfo& info);
	Napi::Number subscribe(const Napi::CallbackInfo& info);
	void unsubscribe(const Napi::CallbackInfo& info);
	Napi::Boolean requestPermissions(const Napi::CallbackInfo& info);
	void runLoop(const Napi::CallbackInfo& info);

	// helpers
	Napi::Object encodeWindowInfo(Napi::Env env, PaymoActiveWindow::WindowInfo* windowInfo);
}

#endif
