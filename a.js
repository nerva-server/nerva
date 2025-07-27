import autocannon from 'autocannon'

autocannon({
  url: 'https://localhost:8443',
  insecure: true,
}, console.log)
