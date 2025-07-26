import cluster from 'cluster'

import { ServerConfig } from "../interfaces"
import { Handler } from '../types'
import { methodMap } from '../maps'

import os from "os"

import { initWorkers } from "../cluster/worker"

import { DefaultServerConfig } from '../config/security'
import { deepMerge } from '../utils/object'

import { RadixRouter } from '../core/radix'
import { createServer } from '../http/http'

export class Server {
    private config: ServerConfig;
    private server!: ReturnType<typeof createServer>;

    constructor(config?: Partial<ServerConfig>) {
        this.config = deepMerge(DefaultServerConfig, config as ServerConfig);

        if (this.config.cluster?.enabled && cluster.isPrimary) {
            this.setupCluster();
        } else {
            this.server = createServer();
        }
    }

    private setupCluster() {
        const workerCount = this.config.cluster?.workerThreads ?? Math.min(os.cpus().length, 8);

        console.log(`[Cluster] Primary PID: ${process.pid}, forking ${workerCount} workers`);

        cluster.on('exit', (worker, code, signal) => {
            const reason = signal || code;
            console.warn(`[Cluster] Worker ${worker.process.pid} died (${reason})`);

            if (!worker.exitedAfterDisconnect) {
                console.log('[Cluster] Restarting worker...');
                setTimeout(() => cluster.fork(), 1000);
            }
        });

        process.on('SIGTERM', () => {
            console.log('[Cluster] Master received SIGTERM, shutting down workers...');
            for (const id in cluster.workers) {
                cluster.workers[id]?.process.kill('SIGTERM');
            }
            setTimeout(() => process.exit(0), 5000); 
        });

        for (let i = 0; i < workerCount; i++) {
            setTimeout(() => {
                const worker = cluster.fork();
                worker.on('message', (msg) => {
                    console.log(`[Cluster] Message from worker ${worker.process.pid}:`, msg);
                });
            }, i * 100); 
        }
    }

    start(listener?: (port: number) => void) {
        if (this.server) {
            this.server.listen(
                this.config.port,
                this.config.backlog,
                () => {
                    console.log(`[Worker ${process.pid}] Listening on port ${this.config.port}`);
                    if (listener) listener(this.config.port as number);
                }
            );
        }
    }

    get(path: string, handler: Handler) {
        if (this.server) {
            this.server.router.get(path, handler);
        }
    }
}