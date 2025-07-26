import { HttpRequest } from "@/http/request"
import { HttpResponse } from "@/http/response"
import { IncomingMessage, ServerResponse } from "http"

export type Handler = (req: HttpRequest, res: HttpResponse) => void