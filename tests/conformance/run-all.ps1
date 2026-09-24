<#
.SYNOPSIS
    Runs the conformance suite against every configured target in turn and prints one scoreboard per
    target (passed / failed / skipped, and how many scenarios its expectations file leaves out).

.DESCRIPTION
    A target is a RealBots.Target.<name> block named in RealBots.Targets. Each is run as
    `tests-conformance --target <name>`, so its Tags decide which scenarios run ([slow] and [load] only
    when it lists them) and its expectations/<name>.txt decides which are left out as known not to pass.

    With -Strict each target is run a second time with --strict, which runs the scenarios its
    expectations file lists too, and the scoreboard says which of those now PASS -- lines that can come
    out of the file. That is the porting progress measure for the `vmangos` target
    (bot-plan.md, "Testing model").

    Exit code: 0 if every target's normal run passed, 1 otherwise. A newly passing scenario is reported,
    not an error.

    Must be able to start tests-conformance, which has to be run from the directory holding its OpenSSL
    DLLs (the "legacy.dll" working-directory gotcha in src/game/RealBots/README.md); the script changes
    into the executable's directory for you.

.PARAMETER Config
    The tests-conformance config file (accounts and target blocks).

.PARAMETER Exe
    tests-conformance.exe; default: the newest one under <repo>/bin.

.PARAMETER Targets
    Only these targets (default: every one in RealBots.Targets except -Skip).

.PARAMETER Skip
    Targets left out of the default list. "deadhost" is the deliberately unreachable one the framework
    scenario uses via TargetOverride, never a target to run against.

.PARAMETER Strict
    Also run each target with --strict and report the expected-fail scenarios that now pass.

.EXAMPLE
    ./run-all.ps1 -Config C:\bots\tests-conformance.conf -Strict
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string] $Config,
    [string] $Exe,
    [string[]] $Targets,
    [string[]] $Skip = @('deadhost'),
    [switch] $Strict
)

$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path

if (-not (Test-Path $Config)) { throw "config not found: $Config" }
$Config = (Resolve-Path $Config).Path

if (-not $Exe) {
    $found = Get-ChildItem -Path (Join-Path $here '..\..\bin') -Recurse -Filter 'tests-conformance.exe' -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $found) { throw "tests-conformance.exe not found under bin/; build it or pass -Exe" }
    $Exe = $found.FullName
}
$Exe = (Resolve-Path $Exe).Path

if (-not $Targets) {
    $line = Get-Content $Config | Where-Object { $_ -match '^\s*RealBots\.Targets\s*=' } | Select-Object -First 1
    if (-not $line) { throw "no RealBots.Targets line in $Config" }
    $Targets = ([regex]::Match($line, '"([^"]*)"').Groups[1].Value -split '\s+') | Where-Object { $_ -and ($Skip -notcontains $_) }
}

# Scenario names an expectations file lists ("name | reason", '#' comments).
function Get-ExpectedFailures([string] $target) {
    $file = Join-Path $here "expectations\$target.txt"
    if (-not (Test-Path $file)) { return @() }
    Get-Content $file | Where-Object { $_ -match '\|' -and $_.TrimStart() -notmatch '^#' } |
        ForEach-Object { ($_ -split '\|', 2)[0].Trim() }
}

# Runs one pass and returns the junit testcases plus the totals.
function Invoke-Pass([string] $target, [bool] $strict) {
    $xml = Join-Path ([IO.Path]::GetTempPath()) ("conformance-$target-" + [Guid]::NewGuid().ToString('N') + '.xml')
    $arguments = @('--target', $target, '--config', $Config, '-r', 'junit', '-o', $xml)
    if ($strict) { $arguments += '--strict' }

    Push-Location (Split-Path -Parent $Exe)
    try {
        & $Exe @arguments | Out-Null
        $exitCode = $LASTEXITCODE
    }
    finally {
        Pop-Location
    }

    $cases = @()
    if (Test-Path $xml) {
        [xml] $doc = Get-Content $xml -Raw
        foreach ($case in $doc.SelectNodes('//testcase')) {
            $status = 'passed'
            if ($case.SelectSingleNode('skipped')) { $status = 'skipped' }
            elseif ($case.SelectSingleNode('failure') -or $case.SelectSingleNode('error')) { $status = 'failed' }
            $cases += [pscustomobject] @{ Name = $case.name; Status = $status }
        }
        Remove-Item $xml -ErrorAction SilentlyContinue
    }
    [pscustomobject] @{ ExitCode = $exitCode; Cases = $cases }
}

$anyFailed = $false

foreach ($target in $Targets) {
    Write-Host ''
    Write-Host "=== $target ===" -ForegroundColor Cyan

    $expected = @(Get-ExpectedFailures $target)
    $normal = Invoke-Pass $target $false

    $passed = @($normal.Cases | Where-Object Status -eq 'passed').Count
    $failed = @($normal.Cases | Where-Object Status -eq 'failed')
    $skipped = @($normal.Cases | Where-Object Status -eq 'skipped').Count

    Write-Host ("  passed {0}   failed {1}   skipped {2}   expected-fail (left out) {3}" -f $passed, $failed.Count, $skipped, $expected.Count)
    foreach ($case in $failed) { Write-Host "    FAILED: $($case.Name)" -ForegroundColor Red }
    if ($normal.ExitCode -ne 0 -and $failed.Count -eq 0) {
        Write-Host "    tests-conformance exited $($normal.ExitCode) with no failed scenario (did it start? check the config and the OpenSSL DLLs)" -ForegroundColor Red
        $anyFailed = $true
    }
    if ($failed.Count -gt 0) { $anyFailed = $true }

    if ($Strict -and $expected.Count -gt 0) {
        $strictRun = Invoke-Pass $target $true
        $newlyPassing = @($strictRun.Cases | Where-Object { $_.Status -eq 'passed' -and ($expected -contains $_.Name) })
        $stillFailing = @($strictRun.Cases | Where-Object { $_.Status -eq 'failed' -and ($expected -contains $_.Name) })
        Write-Host ("  --strict: {0} newly passing (remove from expectations/{1}.txt), {2} still failing as expected" -f $newlyPassing.Count, $target, $stillFailing.Count)
        foreach ($case in $newlyPassing) { Write-Host "    NOW PASSES: $($case.Name)" -ForegroundColor Green }
    }
}

Write-Host ''
if ($anyFailed) { Write-Host 'result: FAILED' -ForegroundColor Red; exit 1 }
Write-Host 'result: ok' -ForegroundColor Green
exit 0
