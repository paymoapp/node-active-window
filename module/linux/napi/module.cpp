#include "module.h"

Napi::Object module::Init(Napi::Env env, Napi::Object exports) {
	env.SetInstanceData(new PaymoActiveWindow::ActiveWindow());

	exports.Set("getActiveWindow", Napi::Function::New(env, module::getActiveWindow));
	exports.Set("initialize", Napi::Function::New(env, module::initialize));

	return exports;
}

Napi::Object module::getActiveWindow(const Napi::CallbackInfo& info) {
	PaymoActiveWindow::ActiveWindow* activeWindow = info.Env().GetInstanceData<PaymoActiveWindow::ActiveWindow>();
	if (activeWindow == NULL) {
		Napi::Error::New(info.Env(), "ActiveWindow module not initialized").ThrowAsJavaScriptException();
		return Napi::Object::New(info.Env());
	}

	PaymoActiveWindow::WindowInfo* windowInfo = activeWindow->getActiveWindow();
	if (windowInfo == NULL) {
		Napi::Error::New(info.Env(), "Failed to get active window").ThrowAsJavaScriptException();
		return Napi::Object::New(info.Env());
	}

	Napi::Object result = Napi::Object::New(info.Env());
	result.Set("title", Napi::String::New(info.Env(), windowInfo->title));
	result.Set("application", Napi::String::New(info.Env(), windowInfo->application));
	result.Set("path", Napi::String::New(info.Env(), windowInfo->path));
	result.Set("pid", Napi::Number::New(info.Env(), windowInfo->pid));
	result.Set("icon", Napi::String::New(info.Env(), windowInfo->icon));

	delete windowInfo;

	return result;
}

void module::initialize(const Napi::CallbackInfo& info) {
	PaymoActiveWindow::ActiveWindow* activeWindow = info.Env().GetInstanceData<PaymoActiveWindow::ActiveWindow>();
	if (activeWindow == NULL) {
		Napi::Error::New(info.Env(), "ActiveWindow module not initialized").ThrowAsJavaScriptException();
		return;
	}

	activeWindow->buildAppCache();
}
