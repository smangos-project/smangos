<#
.SYNOPSIS
    Runs the bot tests against the server in .server: the RealBots unit tests, then the conformance
    suite. Starts the server when it is not already running, creates the bot accounts, and stops it again.

.DESCRIPTION
    Meant to be run through the `bot_tests` CMake target, which builds everything first:

        cmake --build build --config RelWithDebInfo --target bot_tests

    The conformance config is .server/config/tests-conformance.conf (created by `server_setup`). Its
    ports are replaced by the ones in .server/config/bnetserver.conf for the run.

    A server started here has its console held open by this script, and the accounts listed for the
    target (and its -gm companion) are created through it. A server that was already running is used
    as it is, and its accounts have to exist already.

.PARAMETER Exe
    tests-conformance.exe.

.PARAMETER UnitExe
    tests-realbots.exe. Skipped when not given.

.PARAMETER ServerExe
    server.exe, started with .server as its working directory when nothing listens on the RPC port.

.PARAMETER ServerDir
    The server directory (default: <repo>/.server).

.PARAMETER Config
    Conformance config (default: <ServerDir>/config/tests-conformance.conf).

.PARAMETER Target
    The conformance target to run (default: vmangos).

.PARAMETER Filter
    A Catch2 test spec (e.g. "char_screen*") limiting the conformance scenarios run; the unit tests are skipped.

.PARAMETER Strict
    Also run the scenarios the target's expectations file leaves out and report which now pass.

.PARAMETER NoServer
    Never start a server; fail when none is running.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string] $Exe,
    [string] $UnitExe,
    [string] $ServerExe,
    [string] $ServerDir,
    [string] $Config,
    [string] $Target = 'vmangos',
    [string] $Filter,
    [switch] $Strict,
    [switch] $NoServer
)

$ErrorActionPreference = 'Stop'

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $ServerDir) { $ServerDir = Join-Path $here '..\..\.server' }
$ServerDir = (Resolve-Path $ServerDir -ErrorAction SilentlyContinue).Path
if (-not $ServerDir) { throw "no .server directory: build the server_setup target first" }
if (-not $Config) { $Config = Join-Path $ServerDir 'config\tests-conformance.conf' }
if (-not (Test-Path $Config)) { throw "config not found: $Config (build the server_setup target to create it)" }

function Get-ConfValue([string] $file, [string] $key, [string] $default) {
    if (Test-Path $file) {
        $line = Get-Content $file | Where-Object { $_ -match "^\s*$([regex]::Escape($key))\s*=" } | Select-Object -Last 1
        if ($line) { return ($line -split '=', 2)[1].Trim().Trim('"') }
    }
    return $default
}

$bnetConf = Join-Path $ServerDir 'config\bnetserver.conf'
$restPort = [int](Get-ConfValue $bnetConf 'LoginREST.Port' '8081')
$rpcPort = [int](Get-ConfValue $bnetConf 'BattlenetPort' '1119')

function Test-PortListening([int] $port) {
    $client = New-Object Net.Sockets.TcpClient
    try { $client.Connect('127.0.0.1', $port); return $true }
    catch { return $false }
    finally { $client.Close() }
}

# The run's config: the checked-in one with the server's ports, except for the deliberately dead target.
$effective = Join-Path ([IO.Path]::GetTempPath()) ("tests-conformance-" + [Guid]::NewGuid().ToString('N') + '.conf')
$lines = Get-Content $Config | ForEach-Object {
    if ($_ -match '^\s*RealBots\.Target\.(?!deadhost\.)\S+\.RestPort\s*=') { $_ -replace '=.*', "= $restPort" }
    elseif ($_ -match '^\s*RealBots\.Target\.(?!deadhost\.)\S+\.RpcPort\s*=') { $_ -replace '=.*', "= $rpcPort" }
    else { $_ }
}
Set-Content -Path $effective -Value $lines -Encoding ascii

# Accounts named for the target and its -gm companion: "name:password" entries of every Accounts* key.
function Get-Accounts([string] $target) {
    $pattern = "^\s*RealBots\.Target\.$([regex]::Escape($target))\.Accounts\d*\s*="
    Get-Content $Config | Where-Object { $_ -match $pattern } | ForEach-Object {
        ([regex]::Match($_, '"([^"]*)"').Groups[1].Value -split '\s+') | Where-Object { $_ }
    } | ForEach-Object { $name, $password = $_ -split ':', 2; [pscustomobject] @{ Name = $name; Password = $password } }
}

