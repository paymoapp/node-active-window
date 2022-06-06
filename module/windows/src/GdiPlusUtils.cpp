#include "GdiPlusUtils.h"

namespace GdiPlusUtils {
	int GetEncoderClsId(const WCHAR* format, CLSID* pClsid) {
		UINT num = 0;
		UINT size = 0;

		Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

		Gdiplus::GetImageEncodersSize(&num, &size);
		if (size == 0) {
			return -1;
		}

		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL) {
			return -1;
		}

		Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; j++) {
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;
			}
		}

		free(pImageCodecInfo);
		return -1;
	}
}
