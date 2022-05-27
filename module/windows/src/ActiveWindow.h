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
		std::wstring getWindowTitle(HWND hWindow);
		std::wstring getProcessPath(HANDLE hProc);
		std::wstring getProcessName(std::wstring path);
		std::wstring basename(std::wstring path);
		bool isUWPApp(std::wstring path);
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
