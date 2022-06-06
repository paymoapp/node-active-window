#include <windows.h>
#include <gdiplus.h>

#ifndef _PAYMO_GDIPLUSUTILS_H
#define _PAYMO_GDIPLUSUTILS_H

namespace GdiPlusUtils {
	int GetEncoderClsId(const WCHAR* format, CLSID* pClsid);
}

#endif
