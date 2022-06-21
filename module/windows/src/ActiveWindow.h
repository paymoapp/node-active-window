#include <windows.h>
#include <appmodel.h>
#include <appxpackaging.h>
#include <commctrl.h>
#include <commoncontrols.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <functional>
#include "GdiPlusUtils.h"
#include "base64.h"

#ifndef _PAYMO_ACTIVEWINDOW_H
#define _PAYMO_ACTIVEWINDOW_H

namespace PaymoActiveWindow {
	struct WindowInfo {
		std::wstring title = L"";
		std::wstring application = L"";
		std::wstring path = L"";
		unsigned int pid = 0;
		bool isUWPApp = false;
		std::wstring uwpPackage = L"";
		std::string icon = "";
	};

	typedef unsigned int watch_t;
	typedef std::function<void(WindowInfo*)> watch_callback;

	class ActiveWindow {
	public:
		ActiveWindow();
		~ActiveWindow();
		WindowInfo* getActiveWindow();
		watch_t watchActiveWindow(watch_callback cb);
		void unwatchActiveWindow(watch_t watch);
	private:
		ULONG_PTR gdiPlusToken;

		watch_t nextWatchId = 1;

		std::thread* watchThread = NULL;
		std::mutex mutex;
		std::atomic<bool> threadShouldExit;
		std::map<watch_t, watch_callback> watches;

		static inline std::mutex smutex;
		static inline std::map<HWINEVENTHOOK, ActiveWindow*> winEventProcCbCtx;

		std::wstring getWindowTitle(HWND hWindow);
		std::wstring getProcessPath(HANDLE hProc);
		std::wstring getProcessName(std::wstring path);
		std::string getWindowIcon(std::wstring path);
		std::string getUWPIcon(HANDLE hProc);
		std::wstring getUWPPackage(HANDLE hProc);
		std::wstring basename(std::wstring path);
		bool isUWPApp(std::wstring path);
		HICON getHighResolutionIcon(std::wstring path);
		IStream* getPngFromIcon(HICON hIcon);
		std::wstring getUWPPackagePath(HANDLE hProc);
		IAppxManifestProperties* getUWPPackageProperties(std::wstring pkgPath);
		std::wstring getUWPLargestIconPath(std::wstring iconPath);
		std::string encodeImageStream(IStream* pngStream);
		void runWatchThread();
		static BOOL CALLBACK EnumChildWindowsCb(HWND hWindow, LPARAM param);
		static VOID CALLBACK WinEventProcCb(HWINEVENTHOOK hHook, DWORD event, HWND hWindow, LONG idObject, LONG idChild, DWORD eventThread, DWORD eventTime);
	};

	struct EnumChildWindowsCbParam {
		ActiveWindow* aw;
		std::wstring path = L"";
		HANDLE hProc;
		bool ok = false;

		EnumChildWindowsCbParam(ActiveWindow* aw) {
			this->aw = aw;
		}
	};
}

#endif
