import type {
	Module,
	NativeWindowInfo,
	WindowInfo,
	IActiveWindow
} from './types';

const SUPPORTED_PLATFORMS = ['win32', 'linux', 'darwin'];

let addon: Module<NativeWindowInfo> | undefined;

if (SUPPORTED_PLATFORMS.includes(process.platform)) {
	addon = require('../build/Release/PaymoActiveWindow.node');
} else {
	throw new Error(
		`Unsupported platform. The supported platforms are: ${SUPPORTED_PLATFORMS.join(
			','
		)}`
	);
}

const ActiveWindow: IActiveWindow = {
	getActiveWindow: (): WindowInfo => {
		if (!addon) {
			throw new Error('Failed to load native addon');
		}

		const info = addon.getActiveWindow();

		return {
			title: info.title,
			application: info.application,
			path: info.path,
			pid: info.pid,
			icon: info.icon,
			...(process.platform == 'win32'
				? {
						windows: {
							isUWPApp: info['windows.isUWPApp'] || false,
							uwpPackage: info['windows.uwpPackage'] || ''
						}
				  }
				: {})
		};
	},
	initialize: (): void => {
		if (!addon) {
			throw new Error('Failed to load native addon');
		}

		if (addon.initialize) {
			addon.initialize();
		}
	},
	requestPermissions: (): boolean => {
		if (!addon) {
			throw new Error('Failed to load native addon');
		}

		if (addon.requestPermissions) {
			return addon.requestPermissions();
		}

		return true;
	}
};

export * from './types';
export default ActiveWindow;
