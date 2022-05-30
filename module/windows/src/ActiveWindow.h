#include <windows.h>
#include <commctrl.h>
#include <commoncontrols.h>
#include <gdiplus.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "GdiPlusUtils.h"

#ifndef null
#define null 0
#endif

namespace PaymoActiveWindow {
	struct WindowInfo {
		std::wstring title = L"";
		std::wstring application = L"";
		std::wstring path = L"";
		unsigned int pid = 0;
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
		void getWindowIcon(std::wstring path);
		std::wstring basename(std::wstring path);
		bool isUWPApp(std::wstring path);
		HICON getHighResolutionIcon(std::wstring path);
		void getPngFromIcon(HICON hIcon);
		static BOOL CALLBACK EnumChildWindowsCb(HWND hWindow, LPARAM param);
	};

	struct EnumChildWindowsCbParam {
		ActiveWindow* aw;
		std::wstring path = L"";

		EnumChildWindowsCbParam(ActiveWindow* aw) {
			this->aw = aw;
		}
	};
}
