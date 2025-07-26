import cluster from 'cluster'

import { ServerConfig } from "../interfaces"
import { Handler } from '../types'
import { methodMap } from '../maps'

import { initWorkers } from "../cluster/worker"

import { DefaultServerConfig } from '../config/security'
import { deepMerge } from '../utils/object'

import { RadixRouter } from '../core/radix'
import { createServer } from '../http/http'

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
                this.server = createServer({})
            }
        } else {
            this.server = createServer({})
        }
    }

    start(listener?: (port: number) => void) {
        if (!this.config.cluster?.enabled || !cluster.isPrimary) {
            this.server.listen(
                this.config.port,
                this.config.backlog,
                () => {
                    if (listener) listener(this.config.port as number)
                })
        }
    }

    get(path: string, handler: Handler) {
        this.server.router.get(path, handler)
    }
}
