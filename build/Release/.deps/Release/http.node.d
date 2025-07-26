cmd_Release/http.node := ln -f "Release/obj.target/http.node" "Release/http.node" 2>/dev/null || (rm -rf "Release/http.node" && cp -af "Release/obj.target/http.node" "Release/http.node")
