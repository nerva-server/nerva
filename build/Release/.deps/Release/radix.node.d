cmd_Release/radix.node := ln -f "Release/obj.target/radix.node" "Release/radix.node" 2>/dev/null || (rm -rf "Release/radix.node" && cp -af "Release/obj.target/radix.node" "Release/radix.node")
