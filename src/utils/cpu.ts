import os from 'os'

export function getCpus() {
    const cpus = []
    cpus.push(os.cpus)

    return cpus
}