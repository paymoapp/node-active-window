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
					]
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
						"-fexceptions"
					]
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
		}
	]
}
