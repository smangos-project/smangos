<#
.SYNOPSIS
    Starts vibemangos's bnetserver and mangosd from its run directory with the console enabled, and
    forwards lines appended to a console input file to mangosd.

.DESCRIPTION
    vibemangos is the live reference target: its own mangosd.conf is never edited. mangosd runs from a
    copy of it with Console.Enable = 1 (-c), and its stdin is held open by this script so the console
    stays alive. Closing this script (Ctrl-C) closes stdin, which shuts mangosd down cleanly.

    Bot accounts are created by appending `account create` lines to the console input file, which
    provision-accounts.ps1 -ConsoleInputFile does. Only add accounts this way; never change the
    reference's existing accounts or characters.

    Keep this script running (or run it in the background); it exits when mangosd does.

.PARAMETER RunDir
    vibemangos's run directory (mangosd.exe, bnetserver.exe, mangosd.conf, bnetserver.conf).

.PARAMETER ConfigFile
    Where the console-enabled copy of mangosd.conf is written. Untracked.

.PARAMETER InputFile
    The console input file. Truncated at start so old commands are never replayed.

.PARAMETER NoBnet
    Do not start bnetserver (already running).
#>
[CmdletBinding()]
param(
    [string] $RunDir = (Join-Path $PSScriptRoot '..\..\.local\reference\vibemangos-data\run'),
    [string] $ConfigFile = (Join-Path $PSScriptRoot '..\..\.local\vmport\vibemangos-mangosd.conf'),
    [string] $InputFile = (Join-Path $PSScriptRoot '..\..\.local\vmport\vibemangos-console-in.txt'),
    [switch] $NoBnet
)

$ErrorActionPreference = 'Stop'

$RunDir = (Resolve-Path $RunDir).Path
foreach ($file in 'mangosd.exe', 'bnetserver.exe', 'mangosd.conf', 'bnetserver.conf') {
    if (-not (Test-Path (Join-Path $RunDir $file))) { throw "$file not found in $RunDir" }
}

$running = Get-Process mangosd, bnetserver -ErrorAction SilentlyContinue |
    Where-Object { $_.Path -and $_.Path.StartsWith($RunDir, [StringComparison]::OrdinalIgnoreCase) }
if ($running) { throw "vibemangos is already running (pid $($running.Id -join ', '))" }

$config = Get-Content (Join-Path $RunDir 'mangosd.conf') -Raw
$enabled = [regex]::Replace($config, '(?m)^Console\.Enable\s*=.*$', 'Console.Enable = 1')
if ($enabled -eq $config -and $config -notmatch '(?m)^Console\.Enable\s*=\s*1\s*$') {
    throw "Console.Enable not found in mangosd.conf"
}
$ConfigFile = [IO.Path]::GetFullPath($ConfigFile)
$InputFile = [IO.Path]::GetFullPath($InputFile)
Set-Content -Path $ConfigFile -Value $enabled -Encoding ascii -NoNewline
Set-Content -Path $InputFile -Value '' -Encoding ascii -NoNewline

$bnet = $null
if (-not $NoBnet) {
    $bnet = Start-Process -FilePath (Join-Path $RunDir 'bnetserver.exe') -ArgumentList 'bnetserver.conf' `
        -WorkingDirectory $RunDir -WindowStyle Minimized -PassThru
    Start-Sleep -Seconds 3
}

$info = New-Object System.Diagnostics.ProcessStartInfo
$info.FileName = Join-Path $RunDir 'mangosd.exe'
$info.Arguments = "-c `"$ConfigFile`""
$info.WorkingDirectory = $RunDir
$info.UseShellExecute = $false
$info.RedirectStandardInput = $true
$mangosd = [System.Diagnostics.Process]::Start($info)
$stdin = New-Object System.IO.StreamWriter($mangosd.StandardInput.BaseStream, (New-Object System.Text.UTF8Encoding $false))
$stdin.AutoFlush = $true

Write-Host "vibemangos started: mangosd pid $($mangosd.Id)$(if ($bnet) { ", bnetserver pid $($bnet.Id)" })"
Write-Host "Console input: append lines to $InputFile"

$reader = $null
try {
    $stream = New-Object System.IO.FileStream($InputFile, 'Open', 'Read', 'ReadWrite')
    $reader = New-Object System.IO.StreamReader($stream)
    while (-not $mangosd.HasExited) {
        while ($null -ne ($line = $reader.ReadLine())) {
            if ($line.Trim().Length -gt 0) { $stdin.WriteLine($line) }
        }
        Start-Sleep -Milliseconds 300
    }
    Write-Host "mangosd exited with code $($mangosd.ExitCode)"
}
finally {
    if ($reader) { $reader.Dispose() }
    if (-not $mangosd.HasExited) {
        $stdin.Dispose()
        if (-not $mangosd.WaitForExit(30000)) { $mangosd.Kill() }
    }
    if ($bnet -and -not $bnet.HasExited) { Stop-Process -Id $bnet.Id -Force }
}
