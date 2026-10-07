#!/bin/bash
bash /tmp/wmsdbg/restart.sh
for i in $(seq 90); do sleep 5; /tmp/wmsdbg/js.sh '1+1' 2>/dev/null | grep -q '^2' && echo up && exit 0; done
echo timeout
