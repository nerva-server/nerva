import http from 'http'
import { createServer, IncomingMessage, ServerResponse } from 'http'
import cluster from 'cluster'

import { ServerConfig } from "@/types"

import { initWorkers } from "@/cluster/worker"

import { DefaultServerConfig } from '@/config/security'
import { deepMerge } from '@/utils/object'

type Handler = (req: IncomingMessage, res: ServerResponse) => void

export class Server {
    private config: ServerConfig
    private server!: ReturnType<typeof createServer>

    private routes: Record<string, Record<string, Handler>> = {}

    constructor(config?: Partial<ServerConfig>) {
        this.config = deepMerge(DefaultServerConfig, config as ServerConfig)

        if (this.config.cluster?.enabled) {
            const threads = this.config.cluster.workerThreads
            initWorkers({ threads })

            if (!cluster.isPrimary) {
                this.server = createServer(this.requestListener.bind(this))
            }
        } else {
            this.server = createServer(this.requestListener.bind(this))
        }

        http.globalAgent.maxSockets = Infinity
    }

    private requestListener(req: IncomingMessage, res: ServerResponse) {
        req.socket.setNoDelay(true)
        
        res.setHeader('Connection', 'keep-alive')

        const method = req.method?.toUpperCase() || ''
        const url = req.url?.split('?', 1)[0] || ''

        const route = this.routes[url]
        if (route && route[method]) {
            return route[method](req, res)
        }

        res.statusCode = 404
        res.end(`404`)
    }

    private addRoute(method: string, path: string, handler: Handler) {
        if (!this.routes[path]) {
            this.routes[path] = {}
        }
        this.routes[path][method.toUpperCase()] = handler
    }

    start(listener?: (port: number) => void) {
        if (!this.config.cluster?.enabled || !cluster.isPrimary) {
            this.server.listen(this.config.port, this.config.backlog, () => {
                if (listener) listener(this.config.port as number)
            })
        }
    }

    get(path: string, handler: Handler) {
        this.addRoute("GET", path, handler)
    }
}