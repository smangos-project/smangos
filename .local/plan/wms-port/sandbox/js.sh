#!/bin/bash
# usage: js.sh 'code'  (or code on stdin)
code="${1:-$(cat)}"
curl -s -m ${T:-120} --data-binary "$code" 127.0.0.1:8799/run | python3 -c 'import sys,json; d=json.load(sys.stdin); r=d.get("result"); print(r if d.get("ok") else d)'
