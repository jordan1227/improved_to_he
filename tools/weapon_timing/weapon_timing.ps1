param(
    [ValidateSet('export','validate','preview','apply','spread')]
    [string]$Action = 'preview',
    [string]$Profiles = (Join-Path $PSScriptRoot 'weapon_timing_profiles.csv'),
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
    [string]$Group = '',
    [double]$ShowSpread = 1.30,
    [double]$HideSpread = 1.15,
    [double]$MinShow = 0.65,
    [double]$MaxShow = 1.32,
    [double]$MinHide = 0.74,
    [double]$MaxHide = 1.22
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$Cp1251 = [Text.Encoding]::GetEncoding(1251)
$Invariant = [Globalization.CultureInfo]::InvariantCulture
$WeaponsRoot = Join-Path $RepoRoot 'gamedata\config\weapons'

function Read-LegacyText([string]$Path) {
    return $Cp1251.GetString([IO.File]::ReadAllBytes($Path))
}

function Write-LegacyText([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, $Cp1251)
}

function Parse-Number([object]$Value, [string]$Label) {
    $result = 0.0
    if (-not [double]::TryParse([string]$Value, [Globalization.NumberStyles]::Float, $Invariant, [ref]$result)) {
        throw "Invalid number for ${Label}: $Value"
    }
    return $result
}

function Format-Number([double]$Value) {
    return $Value.ToString('0.00', $Invariant)
}

function Get-SectionBlocks([string]$Text) {
    return [regex]::Matches($Text, '(?ms)^\s*\[(?<name>[^\]]+)\][^\r\n]*\r?\n(?<body>.*?)(?=^\s*\[|\z)')
}

function Get-KeyValues([string]$Body, [string]$Key) {
    return [regex]::Matches($Body, '(?m)^\s*' + [regex]::Escape($Key) + '\s*=\s*(?<value>[^;\s]+)')
}

function Get-ProfileRows {
    $rows = [Collections.Generic.List[object]]::new()
    foreach ($file in Get-ChildItem -LiteralPath $WeaponsRoot -Filter 'w_*.ltx' -File | Sort-Object Name) {
        $text = Read-LegacyText $file.FullName
        foreach ($block in Get-SectionBlocks $text) {
            $show = @(Get-KeyValues $block.Groups['body'].Value 'anm_show_speed_k')
            $hide = @(Get-KeyValues $block.Groups['body'].Value 'anm_hide_speed_k')
            if ($show.Count -eq 0 -and $hide.Count -eq 0) { continue }
            if ($show.Count -ne 1 -or $hide.Count -ne 1) {
                throw "$($file.Name) [$($block.Groups['name'].Value)] must contain exactly one show and hide speed key"
            }
            $rows.Add([pscustomobject][ordered]@{
                enabled = 'true'
                file = 'gamedata/config/weapons/' + $file.Name
                hud_section = $block.Groups['name'].Value.Trim()
                group = $file.BaseName
                show_speed = $show[0].Groups['value'].Value
                hide_speed = $hide[0].Groups['value'].Value
                notes = ''
            })
        }
    }
    return $rows
}

function Import-Profiles {
    if (-not (Test-Path -LiteralPath $Profiles)) { throw "Profile file not found: $Profiles" }
    return @(Import-Csv -LiteralPath $Profiles -Encoding UTF8 | Where-Object { $_ })
}

function Is-Enabled([object]$Row) {
    return [string]$Row.enabled -match '^(1|true|yes)$'
}

function Validate-Profiles([object[]]$Rows) {
    $seen = @{}
    $count = 0
    foreach ($row in $Rows) {
        if (-not (Is-Enabled $row)) { continue }
        $count++
        $identity = "$($row.file)|$($row.hud_section)"
        if ($seen.ContainsKey($identity)) { throw "Duplicate profile row: $identity" }
        $seen[$identity] = $true
        $path = Join-Path $RepoRoot ([string]$row.file -replace '/', '\')
        if (-not (Test-Path -LiteralPath $path)) { throw "Missing LTX file: $($row.file)" }
        $show = Parse-Number $row.show_speed "$identity show_speed"
        $hide = Parse-Number $row.hide_speed "$identity hide_speed"
        if ($show -lt 0.50 -or $show -gt 1.50 -or $hide -lt 0.50 -or $hide -gt 1.50) {
            throw "Out-of-range timing value in $identity"
        }
        $matches = @(Get-SectionBlocks (Read-LegacyText $path) | Where-Object { $_.Groups['name'].Value.Trim() -eq [string]$row.hud_section })
        if ($matches.Count -ne 1) { throw "$identity resolved to $($matches.Count) sections" }
        foreach ($key in @('anm_show_speed_k','anm_hide_speed_k')) {
            $keyMatches = @(Get-KeyValues $matches[0].Groups['body'].Value $key)
            if ($keyMatches.Count -ne 1) { throw "$identity has $($keyMatches.Count) $key keys" }
        }
    }
    Write-Host "Validated $count enabled timing profiles."
}

function Set-SectionSpeed([string]$Text, [string]$Section, [string]$Key, [string]$Value) {
    $sectionPattern = '(?ms)(^\s*\[' + [regex]::Escape($Section) + '\][^\r\n]*\r?\n)(?<body>.*?)(?=^\s*\[|\z)'
    $sectionMatches = [regex]::Matches($Text, $sectionPattern)
    if ($sectionMatches.Count -ne 1) { throw "Section [$Section] resolved to $($sectionMatches.Count) blocks" }
    $match = $sectionMatches[0]
    $body = $match.Groups['body'].Value
    $keyPattern = '(?m)^(?<prefix>\s*' + [regex]::Escape($Key) + '\s*=\s*)[^;\s]+(?<suffix>[^\r\n]*)(?=\r?$)'
    if ([regex]::Matches($body, $keyPattern).Count -ne 1) { throw "[$Section] must contain exactly one $Key" }
    $body = [regex]::Replace($body, $keyPattern, { param($m) $m.Groups['prefix'].Value + $Value + $m.Groups['suffix'].Value })
    return $Text.Substring(0, $match.Groups['body'].Index) + $body + $Text.Substring($match.Groups['body'].Index + $match.Groups['body'].Length)
}

function Apply-Profiles([object[]]$Rows, [bool]$Write) {
    $byFile = @($Rows) | Where-Object { $_ -and (Is-Enabled $_) } | Group-Object file
    $changeCount = 0
    foreach ($grouped in $byFile) {
        $path = Join-Path $RepoRoot ($grouped.Name -replace '/', '\')
        $original = Read-LegacyText $path
        $updated = $original
        foreach ($row in $grouped.Group) {
            $show = Format-Number (Parse-Number $row.show_speed "$($row.hud_section) show")
            $hide = Format-Number (Parse-Number $row.hide_speed "$($row.hud_section) hide")
            foreach ($key in @('anm_show_speed_k','anm_show_empty_speed_k')) {
                $block = @(Get-SectionBlocks $updated | Where-Object { $_.Groups['name'].Value.Trim() -eq $row.hud_section })[0]
                if (@(Get-KeyValues $block.Groups['body'].Value $key).Count -eq 1) {
                    $updated = Set-SectionSpeed $updated $row.hud_section $key $show
                }
            }
            foreach ($key in @('anm_hide_speed_k','anm_hide_empty_speed_k')) {
                $block = @(Get-SectionBlocks $updated | Where-Object { $_.Groups['name'].Value.Trim() -eq $row.hud_section })[0]
                if (@(Get-KeyValues $block.Groups['body'].Value $key).Count -eq 1) {
                    $updated = Set-SectionSpeed $updated $row.hud_section $key $hide
                }
            }
        }
        if ($updated -ne $original) {
            $changeCount++
            Write-Host "$([string]::Join('', @($(if ($Write) {'WRITE '} else {'WOULD_WRITE '}), $grouped.Name)))"
            if ($Write) { Write-LegacyText $path $updated }
        }
    }
    Write-Host "$changeCount files $($(if ($Write) {'updated.'} else {'would change.'}))"
}

switch ($Action) {
    'export' {
        $rows = Get-ProfileRows
        $rows | Export-Csv -LiteralPath $Profiles -NoTypeInformation -Encoding UTF8
        Write-Host "Exported $($rows.Count) profiles to $Profiles"
    }
    'validate' {
        $rows = Import-Profiles
        Validate-Profiles $rows
    }
    'preview' {
        $rows = Import-Profiles
        Validate-Profiles $rows
        Apply-Profiles $rows $false
    }
    'apply' {
        $rows = Import-Profiles
        Validate-Profiles $rows
        Apply-Profiles $rows $true
    }
    'spread' {
        $rows = Import-Profiles
        Validate-Profiles $rows
        foreach ($row in $rows) {
            if (-not (Is-Enabled $row) -or ($Group -and $row.group -ne $Group)) { continue }
            $show = Parse-Number $row.show_speed 'show_speed'
            $hide = Parse-Number $row.hide_speed 'hide_speed'
            $show = [Math]::Min($MaxShow, [Math]::Max($MinShow, 1 + ($show - 1) * $ShowSpread))
            $hide = [Math]::Min($MaxHide, [Math]::Max($MinHide, 1 + ($hide - 1) * $HideSpread))
            $row.show_speed = Format-Number $show
            $row.hide_speed = Format-Number $hide
        }
        $candidate = Join-Path (Split-Path -Parent $Profiles) 'weapon_timing_profiles.candidate.csv'
        $rows | Export-Csv -LiteralPath $candidate -NoTypeInformation -Encoding UTF8
        Write-Host "Candidate written to $candidate. Review it, then replace the active CSV manually."
    }
}
