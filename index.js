const net = require('net');
const cluster = require('cluster');
const os = require('os');

const PORT = 3000;
const HOST = '0.0.0.0';

const HEADER_END = Buffer.from('\r\n\r\n');

if (cluster.isMaster) {
    const cpus = os.cpus().length;
    const workersCount = Math.max(1, Math.floor(cpus * 1.5));
    console.log(`Master ${process.pid} running. Forking ${workersCount} workers...`);

    for (let i = 0; i < workersCount; i++) {
        const worker = cluster.fork();
        worker.on('message', (msg) => {
            if (msg === 'ready') {
                console.log(`Worker ${worker.process.pid} is ready`);
            }
        });
    }

    cluster.on('exit', (worker, code, signal) => {
        console.log(`Worker ${worker.process.pid} died. Restarting after 1s...`);
        setTimeout(() => cluster.fork(), 1000);
    });

    return; // Master process burada işi bitiriyor
}

process.setMaxListeners(0);

const server = net.createServer((socket) => {
    socket.setNoDelay(true);
    socket.setKeepAlive(true, 60000);

    let buffer = Buffer.allocUnsafe(0);
    let offset = 0;

    let isWriting = false;
    const writeQueue = [];

    function writeData(data) {
        if (isWriting) {
            if (writeQueue.length > 1000) {
                // Kuyruk aşırı dolduysa bağlantıyı kapat
                socket.destroy();
                return;
            }
            writeQueue.push(data);
            return;
        }
        isWriting = !socket.write(data);
    }

    socket.on('drain', () => {
        isWriting = false;
        while (!isWriting && writeQueue.length) {
            const d = writeQueue.shift();
            isWriting = !socket.write(d);
        }
    });

    socket.on('error', (err) => {
        if (!['ECONNRESET', 'EPIPE'].includes(err.code)) {
            console.error('Socket error:', err);
        }
        socket.destroy();
    });

    socket.on('close', () => {
        buffer = null;
        writeQueue.length = 0;
    });

    function findHeaderEnd() {
        return buffer.indexOf(HEADER_END, offset);
    }

    function tryParseRequest() {
        const headerEnd = findHeaderEnd();
        if (headerEnd === -1) return null;

        const headerPart = buffer.slice(offset, headerEnd).toString('ascii');
        const lines = headerPart.split('\r\n');
        const [method, url] = lines[0].split(' ');
        const headers = {};
        for (let i = 1; i < lines.length; i++) {
            const line = lines[i];
            const idx = line.indexOf(':');
            if (idx === -1) continue;
            const key = line.slice(0, idx).trim().toLowerCase();
            const val = line.slice(idx + 1).trim();
            headers[key] = val;
        }

        const contentLength = parseInt(headers['content-length'] || '0', 10);
        const totalLength = headerEnd + 4 + contentLength;

        if (buffer.length < totalLength) return null;

        const body = buffer.slice(headerEnd + 4, totalLength);

        offset = totalLength;

        return { method, url, headers, body };
    }

    function handleRequest(req) {
        const { method, url, headers } = req;
        const keepAlive = headers['connection']?.toLowerCase() === 'keep-alive';

        let body, statusCode, statusMessage, headersOut;

        if (method === 'GET' && url === '/') {
            body = JSON.stringify({ message: 'Hello World' });
            statusCode = 200;
            statusMessage = 'OK';
            headersOut = {
                'Content-Length': Buffer.byteLength(body),
            };
        } else {
            body = 'Not Found';
            statusCode = 404;
            statusMessage = 'Not Found';
            headersOut = {
                'Content-Length': Buffer.byteLength(body),
            };
        }

        headersOut['Connection'] = keepAlive ? 'keep-alive' : 'close';

        const response =
            `HTTP/1.1 ${statusCode} ${statusMessage}\r\n` +
            Object.entries(headersOut).map(([k, v]) => `${k}: ${v}`).join('\r\n') +
            '\r\n\r\n' +
            body;

        writeData(Buffer.from(response));

        if (!keepAlive) {
            socket.end();
        }
    }

    socket.on('data', (chunk) => {
        if (offset === buffer.length) {
            buffer = chunk;
            offset = 0;
        } else {
            buffer = Buffer.concat([buffer.slice(offset), chunk]);
            offset = 0;
        }

        let req;
        while ((req = tryParseRequest()) !== null) {
            try {
                handleRequest(req);
            } catch (err) {
                console.error('Request handler error:', err);
                socket.end('HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\nInternal Server Error');
                return;
            }
        }
    });
});

server.listen({
    port: PORT,
    host: '0.0.0.0',
    reusePort: true
});
