import { IncomingMessage, ServerResponse } from "http"

export type Handler = (req: IncomingMessage, res: ServerResponse) => void