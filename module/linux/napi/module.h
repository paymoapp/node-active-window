#include <napi.h>
#include "../src/ActiveWindow.h"

#ifndef _PAYMO_MODULE_H
#define _PAYMO_MODULE_H

namespace module {
	void Init(Napi::Env env, Napi::Object exports);

	Napi::Value getActiveWindow(const Napi::CallbackInfo& info);
	Napi::Value subscribe(const Napi::CallbackInfo& info);
	void unsubscribe(const Napi::CallbackInfo& info);
	void initialize(const Napi::CallbackInfo& info);

	// helpers
	Napi::Object encodeWindowInfo(Napi::Env env, PaymoActiveWindow::WindowInfo* windowInfo);
	void tsfnMainThreadCallback(Napi::Env env, Napi::Function jsCallback, void* ctx, PaymoActiveWindow::WindowInfo* data);
}

#endif
