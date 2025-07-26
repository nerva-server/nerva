cmd_Release/rapid.node := ln -f "Release/obj.target/rapid.node" "Release/rapid.node" 2>/dev/null || (rm -rf "Release/rapid.node" && cp -af "Release/obj.target/rapid.node" "Release/rapid.node")
