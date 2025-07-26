import net, { Socket, Server as NetServer } from "net"

import { Router } from "./router"
import { HttpRequest } from "./request"
import { HttpResponse } from "./response"

import addon from "../../build/Release/http.node"
const Http = addon.Http

process.setMaxListeners(0)

export function createServer({

}: {}) {
    const server = new Server()

    return server
}

class Server {
    private server: NetServer
    private sockets: Set<Socket>
    private http: any

    public router: Router

    constructor() {
        this.http = new Http()
        this.sockets = new Set()
        this.router = new Router()

        this.server = net.createServer((socket) => {
            this.setupSocket(socket)
        })
    }

    private setupSocket(socket: Socket) {
        socket.setNoDelay(true)
        socket.setKeepAlive(true, 60000)

        const fd = (socket as any)._handle.fd
        this.sockets.add(socket)

        socket.on("data", (chunk) => this.handleData(chunk, socket, fd))
        socket.on("close", () => this.handleClose(socket))
        socket.on("error", (err) => this.handleError(err, socket))
    }

    private handleData(chunk: Buffer, socket: Socket, fd: number) {
        this.http.addBuffer(chunk)

        while (true) {
            const req = this.parseHeaders()
            if (!req) break

            const contentLength = parseInt(req.headers['content-length'] || '0', 10)
            const totalLength = req.headerEnd + contentLength

            const body = contentLength > 0
                ? this.http.getSlice(req.headerEnd, totalLength)
                : null

            const httpRequest = new HttpRequest(
                req.method,
                req.url,
                req.headers,
                body
            )

            const keepAlive = req.headers['connection']?.toLowerCase() === 'keep-alive'
            const httpResponse = new HttpResponse(socket, fd, keepAlive)

            const handler = this.router.findHandler(httpRequest.method, httpRequest.url)

            if (handler) {
                handler(httpRequest, httpResponse)
            } else {
                httpResponse.writeHead(404, 'Not Found')
                httpResponse.end('Not Found')
            }

            this.http.consume(totalLength)

            if (!keepAlive) {
                socket.end()
                break
            }
        }
    }

    private handleClose(socket: Socket) {
        this.sockets.delete(socket)
        this.http = null
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

    private parseHeaders() {
        const headerEnd = this.http.findHeaderEnd()
        if (headerEnd === -1) return null

        const headerBuffer = this.http.getSlice(0, headerEnd)
        const headerString = headerBuffer.toString('ascii')

        const lines = headerString.split('\r\n')
        const [method, url] = lines[0].split(' ')
        const headers: Record<string, string> = {}

        for (let i = 1; i < lines.length; i++) {
            const line = lines[i]
            if (!line) continue
            const idx = line.indexOf(':')
            if (idx === -1) continue
            const key = line.slice(0, idx).trim().toLowerCase()
            const val = line.slice(idx + 1).trim()
            headers[key] = val
        }

        return { method, url, headers, headerEnd }
    }

    listen(port?: number, backlog?: number, listener?: () => void) {
        this.server.listen(port, "0.0.0.0", backlog, listener)
        return this
    }
}
