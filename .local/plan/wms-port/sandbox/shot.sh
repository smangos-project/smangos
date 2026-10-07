#!/bin/bash
# usage: shot.sh out.png  -> saves editor viewport screenshot
curl -s -m 120 -d 'wms.viewport.Screenshot()' 127.0.0.1:8799/run | python3 -c 'import sys,json,base64; d=json.load(sys.stdin); r=d.get("result"); open(sys.argv[1],"wb").write(base64.b64decode(r)) if r else print(d)' "$1"
