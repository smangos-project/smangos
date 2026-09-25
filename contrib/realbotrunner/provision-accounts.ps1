<#
.SYNOPSIS
    Prints the server-console commands that create a pool of bot accounts, and the matching
    RealBots.Target.<name>.Accounts config line.

.DESCRIPTION
    Bots never create accounts (bot-phase-2.md, "Accounts": they touch no server state), so a target's
    pool is provisioned out-of-band, against that server's own admin console. This script only
    generates text: paste the commands into the server console (or its remote-access console), then put
    the printed Accounts line in your tests-conformance.conf / realbotrunner.conf.

    Each account needs a character for the non-create scenarios; nothing to do here for that -- the
    "login" task creates one on first use (RealBots.Target.<name>.* CreateIfNone).

    Real account lists never go in the repo: keep the printed line in an untracked local config.

.PARAMETER Target
    The target name used in the config keys, e.g. vibemangos (RealBots.Target.vibemangos.Accounts).

.PARAMETER Count
    How many accounts. The [load] scenarios want 10, 50 and 100.

.PARAMETER Prefix
    Account name prefix; accounts are <Prefix>001, <Prefix>002, ... (vibemangos caps names at 16 characters).

.PARAMETER Flavor
    vibemangos, vmangos: `account create <name> <password>` (also provisions the battlenet_accounts row and
    link, AccountMgr::ProvisionBattlenetAccount). vibemangos additionally gets `account set password`
    lines after all the creates, which is what provisions its Battle.net row reliably. trinitycore: `bnetaccount create <name>@<EmailDomain> <password>`.

.PARAMETER Password
    One password for every account. Default: a random one per account.

.PARAMETER EmailDomain
    trinitycore only: the domain of the generated Battle.net emails.

.PARAMETER Start
    First number in the sequence, to add more accounts to an existing pool.

.PARAMETER CommandsFile
    Also write just the console commands to this file (one per line).

.PARAMETER ConsoleInputFile
    Append the console commands to this file instead of only printing them: the input file a running
    start-vibemangos.ps1 forwards to mangosd's console.

.EXAMPLE
    ./provision-accounts.ps1 -Target vibemangos -Count 100 -Prefix bot
    ./provision-accounts.ps1 -Target vibemangos -Count 10 -Prefix vbot -ConsoleInputFile ../../.local/vmport/vibemangos-console-in.txt
    ./provision-accounts.ps1 -Target trinitycore -Flavor trinitycore -Count 10 -Prefix bot -EmailDomain bots
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string] $Target,
    [int] $Count = 10,
    [string] $Prefix = 'bot',
    [ValidateSet('vibemangos', 'vmangos', 'trinitycore')] [string] $Flavor = 'vibemangos',
    [string] $Password,
    [string] $EmailDomain = 'bots',
    [int] $Start = 1,
    [string] $CommandsFile,
    [string] $ConsoleInputFile
)

$ErrorActionPreference = 'Stop'

if ($Count -lt 1) { throw "-Count must be at least 1" }

function New-RandomPassword {
    # Letters and digits only: nothing a console or a config value would need quoting for.
    $alphabet = [char[]]'abcdefghijkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789'
    -join (1..12 | ForEach-Object { $alphabet[(Get-Random -Maximum $alphabet.Length)] })
}

$commands = New-Object System.Collections.Generic.List[string]
$entries = New-Object System.Collections.Generic.List[string]
$followUps = New-Object System.Collections.Generic.List[string]

for ($i = $Start; $i -lt $Start + $Count; $i++) {
    $number = '{0:000}' -f $i
    $name = "$Prefix$number"
    if ($Flavor -ne 'trinitycore' -and $name.Length -gt 16) {
        throw "account name '$name' is longer than vibemangos's 16-character limit; use a shorter -Prefix"
    }

    $pass = if ($Password) { $Password } else { New-RandomPassword }

    if ($Flavor -ne 'trinitycore') {
        $login = $name
        $commands.Add("account create $name $pass")
    }
    else {
        $login = "$name@$EmailDomain"
        $commands.Add("bnetaccount create $login $pass")
    }
    $entries.Add("${login}:${pass}")
    if ($Flavor -eq 'vibemangos') { $followUps.Add("account set password $name $pass $pass") }
}

# vibemangos's `account create` reads the new row back before its async INSERT has landed and so skips
# the battlenet_accounts row; changing the password afterwards provisions it.
foreach ($command in $followUps) { $commands.Add($command) }

Write-Host "# Paste into the $Flavor server console ($Count account(s)):"
$commands | ForEach-Object { Write-Host $_ }
Write-Host ''
Write-Host "# Then in your (untracked) config:"
Write-Host ('RealBots.Target.{0}.Accounts = "{1}"' -f $Target, ($entries -join ' '))

if ($CommandsFile) {
    Set-Content -Path $CommandsFile -Value $commands -Encoding ascii
    Write-Host ''
    Write-Host "# Commands also written to $CommandsFile"
}

if ($ConsoleInputFile) {
    # The file is held open for reading by start-vibemangos.ps1; Add-Content's share mode conflicts with that.
    $stream = New-Object System.IO.FileStream([IO.Path]::GetFullPath($ConsoleInputFile), 'Append', 'Write', 'ReadWrite')
    try {
        $writer = New-Object System.IO.StreamWriter($stream, [System.Text.Encoding]::ASCII)
        foreach ($command in $commands) { $writer.WriteLine($command) }
        $writer.Flush()
    }
    finally { $stream.Dispose() }
    Write-Host ''
    Write-Host "# Commands appended to $ConsoleInputFile"
}
