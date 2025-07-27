const cluster = require('cluster')
const os = require('os')
const http2 = require('http2')
const fs = require('fs')
const path = require('path')

const serverOptions = {
  key: fs.readFileSync(path.join('localhost-key.pem')),
  cert: fs.readFileSync(path.join('localhost.pem')),
}

if (cluster.isMaster) {
  const cpuCount = os.cpus().length
  console.log(`👹 Master process, fork ${cpuCount} worker...`)

  for (let i = 0; i < cpuCount; i++) {
    cluster.fork()
  }

  cluster.on('exit', (worker, code, signal) => {
    console.log(`Worker ${worker.process.pid} öldü, yeniden başlatılıyor...`)
    cluster.fork()
  })
} else {
  // Worker process HTTP/2 server açıyor
  const server = http2.createSecureServer(serverOptions)

  server.on('stream', (stream, headers) => {
    const method = headers[':method']
    const reqPath = headers[':path']

    console.log(`[Worker ${process.pid}] [${method}] ${reqPath}`)

    if (reqPath === '/') {
      stream.respond({
        'content-type': 'text/html',
        ':status': 200,
      })
      stream.end('<h1>Selam HTTP/2 Dünya!</h1>')
    } else if (reqPath === '/json') {
      stream.respond({
        'content-type': 'application/json',
        ':status': 200,
      })
      stream.end(JSON.stringify({ mesaj: 'HTTP/2 ile JSON' }))
    } else {
      stream.respond({ ':status': 404 })
      stream.end('404 Not Found')
    }
  })

  server.listen(8443, () => {
    console.log(`🚀 Worker ${process.pid} HTTP/2 server çalışıyor: https://localhost:8443`)
  })
}
