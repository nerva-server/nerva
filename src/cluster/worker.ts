import cluster from 'cluster'

import { getCpus } from "../utils/cpu"

export function initWorkers({ threads }: { threads?: number }) {
    if (cluster.isPrimary) {
        const threadsCount = threads || getCpus().length
        
        for (let i = 0; i < threadsCount; i++) {
            cluster.fork()
        }

        cluster.on('exit', (worker, code, signal) => {
            cluster.fork()
        })
    }
}