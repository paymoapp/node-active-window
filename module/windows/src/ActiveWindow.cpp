#include "./ActiveWindow.h"

namespace PaymoActiveWindow {
	ActiveWindow::ActiveWindow() {
		// initialize GDI+
		Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
		Gdiplus::GdiplusStartup(&this->gdiPlusToken, &gdiPlusStartupInput, null);
	}

	ActiveWindow::~ActiveWindow() {
		// tear down GDI+
		Gdiplus::GdiplusShutdown(this->gdiPlusToken);
	}

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

		// close process handle
		CloseHandle(hProc);

		// check if app is UWP app
		if (this->isUWPApp(info->path)) {
			EnumChildWindowsCbParam* cbParam = new EnumChildWindowsCbParam(this);
			EnumChildWindows(h, EnumChildWindowsCb, (LPARAM)cbParam);
			
			info->path = cbParam->path;
			delete cbParam;
		}

		// get app name
		info->application = this->getProcessName(info->path);
		if (info->application.size() == 0) {
			info->application = this->basename(info->path);
		}

		// get window icon
		info->icon = this->getWindowIcon(info->path);

		return info;
	}

	std::wstring ActiveWindow::getWindowTitle(HWND hWindow) {
		int len = GetWindowTextLength(hWindow);
		std::vector<wchar_t> buf(len);
		if (!GetWindowTextW(hWindow, &buf[0], len + 1)) {
			return L"";
		}
		std::wstring title(buf.begin(), buf.end());

		return title;
	}

	std::wstring ActiveWindow::getProcessPath(HANDLE hProc) {
		std::vector<wchar_t> buf(MAX_PATH);
		DWORD len = MAX_PATH;
		if (!QueryFullProcessImageNameW(hProc, 0, &buf[0], &len)) {
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

	std::string ActiveWindow::getWindowIcon(std::wstring path) {
		HICON hIcon = this->getHighResolutionIcon(path);
		
		if (hIcon == null) {
			return "";
		}

		IStream* pngStream = this->getPngFromIcon(hIcon);
		std::string iconBase64 = this->encodeImageStream(pngStream);

		pngStream->Release();

		return "data:image/png;base64," + iconBase64;
	}

	std::wstring ActiveWindow::basename(std::wstring path) {
		size_t lastSlash = path.find_last_of(L"\\");

		if (lastSlash == std::string::npos) {
			return path;
		}

		return path.substr(lastSlash + 1);
	}

	bool ActiveWindow::isUWPApp(std::wstring path) {
		return this->basename(path) == L"ApplicationFrameHost.exe";
	}

	HICON ActiveWindow::getHighResolutionIcon(std::wstring path) {
		// get file info
		SHFILEINFOW fileInfo;
		if (!SHGetFileInfoW(path.c_str(), 0, &fileInfo, sizeof(fileInfo), SHGFI_SYSICONINDEX)) {
			return null;
		}

		// get jumbo icon list
		IImageList* imgList;
		if (FAILED(SHGetImageList(SHIL_JUMBO, IID_PPV_ARGS(&imgList)))) {
			return null;
		}

		// get first icon
		HICON hIcon;
		imgList->GetIcon(fileInfo.iIcon, ILD_TRANSPARENT, &hIcon);

		imgList->Release();

		return hIcon;
	}

	IStream* ActiveWindow::getPngFromIcon(HICON hIcon) {
		// convert icon to bitmap
		ICONINFO iconInf;
		GetIconInfo(hIcon, &iconInf);

		BITMAP bmp;
		GetObject(iconInf.hbmColor, sizeof(bmp), &bmp);

		Gdiplus::Bitmap* tmp = new Gdiplus::Bitmap(iconInf.hbmColor, null);
		Gdiplus::BitmapData* lockedBitmapData = new Gdiplus::BitmapData();
		Gdiplus::Rect* rect = new Gdiplus::Rect(0, 0, tmp->GetWidth(), tmp->GetHeight());

		tmp->LockBits(rect, Gdiplus::ImageLockModeRead, tmp->GetPixelFormat(), lockedBitmapData);

		// get bitmap with transparency
		Gdiplus::Bitmap* image = new Gdiplus::Bitmap(lockedBitmapData->Width, lockedBitmapData->Height, lockedBitmapData->Stride, PixelFormat32bppARGB, reinterpret_cast<BYTE*>(lockedBitmapData->Scan0));

		// clean up
		delete tmp;
		delete lockedBitmapData;
		delete rect;

		// convert image to png
		CLSID encoderClsId;
		GdiPlusUtils::GetEncoderClsId(L"image/png", &encoderClsId);

		IStream* pngStream = SHCreateMemStream(null, 0);
		Gdiplus::Status stat = image->Save(pngStream, &encoderClsId, null);

		// prepare stream for reading
		pngStream->Commit(STGC_DEFAULT);
		LARGE_INTEGER seekPos;
		seekPos.QuadPart = 0;
		pngStream->Seek(seekPos, STREAM_SEEK_SET, null);

		delete image;

		if (stat == Gdiplus::Ok) {
			return pngStream;
		}

		// failed to save to stream
		pngStream->Release();
		return null;
	}

	std::string ActiveWindow::encodeImageStream(IStream* pngStream) {
		// get stream size
		STATSTG streamStat;
		pngStream->Stat(&streamStat, STATFLAG_NONAME);

		// convert stream to string
		std::vector<char> buf(streamStat.cbSize.QuadPart);
		long unsigned int read;
		pngStream->Read((void*)&buf[0], streamStat.cbSize.QuadPart, &read);
		
		std::string str(buf.begin(), buf.end());
		return base64_encode(str);
	}

	BOOL CALLBACK ActiveWindow::EnumChildWindowsCb(HWND hWindow, LPARAM param) {
		EnumChildWindowsCbParam* cbParam = (EnumChildWindowsCbParam*)param;

		DWORD pid;
		GetWindowThreadProcessId(hWindow, &pid);
		HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);

		if (hProc == null) {
			return true;
		}

		cbParam->path = cbParam->aw->getProcessPath(hProc);

		CloseHandle(hProc);

		if (cbParam->aw->isUWPApp(cbParam->path)) {
			return true;
		}

		return false;
	}
}
