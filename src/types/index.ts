export interface ServerConfig {
    port?: number;
    host?: string;
    backlog?: number;
    uploadDir?: string;
    cluster?: ClusterConfig;
    cors?: CorsConfig;
    rateLimiter?: RateLimiterConfig;
    files?: FileConfig;
    logging?: LoggingConfig;
    compression?: CompressionConfig;
}

export interface ClusterConfig {
    enabled: boolean;
    workerThreads: number;
}

export interface CorsConfig {
    allowedOrigins: string[];
    methods: string[];
    allowCredentials: boolean;
}

export interface RateLimiterConfig {
    windowMs: number;
    maxRequests: number;
}

export interface FileConfig {
    allowedMaxFileSize: number;
    allowedMimetypes: string[];
}

export interface LoggingConfig {
    enabled: boolean;
    format: string;
}

export interface CompressionConfig {
    enabled: boolean;
    threshold: number;
    level: number;
}