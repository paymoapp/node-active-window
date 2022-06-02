#include "./ActiveWindow.h"

namespace PaymoActiveWindow {
	ActiveWindow::ActiveWindow() {
		// initialize GDI+
		Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
		Gdiplus::GdiplusStartup(&this->gdiPlusToken, &gdiPlusStartupInput, null);

		// initialize COM
		CoInitializeEx(null, COINIT_MULTITHREADED);
	}

	ActiveWindow::~ActiveWindow() {
		// tear down GDI+
		Gdiplus::GdiplusShutdown(this->gdiPlusToken);

		// tear down COM
		CoUninitialize();
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
		CloseHandle(hProc);

		// check if app is UWP app
		if (this->isUWPApp(info->path)) {
			info->isUWPApp = true;

			EnumChildWindowsCbParam* cbParam = new EnumChildWindowsCbParam(this);
			EnumChildWindows(h, EnumChildWindowsCb, (LPARAM)cbParam);
			
			info->path = cbParam->path;
			// save handle of UWP process
			hProc = cbParam->hProc;
			delete cbParam;
		}

		// get app name
		info->application = this->getProcessName(info->path);
		if (info->application.size() == 0) {
			info->application = this->basename(info->path);
		}

		if (info->isUWPApp) {
			info->uwpPackage = this->getUWPPackage(hProc);
			info->icon = this->getUWPIcon(hProc);

			// we need to close the handle of the UWP process
			CloseHandle(hProc);
		}
		else {
			// get window icon
			info->icon = this->getWindowIcon(info->path);
		}

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

	std::string ActiveWindow::getUWPIcon(HANDLE hProc) {
		std::wstring pkgPath = this->getUWPPackagePath(hProc);
		IAppxManifestProperties* properties = this->getUWPPackageProperties(pkgPath);

		if (properties == null) {
			return "";
		}

		LPWSTR logo = null;
		properties->GetStringValue(L"Logo", &logo);
		properties->Release();
		std::wstring logoPath = pkgPath + L"\\" + logo;

		if (!PathFileExistsW(logoPath.c_str())) {
			// we need to use scale 100
			size_t dotPos = logoPath.find_last_of(L".");
			logoPath.insert(dotPos, L".scale-100");
		}

		IStream* pngStream = null;
		if (FAILED(SHCreateStreamOnFileEx(logoPath.c_str(), STGM_READ | STGM_SHARE_EXCLUSIVE, 0, FALSE, null, &pngStream))) {
			return "";
		}

		std::string iconBase64 = this->encodeImageStream(pngStream);

		pngStream->Release();

		return "data:image/png;base64," + iconBase64;
	}

	std::wstring ActiveWindow::getUWPPackage(HANDLE hProc) {
		UINT32 len = 0;
		GetPackageFullName(hProc, &len, null);

		std::vector<wchar_t> buf(len);
		GetPackageFullName(hProc, &len, &buf[0]);

		std::wstring package(buf.begin(), buf.begin() + len - 1);

		return package;
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

	std::wstring ActiveWindow::getUWPPackagePath(HANDLE hProc) {
		UINT32 pkgIdLen = 0;
		GetPackageId(hProc, &pkgIdLen, null);
		PACKAGE_ID* pkgId = (PACKAGE_ID*)malloc(pkgIdLen);
		GetPackageId(hProc, &pkgIdLen, (BYTE*)pkgId);

		UINT32 len = 0;
		GetPackagePath(pkgId, 0, &len, null);

		std::vector<wchar_t> buf(len);
		GetPackagePath(pkgId, 0, &len, &buf[0]);

		std::wstring pkgPath(buf.begin(), buf.begin() + len - 1);

		free(pkgId);

		return pkgPath;
	}

	IAppxManifestProperties* ActiveWindow::getUWPPackageProperties(std::wstring pkgPath) {
		IAppxFactory* appxFactory = null;
		if (FAILED(CoCreateInstance(__uuidof(AppxFactory), null, CLSCTX_INPROC_SERVER, __uuidof(IAppxFactory), (LPVOID*)&appxFactory))) {
			return null;
		}

		IStream* manifestStream;
		std::wstring manifestPath = pkgPath + L"\\AppxManifest.xml";
		if (FAILED(SHCreateStreamOnFileEx(manifestPath.c_str(), STGM_READ | STGM_SHARE_EXCLUSIVE, 0, FALSE, null, &manifestStream))) {
			appxFactory->Release();
			return null;
		}

		IAppxManifestReader* manifestReader = null;
		if (FAILED(appxFactory->CreateManifestReader(manifestStream, &manifestReader))) {
			appxFactory->Release();
			manifestStream->Release();
			return null;
		}

		IAppxManifestProperties* properties = null;
		if (FAILED(manifestReader->GetProperties(&properties))) {
			appxFactory->Release();
			manifestStream->Release();
			manifestReader->Release();
			return null;
		}
		
		appxFactory->Release();
		manifestStream->Release();
		manifestReader->Release();
		return properties;
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
		cbParam->hProc = hProc;

		if (cbParam->aw->isUWPApp(cbParam->path)) {
			CloseHandle(hProc);
			return true;
		}

		return false;
	}
}