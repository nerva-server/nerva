import cluster from 'cluster';
import os from 'os';
import { createServer } from "./http/http";

const numCPUs = os.cpus().length;

if (cluster.isPrimary) {
    console.log(`Master ${process.pid} is running`);

    for (let i = 0; i < numCPUs; i++) {
        cluster.fork();
    }

    cluster.on('exit', (worker, code, signal) => {
        console.warn(`Worker ${worker.process.pid} died. Respawning...`);
        cluster.fork();
    });

} else {
    const server = createServer({});

    server.listen({
        port: 3000,
        backlog: 512,
        listener() {
            console.log(`Worker ${process.pid} listening on port 3000`);
        },
    });
}
