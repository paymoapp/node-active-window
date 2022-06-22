{
	"targets": [
		{
			"target_name": "<(module_name)",
			"conditions": [
				["OS=='win'", {
					"sources": [
						"module/windows/napi/main.cpp",
						"module/windows/napi/module.cpp",
						"module/windows/src/ActiveWindow.cpp",
						"module/windows/src/base64.cpp",
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
								"-std:c++20"
							]
						}
					}
				}],
				["OS=='linux'", {
					"sources": [
						"module/linux/napi/main.cpp",
						"module/linux/napi/module.cpp",
						"module/linux/src/ActiveWindow.cpp",
						"module/linux/src/base64.cpp",
						"module/linux/src/ConvertUTF.c"
					],
					"libraries": [
						"-lX11",
						"-licuuc"
					],
					"cflags_cc": [
						"-fexceptions",
						"--std=c++2a"
					]
				}],
				["OS=='mac'", {
					"sources": [
						"module/macos/napi/main.mm",
						"module/macos/napi/module.mm",
						"module/macos/src/ActiveWindow.mm",
						"module/macos/src/base64.cpp"
					],
					"libraries": [
						"-lc++",
						"-framework Foundation",
						"-framework AppKit",
						"-framework ApplicationServices"
					],
					"xcode_settings": {
						"CLANG_CXX_LANGUAGE_STANDARD": "c++20",
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
				"NAPI_DISABLE_CPP_EXCEPTIONS"
			]
		},
		{
			"target_name": "copy_artifacts",
			"type": "none",
			"dependencies": [
				"<(module_name)"
			],
			"copies": [
				{
					"files": [
						"<(PRODUCT_DIR)/<(module_name).node"
					],
					"destination": "<(module_path)"
				}
			]
		}
	]
}
