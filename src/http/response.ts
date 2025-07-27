import { Socket } from "net"

import HTTPServer from "../native/HTTPServer"
import { stringify } from "@/native/RapidJson"

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

    end(body?: string, statusCode?: number) {
        if (body !== undefined) {
            this.body = body
        }

        const statusMessage = statusCode ? this.getStatusText(statusCode) : this.statusMessage

        this.headers['content-length'] = Buffer.byteLength(this.body)
        this.headers['connection'] = this.keepAlive ? 'keep-alive' : 'close'

        const resBuffer = new HTTPServer()
        resBuffer.addString(`HTTP/1.1 ${statusCode ? statusCode : this.statusCode} ${statusMessage}\r\n`)

        for (const [k, v] of Object.entries(this.headers)) {
            resBuffer.addString(`${k}: ${v}\r\n`)
        }

        resBuffer.addString('\r\n')
        resBuffer.addString(this.body)

        try {
            resBuffer.writeToFd(this.fd)
        } catch (e) {

        }
    }

    private getStatusText(statusCode: number) {
        return {
            200: "OK",
            404: "Not Found",
            502: "Internal Server Error"
        }[statusCode] || "Undefined"
    }

    status(statusCode: number) {
        const responseRoute = new FluentResponse(this, statusCode)
        return responseRoute
    }
}

class FluentResponse {
    res: HttpResponse

    statusCode: number

    constructor(res: HttpResponse, statusCode: number) {
        this.res = res
        this.statusCode = statusCode
    }

    send(object: any) {
        let data: string;

        if (typeof object === 'object' && object !== null) {
            data = stringify(object);
        } else {
            data = String(object);
        }

        this.res.end(data);
    }
}