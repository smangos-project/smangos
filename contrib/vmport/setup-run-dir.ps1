# Prepares the directory a runnable server is started from: the client data it reads (linked, never
# copied or written), a mangosd.conf and bnetserver.conf made from the .dist files with the databases
# and ports of this setup, the development TLS certificate, and the realm row.
#
# Start the servers from the build output directory so the OpenSSL DLLs are found:
#   bin\Release\bnetserver.exe -c <RunDir>\bnetserver.conf
#   bin\Release\mangosd.exe -c <RunDir>\mangosd.conf
param(
    [string]$RunDir = (Join-Path $PSScriptRoot "..\..\.local\vmport\run"),
    [string]$ClientData = "",          # the 5875 folder: holds dbc\ (the 1.12 DBC set); maps\ sits next to it
    [string]$MapsData = "",            # folder of the map files; defaults to <ClientData>\..\maps
    [string]$Client = "C:\Program Files\MariaDB 13.0\bin\mariadb.exe",
    [string]$DbHost = "127.0.0.1",
    [int]$DbPort = 3309,
    [string]$DbUser = "vmport",
    [string]$DbPassword = "vmport",
    [string]$Prefix = "vmport",
    [int]$RestPort = 8091,
    [int]$RpcPort = 1129,
    [int]$WorldPort = 8095,
    [int]$InstancePort = 8096,
    [int]$RealmId = 1,
    [string]$RealmName = "VMangosPort"
)

$ErrorActionPreference = "Stop"
$repo = Resolve-Path (Join-Path $PSScriptRoot "..\..")
New-Item -ItemType Directory -Force $RunDir | Out-Null
$RunDir = Resolve-Path $RunDir

if (-not $MapsData)
{
    $MapsData = Join-Path (Split-Path $ClientData -Parent) "maps"
}

$data = Join-Path $RunDir "data"
New-Item -ItemType Directory -Force $data | Out-Null
foreach ($link in @(@{ Name = "5875"; Target = $ClientData }, @{ Name = "maps"; Target = $MapsData }))
{
    $path = Join-Path $data $link.Name
    if (-not (Test-Path $path))
    {
        New-Item -ItemType Junction -Path $path -Target $link.Target | Out-Null
    }
}

function Set-ConfigValues([string]$Template, [string]$Output, [hashtable]$Values)
{
    $lines = Get-Content $Template
    $remaining = $Values.Clone()
    $result = foreach ($line in $lines)
    {
        $replaced = $false
        foreach ($key in @($remaining.Keys))
        {
            if ($line -match "^\s*$([regex]::Escape($key))\s*=")
            {
                "$key = $($remaining[$key])"
                $remaining.Remove($key)
                $replaced = $true
                break
            }
        }
        if (-not $replaced) { $line }
    }
    $result | Set-Content $Output -Encoding utf8
    if ($remaining.Count -gt 0)
    {
        Write-Warning "${Output}: no such key in the template: $($remaining.Keys -join ', ')"
    }
}

function Database-Info([string]$Suffix) { "`"$DbHost;$DbPort;$DbUser;$DbPassword;${Prefix}_$Suffix`"" }

$dataForConf = ($data -replace "\\", "/")

Set-ConfigValues (Join-Path $repo "src\mangosd\mangosd.conf.dist.in") (Join-Path $RunDir "mangosd.conf") @{
    "RealmID"                   = $RealmId
    "DataDir"                   = "`"$dataForConf`""
    "LoginDatabase.Info"        = Database-Info "realmd"
    "WorldDatabase.Info"        = "`"$($repo -replace '\\', '/')/.world/world.doltlite`""
    "CharacterDatabase.Info"    = Database-Info "characters"
    "LogsDatabase.Info"         = Database-Info "logs"
    "WorldServerPort"           = $WorldPort
    "InstanceServerPort"        = $InstancePort
    "vmap.enableLOS"            = 0
    "vmap.enableHeight"         = 0
    "vmap.enableIndoorCheck"    = 0
    "mmap.enabled"              = 0
}

Copy-Item (Join-Path $repo "contrib\bnetserver\bnetserver.cert.pem") $RunDir -Force
Copy-Item (Join-Path $repo "contrib\bnetserver\bnetserver.key.pem") $RunDir -Force
$certificate = ((Join-Path $RunDir "bnetserver.cert.pem") -replace "\\", "/")
$privateKey = ((Join-Path $RunDir "bnetserver.key.pem") -replace "\\", "/")

Set-ConfigValues (Join-Path $repo "src\bnetserver\bnetserver.conf.dist.in") (Join-Path $RunDir "bnetserver.conf") @{
    "LoginDatabaseInfo"  = Database-Info "realmd"
    "LoginREST.Port"     = $RestPort
    "BattlenetPort"      = $RpcPort
    "CertificatesFile"   = "`"$certificate`""
    "PrivateKeyFile"     = "`"$privateKey`""
}

$sql = "REPLACE INTO ``realmlist`` (``id``, ``name``, ``address``, ``localAddress``, ``localSubnetMask``, ``port``, ``icon``, ``realmflags``, ``timezone``, ``allowedSecurityLevel``, ``population``) " +
    "VALUES ($RealmId, '$RealmName', '127.0.0.1', '127.0.0.1', '255.255.255.0', $WorldPort, 0, 0, 1, 0, 0);"
$sqlFile = Join-Path $RunDir "realm.sql"
$sql | Set-Content $sqlFile -Encoding ascii
$process = Start-Process -FilePath $Client -ArgumentList @("--host=$DbHost", "--port=$DbPort", "--user=$DbUser", "--password=$DbPassword", "${Prefix}_realmd") `
    -RedirectStandardInput $sqlFile -NoNewWindow -Wait -PassThru
if ($process.ExitCode -ne 0) { Write-Error "could not write the realm row" }
Remove-Item $sqlFile
