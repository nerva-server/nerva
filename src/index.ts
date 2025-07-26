import { Server } from "./core/server";

import RapidJson from "../build/Release/rapid.node"

const server = new Server({
    port: 3000,
    cluster: {
        enabled: true,
        workerThreads: 8
    }
})

server.get("/", (req, res) => {
    res.end(RapidJson.stringify({ message: 'Hello World' }))
})

server.start()