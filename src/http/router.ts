import { RadixRouter } from "@/native/Radix"
import { HttpRequest } from "./request"
import { HttpResponse } from "./response"

export class Router {
    private radixRouter = new RadixRouter()

    add(method: string, path: string, handler: (req: HttpRequest, res: HttpResponse) => void) {
        this.radixRouter.addRoute(method, path, handler)
    }

    get(path: string, handler: (req: HttpRequest, res: HttpResponse) => void) {
        this.add('GET', path, handler)
    }

    post(path: string, handler: (req: HttpRequest, res: HttpResponse) => void) {
        this.add('POST', path, handler)
    }

    put(path: string, handler: (req: HttpRequest, res: HttpResponse) => void) {
        this.add('PUT', path, handler)
    }

    delete(path: string, handler: (req: HttpRequest, res: HttpResponse) => void) {
        this.add('DELETE', path, handler)
    }

    findHandler(method: string, path: string) {
        const handler = this.radixRouter.findHandler(method, path)
        return handler
    }
}