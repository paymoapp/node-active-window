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

	class ActiveWindow {
	public:
		ActiveWindow();
		~ActiveWindow();
		WindowInfo* getActiveWindow();
	private:
		ULONG_PTR gdiPlusToken;

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
		static BOOL CALLBACK EnumChildWindowsCb(HWND hWindow, LPARAM param);
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
