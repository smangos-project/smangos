#!/bin/bash
# usage: look.sh camx camy camz tx ty tz
curl -s -m 60 -d "(()=>{const e=wms.scene.Create(); wms.scene.SetPosition(e,$4,$5,$6); wms.viewport.SetPosition($1,$2,$3); wms.viewport.Focus(e); wms.viewport.SetPosition($1,$2,$3); wms.scene.Delete(e); return JSON.stringify(wms.viewport.GetPosition())})()" 127.0.0.1:8799/run; echo
