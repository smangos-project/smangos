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
    vibemangos: `account create <name> <password>` (also provisions the battlenet_accounts row and link,
    AccountMgr::ProvisionBattlenetAccount). trinitycore: `bnetaccount create <name>@<EmailDomain> <password>`.

.PARAMETER Password
    One password for every account. Default: a random one per account.

.PARAMETER EmailDomain
    trinitycore only: the domain of the generated Battle.net emails.

.PARAMETER Start
    First number in the sequence, to add more accounts to an existing pool.

.PARAMETER CommandsFile
    Also write just the console commands to this file (one per line).

.EXAMPLE
    ./provision-accounts.ps1 -Target vibemangos -Count 100 -Prefix bot
    ./provision-accounts.ps1 -Target trinitycore -Flavor trinitycore -Count 10 -Prefix bot -EmailDomain bots
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string] $Target,
    [int] $Count = 10,
    [string] $Prefix = 'bot',
    [ValidateSet('vibemangos', 'trinitycore')] [string] $Flavor = 'vibemangos',
    [string] $Password,
    [string] $EmailDomain = 'bots',
    [int] $Start = 1,
    [string] $CommandsFile
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

for ($i = $Start; $i -lt $Start + $Count; $i++) {
    $number = '{0:000}' -f $i
    $name = "$Prefix$number"
    if ($Flavor -eq 'vibemangos' -and $name.Length -gt 16) {
        throw "account name '$name' is longer than vibemangos's 16-character limit; use a shorter -Prefix"
    }

    $pass = if ($Password) { $Password } else { New-RandomPassword }

    if ($Flavor -eq 'vibemangos') {
        $login = $name
        $commands.Add("account create $name $pass")
    }
    else {
        $login = "$name@$EmailDomain"
        $commands.Add("bnetaccount create $login $pass")
    }
    $entries.Add("${login}:${pass}")
}

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
