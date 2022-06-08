/* eslint-disable @typescript-eslint/no-explicit-any */
/* eslint-disable no-console */

import http from 'http';
import fs from 'fs';
import path from 'path';

import ws from 'ws';

import ActiveWindow, { WindowInfo } from '../src';

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
		if(cur == 'icon') {
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

const checkInterval = setInterval(() => {
	console.log('Checking window info');
	try {
		const curWinInfo = ActiveWindow.getActiveWindow();
		console.log('ok');
		console.log(curWinInfo);

		if (!objectDeepCompare(curWinInfo, winInfo)) {
			// different
			console.log('Broadcasting changes...');
			broadcastData(wss, curWinInfo);
			winInfo = curWinInfo;
		}
	} catch(e) {
		console.log('ERROR:', e);
	}
}, 3000);

process.on('SIGINT', () => {
	console.log('Closing');
	clearInterval(checkInterval);

	wss.clients.forEach(client => {
		client.terminate();
	})

	wss.close();
	server.close();

	console.log('Exited');
});

