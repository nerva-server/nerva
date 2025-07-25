import { RadixNode } from "../nodes/radix"
import { Handler } from "../types"

export class RadixRouter {
    private root = new RadixNode()

    addRoute(method: string, path: string, handler: Handler) {
        const fullPath = `${method}:${path}`
        this.insert(this.root, fullPath, handler)
    }

    private insert(node: RadixNode, path: string, handler: Handler) {
        let curNode = node
        let curPath = path

        while (true) {
            let matchedChild: RadixNode | null = null
            let matchedKey: string | null = null

            for (const [key, child] of curNode.children.entries()) {
                const prefixLen = this.commonPrefixLength(curPath, key)
                if (prefixLen > 0) {
                    matchedChild = child
                    matchedKey = key
                    break
                }
            }

            if (!matchedChild || !matchedKey) {
                const newNode = new RadixNode(curPath)
                newNode.handler = handler
                curNode.children.set(curPath, newNode)
                return
            }

            const prefixLen = this.commonPrefixLength(curPath, matchedKey)

            if (prefixLen === matchedKey.length && prefixLen === curPath.length) {
                matchedChild.handler = handler
                return
            }

            if (prefixLen < matchedKey.length) {
                const oldSuffix = matchedKey.slice(prefixLen)
                const newChild = new RadixNode(oldSuffix)
                newChild.children = matchedChild.children
                newChild.handler = matchedChild.handler

                matchedChild.prefix = matchedKey.slice(0, prefixLen)
                matchedChild.children = new Map([[oldSuffix, newChild]])
                matchedChild.handler = null
            }

            curNode = matchedChild
            curPath = curPath.slice(prefixLen)

            if (curPath.length === 0) {
                curNode.handler = handler
                return
            }
        }
    }

    findHandler(method: string, path: string): Handler | null {
        const fullPath = `${method}:${path}`
        let node = this.root
        let curPath = fullPath

        while (curPath.length > 0) {
            let foundChild: RadixNode | null = null
            for (const [key, child] of node.children.entries()) {
                if (curPath.startsWith(key)) {
                    foundChild = child
                    curPath = curPath.slice(key.length)
                    node = child
                    break
                }
            }
            if (!foundChild) return null
        }
        return node.handler
    }

    private commonPrefixLength(a: string, b: string): number {
        const len = Math.min(a.length, b.length)
        for (let i = 0; i < len; i++) {
            if (a[i] !== b[i]) return i
        }
        return len
    }
}