$server = $null
$exitCode = 0
try {
    if (Test-PortListening $rpcPort) {
        Write-Host "using the server already running on port $rpcPort (bot accounts are not created for it)" -ForegroundColor Yellow
    }
    elseif ($NoServer) {
        throw "nothing is listening on port $rpcPort and -NoServer was given"
    }
    else {
        if (-not $ServerExe) { throw "-ServerExe is needed to start the server" }
        $serverLog = Join-Path $ServerDir 'logs\Server.log'
        $startedAt = Get-Date

        Write-Host "starting the server from $ServerDir ..." -ForegroundColor Cyan
        $info = New-Object Diagnostics.ProcessStartInfo
        $info.FileName = (Resolve-Path $ServerExe).Path
        $info.WorkingDirectory = $ServerDir
        $info.UseShellExecute = $false
        $info.RedirectStandardInput = $true
        $info.RedirectStandardOutput = $true
        $info.RedirectStandardError = $true
        $info.CreateNoWindow = $true
        $server = [Diagnostics.Process]::Start($info)
        # Nothing reads the console output; drain it so the server never blocks on a full pipe.
        $null = $server.StandardOutput.BaseStream.CopyToAsync([IO.Stream]::Null)
        $null = $server.StandardError.BaseStream.CopyToAsync([IO.Stream]::Null)
        # A plain StandardInput writer starts with a byte order mark, which the console reads as part of the first command.
        $console = New-Object IO.StreamWriter($server.StandardInput.BaseStream, (New-Object Text.UTF8Encoding($false)))
        $console.AutoFlush = $true

        $deadline = (Get-Date).AddSeconds(180)
        $ready = $false
        while ((Get-Date) -lt $deadline) {
            if ($server.HasExited) { throw "the server exited with code $($server.ExitCode) while starting; see $serverLog" }
            # The server truncates its log on start, so only a log written since then counts.
            if ((Test-Path $serverLog) -and (Get-Item $serverLog).LastWriteTime -ge $startedAt) {
                $stream = [IO.File]::Open($serverLog, 'Open', 'Read', 'ReadWrite')
                try { $text = (New-Object IO.StreamReader($stream)).ReadToEnd() }
                finally { $stream.Close() }
                if ($text -match 'World initialized') { $ready = $true; break }
            }
            Start-Sleep -Milliseconds 500
        }
        if (-not $ready) { throw "the server was not ready after 180 s; see $serverLog" }
        Write-Host 'server ready' -ForegroundColor Cyan

        # An account that already exists keeps its old password on `create`, so the password is set as well.
        $commands = @()
        foreach ($account in @(Get-Accounts $Target) + @(Get-Accounts "$Target-gm")) {
            $commands += "account create $($account.Name) $($account.Password)"
            $commands += "account set password $($account.Name) $($account.Password) $($account.Password)"
        }
        foreach ($account in @(Get-Accounts "$Target-gm")) { $commands += "account set gmlevel $($account.Name) 4 -1" }
        if ($commands.Count -gt 0) {
            Write-Host "creating the bot accounts ($($commands.Count) console commands; existing ones get their password reset)"
            foreach ($command in $commands) {
                $console.WriteLine($command)
                Start-Sleep -Milliseconds 150
            }
            Start-Sleep -Seconds 2
        }
    }

    if ($UnitExe -and -not $Filter) {
        Write-Host ''
        Write-Host '=== RealBots unit tests ===' -ForegroundColor Cyan
        Push-Location (Split-Path -Parent (Resolve-Path $UnitExe).Path)
        try { & (Resolve-Path $UnitExe).Path '~[live]'; if ($LASTEXITCODE -ne 0) { $exitCode = 1 } }
        finally { Pop-Location }
    }

    $runAll = @{ Config = $effective; Exe = $Exe; Targets = @($Target) }
    if ($Strict) { $runAll['Strict'] = $true }
    if ($Filter) { $runAll['Filter'] = $Filter }
    & (Join-Path $here 'run-all.ps1') @runAll
    if ($LASTEXITCODE -ne 0) { $exitCode = 1 }
}
finally {
    Remove-Item $effective -ErrorAction SilentlyContinue
    if ($server -and -not $server.HasExited) {
        # Closing the console shuts the server down cleanly.
        $server.StandardInput.BaseStream.Close()
        if (-not $server.WaitForExit(60000)) { $server.Kill() }
    }
}

exit $exitCode
