#include "./ActiveWindow.h"
#include <iostream>

namespace PaymoActiveWindow {
	WindowInfo* ActiveWindow::getActiveWindow() {
		HWND h = GetForegroundWindow();

		if (h == null) {
			return null;
		}

		WindowInfo* info = new WindowInfo();

		// get window title
		info->title = this->getWindowTitle(h);

		// get process pid
		DWORD pid;
		GetWindowThreadProcessId(h, &pid);
		info->pid = (unsigned int)pid;

		// get process handle
		HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);

		if (hProc == null) {
			return null;
		}

		// get app path
		info->path = this->getProcessPath(hProc);

		// get app name
		info->application = this->getProcessName(info->path);
		if (info->application.size() == 0) {
			size_t lastSlash = info->path.find_last_of(L"\\");
			info->application = info->path.substr(lastSlash + 1);
		}

		return info;
	}

	std::wstring ActiveWindow::getWindowTitle(HWND h) {
		int len = GetWindowTextLength(h);
		std::vector<wchar_t> buf(len);
		if (!GetWindowTextW(h, &buf[0], len + 1)) {
			return L"";
		}
		std::wstring title(buf.begin(), buf.end());

		return title;
	}

	std::wstring ActiveWindow::getProcessPath(HANDLE h) {
		std::vector<wchar_t> buf(MAX_PATH);
		DWORD len = MAX_PATH;
		if (!QueryFullProcessImageNameW(h, 0, &buf[0], &len)) {
			return L"";
		}
		std::wstring name(buf.begin(), buf.begin() + len);

		return name;
	}

	std::wstring ActiveWindow::getProcessName(std::wstring path) {
		DWORD infoSize = GetFileVersionInfoSizeW(path.c_str(), null);

		if (!infoSize) {
			return L"";
		}

		LPBYTE data = new BYTE[infoSize];
		if (!GetFileVersionInfoW(path.c_str(), 0, infoSize, data)) {
			delete data;
			return L"";
		}

		struct LANGCODEPAGE {
			WORD lang;
			WORD codePage;
		} *langData, active;

		active.lang = 0x0409;
		active.codePage = 0x04E4;

		unsigned int langDataLen = 0;
		if (VerQueryValueW(data, L"\\VarFileInfo\\Translation", (void**)&langData, &langDataLen)) {
			active.lang = langData[0].lang;
			active.codePage = langData[0].codePage;
		}

		// build path to query file description
		std::wstringstream localePath;
		std::ios_base::fmtflags flags(localePath.flags());

		localePath<<L"\\StringFileInfo\\";
		localePath<<std::uppercase<<std::setfill(L'0')<<std::setw(4)<<std::hex<<active.lang;
		localePath<<std::uppercase<<std::setfill(L'0')<<std::setw(4)<<std::hex<<active.codePage;
		localePath.flags(flags);
		localePath<<L"\\FileDescription";

		void* localDesc;
		unsigned int localDescLen = 0;
		if (!VerQueryValueW(data, localePath.str().c_str(), &localDesc, &localDescLen)) {
			delete data;
			return L"";
		}

		std::wstring name((wchar_t*)localDesc);

		delete data;
		return name;
	}
}