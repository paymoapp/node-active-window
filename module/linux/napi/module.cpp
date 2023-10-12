#include "module.h"

void module::Init(Napi::Env env, Napi::Object exports) {
	env.SetInstanceData(new PaymoActiveWindow::ActiveWindow(15));

	exports.Set("getActiveWindow", Napi::Function::New<module::getActiveWindow>(env));
	exports.Set("subscribe", Napi::Function::New<module::subscribe>(env));
	exports.Set("unsubscribe", Napi::Function::New<module::unsubscribe>(env));
	exports.Set("initialize", Napi::Function::New<module::initialize>(env));
}

Napi::Value module::getActiveWindow(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	PaymoActiveWindow::ActiveWindow* activeWindow = env.GetInstanceData<PaymoActiveWindow::ActiveWindow>();
	if (activeWindow == NULL) {
		Napi::Error::New(env, "ActiveWindow module not initialized").ThrowAsJavaScriptException();
		return env.Null();
	}

	PaymoActiveWindow::WindowInfo* windowInfo = activeWindow->getActiveWindow();
	if (windowInfo == NULL) {
		Napi::Error::New(env, "Failed to get active window").ThrowAsJavaScriptException();
		return env.Null();
	}

	Napi::Object result = encodeWindowInfo(env, windowInfo);

	delete windowInfo;

	return result;
}

Napi::Value module::subscribe(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	PaymoActiveWindow::ActiveWindow* activeWindow = env.GetInstanceData<PaymoActiveWindow::ActiveWindow>();
	if (activeWindow == NULL) {
		Napi::Error::New(env, "ActiveWindow module not initialized").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (info.Length() != 1) {
		Napi::TypeError::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
		return env.Null();
	}

	if (!info[0].IsFunction()) {
		Napi::TypeError::New(env, "Expected first argument to be function").ThrowAsJavaScriptException();
		return env.Null();
	}

	Napi::TypedThreadSafeFunction tsfn = Napi::TypedThreadSafeFunction<
		void,
		PaymoActiveWindow::WindowInfo,
		tsfnMainThreadCallback
	>::New(env, info[0].As<Napi::Function>(), "Active Window Callback", 0, 1);

	PaymoActiveWindow::watch_t watchId = activeWindow->watchActiveWindow([tsfn](PaymoActiveWindow::WindowInfo* windowInfo) {
		if (windowInfo == NULL) {
			tsfn.BlockingCall((PaymoActiveWindow::WindowInfo*)NULL);
			return;
		}

		// clone window info
		PaymoActiveWindow::WindowInfo* arg = new PaymoActiveWindow::WindowInfo();
		*arg = *windowInfo;

		tsfn.BlockingCall(arg);
	});

	return Napi::Number::New(env, watchId);
}

void module::unsubscribe(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	PaymoActiveWindow::ActiveWindow* activeWindow = env.GetInstanceData<PaymoActiveWindow::ActiveWindow>();
	if (activeWindow == NULL) {
		Napi::Error::New(env, "ActiveWindow module not initialized").ThrowAsJavaScriptException();
		return;
	}

	if (info.Length() != 1) {
		Napi::TypeError::New(env, "Expected 1 argument").ThrowAsJavaScriptException();
		return;
	}

	if (!info[0].IsNumber()) {
		Napi::TypeError::New(env, "Expected first argument to be number").ThrowAsJavaScriptException();
		return;
	}

	PaymoActiveWindow::watch_t watchId = info[0].As<Napi::Number>().Uint32Value();

	activeWindow->unwatchActiveWindow(watchId);
}

void module::initialize(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();

	PaymoActiveWindow::ActiveWindow* activeWindow = env.GetInstanceData<PaymoActiveWindow::ActiveWindow>();
	if (activeWindow == NULL) {
		Napi::Error::New(env, "ActiveWindow module not initialized").ThrowAsJavaScriptException();
		return;
	}

	activeWindow->buildAppCache();
}

Napi::Object module::encodeWindowInfo(Napi::Env env, PaymoActiveWindow::WindowInfo* windowInfo) {
	Napi::Object result = Napi::Object::New(env);

	result.Set("title", Napi::String::New(env, windowInfo->title));
	result.Set("application", Napi::String::New(env, windowInfo->application));
	result.Set("path", Napi::String::New(env, windowInfo->path));
	result.Set("pid", Napi::Number::New(env, windowInfo->pid));
	result.Set("icon", Napi::String::New(env, windowInfo->icon));

	return result;
}

void module::tsfnMainThreadCallback(Napi::Env env, Napi::Function jsCallback, void* ctx, PaymoActiveWindow::WindowInfo* data) {
	if (data == NULL) {
		jsCallback.Call({ env.Null() });
	}
	else {
		jsCallback.Call({ encodeWindowInfo(env, data) });
		delete data;
	}
}
