import net, { Socket, Server as NetServer } from "net"

import { Router } from "./router"
import { HttpRequest } from "./request"
import { HttpResponse } from "./response"

import HTTPServer from "../native/HTTPServer"

process.setMaxListeners(0)

export function createServer(router: Router) {
    const server = new Server(router)

    return server
}

class Server {
    private server: NetServer
    private sockets: Set<Socket>
    private http: any

    public router: Router

    constructor(router: Router) {
        this.http = new HTTPServer()
        this.sockets = new Set()
        this.router = router

        this.server = net.createServer({
            allowHalfOpen: false,
            pauseOnConnect: false,
            noDelay: true,
            keepAlive: true,
            keepAliveInitialDelay: 60000,
            highWaterMark: 1024 * 1024,
        }, (socket) => {
            this.setupSocket(socket)
        })
    }

    private setupSocket(socket: Socket) {
        socket.setNoDelay(true)
        socket.setKeepAlive(true, 60000)
        socket.setTimeout(0)

        const fd = (socket as any)._handle.fd
        this.sockets.add(socket)

        socket.on("data", (chunk) => this.handleData(chunk, socket, fd))
        socket.on("close", () => this.handleClose(socket))
        socket.on("error", (err) => this.handleError(err, socket))
    }

    private handleData(chunk: Buffer, socket: Socket, fd: number) {
        this.http.addBuffer(chunk);

        while (true) {
            const req = this.http.parseHeaders()
            if (!req) break;

            const contentLength = parseInt(req.headers['content-length'] || '0', 10);
            const totalLength = req.headerEnd + contentLength;

            const body = contentLength > 0
                ? this.http.getSlice(req.headerEnd, totalLength)
                : null;

            const httpRequest = new HttpRequest(
                req.method,
                req.url,
                req.headers,
                body
            );

            const keepAlive = req.headers['connection']?.toLowerCase() === 'keep-alive';
            const httpResponse = new HttpResponse(socket, fd, keepAlive);

            const handler = this.router.findHandler(httpRequest.method, httpRequest.url);

            if (handler) {
                handler(httpRequest, httpResponse);
            } else {
                httpResponse.writeHead(404, 'Not Found');
                httpResponse.end('Not Found');
            }

            this.http.consume(totalLength);
        }
    }

    private handleClose(socket: Socket) {
        this.sockets.delete(socket)
    }

    private handleError(err: Error, socket: Socket) {
        const msg = err.message?.toLowerCase();

        if (
            msg?.includes('econnreset') ||
            msg?.includes('epipe') ||
            msg?.includes('client hang up')
        ) {
        } else {
            console.error('Socket error:', err);
        }

        socket.destroy();
    }

    listen(port?: number, backlog?: number, listener?: () => void) {
        this.server.listen({
            port,
            host: "127.0.0.1",
            backlog,
        }, listener)
        return this
    }
}
