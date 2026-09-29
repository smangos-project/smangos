# Loads the three MySQL databases of a runnable server: the base dumps in sql/, then every migration in
# sql/migrations, in name order. The world database is not MySQL: build .world/world.doltlite with
# `database_tool create`. Safe to run again: migrations skip themselves once recorded.
#
# The schemas must already exist and $User must have all privileges on them, e.g. on a scratch
# MariaDB/MySQL instance:
#   CREATE DATABASE vmport_realmd; CREATE DATABASE vmport_characters; CREATE DATABASE vmport_logs;
#   GRANT ALL ON vmport_realmd.* TO 'vmport'@'127.0.0.1' IDENTIFIED BY 'vmport';   (and the other two)
param(
    [string]$Client = "C:\Program Files\MariaDB 13.0\bin\mariadb.exe",
    [string]$DbHost = "127.0.0.1",
    [int]$Port = 3309,
    [string]$User = "vmport",
    [string]$Password = "vmport",
    [string]$Prefix = "vmport",
    [string]$SqlDir = (Join-Path $PSScriptRoot "..\..\sql")
)

$ErrorActionPreference = "Stop"

function Invoke-SqlFile([string]$Database, [string]$File)
{
    $arguments = @("--host=$DbHost", "--port=$Port", "--user=$User", "--password=$Password", "--default-character-set=utf8mb4", $Database)
    $errors = [System.IO.Path]::GetTempFileName()
    $process = Start-Process -FilePath $Client -ArgumentList $arguments -RedirectStandardInput $File -RedirectStandardError $errors -NoNewWindow -Wait -PassThru
    if ($process.ExitCode -ne 0)
    {
        Write-Error "$File into $Database failed: $(Get-Content $errors -Raw)"
    }
    Remove-Item $errors
}

$schemas = @{ logon = "${Prefix}_realmd"; characters = "${Prefix}_characters"; logs = "${Prefix}_logs" }

foreach ($name in @("logon", "characters", "logs"))
{
    Write-Host "$name.sql -> $($schemas[$name])"
    Invoke-SqlFile $schemas[$name] (Join-Path $SqlDir "$name.sql")
}

foreach ($name in @("logon", "characters", "logs"))
{
    $migrations = Get-ChildItem (Join-Path $SqlDir "migrations") -Filter "*_$name.sql" | Sort-Object Name
    Write-Host "$($migrations.Count) $name migrations -> $($schemas[$name])"
    foreach ($migration in $migrations)
    {
        Invoke-SqlFile $schemas[$name] $migration.FullName
    }
}
