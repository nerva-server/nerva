import { Server } from "./core/server"

import { stringify, parse } from "./native/RapidJson"

const server = new Server({
    port: 3000,
    cluster: {
        enabled: true,
        workerThreads: 8
    }
})

server.get("/", (req, res) => {
    res.status(200).send({ message: "Hello World" })
})

server.start((port) => {
    console.log(`Server is Running On ${port}`)
})