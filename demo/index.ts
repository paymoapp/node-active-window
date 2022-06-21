/* eslint-disable @typescript-eslint/no-explicit-any */
/* eslint-disable no-console */

import http from 'http';
import fs from 'fs';
import path from 'path';

import ws from 'ws';

import ActiveWindow, { WindowInfo } from '../src';

ActiveWindow.initialize();
ActiveWindow.requestPermissions();

const server = http.createServer((req, res) => {
	fs.readFile(path.join(__dirname, 'demo.html'), (err, data) => {
		if (err) {
			res.writeHead(500);
			res.end('Internal server error');
			return;
		}

		res.writeHead(200);
		res.end(data);
	});
});

const wss = new ws.Server({ server: server, path: '/ws' });
const broadcastData = (wss: ws.Server, data: any) => {
	wss.clients.forEach(client => {
		if (client.readyState == ws.OPEN) {
			client.send(JSON.stringify(data));
		}
	});
};

server.listen(3000, () => {
	console.log('Started listening');
	console.log('Open http://localhost:3000 to get started');
});

let winInfo: WindowInfo | null = null;

const objectDeepCompare = (a: any, b: any): boolean => {
	return Object.keys(a).reduce((acc: boolean, cur) => {
		if (cur == 'icon') {
			return acc;
		}

		if (!b) {
			return false;
		}

		if (typeof a[cur] == 'object') {
			return acc && objectDeepCompare(a[cur], b[cur]);
		}

		return acc && a[cur] == b[cur];
	}, true);
};

const watchId = ActiveWindow.subscribe(curWinInfo => {
	console.log('Got new window info');
	if (curWinInfo == null) {
		console.log('Got null window');
		return;
	}

	if (!objectDeepCompare(curWinInfo, winInfo)) {
		// different
		console.log('Broadcasting changes...');
		broadcastData(wss, curWinInfo);
		winInfo = curWinInfo;
	}
});

console.log('Started watching with ID:', watchId);

process.on('SIGINT', () => {
	console.log('Closing');
	ActiveWindow.unsubscribe(watchId);

	wss.clients.forEach(client => {
		client.terminate();
	});

	wss.close();
	server.close();

	setTimeout(() => {
		console.log('Exited');
		process.exit(0);
	}, 1000);
});
