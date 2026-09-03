param(
    [ValidateSet('scan','validate','preview','render','apply-config')]
    [string]$Action = 'preview',
    [ValidateSet('reload','ready','all')]
    [string]$Mode = 'reload',
    [string]$Profiles = (Join-Path $PSScriptRoot 'audio_timing_profiles.csv'),
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
    [string]$SourceRoot = 'C:\Games\NLC_OGSR_HE files\unpacked sounds\sounds\weapons',
    [switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$Cp1251 = [Text.Encoding]::GetEncoding(1251)
$Invariant = [Globalization.CultureInfo]::InvariantCulture
$WeaponsRoot = Join-Path $RepoRoot 'gamedata\config\weapons'
$OutputRoot = Join-Path $RepoRoot 'gamedata\sounds\weapons'

function Read-LegacyText([string]$Path) {
    return $Cp1251.GetString([IO.File]::ReadAllBytes($Path))
}

function Write-LegacyText([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, $Cp1251)
}

function Is-Enabled([object]$Row) {
    return $row -and ([string]$Row.enabled -match '^(1|true|yes)$')
}

function Parse-Number([object]$Value, [string]$Label) {
    $result = 0.0
    if (-not [double]::TryParse([string]$Value, [Globalization.NumberStyles]::Float, $Invariant, [ref]$result)) {
        throw "Invalid number for ${Label}: $Value"
    }
    return $result
}

function Format-Speed([double]$Value) {
    return $Value.ToString('0.00', $Invariant)
}

function Get-SectionBlocks([string]$Text) {
    return [regex]::Matches($Text, '(?ms)^\s*\[(?<name>[^\]]+)\][^\r\n]*\r?\n(?<body>.*?)(?=^\s*\[|\z)')
}

function Key-Is-InMode([string]$Key) {
    if ($Mode -eq 'reload') { return $Key -match '^snd_reload' }
    if ($Mode -eq 'ready') { return $Key -match '^snd_(draw|holster)' }
    return $Key -match '^snd_(reload|draw|holster)'
}

function Clean-SoundReference([string]$Value) {
    $sound = ($Value -split ',', 2)[0].Trim().Trim('"')
    return $sound -replace '/', '\'
}

function Sound-To-SourcePath([string]$Sound) {
    $relative = $Sound -replace '^weapons\\', ''
    if (-not $relative.EndsWith('.ogg', [StringComparison]::OrdinalIgnoreCase)) { $relative += '.ogg' }
    return Join-Path $SourceRoot $relative
}

function Sound-To-OutputPath([string]$Sound) {
    $relative = $Sound -replace '^weapons\\', ''
    if (-not $relative.EndsWith('.ogg', [StringComparison]::OrdinalIgnoreCase)) { $relative += '.ogg' }
    return Join-Path $OutputRoot $relative
}

function Make-OutputReference([string]$Section, [string]$Key, [double]$Speed) {
    $safe = ($Section + '_' + $Key) -replace '[^A-Za-z0-9_\-]', '_'
    $tag = [Math]::Round($Speed * 100).ToString('000', $Invariant)
    return "weapons\sivol_timing\${safe}_x${tag}"
}

function Get-OutputReference([object]$Row) {
    if ($Row.output_sound -and -not [string]::IsNullOrWhiteSpace([string]$Row.output_sound)) {
        return ([string]$Row.output_sound -replace '/', '\')
    }
    return Make-OutputReference $Row.weapon_section $Row.sound_key (Parse-Number $Row.speed 'speed')
}

function Get-ScanRows {
    $raw = [Collections.Generic.List[object]]::new()
    foreach ($file in Get-ChildItem -LiteralPath $WeaponsRoot -Filter 'w_*.ltx' -File | Sort-Object Name) {
        $text = Read-LegacyText $file.FullName
        foreach ($block in Get-SectionBlocks $text) {
            foreach ($line in ($block.Groups['body'].Value -split '\r?\n')) {
                $match = [regex]::Match($line, '^\s*(?<key>snd_[A-Za-z0-9_]+)\s*=\s*(?<value>[^;]+)')
                if (-not $match.Success -or -not (Key-Is-InMode $match.Groups['key'].Value)) { continue }
                $sound = Clean-SoundReference $match.Groups['value'].Value
                if (-not $sound.StartsWith('weapons\', [StringComparison]::OrdinalIgnoreCase)) { continue }
                $raw.Add([pscustomobject]@{
                    file = 'gamedata/config/weapons/' + $file.Name
                    weapon_section = $block.Groups['name'].Value.Trim()
                    sound_key = $match.Groups['key'].Value
                    source_sound = $sound
                })
            }
        }
    }
    $usage = @{}
    foreach ($entry in $raw) {
        $key = $entry.source_sound.ToLowerInvariant()
        if (-not $usage.ContainsKey($key)) { $usage[$key] = 0 }
        $usage[$key]++
    }
    foreach ($entry in $raw) {
        $speed = 1.0
        [pscustomobject][ordered]@{
            enabled = 'false'
            file = $entry.file
            weapon_section = $entry.weapon_section
            sound_key = $entry.sound_key
            source_sound = $entry.source_sound
            speed = Format-Speed $speed
            output_sound = ''
            shared_references = $usage[$entry.source_sound.ToLowerInvariant()]
            notes = ''
        }
    }
}

function Import-Profiles {
    if (-not (Test-Path -LiteralPath $Profiles)) { throw "Profile file not found: $Profiles" }
    return @(Import-Csv -LiteralPath $Profiles -Encoding UTF8 | Where-Object { $_ })
}

function Validate-Profiles([object[]]$Rows, [bool]$RequireSource) {
    $seenTargets = @{}
    $enabled = 0
    foreach ($row in @($Rows)) {
        if (-not (Is-Enabled $row)) { continue }
        $enabled++
        $speed = Parse-Number $row.speed "$($row.weapon_section) $($row.sound_key) speed"
        if ($speed -lt 0.25 -or $speed -gt 4.0) { throw "Audio speed must be between 0.25 and 4.0" }
        $configPath = Join-Path $RepoRoot ([string]$row.file -replace '/', '\')
        if (-not (Test-Path -LiteralPath $configPath)) { throw "Missing config: $($row.file)" }
        if (-not (Key-Is-InMode $row.sound_key)) { throw "$($row.sound_key) is outside selected mode $Mode" }
        $sourcePath = Sound-To-SourcePath $row.source_sound
        if ($RequireSource -and -not (Test-Path -LiteralPath $sourcePath)) { throw "Missing source sound: $sourcePath" }
        $outputSound = Get-OutputReference $row
        $outputPath = Sound-To-OutputPath $outputSound
        $resolvedOutput = [IO.Path]::GetFullPath($outputPath)
        $resolvedRoot = [IO.Path]::GetFullPath($OutputRoot) + [IO.Path]::DirectorySeparatorChar
        if (-not $resolvedOutput.StartsWith($resolvedRoot, [StringComparison]::OrdinalIgnoreCase)) { throw "Unsafe output path: $outputPath" }
        if ($seenTargets.ContainsKey($resolvedOutput)) { throw "Duplicate output path: $outputPath" }
        $seenTargets[$resolvedOutput] = $true
    }
    Write-Host "Validated $enabled enabled audio profiles in $Mode mode."
}

function Get-AtempoFilter([double]$Speed) {
    $parts = [Collections.Generic.List[string]]::new()
    while ($Speed -gt 2.0) { $parts.Add('atempo=2.0'); $Speed /= 2.0 }
    while ($Speed -lt 0.5) { $parts.Add('atempo=0.5'); $Speed /= 0.5 }
    $parts.Add('atempo=' + $Speed.ToString('0.######', $Invariant))
    return [string]::Join(',', $parts)
}

function Get-Duration([string]$Ffprobe, [string]$Path) {
    $value = & $Ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 -- $Path
    if ($LASTEXITCODE -ne 0) { throw "ffprobe failed for $Path" }
    return Parse-Number $value "duration of $Path"
}

function Preview-Profiles([object[]]$Rows, [string]$Ffprobe) {
    foreach ($row in @($Rows)) {
        if (-not (Is-Enabled $row)) { continue }
        $speed = Parse-Number $row.speed 'speed'
        $sourcePath = Sound-To-SourcePath $row.source_sound
        $duration = Get-Duration $Ffprobe $sourcePath
        $target = $duration / $speed
        $outputSound = Get-OutputReference $row
        Write-Host ("{0} [{1}] {2}: {3:N3}s -> {4:N3}s, output {5}" -f $row.file, $row.weapon_section, $row.sound_key, $duration, $target, $outputSound)
    }
}

function Render-Profiles([object[]]$Rows, [string]$Ffmpeg, [string]$Ffprobe) {
    foreach ($row in @($Rows)) {
        if (-not (Is-Enabled $row)) { continue }
        $speed = Parse-Number $row.speed 'speed'
        $sourcePath = Sound-To-SourcePath $row.source_sound
        $outputSound = Get-OutputReference $row
        $outputPath = Sound-To-OutputPath $outputSound
        if ((Test-Path -LiteralPath $outputPath) -and -not $Force) { throw "Output exists; use -Force to replace it: $outputPath" }
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $outputPath) | Out-Null
        $filter = Get-AtempoFilter $speed
        & $Ffmpeg -hide_banner -loglevel error -y -i $sourcePath -map_metadata 0 -filter:a $filter -c:a libvorbis -q:a 6 -- $outputPath
        if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $sourcePath" }
        $sourceDuration = Get-Duration $Ffprobe $sourcePath
        $outputDuration = Get-Duration $Ffprobe $outputPath
        $targetDuration = $sourceDuration / $speed
        $tolerance = [Math]::Max(0.03, $targetDuration * 0.03)
        if ([Math]::Abs($outputDuration - $targetDuration) -gt $tolerance) {
            throw "Duration validation failed for ${outputPath}: expected $targetDuration, got $outputDuration"
        }
        Write-Host ("RENDERED {0} ({1:N3}s)" -f $outputSound, $outputDuration)
    }
}

function Set-ConfigSound([string]$Text, [string]$Section, [string]$Key, [string]$Value) {
    $sectionPattern = '(?ms)(^\s*\[' + [regex]::Escape($Section) + '\][^\r\n]*\r?\n)(?<body>.*?)(?=^\s*\[|\z)'
    $sections = [regex]::Matches($Text, $sectionPattern)
    if ($sections.Count -ne 1) { throw "Section [$Section] resolved to $($sections.Count) blocks" }
    $match = $sections[0]
    $body = $match.Groups['body'].Value
    $keyPattern = '(?m)^(?<prefix>\s*' + [regex]::Escape($Key) + '\s*=\s*)[^,;\s]+(?<suffix>\s*(?:,[^;\r\n]*)?(?:;[^\r\n]*)?)(?=\r?$)'
    if ([regex]::Matches($body, $keyPattern).Count -ne 1) { throw "[$Section] must contain exactly one $Key" }
    $body = [regex]::Replace($body, $keyPattern, { param($m) $m.Groups['prefix'].Value + $Value + $m.Groups['suffix'].Value })
    return $Text.Substring(0, $match.Groups['body'].Index) + $body + $Text.Substring($match.Groups['body'].Index + $match.Groups['body'].Length)
}

function Apply-Config([object[]]$Rows) {
    foreach ($grouped in (@($Rows) | Where-Object { Is-Enabled $_ } | Group-Object file)) {
        $path = Join-Path $RepoRoot ($grouped.Name -replace '/', '\')
        $original = Read-LegacyText $path
        $updated = $original
        foreach ($row in $grouped.Group) {
            $outputSound = Get-OutputReference $row
            $outputPath = Sound-To-OutputPath $outputSound
            if (-not (Test-Path -LiteralPath $outputPath)) { throw "Render output before patching config: $outputPath" }
            $updated = Set-ConfigSound $updated $row.weapon_section $row.sound_key $outputSound
        }
        if ($updated -ne $original) {
            Write-LegacyText $path $updated
            Write-Host "UPDATED $($grouped.Name)"
        }
    }
}

$ffmpegCommand = Get-Command ffmpeg -ErrorAction SilentlyContinue
$ffprobeCommand = Get-Command ffprobe -ErrorAction SilentlyContinue

switch ($Action) {
    'scan' {
        $rows = @(Get-ScanRows)
        $rows | Export-Csv -LiteralPath $Profiles -NoTypeInformation -Encoding UTF8
        Write-Host "Scanned $($rows.Count) sound references into $Profiles. All rows are disabled."
    }
    'validate' {
        $rows = Import-Profiles
        Validate-Profiles $rows $true
    }
    'preview' {
        if (-not $ffprobeCommand) { throw 'ffprobe was not found on PATH' }
        $rows = Import-Profiles
        Validate-Profiles $rows $true
        Preview-Profiles $rows $ffprobeCommand.Source
    }
    'render' {
        if (-not $ffmpegCommand -or -not $ffprobeCommand) { throw 'ffmpeg and ffprobe must be on PATH' }
        $rows = Import-Profiles
        Validate-Profiles $rows $true
        Render-Profiles $rows $ffmpegCommand.Source $ffprobeCommand.Source
    }
    'apply-config' {
        $rows = Import-Profiles
        Validate-Profiles $rows $false
        Apply-Config $rows
    }
}
