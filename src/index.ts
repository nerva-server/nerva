import { Server } from "@/core/server"

const server = new Server()

server.get("/", (req, res) => {
    res.end(JSON.stringify({m: "H"}))
})

server.start((port) => {
    console.log("Sunucu çalıştı" + port)
})