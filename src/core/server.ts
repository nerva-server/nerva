import cluster from 'cluster';
import { ServerConfig } from "../interfaces";
import os from "os";
import { DefaultServerConfig } from '../config/security';
import { deepMerge } from '../utils/object';
import { createServer } from '../http/http';
import { Router } from '../http/router';

interface WorkerInfo {
    id: number;
    load: number;
    restartCount: number;
    lastRestart?: Date;
}

export class Server extends Router {
    private config: ServerConfig;
    private server!: ReturnType<typeof createServer>;
    private workerLoads: Record<number, WorkerInfo> = {};

    constructor(config?: Partial<ServerConfig>) {
        super();
        this.config = deepMerge(DefaultServerConfig, config as ServerConfig);

        if (this.config.cluster?.enabled && cluster.isPrimary) {
            this.setupCluster();
        } else {
            this.server = createServer(this);
            this.setupWorkerMetrics();
        }
    }

    private setupCluster(): void {
        const workerCount = this.getOptimalWorkerCount();
        const startupDelay = this.calculateStartupDelay(workerCount);

        cluster.on('exit', this.handleWorkerExit.bind(this));
        this.setupShutdownHandlers();
        this.startWorkers(workerCount, startupDelay);
    }

    private getOptimalWorkerCount(): number {
        return this.config.cluster?.workerThreads
            || Math.min(os.cpus().length, 8);
    }

    private calculateStartupDelay(workerCount: number): number {
        return Math.max(100, 2000 / workerCount);
    }

    private handleWorkerExit(worker: any, code: number, signal: string): void {
        if (!worker.exitedAfterDisconnect) {
            const restartCount = (this.workerLoads[worker.id]?.restartCount || 0) + 1;
            const delay = Math.min(1000 * Math.pow(2, restartCount), 30000);

            setTimeout(() => {
                const newWorker = cluster.fork();
                this.workerLoads[newWorker.id] = {
                    id: newWorker.id,
                    load: 0,
                    restartCount,
                    lastRestart: new Date()
                };
            }, delay);
        }
    }

    private setupShutdownHandlers(): void {
        const gracefulShutdown = () => {
            Object.values(cluster.workers || {}).forEach(worker => {
                if (worker) {
                    const timeout = setTimeout(() => worker.kill('SIGKILL'), 5000);
                    worker.on('disconnect', () => clearTimeout(timeout));
                    worker.disconnect();
                }
            });
        };

        process.on('SIGTERM', gracefulShutdown);
        process.on('SIGINT', gracefulShutdown);
    }

    private startWorkers(count: number, delay: number): void {
        const workerPromises = Array.from({ length: count }, (_, i) =>
            new Promise<void>(resolve => {
                setTimeout(() => {
                    const worker = cluster.fork({
                        WORKER_ID: i + 1,
                        NODE_ENV: process.env.NODE_ENV
                    });

                    this.workerLoads[worker.id] = {
                        id: worker.id,
                        load: 0,
                        restartCount: 0
                    };

                    worker.on('online', () => {
                        resolve();
                    });

                    worker.on('message', (msg: any) => {
                        if (msg.type === 'loadUpdate') {
                            this.workerLoads[worker.id].load = msg.load;
                        }
                    });
                }, i * delay);
            })
        );
    }

    private setupWorkerMetrics(): void {
        if (cluster.isWorker) {
            setInterval(() => {
                const load = process.memoryUsage().heapUsed / process.memoryUsage().heapTotal;
                if (process.send && process.connected) {
                    try {
                        process.send({
                            type: 'loadUpdate',
                            load,
                            workerId: cluster.worker?.id,
                            timestamp: Date.now()
                        });
                    } catch (err) {
                    }
                }
            }, 5000);
        }
    }

    public getLeastLoadedWorker(): number | null {
        if (!cluster.isPrimary) return null;

        return Object.values(this.workerLoads).reduce(
            (min, worker) => worker.load < min.load ? worker : min,
            { id: -1, load: Infinity }
        ).id;
    }

    public broadcast(message: any): void {
        if (!cluster.isPrimary) return;

        for (const id in cluster.workers) {
            cluster.workers[id]?.send(message);
        }
    }

    public updateConfig(newConfig: Partial<ServerConfig>): void {
        this.config = deepMerge(this.config, newConfig);
        this.broadcast({
            type: 'configUpdate',
            config: newConfig
        });
    }

    start(listener?: (port: number) => void): void {
        if (!this.server) return;

        const isFirstWorker = !cluster.worker || cluster.worker.id === 1;
        const shouldRunListener = !this.config.cluster?.enabled || isFirstWorker;

        this.server.listen(
            this.config.port,
            this.config.backlog,
            () => {
                if (shouldRunListener && listener) {
                    listener(this.config.port as number);
                }
            }
        );
    }
}