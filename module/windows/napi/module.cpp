#include "module.h"

Napi::Object module::Init(Napi::Env env, Napi::Object exports) {
	env.SetInstanceData(new PaymoActiveWindow::ActiveWindow());

	exports.Set("getActiveWindow", Napi::Function::New(env, module::getActiveWindow));

	return exports;
}

Napi::Object module::getActiveWindow(const Napi::CallbackInfo& info) {
	PaymoActiveWindow::ActiveWindow* activeWindow = info.Env().GetInstanceData<PaymoActiveWindow::ActiveWindow>();
	if (activeWindow == NULL) {
		Napi::Error::New(info.Env(), "ActiveWindow module not initialized").ThrowAsJavaScriptException();
		return Napi::Object::Object();
	}

	PaymoActiveWindow::WindowInfo* windowInfo = activeWindow->getActiveWindow();
	if (windowInfo == NULL) {
		Napi::Error::New(info.Env(), "Failed to get active window").ThrowAsJavaScriptException();
		return Napi::Object::Object();
	}

	Napi::Object result = Napi::Object::New(info.Env());
	result.Set("title", Napi::String::New(info.Env(), std::u16string(windowInfo->title.begin(), windowInfo->title.end())));
	result.Set("application", Napi::String::New(info.Env(), std::u16string(windowInfo->application.begin(), windowInfo->application.end())));
	result.Set("path", Napi::String::New(info.Env(), std::u16string(windowInfo->path.begin(), windowInfo->path.end())));
	result.Set("pid", Napi::Number::New(info.Env(), windowInfo->pid));
	result.Set("windows.isUWPApp", Napi::Boolean::New(info.Env(), windowInfo->isUWPApp));
	result.Set("windows.uwpPackage", Napi::String::New(info.Env(), std::u16string(windowInfo->uwpPackage.begin(), windowInfo->uwpPackage.end())));
	result.Set("icon", Napi::String::New(info.Env(), windowInfo->icon));

	delete windowInfo;

	return result;
}
