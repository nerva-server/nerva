{
  "targets": [
    {
      "target_name": "http",
      "sources": [ "native/http.cpp" ],
      "include_dirs": [
        "node_modules/.pnpm/node-addon-api@8.5.0/node_modules/node-addon-api",
        "./rapidjson/include"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ]
    },
    {
      "target_name": "rapid",
      "sources": [ "native/rapid/rapidjson.cpp" ],
      "include_dirs": [
        "node_modules/.pnpm/node-addon-api@8.5.0/node_modules/node-addon-api",
        "./rapidjson/include"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ]
    }
  ]
}
