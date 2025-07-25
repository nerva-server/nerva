import { Handler } from "../types"

export class RadixNode {
    prefix: string
    children: Map<string, RadixNode>
    handler: Handler | null = null

    constructor(prefix = '') {
        this.prefix = prefix
        this.children = new Map()
    }
}