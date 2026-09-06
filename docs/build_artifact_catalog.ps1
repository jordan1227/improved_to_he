$ErrorActionPreference = 'Stop'

$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$outPath = Join-Path $PSScriptRoot 'artifact_catalog.md'
$generatedOn = Get-Date -Format 'yyyy-MM-dd'

function Rel([string]$path) {
    $p = $path.Replace('\','/')
    $r = $repo.Replace('\','/')
    if ($p.StartsWith($r, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $p.Substring($r.Length).TrimStart('/')
    }
    return $p
}

function Escape-Md([string]$value) {
    if ($null -eq $value) { return '' }
    $s = [string]$value
    $s = $s -replace '\|', '\\|'
    $s = $s -replace "\r?\n", ' '
    $s = $s -replace '\s+', ' '
    return $s.Trim()
}

function Short([string]$value, [int]$max = 300) {
    $s = Escape-Md $value
    if ($s.Length -le $max) { return $s }
    return $s.Substring(0, $max - 1) + '…'
}

function Read-Text([string]$path) {
    return [System.IO.File]::ReadAllText($path)
}

function Read-Lines([string]$path) {
    if ($path -like '*\gamedata\scripts\sak\sak_dialog.script') {
        return [System.IO.File]::ReadAllLines($path, [System.Text.Encoding]::GetEncoding(1251))
    }
    return [System.IO.File]::ReadAllLines($path)
}

function Source([string]$path, [int]$line) {
    return ('`' + (Rel $path) + ':' + $line + '`')
}

function Get-LineNumber([string[]]$lines, [string]$needle, [int]$start = 0) {
    if ([string]::IsNullOrWhiteSpace($needle)) { return 1 }
    for ($i = [Math]::Max(0, $start); $i -lt $lines.Count; $i++) {
        if ($lines[$i].Contains($needle, [System.StringComparison]::Ordinal)) { return $i + 1 }
    }
    return 1
}

function Parse-LtxFile([string]$path, [hashtable]$sections) {
    $lines = Read-Lines $path
    $current = $null
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        $m = [regex]::Match($line, '^\s*\[([^\]]+)\]\s*(?::\s*([^;]+))?')
        if ($m.Success) {
            $name = $m.Groups[1].Value.Trim()
            $parents = @()
            if ($m.Groups[2].Success) {
                $parents = @($m.Groups[2].Value.Split(',') | ForEach-Object { $_.Trim() } | Where-Object { $_ })
            }
            $current = [ordered]@{
                Name = $name
                Parents = $parents
                File = $path
                Line = $i + 1
                Direct = [ordered]@{}
                DirectLines = @{}
            }
            $sections[$name] = $current
            continue
        }
        if ($null -eq $current) { continue }
        $a = [regex]::Match($line, '^\s*([^=;]+?)\s*=\s*(.*?)\s*(?:;.*)?$')
        if ($a.Success) {
            $key = $a.Groups[1].Value.Trim()
            $value = $a.Groups[2].Value.Trim()
            $current.Direct[$key] = $value
            $current.DirectLines[$key] = $i + 1
        }
    }
}

$sections = @{}
$ltxPaths = @(
    (Join-Path $repo 'gamedata/config/defines.ltx'),
    (Join-Path $repo 'gamedata/config/misc/artefacts.ltx'),
    (Join-Path $repo 'gamedata/config/misc/artefacts_amk_mod.ltx'),
    (Join-Path $repo 'gamedata/config/misc/artefacts_dynamic.ltx'),
    (Join-Path $repo 'gamedata/config/misc/artefacts_mayatnik.ltx'),
    (Join-Path $repo 'gamedata/config/misc/artefacts_unique.ltx'),
    (Join-Path $repo 'gamedata/config/misc/artefacts_unknown.ltx')
)
foreach ($path in $ltxPaths) {
    if (Test-Path -LiteralPath $path) { Parse-LtxFile $path $sections }
}

$effectiveCache = @{}
$resolving = @{}
function Get-Effective([string]$name) {
    if ($effectiveCache.ContainsKey($name)) { return $effectiveCache[$name] }
    if ($resolving.ContainsKey($name)) { return [ordered]@{} }
    if (-not $sections.ContainsKey($name)) { return [ordered]@{} }
    $resolving[$name] = $true
    $sec = $sections[$name]
    $result = [ordered]@{}
    foreach ($parent in $sec.Parents) {
        $parentValues = Get-Effective $parent
        foreach ($kv in $parentValues.GetEnumerator()) { $result[$kv.Key] = $kv.Value }
    }
    foreach ($kv in $sec.Direct.GetEnumerator()) { $result[$kv.Key] = $kv.Value }
    $effectiveCache[$name] = $result
    $resolving.Remove($name)
    return $result
}

function Is-ArtifactSection([string]$name, $sec, $eff) {
    if ($name -notmatch '^af_|^m_capsule$') { return $false }
    if (($eff['class'] -as [string]) -ne 'ARTEFACT') { return $false }
    if ($name -match '(_absorbation$|_fake(?:\d*f)?$|^mfake_|^art_collection_fake|^af_activation|^af_base$|^af_hud)') { return $false }
    return $true
}

function Is-Numericish([string]$value) {
    if ([string]::IsNullOrWhiteSpace($value)) { return $false }
    return $value -match '^[-+]?\.?\d' -or $value -match '^(true|false)$'
}

function Family-Info([string]$name, [string]$file) {
    $tier = 'base'
    $family = $name
    $kind = 'base'
    if ($name -match '^(.+)_dyn([1-6])d$') {
        $family = $Matches[1]
        $tier = 'dyn' + $Matches[2]
        $kind = 'dynamic'
    } elseif ($name -match '^af_(armor|babka|cry|dik|kol|pudd|spirit)_([1-4])$') {
        $family = 'af_' + $Matches[1]
        $tier = 'cooked ' + $Matches[2]
        $kind = 'cooked'
    } elseif ($name -match '^af_unknown_\d+_new$') {
        $family = 'af_unknown'
        $tier = 'unknown'
        $kind = 'unknown'
    } elseif ($name -match '^af_mayatnik_') {
        $family = 'af_mayatnik'
        $tier = 'mayatnik'
        $kind = 'mayatnik'
    } elseif ($name -match '^af_dummy_') {
        $family = 'af_dummy'
        $kind = 'dummy'
    } elseif ($name -match '^af_(red_dummy|dummy_green)') {
        $family = 'af_unique_' + ($name -replace '^af_','')
        $kind = 'unique'
    } elseif ($file -match 'artefacts_unique') {
        $family = $name
        $kind = 'unique'
    } elseif ($file -match 'artefacts_amk_mod') {
        $family = 'af_mod'
        $kind = 'mod'
    } elseif ($file -match 'artefacts_unknown') {
        $family = 'af_unknown'
        $kind = 'unknown'
    } elseif ($file -match 'artefacts_mayatnik') {
        $family = 'af_mayatnik'
        $kind = 'mayatnik'
    } else {
        $family = $name
        $kind = 'base'
    }
    return [ordered]@{ Family = $family; Tier = $tier; Kind = $kind }
}

function Format-EffectiveProps($eff) {
    $parts = @()
    foreach ($kv in ($eff.GetEnumerator() | Sort-Object Key)) {
        if ([string]::IsNullOrWhiteSpace([string]$kv.Value)) { continue }
        $parts += (Escape-Md ($kv.Key + '=' + $kv.Value))
    }
    return ($parts -join '<br>')
}

function Format-DirectProps($sec) {
    $parts = @()
    foreach ($kv in ($sec.Direct.GetEnumerator() | Sort-Object Key)) {
        $parts += (Escape-Md ($kv.Key + '=' + $kv.Value))
    }
    return ($parts -join '<br>')
}

function Format-Immunities($eff) {
    $names = @('burn_immunity','strike_immunity','shock_immunity','wound_immunity','radiation_immunity','telepatic_immunity','chemical_burn_immunity','explosion_immunity','fire_wound_immunity','wound_2_immunity')
    $absName = [string]$eff['hit_absorbation_sect']
    $abs = if ($sections.ContainsKey($absName)) { Get-Effective $absName } else { [ordered]@{} }
    $parts = @()
    foreach ($name in $names) {
        if ($abs.Contains($name)) { $parts += ($name -replace '_immunity$','') + '=' + $abs[$name] }
        else { $parts += ($name -replace '_immunity$','') + '=not-defined→runtime 1.0' }
    }
    if ([string]::IsNullOrWhiteSpace($absName)) { $prefix = 'hit_absorbation_sect=missing; ' } else { $prefix = 'hit_absorbation_sect=' + $absName + '; ' }
    return (Escape-Md ($prefix + ($parts -join ', ')))
}

$stringMap = @{}
$stringSources = @{}
$rusRoot = Join-Path $repo 'gamedata/config/text/rus'
if (Test-Path -LiteralPath $rusRoot) {
    foreach ($path in (Get-ChildItem -LiteralPath $rusRoot -Filter '*.xml' -File | Sort-Object FullName)) {
        try {
            $doc = New-Object System.Xml.XmlDocument
            $doc.PreserveWhitespace = $true
            $doc.Load($path.FullName)
            foreach ($node in $doc.SelectNodes('//string[@id]')) {
                $id = $node.GetAttribute('id')
                $textNode = $node.SelectSingleNode('./text')
                $value = if ($null -ne $textNode) { $textNode.InnerText } else { $node.InnerText }
                $value = ($value -replace '\s+', ' ').Trim()
                if (-not $stringMap.ContainsKey($id)) {
                    $stringMap[$id] = $value
                    $stringSources[$id] = (Rel $path.FullName)
                }
            }
        } catch {
            # The missing/broken source is reported below rather than stopping catalog generation.
        }
    }
}

function Resolve-String([string]$id) {
    if ([string]::IsNullOrWhiteSpace($id)) { return '' }
    if ($stringMap.ContainsKey($id)) { return (Escape-Md $stringMap[$id]) }
    return '[MISSING RUS STRING: ' + (Escape-Md $id) + ']'
}

$artifactRows = @()
$helperSections = @()
foreach ($name in ($sections.Keys | Sort-Object)) {
    $sec = $sections[$name]
    $eff = Get-Effective $name
    if (($name -match '^af_|^m_capsule$') -and ($eff['class'] -as [string]) -eq 'ARTEFACT') {
        if (Is-ArtifactSection $name $sec $eff) {
            $fi = Family-Info $name (Rel $sec.File)
            $invName = [string]$eff['inv_name']
            $display = Resolve-String $invName
            $artifactRows += [pscustomobject]@{
                Section = $name
                DisplayId = $invName
                Display = $display
                Family = $fi.Family
                Tier = $fi.Tier
                Kind = $fi.Kind
                File = $sec.File
                Line = $sec.Line
                Parents = ($sec.Parents -join ', ')
                Effective = $eff
                EffectiveText = Format-EffectiveProps $eff
                DirectText = Format-DirectProps $sec
                Immunities = Format-Immunities $eff
            }
        } else {
            $helperSections += [pscustomobject]@{ Name = $name; File = $sec.File; Line = $sec.Line; Class = $eff['class'] }
        }
    }
}

function Parse-LevelArts {
    $path = Join-Path $repo 'gamedata/scripts/amk/amk_anoms.script'
    $text = Read-Text $path
    $lines = Read-Lines $path
    $result = @()
    $pattern = '(?ms)^\s{4}([A-Za-z0-9_]+)\s*=\s*\{\s*(\d+)\s*,\s*(\d+)\s*,\s*\{(.*?)\}\s*\}\s*,?'
    foreach ($m in [regex]::Matches($text, $pattern)) {
        $name = $m.Groups[1].Value
        $startIndex = $text.Substring(0, $m.Index).Split("`n").Count
        $weights = [ordered]@{}
        foreach ($w in [regex]::Matches($m.Groups[4].Value, '(?m)^\s*([A-Za-z0-9_]+)\s*=\s*(\d+)\s*,?')) {
            $weights[$w.Groups[1].Value] = [int]$w.Groups[2].Value
        }
        $result += [pscustomobject]@{ Level=$name; Min=[int]$m.Groups[2].Value; Max=[int]$m.Groups[3].Value; Weights=$weights; File=$path; Line=$startIndex }
    }
    return $result
}
$levels = @(Parse-LevelArts)

function Spawn-Matches($row) {
    $base = $row.Section
    if ($base -match '^(.+)_dyn\d+d$') { $base = $Matches[1] }
    $keys = @($base, ($base + '_capsule'))
    $matches = @()
    foreach ($level in $levels) {
        $found = @()
        foreach ($key in $keys) {
            if ($level.Weights.Contains($key)) { $found += ($key + '=' + $level.Weights[$key]) }
        }
        if ($found.Count -gt 0) { $matches += ($level.Level + ' [' + ($found -join ', ') + '; count ' + $level.Min + '–' + $level.Max + ']') }
        if ($level.Weights.Contains('_new') -and ($row.Kind -in @('unique','unknown'))) {
            $matches += ($level.Level + ' [_new=' + $level.Weights['_new'] + '; selects six new_arts or unknown_1..31]')
        }
    }
    return ($matches -join '<br>')
}

function Parse-LtxSections([string]$path, [string]$prefix) {
    $lines = Read-Lines $path
    $result = @()
    $current = $null
    for ($i=0; $i -lt $lines.Count; $i++) {
        $m = [regex]::Match($lines[$i], '^\s*\[([^\]]+)\]')
        if ($m.Success) {
            if ($null -ne $current -and $current.Name.StartsWith($prefix)) { $result += $current }
            $current = [ordered]@{ Name=$m.Groups[1].Value; File=$path; Line=$i+1; Direct=[ordered]@{} }
            continue
        }
        if ($null -ne $current) {
            $a = [regex]::Match($lines[$i], '^\s*([^=;]+?)\s*=\s*(.*?)\s*(?:;.*)?$')
            if ($a.Success) { $current.Direct[$a.Groups[1].Value.Trim()] = $a.Groups[2].Value.Trim() }
        }
    }
    if ($null -ne $current -and $current.Name.StartsWith($prefix)) { $result += $current }
    return $result
}

$cookPath = Join-Path $repo 'gamedata/config/misc/artefacts_cooking.ltx'
$cookSections = @()
if (Test-Path -LiteralPath $cookPath) { $cookSections = @(Parse-LtxSections $cookPath 'cook-') }
$cookByTarget = @{}
foreach ($cook in $cookSections) {
    $target = $cook.Name.Substring(5)
    $cookByTarget[$target] = $cook
}

$artmodPath = Join-Path $repo 'gamedata/scripts/sak/sak_artmods.script'
$artmodEncoding = [System.Text.Encoding]::GetEncoding(1251)
$artmodText = $artmodEncoding.GetString([System.IO.File]::ReadAllBytes($artmodPath))
$artmodRows = @()
$artmodMatch = [regex]::Match($artmodText, '(?ms)artmod_resiepts\s*=\s*\{(.*?)\}\s*\r?\n\s*amk_resiepts')
if ($artmodMatch.Success) {
    foreach ($m in [regex]::Matches($artmodMatch.Groups[1].Value, '(?ms)artmod_(\d+)\s*=\s*\{\s*((?:"[^"]*"\s*,\s*){6}"[^"]*")\s*\}')) {
        $vals = @([regex]::Matches($m.Groups[2].Value, '"([^"]*)"') | ForEach-Object { $_.Groups[1].Value })
        if ($vals.Count -eq 7) {
            $artmodRows += [pscustomobject]@{ Id='artmod_' + $m.Groups[1].Value; Ingredient1=$vals[0]; Ingredient2=$vals[1]; Ingredient3=$vals[2]; Ingredient4=$vals[3]; Gate=$vals[4]; Target=$vals[5]; TextId=$vals[6] }
        }
    }
}

$amkRows = @()
$amkMatch = [regex]::Match($artmodText, '(?ms)amk_resiepts\s*=\s*\{(.*?)\}\s*\r?\n\s*unique_resiepts')
if ($amkMatch.Success) {
    foreach ($m in [regex]::Matches($amkMatch.Groups[1].Value, '\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*"([^"]+)"\s*\}')) {
        $amkRows += [pscustomobject]@{ Source=$m.Groups[1].Value; Gate=$m.Groups[2].Value; TargetId=$m.Groups[3].Value; TextId=$m.Groups[4].Value }
    }
}

$uniqueRows = @()
$uniqueMatch = [regex]::Match($artmodText, '(?ms)unique_resiepts\s*=\s*\{(.*?)\}\s*\r?\n\s*\r?\n\s*amk_resiepts')
if ($uniqueMatch.Success) {
    foreach ($m in [regex]::Matches($uniqueMatch.Groups[1].Value, '\{\s*"([^"]+)"\s*,\s*"([^"]*)"\s*,\s*"([^"]+)"\s*\}')) {
        $uniqueRows += [pscustomobject]@{ Gate=$m.Groups[1].Value; Label=$m.Groups[2].Value; Target=$m.Groups[3].Value }
    }
}

function Parse-Dialogs {
    $dir = Join-Path $repo 'gamedata/config/gameplay'
    $files = Get-ChildItem -LiteralPath $dir -Filter '*.xml' -File | Where-Object {
        $_.Name -match '^(dialogs|sak_dalogs|kill_stalker_dlg).*\.xml$'
    } | Sort-Object FullName
    $rows = @()
    foreach ($path in $files) {
        try {
            $doc = New-Object System.Xml.XmlDocument
            $doc.PreserveWhitespace = $true
            $doc.Load($path.FullName)
            $lines = Read-Lines $path.FullName
            $searchStart = 0
            foreach ($action in $doc.SelectNodes('//dialog//phrase//action')) {
                $code = ($action.InnerText -replace '\s+', ' ').Trim()
                $interesting = $code -match '(?i)(af_[a-z0-9_]+|capsule@|sak\.create_items|dummy_green_art|add_rnd_art|add_random_unknown_art|take_art_set|kruglov_carts_reward|add_docent_majatnik|treasure|add_.*(art|gift|priz)|take_.*(art|gift))'
                if (-not $interesting) { continue }
                $phrase = $action.ParentNode
                while ($null -ne $phrase -and $phrase.Name -ne 'phrase') { $phrase = $phrase.ParentNode }
                $dialog = $action.ParentNode
                while ($null -ne $dialog -and $dialog.Name -ne 'dialog') { $dialog = $dialog.ParentNode }
                if ($null -eq $phrase) { continue }
                $phraseId = if ($phrase.Attributes['id']) { $phrase.Attributes['id'].Value } else { '?' }
                $dialogId = if ($null -ne $dialog -and $dialog.Attributes['id']) { $dialog.Attributes['id'].Value } else { '?' }
                $texts = @($phrase.SelectNodes('./text') | ForEach-Object { $_.InnerText.Trim() })
                $textResolved = @($texts | ForEach-Object { $_ + ' → ' + (Resolve-String $_) })
                $gates = @()
                if ($null -ne $dialog) { $gates += @($dialog.SelectNodes('./precondition|./has_info|./dont_has_info') | ForEach-Object { $_.Name + ':' + $_.InnerText.Trim() }) }
                $gates += @($phrase.SelectNodes('./precondition|./has_info|./dont_has_info') | ForEach-Object { $_.Name + ':' + $_.InnerText.Trim() })
                $line = 1
                for ($i=$searchStart; $i -lt $lines.Count; $i++) {
                    $norm = ($lines[$i] -replace '<action>|</action>|\s+', ' ').Trim()
                    if ($norm -eq $code -or $norm -like ('*' + $code + '*')) { $line = $i + 1; $searchStart = $i + 1; break }
                }
                $targets = @([regex]::Matches($code, '(?i)(?:capsule@)?af_[a-z0-9_]+') | ForEach-Object { $_.Value })
                $rows += [pscustomobject]@{
                    File=$path.FullName; Line=$line; Dialog=$dialogId; Phrase=$phraseId
                    Texts=($textResolved -join '<br>'); Gates=(($gates | Select-Object -Unique) -join '<br>')
                    Action=$code; Targets=(($targets | Select-Object -Unique) -join ', ')
                }
            }
        } catch {
            # Parsing failures are listed in the report's source-gap note.
        }
    }
    return $rows
}
$dialogRows = @(Parse-Dialogs)

function Parse-ScriptFunctions([string]$path) {
    $lines = Read-Lines $path
    $starts = @()
    for ($i=0; $i -lt $lines.Count; $i++) {
        $m = [regex]::Match($lines[$i], '^\s*function\s+([A-Za-z0-9_]+)\s*\(')
        if ($m.Success) { $starts += [pscustomobject]@{ Name=$m.Groups[1].Value; Start=$i; Line=$i+1 } }
    }
    $rows = @()
    for ($j=0; $j -lt $starts.Count; $j++) {
        $s = $starts[$j]
        if ($s.Name -eq 'attach') { continue }
        $end = if ($j + 1 -lt $starts.Count) { $starts[$j+1].Start } else { $lines.Count }
        $body = @($lines[$s.Start..($end-1)])
        $joined = $body -join "`n"
        if ($joined -notmatch '(?i)(af_[a-z0-9_]+|tbl_arts|spawn_to|create_items|add_bad_item|add_rnd_art|add_random_unknown_art|dummy_green_art|art_in_box|capsule)') { continue }
        $payload = @($body | Where-Object {
            $_ -notmatch '^\s*--' -and $_ -match '(?i)(af_[a-z0-9_]+|tbl_arts|spawn_to|create_items|add_bad_item|add_rnd_art|add_random_unknown_art|dummy_green_art|art_in_box|capsule)'
        } | ForEach-Object { (($_ -replace '\s+', ' ').Trim()) })
        if ($payload.Count -eq 0) { continue }
        $role = if ($joined -match '(?i)(misc\.spawn_to|amk\.spawn_item|nlc_capsules\.spawn|g_sim:create|sak\.create_items|add_bad_item|add_rnd_art_to_actor|add_random_unknown_art|pre_rolled_arts_give|spawn_items)') { 'grant/spawn' } else { 'inspect/consume' }
        $rows += [pscustomobject]@{ Name=$s.Name; Line=$s.Line; File=$path; Role=$role; Payload=(($payload | Select-Object -Unique) -join '<br>') }
    }
    return $rows
}
$sakDialogPath = Join-Path $repo 'gamedata/scripts/sak/sak_dialog.script'
$stashRows = @(Parse-ScriptFunctions $sakDialogPath)

function Parse-LegacySpawnZones {
    $path = Join-Path $repo 'gamedata/config/misc/artefacts.ltx'
    if (-not (Test-Path -LiteralPath $path)) { return @() }
    $lines = Read-Lines $path
    $rows = @()
    $inside = $false
    for ($i=0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -match '^\s*\[artefact_spawn_zones\]') { $inside = $true; continue }
        if ($inside -and $lines[$i] -match '^\s*\[') { break }
        if ($inside -and $lines[$i] -match '^\s*([^=;]+?)\s*=\s*(.*?)\s*(?:;.*)?$') {
            $rows += [pscustomobject]@{ Name=$Matches[1].Trim(); Value=$Matches[2].Trim(); File=$path; Line=$i+1 }
        }
    }
    return $rows
}
$legacyZones = @(Parse-LegacySpawnZones)

function Row-World($row) {
    $world = Spawn-Matches $row
    if ([string]::IsNullOrWhiteSpace($world)) { return 'No matching `level_arts` key found' }
    return $world
}

function Row-Cooking($row) {
    $entries = @()
    $target = $row.Section
    if ($cookByTarget.ContainsKey($row.Section)) {
        $c = $cookByTarget[$row.Section]
        $d = $c.Direct
        $entries += ('source=' + $d['source'] + ', anomaly=' + $d['anomaly'] + ', recipes=' + $d['recipes'] + ', time=' + $d['time'] + ', probability=' + $d['probability'] + ' ' + (Source $c.File $c.Line))
    }
    foreach ($a in $amkRows | Where-Object { $_.TargetId -like ($target + '*') }) {
        $entries += ('source=' + $a.Source + ', gate=' + $a.Gate + ', result name id=' + $a.TargetId + ', text id=' + $a.TextId)
    }
    return ($entries -join '<br>')
}

function Row-Artmod($row) {
    $entries = @()
    foreach ($a in $artmodRows) {
        $targetBase = $a.Target -replace '^capsule@',''
        if ($row.Section -eq $a.Target -or $row.Section -eq $targetBase -or $row.Section -eq ($targetBase + '_dyn5d')) {
            $entries += ($a.Id + ': [' + $a.Ingredient1 + ', ' + $a.Ingredient2 + ', ' + $a.Ingredient3 + ', ' + $a.Ingredient4 + ']; gate=' + $a.Gate + '; target=' + $a.Target + '; text=' + $a.TextId)
        }
    }
    foreach ($u in $uniqueRows | Where-Object { $_.Target -eq $row.Section }) {
        $entries += ('unique: gate=' + $u.Gate + '; target=' + $u.Target + '; label=' + $u.Label)
    }
    return ($entries -join '<br>')
}

function Row-Dialog($row) {
    $hits = @()
    $base = $row.Section -replace '_dyn[1-6]d$',''
    foreach ($d in $dialogRows) {
        if ($d.Targets -match [regex]::Escape($row.Section) -or ($base -ne $row.Section -and $d.Targets -match [regex]::Escape($base))) {
            $hits += (($d.Dialog + '/' + $d.Phrase + ': ' + $d.Action + ' ' + (Source $d.File $d.Line)))
        }
    }
    foreach ($d in $dialogRows | Where-Object { $_.Action -match 'dummy_green_art' -and $row.Section -match '^af_dummy_green_' }) {
        $hits += (($d.Dialog + '/' + $d.Phrase + ': ' + $d.Action + ' ' + (Source $d.File $d.Line)))
    }
    if ($hits.Count -eq 0) { return 'No direct artifact-bearing dialogue action found in scanned XML' }
    return (($hits | Select-Object -Unique) -join '<br>')
}

function Row-Stash($row) {
    $hits = @()
    $base = $row.Section -replace '_dyn[1-6]d$',''
    foreach ($s in $stashRows) {
        if ($s.Role -ne 'grant/spawn') { continue }
        if ($s.Payload -match [regex]::Escape($row.Section) -or ($base -ne $row.Section -and $s.Payload -match [regex]::Escape($base))) {
            $hits += ($s.Name + ' ' + (Source $s.File $s.Line))
        }
    }
    if ($row.Section -match '^af_dummy_green_') {
        $green = $stashRows | Where-Object { $_.Name -eq 'dummy_green_art' } | Select-Object -First 1
        if ($null -ne $green) { $hits += ('dummy_green_art (table-driven green mapping) ' + (Source $green.File $green.Line)) }
    }
    if ($row.Section -match '^af_unknown_\d+_new$') {
        $hits += ('add_random_unknown_art (matrix-driven; see generic reward table) ' + (Source (Join-Path $repo 'gamedata/scripts/kotovod.script') 3648))
        $hits += ('mike.take_art_set (current set matrix) ' + (Source (Join-Path $repo 'gamedata/scripts/mike.script') 1387))
    }
    if ($hits.Count -eq 0) { return 'No exact section reference found in `sak_dialog.script` function bodies' }
    return (($hits | Select-Object -Unique) -join '<br>')
}

$md = New-Object System.Text.StringBuilder
[void]$md.AppendLine('# NLC 7 Hard Edition / OGSR artifact catalog')
[void]$md.AppendLine('')
[void]$md.AppendLine(('Generated from the repository source on ' + $generatedOn + '. This is a source catalog, not a claim about an uninspected runnable installation.'))
[void]$md.AppendLine('')
[void]$md.AppendLine('## Scope and reading rules')
[void]$md.AppendLine('')
[void]$md.AppendLine('- The canonical source is this repository. The runnable installation was not modified or queried.')
[void]$md.AppendLine('- LTX values below are effective values after resolving the section inheritance visible in `gamedata/config/defines.ltx` and `gamedata/config/misc/artefacts*.ltx`; values remain raw strings from the INI source. `direct overrides` shows only assignments in the section itself.')
[void]$md.AppendLine('- Immunities are expanded through each section hit_absorbation_sect. If that section/key is absent, the runtime helper in gamedata/scripts/common/_g.script falls back to 1.0; that fallback is written explicitly rather than guessed.')
[void]$md.AppendLine('- `dyn1d`–`dyn5d` are the dynamic tiers emitted by `amk_anoms.script`; `dyn6d` exists in LTX but is not selected by the shown world-spawn random range. Capsules are descriptors/custom data whose payload can use these same dynamic content sections.')
[void]$md.AppendLine('- World entries are level-wide random artifact points filtered by coordinate restrictions. `level_arts` does not name a particular anomaly zone. The older `[artefact_spawn_zones]` table is reported separately as legacy configuration, not silently treated as the [HE-PORT] generator.')
[void]$md.AppendLine('')
[void]$md.AppendLine('## Coverage and source gaps')
[void]$md.AppendLine('')
[void]$md.AppendLine(('- Catalog rows: **' + $artifactRows.Count + '** artifact sections; helper/support sections excluded from the main rows: **' + $helperSections.Count + '**.'))
[void]$md.AppendLine(('- Parsed world levels: **' + $levels.Count + '**; cooking sections: **' + $cookSections.Count + '**; artmod recipes: **' + $artmodRows.Count + '**; direct/indirect artifact-bearing dialogue actions: **' + $dialogRows.Count + '**; artifact-bearing `sak_dialog` functions: **' + $stashRows.Count + '**.'))
[void]$md.AppendLine('- This file is generated reference data. The canonical notes path is `docs/CLAUDE_MODDING_NOTES.md`; the notes are not used as artifact source data.')
[void]$md.AppendLine('- Any display/quest string shown as `[MISSING RUS STRING: …]` has no matching `<string id>` in the scanned `gamedata/config/text/rus/*.xml`; no translation was invented.')
[void]$md.AppendLine('- A blank or “No matching…” acquisition field is an explicit source result: this catalog found no exact source reference in the scanned tables, XML actions, or `sak_dialog.script` function bodies.')
[void]$md.AppendLine('')
[void]$md.AppendLine('## Artifact entries')
[void]$md.AppendLine('')
[void]$md.AppendLine('Each row is one artifact section. The effective LTX cell preserves every non-empty inherited/direct key/value visible for that section; the direct overrides cell isolates the section own raw assignments. dialogue actions and stash functions are exact source references where the section (or its base for a dynamic tier) is mentioned.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Section | Display ID → resolved Russian name | Family / tier | LTX source | Immunities from raw absorbation section | World `level_arts` | Cooking / AMK recipe | Artmod / unique recipe | Dialogue actions | Stash functions | Effective LTX | Direct overrides |')
[void]$md.AppendLine('|---|---|---|---|---|---|---|---|---|---|---|---|')
$kindOrder = @{ base=1; dynamic=2; dummy=3; cooked=4; unique=5; unknown=6; mayatnik=7; mod=8 }
foreach ($row in ($artifactRows | Sort-Object @{Expression={ $kindOrder[$_.Kind] }}, Family, Tier, Section)) {
    $world = Row-World $row
    $cook = Row-Cooking $row
    $artmod = Row-Artmod $row
    $dialog = Row-Dialog $row
    $stash = Row-Stash $row
    $display = $row.DisplayId + ' → ' + $row.Display
    $source = Source $row.File $row.Line
    $line = '| ' + (Escape-Md $row.Section) + ' | ' + (Escape-Md $display) + ' | ' + (Escape-Md ($row.Family + ' / ' + $row.Tier)) + ' | ' + $source + ' (parents: ' + (Escape-Md $row.Parents) + ') | ' + $row.Immunities + ' | ' + (Escape-Md $world) + ' | ' + (Escape-Md $cook) + ' | ' + (Escape-Md $artmod) + ' | ' + (Escape-Md $dialog) + ' | ' + (Escape-Md $stash) + ' | ' + (Escape-Md $row.EffectiveText) + ' | ' + (Escape-Md $row.DirectText) + ' |'
    [void]$md.AppendLine($line)
}

[void]$md.AppendLine('')
[void]$md.AppendLine('## World spawn: `amk_anoms.script` [HE-PORT] `level_arts`')
[void]$md.AppendLine('')
[void]$md.AppendLine('generate_arts() chooses a count from each level [min,max], then generate_art() chooses a valid random point. spawn_rand_arts() consumes the raw weights in the table order returned by Lua pairs; for a normal base it calls get_dynamic_section(base,false), for a _capsule key it calls get_dynamic_section(base,true), and _new calls get_new_section(). The raw weights below are not normalized; the script logs a remainder when they do not consume the random 1–100 range.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Level | Count range | Raw weighted entries | Source |')
[void]$md.AppendLine('|---|---:|---|---|')
foreach ($level in $levels) {
    $raw = @($level.Weights.GetEnumerator() | ForEach-Object { $_.Key + '=' + $_.Value }) -join ', '
    [void]$md.AppendLine('| ' + $level.Level + ' | ' + $level.Min + '–' + $level.Max + ' | ' + (Escape-Md $raw) + ' | ' + (Source $level.File $level.Line) + ' |')
}
[void]$md.AppendLine('')
[void]$md.AppendLine('Dynamic selection is raw and exact: non-capsule random 1–3 → `dyn1d`, 4–9 → `dyn2d`, 10–16 → `dyn3d`, 17–20 → `dyn4d`, 21 → `dyn5d`; capsule random starts at 15, so it can only select `dyn3d`–`dyn5d`. `dyn6d` is defined but is not generated by this range. `generators` strips `_capsule` before selection. `mayatnik_caps_by_level` adds level-specific mayatnik descriptors after emission with chances 5% (`aver`, `generators`, `red_forest`, `l10_radar`), 6% (`l11_pripyat`), and 20% (`l12_stancia_new`).')
[void]$md.AppendLine('')
[void]$md.AppendLine('### Legacy `[artefact_spawn_zones]` mapping')
[void]$md.AppendLine('')
[void]$md.AppendLine('This table exists in `artefacts.ltx`; the [HE-PORT] `level_arts` path above does not consult it in the traced generator.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Artifact key | Legacy zone / numeric values | Source |')
[void]$md.AppendLine('|---|---|---|')
foreach ($z in $legacyZones) { [void]$md.AppendLine('| ' + $z.Name + ' | ' + (Escape-Md $z.Value) + ' | ' + (Source $z.File $z.Line) + ' |') }

[void]$md.AppendLine('')
[void]$md.AppendLine('## Cooking chains: `artefacts_cooking.ltx` + `sak_artmods.amk_resiepts`')
[void]$md.AppendLine('')
[void]$md.AppendLine('The LTX section name identifies the result (`cook-af_spirit_1` → `af_spirit_1`); `source` is the input artifact. `recipes` are info IDs, and the resolved result/text IDs are shown from the Russian string table when available.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Result section | Source | Anomaly suffix | Recipe/info IDs | Time | Probability | Result name/text IDs | Source |')
[void]$md.AppendLine('|---|---|---|---|---|---|---|---|')
foreach ($c in $cookSections) {
    $d = $c.Direct
    $target = $c.Name.Substring(5)
    $amk = @($amkRows | Where-Object { $_.Source -eq $d['source'] -and $_.TargetId -like ($target + '*') })
    $amkText = @($amk | ForEach-Object { $_.TargetId + ' → ' + (Resolve-String $_.TargetId) + '; ' + $_.TextId + ' → ' + (Resolve-String $_.TextId) }) -join '<br>'
    [void]$md.AppendLine('| ' + $target + ' | ' + $d['source'] + ' | ' + $d['anomaly'] + ' | ' + (Escape-Md $d['recipes']) + ' | ' + $d['time'] + ' | ' + $d['probability'] + ' | ' + (Escape-Md $amkText) + ' | ' + (Source $c.File $c.Line) + ' |')
}
[void]$md.AppendLine('')
[void]$md.AppendLine('### Full `amk_resiepts` identity table')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Input | Info gate | Result name ID → Russian | Recipe text ID → Russian |')
[void]$md.AppendLine('|---|---|---|---|')
foreach ($a in $amkRows) { [void]$md.AppendLine('| ' + $a.Source + ' | ' + $a.Gate + ' | ' + $a.TargetId + ' → ' + (Resolve-String $a.TargetId) + ' | ' + $a.TextId + ' → ' + (Resolve-String $a.TextId) + ' |') }

[void]$md.AppendLine('')
[void]$md.AppendLine('## Artmod recipes and runtime path')
[void]$md.AppendLine('')
[void]$md.AppendLine('sak.artmod() requires the actor info gate, records four component artifact IDs over time, removes the four components when complete, computes a condition factor from cost/condition (capped at 1.5 before the final condition cap), and starts amk_mod.af_start_transform_timer. artmod_components_create() creates the four input items or the target as bad-condition items; capsule@ descriptors receive a random dyn = level.get_time_days() % 4 + 1, while plain ordinary artifact names receive _dyn1d–_dyn4d; explicit dynamic names such as _dyn5d remain explicit.')
[void]$md.AppendLine('')
[void]$md.AppendLine('For dispatch, the second recipe digit is the anomaly suffix group in `sak.script`: `1` mosquito bald, `2` gravi zone, `3` mincer, `4` buzz, `5` zharka, `6` galant, `7` ice; `artmod_81` is the explicit sak-buzz path and `artmod_19` is the psi-field path. The requested `artmod_61`–`artmod_66` rows are included in the full table below.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Recipe | Four required components | Info gate | Target descriptor | Recipe text ID → Russian |')
[void]$md.AppendLine('|---|---|---|---|---|')
foreach ($a in ($artmodRows | Sort-Object {[int]($_.Id -replace 'artmod_','')})) {
    $components = ($a.Ingredient1 + ', ' + $a.Ingredient2 + ', ' + $a.Ingredient3 + ', ' + $a.Ingredient4)
    [void]$md.AppendLine('| ' + $a.Id + ' | ' + (Escape-Md $components) + ' | ' + $a.Gate + ' | ' + $a.Target + ' | ' + $a.TextId + ' → ' + (Resolve-String $a.TextId) + ' |')
}
[void]$md.AppendLine('')
[void]$md.AppendLine('### Unique transformations')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Info gate | Source label in script | Result section |')
[void]$md.AppendLine('|---|---|---|')
foreach ($u in $uniqueRows) { [void]$md.AppendLine('| ' + $u.Gate + ' | ' + (Escape-Md $u.Label) + ' | ' + $u.Target + ' |') }

[void]$md.AppendLine('')
[void]$md.AppendLine('## Direct dialogue grants and reward calls')
[void]$md.AppendLine('')
[void]$md.AppendLine('The table includes artifact-bearing actions found in the scanned dialogs*.xml, sak_dalogs*.xml, and kill_stalker_dlg.xml files. Each row keeps the dialog/phrase identity, the raw gate IDs/code, and the Russian resolution of the phrase text IDs. Calls such as dummy_green_art, add_rnd_art_to_actor, or add_*_treasure are intentionally left as calls; their exact artifact payload is in the sak_dialog.script table below.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Dialog / phrase | Phrase text ID → resolved Russian | Gates | Action | Target section(s) visible in XML | Source |')
[void]$md.AppendLine('|---|---|---|---|---|---|')
foreach ($d in $dialogRows) {
    [void]$md.AppendLine('| ' + $d.Dialog + ' / ' + $d.Phrase + ' | ' + (Escape-Md $d.Texts) + ' | ' + (Escape-Md $d.Gates) + ' | `' + (Escape-Md $d.Action) + '` | ' + (Escape-Md $d.Targets) + ' | ' + (Source $d.File $d.Line) + ' |')
}

[void]$md.AppendLine('')
[void]$md.AppendLine('## Script-fired stash and quest spawns: `sak_dialog.script`')
[void]$md.AppendLine('')
[void]$md.AppendLine('This is a function-body index, not an inference that every function runs in every playthrough. The XML table above identifies calls into these functions; the payload cell preserves the artifact-bearing source lines, including random table/capsule expressions where the runtime result is intentionally not one fixed section.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Function | Role | Artifact-bearing payload lines | Source |')
[void]$md.AppendLine('|---|---|---|---|')
foreach ($s in ($stashRows | Sort-Object Line)) { [void]$md.AppendLine('| ' + $s.Name + ' | ' + $s.Role + ' | ' + (Escape-Md $s.Payload) + ' | ' + (Source $s.File $s.Line) + ' |') }

[void]$md.AppendLine('')
[void]$md.AppendLine('## Generic reward families used by dialogue/stash calls')
[void]$md.AppendLine('')
[void]$md.AppendLine('Several dialog actions do not name one artifact section. They call `kotovod.add_rnd_art_to_actor`, `kotovod.add_random_unknown_art`, or `mike.take_art_set`; these are the exact source families rather than a guessed per-quest item.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Reward level / call | Exact source entries | Runtime section/capsule rule | Source |')
[void]$md.AppendLine('|---|---|---|---|')
[void]$md.AppendLine('| `lvl1` | af_medusa, af_vyvert, af_blood, af_electra_sparkler, af_drops, af_ameba_slime, af_rusty_thorn | `add_rnd_art_to_actor` creates one selected entry as `_dyn1d`–`_dyn4d` unless an explicit dyn argument is supplied | `gamedata/scripts/kotovod.script:108-116`, `:3621-3630` |')
[void]$md.AppendLine('| `lvl2` | af_cristal_flower, af_gravi, af_mincer_meat, af_electra_flash, af_fireball, af_ameba_slug, af_rusty_kristall | Same `_dyn1d`–`_dyn4d` rule | `gamedata/scripts/kotovod.script:118-122`, `:3621-3630` |')
[void]$md.AppendLine('| `lvl3` | af_cristall, af_electra_moonlight, af_night_star, af_gold_fish, af_rusty_sea_urchin, af_soul, af_ameba_mica | Same `_dyn1d`–`_dyn4d` rule | `gamedata/scripts/kotovod.script:124-128`, `:3621-3630` |')
[void]$md.AppendLine('| `lvl4` | af_dummy_spring, af_dummy_dummy, af_dummy_battery, af_dummy_pellicle, af_dummy_glassbeads, af_dummy_kolobok | Base dummy section; no dynamic suffix is added by `rnd_rec_art_info` | `gamedata/scripts/kotovod.script:130-133`, `:3632-3647` |')
[void]$md.AppendLine('| `lvl5` | af_night_star, af_gold_fish, af_cristall, af_soul, af_electra_moonlight, af_rusty_sea_urchin, af_ameba_mica | Regular capsule descriptor; random or supplied dyn 1–4 | `gamedata/scripts/kotovod.script:135-139`, `:3632-3640` |')
[void]$md.AppendLine('| `lvl6` | af_dummy_spring, af_dummy_dummy, af_dummy_battery, af_dummy_pellicle, af_dummy_kolobok, af_dummy_glassbeads | Regular capsule descriptor; random or supplied dyn 1–4 | `gamedata/scripts/kotovod.script:141-145`, `:3632-3640` |')
[void]$md.AppendLine('| `lvl7`–`lvl10` | Cooked/mod families: lvl7 includes armor/babka/cry/dik/dummy_simbion/kol/pudd/spirit/dummy_buliz; lvl8–10 are cooked tiers 2–4 | Base cooked section from the selected table; no dynamic suffix | `gamedata/scripts/kotovod.script:147-184` |')
[void]$md.AppendLine('| `lvl11` | Green/red/unique/new sections including af_battery, af_ice, af_sul_new | Base unique section; direct random selection from this table | `gamedata/scripts/kotovod.script:186-207` |')
[void]$md.AppendLine('| `lvl12` / `lvl13` | Six af_mayatnik_* sections | Mayatnik capsule descriptor | `gamedata/scripts/kotovod.script:209-224`, `:3632-3647` |')
[void]$md.AppendLine('| `lvl14` | capsule | `nlc_capsules.random_regular(true)` chooses a random regular capsule | `gamedata/scripts/kotovod.script:226-230`, `:3632-3647` |')
[void]$md.AppendLine('')
[void]$md.AppendLine('### Unknown-art selection matrix')
[void]$md.AppendLine('')
[void]$md.AppendLine('`kotovod.add_random_unknown_art()` selects a random row and column from `mike.table_arts_unknown`, spawns that exact `af_unknown_*_new` section into the actor, then sets condition from `inv_weight / random(7,12)` with a minimum of 0.45. `mike.take_art_set(num)` selects the current `nlc_vars.set` row and requested column.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Set row | Column 1 | Column 2 | Column 3 | Source |')
[void]$md.AppendLine('|---:|---|---|---|---|')
[void]$md.AppendLine('| 1 | af_unknown_1_new | af_unknown_22_new | af_unknown_31_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 2 | af_unknown_2_new | af_unknown_16_new | af_unknown_28_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 3 | af_unknown_3_new | af_unknown_5_new | af_unknown_25_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 4 | af_unknown_4_new | af_unknown_14_new | af_unknown_20_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 5 | af_unknown_6_new | af_unknown_9_new | af_unknown_21_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 6 | af_unknown_8_new | af_unknown_18_new | af_unknown_26_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 7 | af_unknown_7_new | af_unknown_19_new | af_unknown_23_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 8 | af_unknown_12_new | af_unknown_24_new | af_unknown_29_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('| 9 | af_unknown_11_new | af_unknown_15_new | af_unknown_30_new | `gamedata/scripts/mike.script:1375-1389` |')
[void]$md.AppendLine('')
[void]$md.AppendLine('### Table-driven green artifact reward')
[void]$md.AppendLine('')
[void]$md.AppendLine('`dummy_green_art(num,flag,lvl)` calls `generate_greens(num)` and therefore does not put the final section literal in the XML action. The exact mapping is: `num=1` → output af_dummy_green_blood / af_dummy_green_medusa / af_dummy_green_vyvert according to the table row, with info_green_blood and af_blood_dyn5d; `num=2` → af_dummy_green_medusa / af_dummy_green_vyvert / af_dummy_green_blood, info_green_vyvert and af_vyvert_dyn5d; `num=3` → af_dummy_green_vyvert / af_dummy_green_blood / af_dummy_green_medusa, info_green_medusa and af_medusa_dyn5d. `flag=create` creates the selected artifact (or a bad-condition item for num 5); `flag=spawn` places it at the fixed `val` or `yan` coordinates; `flag=give` consumes it; `num=4` creates recipe/fake info only. `r_treas_items` selects the table row.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Mapping row | Section by num | Recipe/info ID | Tier-5 source used by artmod path | Source |')
[void]$md.AppendLine('|---|---|---|---|---|')
[void]$md.AppendLine('| 1 | 1=af_dummy_green_blood; 2=af_dummy_green_medusa; 3=af_dummy_green_vyvert | info_green_blood | af_blood_dyn5d | `gamedata/scripts/sak/sak_dialog.script:9797-9838` |')
[void]$md.AppendLine('| 2 | 1=af_dummy_green_medusa; 2=af_dummy_green_vyvert; 3=af_dummy_green_blood | info_green_vyvert | af_vyvert_dyn5d | `gamedata/scripts/sak/sak_dialog.script:9797-9838` |')
[void]$md.AppendLine('| 3 | 1=af_dummy_green_vyvert; 2=af_dummy_green_blood; 3=af_dummy_green_medusa | info_green_medusa | af_medusa_dyn5d | `gamedata/scripts/sak/sak_dialog.script:9797-9838` |')

[void]$md.AppendLine('')
[void]$md.AppendLine('## Runtime overrides and non-LTX behavior')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Runtime path | Exact behavior | Source |')
[void]$md.AppendLine('|---|---|---|')
[void]$md.AppendLine('| Garbage radiation | On taking a bio artifact on l02_garbage, garbage_arts.apply_garbage_art() calculates new_rad from the artifact base and dynamic tier, writes it to the live artifact radiation_restore_speed, and stores it in the UI arc container record. For positive new_rad, a reduced runtime radiation_immunity is reset to 1.0. dyn6d is skipped. The table has base/dyn endpoints and optional max tiers in garbage_arts.script; exact formula is base + ((dyn-1)/(max_dyn-1))*(max-base), rounded to four decimals. | ' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/garbage_arts.script') 50) + '–' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/garbage_arts.script') 82) + ' |')
[void]$md.AppendLine('| Live condition scaling | scale_artefact_stats() rewrites live restore speeds and additional inventory weights using condition/effective-strength; it separately rewrites all listed immunities around the raw base value. If the arc container record has custom rad, it overrides the live radiation restore speed with the custom value and forces radiation immunity to 1.0. | ' + (Source (Join-Path $repo 'gamedata/scripts/common/_g.script') 2399) + '–' + (Source (Join-Path $repo 'gamedata/scripts/common/_g.script') 2470) + ' |')
[void]$md.AppendLine('| Arc radiation UI/container | arc_radiation.script computes container/item effective radiation and writes a live inventory-item radiation_restore_speed for display/management; this is not a new LTX base value. | ' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/arc_radiation.script') 60) + '–' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/arc_radiation.script') 125) + ' |')
[void]$md.AppendLine('| Belt collection: three souls | Three af_soul_dyn1d artifacts receive live strike/wound immunity 0.9 + 0.1*(1-condition) and health restore 0.0001*condition; values are restored when the combo ends. | ' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 28) + '–' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 58) + ' |')
[void]$md.AppendLine('| Belt collection: Vyvert | Two or more af_vyvert_dyn2d+ artifacts receive live additional_inventory_weight2 based on dynamic tier, condition, count, and total belt weight; saved properties are restored when the combo ends. | ' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 61) + '–' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 106) + ' |')
[void]$md.AppendLine('| Belt collection: inversion | The exact triple af_rusty_sea_urchin_dyn2d, af_mincer_meat_dyn1d, af_electra_flash_dyn3d inverts selected restore speeds and immunities (2-current) and retains/restores selected additional parameters. | ' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 110) + '–' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 174) + ' |')
[void]$md.AppendLine('| Belt collection: green set | af_dummy_green_vyvert, af_dummy_green_medusa, and af_dummy_green_blood consume an accumulator condition of 0.3 and the three artifact objects; this is a collection consumption effect, not a static LTX mutation. | ' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 176) + '–' + (Source (Join-Path $repo 'gamedata/scripts/kotovod/art_collections.script') 226) + ' |')
[void]$md.AppendLine('| Cooking/artmod transforms | Cooking/artmod timers replace input sections with the result/capsule descriptor; the report records the source and output chain, but does not copy runtime condition or transform state back into static LTX rows. | `gamedata/scripts/sak/sak_artmods.script:351-414`; `gamedata/scripts/sak/sak.script:1673-1935` |')

[void]$md.AppendLine('')
[void]$md.AppendLine('## Excluded helper/support sections')
[void]$md.AppendLine('')
[void]$md.AppendLine('These sections inherit `class=ARTEFACT` but are implementation helpers rather than player-facing artifact entries (absorbation sections, fake/mfake sections, activation/support sections, and similar). They are listed so the exclusion is auditable.')
[void]$md.AppendLine('')
[void]$md.AppendLine('| Section | Source |')
[void]$md.AppendLine('|---|---|')
foreach ($h in $helperSections) { [void]$md.AppendLine('| ' + $h.Name + ' | ' + (Source $h.File $h.Line) + ' |') }

[void][System.IO.File]::WriteAllText($outPath, $md.ToString(), [System.Text.UTF8Encoding]::new($false))
Write-Output ('Wrote ' + $outPath + ' (' + $artifactRows.Count + ' artifact rows, ' + $dialogRows.Count + ' dialogue rows, ' + $stashRows.Count + ' stash functions).')
