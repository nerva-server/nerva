export class HttpRequest {
    method: string
    url: string
    headers: Record<string, string>
    body: Buffer | null
    params: Record<string, string>

    constructor(method: string, url: string, headers: Record<string, string>, body: Buffer | null = null) {
        this.method = method
        this.url = url
        this.headers = headers
        this.body = body
        this.params = {}
    }
}