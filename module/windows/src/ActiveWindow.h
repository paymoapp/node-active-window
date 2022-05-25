#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#define null 0

namespace PaymoActiveWindow {
	struct WindowInfo {
		std::wstring title = L"";
		std::wstring application = L"";
		std::wstring path = L"";
		unsigned int pid = 0;
	};

	class ActiveWindow {
	public:
		WindowInfo* getActiveWindow();
	private:
		std::wstring getWindowTitle(HWND h);
		std::wstring getProcessPath(HANDLE h);
		std::wstring getProcessName(std::wstring path);
	};
}