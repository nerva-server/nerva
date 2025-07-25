const net = require('net');
const EventEmitter = require('events');
const { StringDecoder } = require('string_decoder');

class ServerResponse extends EventEmitter {
    constructor(socket) {
        super();
        this.socket = socket;
        this.statusCode = 200;
        this.headers = {};
        this._headersSent = false;
        this._ended = false;
        this.setHeader('Connection', 'keep-alive');
    }

    setHeader(name, value) {
        this.headers[name.toLowerCase()] = value;
        return this;
    }

    writeHead(statusCode, headers = {}) {
        if (this._headersSent) {
            throw new Error('Headers already sent');
        }
        this.statusCode = statusCode;
        this.headers = { ...this.headers, ...headers };
        return this;
    }

    _writeHeaders() {
        if (this._headersSent) return;

        let response = `HTTP/1.1 ${this.statusCode} ${this._getStatusMessage()}\r\n`;

        // Default headers
        if (!this.headers['content-type']) {
            this.setHeader('Content-Type', 'text/plain');
        }

        // Add Content-Length if not set
        if (!this.headers['content-length'] && this._body) {
            this.setHeader('Content-Length', Buffer.byteLength(this._body));
        }

        // Write all headers
        for (const [key, value] of Object.entries(this.headers)) {
            response += `${key}: ${value}\r\n`;
        }
        response += '\r\n';

        this.socket.write(response);
        this._headersSent = true;
    }

    write(data) {
        if (this._ended) return false;

        if (!this._body) this._body = '';
        this._body += data;
        return true;
    }

    end(data = '') {
        if (this._ended) return;

        if (data) this.write(data);

        this._writeHeaders();

        if (this._body) {
            this.socket.write(this._body);
        }

        // Keep connection alive unless Connection: close
        if (this.headers['connection'] === 'close') {
            this.socket.end();
        }

        this._ended = true;
        this.emit('finish');
    }

    _getStatusMessage() {
        const statusMessages = {
            200: 'OK',
            201: 'Created',
            404: 'Not Found',
            500: 'Internal Server Error'
        };
        return statusMessages[this.statusCode] || 'Unknown';
    }
}

class IncomingMessage extends EventEmitter {
    constructor(socket) {
        super();
        this.socket = socket;
        this.method = null;
        this.url = null;
        this.headers = {};
        this.httpVersion = null;
        this._decoder = new StringDecoder('utf8');
        this._body = '';
        this._complete = false;
    }

    _parseRequest(data) {
        const request = data.toString();
        const [requestLine, ...headerLines] = request.split('\r\n');
        const [method, url, httpVersion] = requestLine.split(' ');

        this.method = method;
        this.url = url;
        this.httpVersion = httpVersion;

        let i = 0;
        for (; i < headerLines.length; i++) {
            const line = headerLines[i];
            if (line.trim() === '') break;
            const [key, value] = line.split(': ');
            this.headers[key.toLowerCase()] = value;
        }

        // Parse body if exists
        if (i + 1 < headerLines.length) {
            this._body = headerLines.slice(i + 1).join('\r\n');
        }
    }
}

function createServer(requestListener) {
    const server = net.createServer((socket) => {
        let buffer = '';

        socket.on('data', (data) => {
            buffer += data.toString();

            // Handle pipelined requests
            while (buffer.includes('\r\n\r\n')) {
                const req = new IncomingMessage(socket);
                const res = new ServerResponse(socket);

                const requestEnd = buffer.indexOf('\r\n\r\n') + 4;
                const requestData = buffer.substring(0, requestEnd);
                buffer = buffer.substring(requestEnd);

                req._parseRequest(requestData);

                // Check for body
                const contentLength = parseInt(req.headers['content-length'] || '0');
                if (contentLength > 0 && buffer.length >= contentLength) {
                    req._body = buffer.substring(0, contentLength);
                    buffer = buffer.substring(contentLength);
                }

                requestListener(req, res);
            }
        });

        socket.on('error', (err) => {
            if (err.code !== 'ECONNRESET') {
                console.error('Socket error:', err);
            }
        });
    });

    return server;
}

module.exports = {
    createServer,
    ServerResponse,
    IncomingMessage
};