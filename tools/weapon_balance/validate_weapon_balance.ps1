[CmdletBinding()]
param(
    [switch]$ShowAll,
    [switch]$Strict
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$weaponRoot = Join-Path $repoRoot 'gamedata\config\weapons'
$profilePath = Join-Path $repoRoot 'gamedata\scripts\kotovod\sivol_fuzz_recoil_profiles.script'
$masteryPath = Join-Path $repoRoot 'gamedata\scripts\kotovod\sivol_weapon.script'
$identityPath = Join-Path $repoRoot 'WEAPON_ID_MAP.md'

function Get-LuaMap {
    param([string]$Path, [string]$TableName)
    $lines = [IO.File]::ReadAllLines($Path, [Text.Encoding]::UTF8)
    $inside = $false
    $depth = 0
    $result = @{}
    foreach ($line in $lines) {
        if (-not $inside) {
            if ($line -match ('^local\s+' + [regex]::Escape($TableName) + '\s*=\s*\{')) {
                $inside = $true
                $depth = 1
            }
            continue
        }
        $depth += ([regex]::Matches($line, '\{')).Count
        $depth -= ([regex]::Matches($line, '\}')).Count
        foreach ($match in [regex]::Matches($line, '(wpn_[A-Za-z0-9_]+)\s*=\s*"([^"]+)"')) {
            $result[$match.Groups[1].Value] = $match.Groups[2].Value
        }
        if ($depth -le 0) { break }
    }
    return $result
}

function Read-WeaponSections {
    $sections = @{}
    foreach ($file in Get-ChildItem -LiteralPath $weaponRoot -Filter '*.ltx' -File) {
        $current = $null
        foreach ($rawLine in [IO.File]::ReadAllLines($file.FullName, [Text.Encoding]::GetEncoding(1251))) {
            $line = ($rawLine -replace ';.*$', '').Trim()
            if (-not $line) { continue }
            if ($line -match '^\[([^\]]+)\]\s*(?::\s*(.+))?$') {
                $name = $Matches[1].Trim()
                $parents = @()
                if ($Matches[2]) {
                    $parents = @($Matches[2].Split(',') | ForEach-Object { $_.Trim() } | Where-Object { $_ })
                }
                if (-not $sections.ContainsKey($name)) {
                    $sections[$name] = [ordered]@{
                        Parents = $parents
                        Values = @{}
                        File = $file.FullName
                    }
                }
                $current = $sections[$name]
                continue
            }
            if ($current -and $line -match '^([^=]+?)\s*=\s*(.*)$') {
                $current.Values[$Matches[1].Trim()] = $Matches[2].Trim()
            }
        }
    }
    return $sections
}

function Get-EffectiveValue {
    param(
        [hashtable]$Sections,
        [string]$Section,
        [string]$Key,
        [System.Collections.Generic.HashSet[string]]$Visited = $null
    )
    if (-not $Visited) { $Visited = [System.Collections.Generic.HashSet[string]]::new() }
    if (-not $Visited.Add($Section) -or -not $Sections.ContainsKey($Section)) { return $null }
    $entry = $Sections[$Section]
    if ($entry.Values.ContainsKey($Key)) { return $entry.Values[$Key] }
    foreach ($parent in $entry.Parents) {
        $value = Get-EffectiveValue -Sections $Sections -Section $parent -Key $Key -Visited $Visited
        if ($null -ne $value) { return $value }
    }
    return $null
}

function Get-IdentityMap {
    $result = @{}
    $mergedMap = $false
    foreach ($line in [IO.File]::ReadAllLines($identityPath, [Text.Encoding]::UTF8)) {
        if ($line -match '^## Step 3') { $mergedMap = $true; continue }
        if ($mergedMap -and $line -match '^## Step 4') { break }
        if (-not $mergedMap) { continue }
        if ($line -match '^\|\s*(wpn_[^|\s]+)\s*\|\s*(?:—|-)\s*\|\s*base weapon\s*\|\s*([^|]+?)\s*\|') {
            if (-not $result.ContainsKey($Matches[1])) {
                $result[$Matches[1]] = $Matches[2].Trim()
            }
        }
    }
    return $result
}

function Get-CartridgeFamily {
    param([string]$Ammo)
    foreach ($pattern in @(
        '5\.45x39', '5\.56x45', '7\.62x39', '7\.62x51', '7\.62x54',
        '8\.6x70', '9x18', '9x19', '9x39', '11\.43x23', '12x70', '12x76')) {
        if ($Ammo -match $pattern) { return $Matches[0] }
    }
    return $null
}

$sections = Read-WeaponSections
$recoil = Get-LuaMap -Path $profilePath -TableName 'SECTION_CLASS'
$excluded = Get-LuaMap -Path $profilePath -TableName 'EXCLUDED_SECTIONS'
$mastery = Get-LuaMap -Path $masteryPath -TableName 'AUDITED_SECTION_TO_CLASS'
$identities = Get-IdentityMap
$quarantine = @{
    wpn_ak108 = 'RPK-16: post-2012 roster removal candidate'
    wpn_spas15 = 'USAS-12: roster removal candidate'
    wpn_spas15_m1 = 'USAS-12 M1: roster removal candidate'
}

$allSections = @($recoil.Keys + $mastery.Keys | Sort-Object -Unique)
$rows = foreach ($section in $allSections) {
    $ammoClass = Get-EffectiveValue -Sections $sections -Section $section -Key 'ammo_class'
    [pscustomobject]@{
        Section = $section
        Identity = if ($identities[$section]) { $identities[$section] } else { '<unresolved>' }
        AmmoClass = if ($ammoClass) { $ammoClass } else { '<none>' }
        RecoilClass = if ($recoil[$section]) { $recoil[$section] } else { '<missing>' }
        MasteryClass = if ($mastery[$section]) { $mastery[$section] } else { '<missing>' }
        Status = if ($quarantine[$section]) { 'quarantine' }
            elseif ($excluded[$section]) { 'excluded' }
            else { 'active' }
    }
}

$errors = [System.Collections.Generic.List[string]]::new()
$warnings = [System.Collections.Generic.List[string]]::new()
foreach ($row in $rows) {
    if ($row.Status -eq 'active' -and ($row.RecoilClass -eq '<missing>' -or $row.MasteryClass -eq '<missing>')) {
        $errors.Add("class coverage: $($row.Section) recoil=$($row.RecoilClass) mastery=$($row.MasteryClass)")
    }
    if ($row.Identity -eq '<unresolved>') {
        $warnings.Add("identity unresolved: $($row.Section)")
    }
}

$scanExtensions = @('.ltx', '.xml', '.script')
$runtimeFiles = Get-ChildItem -LiteralPath (Join-Path $repoRoot 'gamedata') -Recurse -File |
    Where-Object { $scanExtensions -contains $_.Extension.ToLowerInvariant() }
foreach ($section in $quarantine.Keys) {
    $count = 0
    $files = [System.Collections.Generic.List[string]]::new()
    foreach ($file in $runtimeFiles) {
        $content = [IO.File]::ReadAllText($file.FullName, [Text.Encoding]::GetEncoding(1251))
        $matches = ([regex]::Matches($content, ('(?<![A-Za-z0-9_])' + [regex]::Escape($section) + '(?![A-Za-z0-9_])'))).Count
        if ($matches -gt 0) {
            $count += $matches
            $files.Add($file.FullName.Substring($repoRoot.Length + 1))
        }
    }
    $warnings.Add("quarantine references: $section count=$count files=$($files.Count)")
}

$pairFiles = @(
    'gamedata\config\gameplay\character_desc_nlc30_spawn.xml',
    'gamedata\config\misc\treasure_manager.ltx',
    'gamedata\scripts\new_quests.script',
    'gamedata\scripts\sak\sak_dialog.script'
)
foreach ($relative in $pairFiles) {
    $path = Join-Path $repoRoot $relative
    $lines = [IO.File]::ReadAllLines($path, [Text.Encoding]::GetEncoding(1251))
    for ($index = 0; $index -lt $lines.Count; $index++) {
        foreach ($weaponMatch in [regex]::Matches($lines[$index], '(wpn_[A-Za-z0-9_]+)')) {
            $weapon = $weaponMatch.Groups[1].Value
            if (-not $sections.ContainsKey($weapon)) { continue }
            $ammoClass = Get-EffectiveValue -Sections $sections -Section $weapon -Key 'ammo_class'
            $weaponFamilies = @([regex]::Matches([string]$ammoClass, 'ammo_[A-Za-z0-9\._-]+') |
                ForEach-Object { Get-CartridgeFamily $_.Value } | Where-Object { $_ } | Sort-Object -Unique)
            $window = $lines[$index]
            if (($window -notmatch 'ammo_') -and $relative.EndsWith('.xml') -and
                ($index + 1 -lt $lines.Count)) {
                $window = $lines[$index + 1]
            }
            if ($window -notmatch 'ammo_') { continue }
            foreach ($ammoMatch in [regex]::Matches($window, 'ammo_[A-Za-z0-9\._-]+')) {
                $ammoFamily = Get-CartridgeFamily $ammoMatch.Value
                if ($ammoFamily -and ($weaponFamilies.Count -gt 0) -and
                    ($weaponFamilies -notcontains $ammoFamily)) {
                    $warnings.Add("ammo pair: ${relative}:$($index + 1) $weapon + $($ammoMatch.Value) expected=$($weaponFamilies -join ',')")
                }
            }
        }
    }
}

if ($ShowAll) {
    $rows | Sort-Object Status, MasteryClass, Section | Format-Table -AutoSize
}
Write-Host "Weapon balance validation: sections=$($rows.Count) errors=$($errors.Count) warnings=$($warnings.Count)"
foreach ($errorText in $errors) { Write-Host "ERROR: $errorText" -ForegroundColor Red }
foreach ($warningText in $warnings | Sort-Object -Unique) { Write-Host "WARN:  $warningText" -ForegroundColor Yellow }

if ($errors.Count -gt 0 -or ($Strict -and $warnings.Count -gt 0)) { exit 1 }
