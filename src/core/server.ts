import cluster from 'cluster'
import { ServerConfig } from "../interfaces"
import { Handler } from '../types'
import os from "os"
import { DefaultServerConfig } from '../config/security'
import { deepMerge } from '../utils/object'
import { createServer } from '../http/http'
import { Router } from '@/http/router'

export class Server extends Router {
    private config: ServerConfig
    private server!: ReturnType<typeof createServer>

    constructor(config?: Partial<ServerConfig>) {
        super()
        this.config = deepMerge(DefaultServerConfig, config as ServerConfig)

        if (this.config.cluster?.enabled && cluster.isPrimary) {
            this.setupCluster()
        } else {
            this.server = createServer(this)
        }
    }

    private setupCluster() {
        const workerCount = this.config.cluster?.workerThreads ?? Math.min(os.cpus().length, 8)

        cluster.on('exit', (worker, code, signal) => {
            if (!worker.exitedAfterDisconnect) {
                setTimeout(() => cluster.fork(), 1000)
            }
        })

        process.on('SIGTERM', () => {
            for (const id in cluster.workers) {
                cluster.workers[id]?.process.kill('SIGTERM')
            }
            setTimeout(() => process.exit(0), 5000)
        })

        for (let i = 0; i < workerCount; i++) {
            setTimeout(() => cluster.fork(), i * 100)
        }
    }

    start(listener?: (port: number) => void) {
        if (!this.server) return

        const isFirstWorker = !cluster.worker || cluster.worker.id === 1
        const shouldRunListener = !this.config.cluster?.enabled || isFirstWorker

        this.server.listen(
            this.config.port,
            this.config.backlog,
            () => {
                if (shouldRunListener && listener) {
                    listener(this.config.port as number)
                }
            }
        )
    }    
}