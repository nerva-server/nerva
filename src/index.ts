import { Server } from "@/core/server"

const server = new Server({
    cluster: {
        enabled: true,
        workerThreads: 6
    }
})

import fastJson from 'fast-json-stringify'

const stringify = fastJson({
  type: 'object',
  properties: { data: { type: 'string' } }
});

server.get("/", (req, res) => {
    res.end(stringify({m: "H"}))
})

server.start((port) => {
    console.log("Sunucu çalıştı" + port)
})