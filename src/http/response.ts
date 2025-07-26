import { Socket } from "net"
import addon from "../../build/Release/http.node"

const Http = addon.Http

export class HttpResponse {
    statusCode: number
    statusMessage: string
    headers: Record<string, string | number>
    body: string
    private socket: Socket
    private fd: number
    private keepAlive: boolean

    constructor(socket: Socket, fd: number, keepAlive: boolean) {
        this.socket = socket
        this.fd = fd
        this.keepAlive = keepAlive
        this.statusCode = 200
        this.statusMessage = 'OK'
        this.headers = {}
        this.body = ''
    }

    setHeader(key: string, value: string | number) {
        this.headers[key.toLowerCase()] = value
    }

    writeHead(statusCode: number, statusMessage: string, headers?: Record<string, string | number>) {
        this.statusCode = statusCode
        this.statusMessage = statusMessage
        if (headers) {
            this.headers = { ...this.headers, ...headers }
        }
    }

    end(body?: string) {
        if (body !== undefined) {
            this.body = body
        }

        this.headers['content-length'] = Buffer.byteLength(this.body)
        this.headers['connection'] = this.keepAlive ? 'keep-alive' : 'close'

        const resBuffer = new Http()
        resBuffer.addString(`HTTP/1.1 ${this.statusCode} ${this.statusMessage}\r\n`)

        for (const [k, v] of Object.entries(this.headers)) {
            resBuffer.addString(`${k}: ${v}\r\n`)
        }

        resBuffer.addString('\r\n')
        resBuffer.addString(this.body)

        resBuffer.writeToFd(this.fd)

        if (!this.keepAlive) {
            this.socket.end()
        }
    }
}