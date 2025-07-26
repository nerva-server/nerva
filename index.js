const net = require('net');
const cluster = require('cluster');
const os = require('os');

const addon = require('./build/Release/buffer.node');

const UltraHttpBuffer = addon.UltraHttpBuffer;

const PORT = 3000;
const HOST = '0.0.0.0';

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

  return;
}

process.setMaxListeners(0);

const server = net.createServer((socket) => {
  socket.setNoDelay(true);
  socket.setKeepAlive(true, 60000);

  let buffer = new UltraHttpBuffer();

  // socket fd, hacky ama işe yarıyor
  const fd = socket._handle.fd;

  function writeData(dataBuffer) {
    if (typeof dataBuffer === 'string') {
      const b = Buffer.from(dataBuffer);
      return socket.write(b);
    }
    return socket.write(dataBuffer);
  }

  function parseHeaders() {
    const headerEnd = buffer.findHeaderEnd();
    if (headerEnd === -1) return null;

    // Header + Body toplam uzunluk burada yok, basit örnek sadece header okuyoruz
    const headerBuffer = buffer.getSlice(0, headerEnd);
    const headerString = headerBuffer.toString('ascii');

    const lines = headerString.split('\r\n');
    const [method, url] = lines[0].split(' ');
    const headers = {};
    for (let i = 1; i < lines.length; i++) {
      const line = lines[i];
      if (!line) continue;
      const idx = line.indexOf(':');
      if (idx === -1) continue;
      const key = line.slice(0, idx).trim().toLowerCase();
      const val = line.slice(idx + 1).trim();
      headers[key] = val;
    }

    return { method, url, headers, headerEnd };
  }

  socket.on('data', (chunk) => {
    buffer.addBuffer(chunk);

    while (true) {
      const req = parseHeaders();
      if (!req) break;

      // Calculate total length of full request (header + body)
      const contentLength = parseInt(req.headers['content-length'] || '0', 10);
      const totalLength = req.headerEnd + contentLength;

      const totalData = buffer.concatAllData ? buffer.concatAllData() : null;
      // Eğer concatAllData yoksa bu kısmı native fonksiyon olarak yazabilirsin.

      // Basitçe JS'de buffer toplam uzunluğunu tahmin et
      // Ama şimdilik burası demo, native fonksiyon eklemeni öneririm.

      // Burada sadece headerLength'ı consume ediyoruz, body parsing ve yönetimi aynı mantık.

      // Basit örnek: Response hazırla
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

      // Native response buffer hazırla
      const resBuffer = new UltraHttpBuffer();
      resBuffer.addString(`HTTP/1.1 ${statusCode} ${statusMessage}\r\n`);
      for (const [k, v] of Object.entries(headersOut)) {
        resBuffer.addString(`${k}: ${v}\r\n`);
      }
      resBuffer.addString('\r\n');
      resBuffer.addString(body);

      resBuffer.writeToFd(fd);

      buffer.consume(totalLength);

      if (!keepAlive) {
        socket.end();
        break;
      }
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
  });
});

server.listen(PORT, HOST, () => {
  console.log(`Worker ${process.pid} listening on ${HOST}:${PORT}`);
});
