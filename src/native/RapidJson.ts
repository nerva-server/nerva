import RapidJson from "../../build/Release/rapid.node"

function parse(object: any) {
    return RapidJson.stringify(object)
}

function stringify(object: any) {
    return RapidJson.stringify(object)
}

export {
    parse,
    stringify
}