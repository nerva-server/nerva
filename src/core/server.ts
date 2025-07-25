import http from 'http'
import { createServer, IncomingMessage, ServerResponse } from 'http'
import cluster from 'cluster'

import { ServerConfig } from "../interfaces"
import { Handler } from '../types'
import { methodMap } from '../maps'

import { initWorkers } from "../cluster/worker"

import { DefaultServerConfig } from '../config/security'
import { deepMerge } from '../utils/object'

import { RadixRouter } from '../core/radix'

export class Server {
    private config: ServerConfig
    private server!: ReturnType<typeof createServer>

    private router = new RadixRouter()

    constructor(config?: Partial<ServerConfig>) {
        this.config = deepMerge(DefaultServerConfig, config as ServerConfig)

        if (this.config.cluster?.enabled) {
            const threads = this.config.cluster.workerThreads
            initWorkers({ threads })

            if (!cluster.isPrimary) {
                this.server = createServer(this.requestListener.bind(this))
                this.server.keepAliveTimeout = 60_000
                this.server.headersTimeout = 65_000
            }
        } else {
            this.server = createServer(this.requestListener.bind(this))
            this.server.keepAliveTimeout = 60_000
            this.server.headersTimeout = 65_000
        }

        http.globalAgent.maxSockets = Infinity
    }

    private requestListener(req: IncomingMessage, res: ServerResponse) {
        req.socket.setNoDelay(true)
        res.setHeader('Connection', 'keep-alive')

        const method = methodMap[req.method as string] ?? req.method

        if (method === 'HEAD') {
            res.writeHead(200, {
                'Content-Length': 0
            })
            res.end()
            return
        }

        const rawUrl = req.url!
        const qIndex = rawUrl.indexOf('?')
        const url = qIndex === -1 ? rawUrl : rawUrl.slice(0, qIndex)

        const handler = this.router.findHandler(method, url)

        if (handler) {
            handler(req, res)
            return
        }

        res.statusCode = 404
        res.end(`404`)
    }

    addRoute(method: string, path: string, handler: Handler) {
        this.router.addRoute(method, path, handler)
    }

    start(listener?: (port: number) => void) {
        if (!this.config.cluster?.enabled || !cluster.isPrimary) {
            this.server.listen({
                port: this.config.port,
                backlog: this.config.backlog,
                exclusive: false
            }, () => {
                if (listener) listener(this.config.port as number)
            })
        }
    }

    get(path: string, handler: Handler) {
        this.addRoute("GET", path, handler)
    }
}
