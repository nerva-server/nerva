cmd_Release/buffer.node := ln -f "Release/obj.target/buffer.node" "Release/buffer.node" 2>/dev/null || (rm -rf "Release/buffer.node" && cp -af "Release/obj.target/buffer.node" "Release/buffer.node")
