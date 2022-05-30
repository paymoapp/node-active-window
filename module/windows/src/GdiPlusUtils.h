#include <windows.h>
#include <gdiplus.h>

#ifndef null
#define null 0
#endif

namespace GdiPlusUtils {
	int GetEncoderClsId(const WCHAR* format, CLSID* pClsid);
}
