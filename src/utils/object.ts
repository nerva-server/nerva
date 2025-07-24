export function deepMerge<T extends Record<string, any>, U extends Record<string, any>>(target: T, source: U): T & U {
    const output = { ...target } as T & U;

    for (const key in source) {
        if (!Object.prototype.hasOwnProperty.call(source, key)) continue;

        const sourceValue = source[key];
        const targetValue = output[key];

        if (
            isPlainObject(sourceValue) &&
            isPlainObject(targetValue)
        ) {
            output[key] = deepMerge(targetValue, sourceValue);
        } else {
            output[key] = sourceValue as any;
        }
    }

    return output;
}

export function isPlainObject(obj: any): obj is Record<string, any> {
    return typeof obj === "object" &&
        obj !== null &&
        !Array.isArray(obj) &&
        Object.prototype.toString.call(obj) === "[object Object]";
}
