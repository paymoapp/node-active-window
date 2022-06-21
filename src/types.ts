export interface NativeWindowInfo {
	title: string;
	application: string;
	path: string;
	pid: number;
	icon: string;
	'windows.isUWPApp'?: boolean;
	'windows.uwpPackage'?: string;
}

export interface WindowInfo {
	title: string;
	application: string;
	path: string;
	pid: number;
	icon: string;
	windows?: {
		isUWPApp: boolean;
		uwpPackage: string;
	};
}

export interface Module<T> {
	getActiveWindow(): T;
	subscribe(callback: (windowInfo: WindowInfo) => void): number;
	unsubscribe(watchId: number): void;
	initialize?(): void;
	requestPermissions?(): boolean;
}

export interface IActiveWindow extends Module<WindowInfo> {
	initialize(): void;
	requestPermissions(): boolean;
}
