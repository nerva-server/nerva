import { getCpus } from "@/utils/cpu"

import { ServerConfig } from "../interfaces"

export const DefaultServerConfig: ServerConfig = {
    port: 3000,
    host: '0.0.0.0',
    cluster: {
        enabled: true,
        workerThreads: getCpus().length
    }
}