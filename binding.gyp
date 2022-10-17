{
	"targets": [
		{
			"target_name": "PaymoActiveWindow",
			"conditions": [
				["OS=='win'", {
					"sources": [
						"module/windows/napi/main.cpp",
						"module/windows/napi/module.cpp",
						"module/windows/src/ActiveWindow.cpp",
						"module/windows/src/IconCache.cpp",
						"module/windows/src/base64/base64.cpp",
						"module/windows/src/GdiPlusUtils.cpp"
					],
					"libraries": [
						"User32.lib",
						"Shell32.lib",
						"Version.lib",
						"Shlwapi.lib",
						"Gdi32.lib",
						"Gdiplus.lib",
						"Windowsapp.lib"
					],
					"msvs_settings": {
						"VCCLCompilerTool": {
							"AdditionalOptions": [
								"/std:c++17"
							]
						}
					}
				}],
				["OS=='linux'", {
					"sources": [
						"module/linux/napi/main.cpp",
						"module/linux/napi/module.cpp",
						"module/linux/src/ActiveWindow.cpp",
						"module/linux/src/IconCache.cpp",
						"module/linux/src/base64/base64.cpp",
						"module/linux/src/SimpleIni/ConvertUTF.c",
						"module/linux/src/lodepng/lodepng.cpp"
					],
					"libraries": [
						"-lX11",
						"-lpthread",
						"-static-libstdc++"
					],
					"cflags_cc": [
						"-fexceptions",
						"--std=c++17"
					]
				}],
				["OS=='mac'", {
					"sources": [
						"module/macos/napi/main.mm",
						"module/macos/napi/module.mm",
						"module/macos/src/ActiveWindow.mm",
						"module/macos/src/IconCache.cpp",
						"module/macos/src/base64/base64.cpp"
					],
					"libraries": [
						"-lc++",
						"-framework Foundation",
						"-framework AppKit",
						"-framework ApplicationServices"
					],
					"xcode_settings": {
						"CLANG_CXX_LANGUAGE_STANDARD": "c++17",
						"GCC_ENABLE_CPP_EXCEPTIONS": "YES"
					}
				}]
			],
			"include_dirs": [
				"<!@(node -p \"require('node-addon-api').include\")"
			],
			"libraries": [],
			"dependencies": [
				"<!(node -p \"require('node-addon-api').gyp\")"
			],
			"defines": [
				"NAPI_DISABLE_CPP_EXCEPTIONS",
				"NAPI_VERSION=<(napi_build_version)"
			]
		}
	]
}
