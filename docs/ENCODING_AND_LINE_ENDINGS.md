# Encoding and line-ending guidance

This repository contains legacy game assets that are not interchangeable with
modern UTF-8 text files. The menu XML, localization XML, and many related
runtime scripts use Windows-1251 and are consumed by an older engine/parser.
Keep that format when editing runtime files.

## Runtime-file rules

- Preserve Windows-1251 encoding for legacy Cyrillic UI/config files.
- Preserve the existing BOM state. The affected game files are normally
  Windows-1251 without a BOM.
- Preserve the file's existing line endings. Do not run a formatter or rewrite
  the whole file merely to change LF to CRLF or vice versa.
- Keep Russian text as literal Windows-1251 Cyrillic. Do not replace it with
  UTF-8 text or XML numeric entities; this UI can display those entities
  literally instead of decoding them.
- Treat documentation files separately: Markdown documentation may use normal
  UTF-8, but it must never be copied into the runnable installation.

## Safe PowerShell fallback

When a legacy file cannot be edited safely with patch tooling, read and write
it explicitly with code page 1251. `ReadAllText` preserves the line-ending
characters already present in the string; `WriteAllText` writes them back
without an implicit newline conversion.

```powershell
$cp = [System.Text.Encoding]::GetEncoding(1251)
$text = [System.IO.File]::ReadAllText($path, $cp)

# Make a narrow, verified edit to $text here.

[System.IO.File]::WriteAllText($path, $text, $cp)
```

Avoid `Get-Content | Set-Content`, `Out-File`, or editor settings that silently
convert the file to UTF-8, add a BOM, normalize line endings, or rewrite XML.
Use the smallest targeted replacement possible and inspect the surrounding
text afterward.

## Validation checklist

After editing a runtime file:

1. Check the changed-file diff and run `git diff --check`.
2. Confirm the file is still decodable as Windows-1251 and has the expected
   BOM state.
3. For XML, parse the exact file rather than relying only on visual inspection.
4. Check nearby script/table commas and XML nesting after text replacements.
5. When deploying, copy only the changed runtime files to
   `C:\Games\NLC_OGSR_HE` and compare source/target SHA-256 hashes.

Terminal display is not an encoding test: a CP1251 file can look garbled when
read as UTF-8 even though its bytes are correct. Verify the bytes and decode
with the intended code page before converting anything.
