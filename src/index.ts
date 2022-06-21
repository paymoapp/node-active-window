import path from 'path';

import binary from '@mapbox/node-pre-gyp';

import type {
	Module,
	NativeWindowInfo,
	WindowInfo,
	IActiveWindow
} from './types';

const SUPPORTED_PLATFORMS = ['win32', 'linux', 'darwin'];

let addon: Module<NativeWindowInfo> | undefined;

if (SUPPORTED_PLATFORMS.includes(process.platform)) {
	const bindingPath = binary.find(
		path.resolve(path.join(__dirname, '../package.json'))
	);
	addon = require(bindingPath); // eslint-disable-line import/no-dynamic-require
} else {
	throw new Error(
		`Unsupported platform. The supported platforms are: ${SUPPORTED_PLATFORMS.join(
			','
		)}`
	);
}

const encodeWindowInfo = (info: NativeWindowInfo): WindowInfo => {
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
};

const ActiveWindow: IActiveWindow = {
	getActiveWindow: (): WindowInfo => {
		if (!addon) {
			throw new Error('Failed to load native addon');
		}

		const info = addon.getActiveWindow();

		return encodeWindowInfo(info);
	},
	subscribe: (callback: (windowInfo: WindowInfo) => void): number => {
		if (!addon) {
			throw new Error('Failed to load native addon');
		}

		const watchId = addon.subscribe(callback);

		return watchId;
	},
	unsubscribe: (watchId: number): void => {
		if (!addon) {
			throw new Error('Failed to load native addon');
		}

		if (watchId < 0) {
			throw new Error('Watch ID must be a positive number');
		}

		addon.unsubscribe(watchId);
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
