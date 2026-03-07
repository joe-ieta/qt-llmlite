Write-Host "Installing qt-llm (editable) ..."
python -m pip install --upgrade pip
python -m pip install -e .

Write-Host "Installing Qt Creator External Tools XML..."
$qtTools = Join-Path $env:APPDATA "QtProject\qtcreator\externaltools"
New-Item -ItemType Directory -Force -Path $qtTools | Out-Null
Copy-Item ".\tools\qtcreator\*.xml" -Destination $qtTools -Force

Write-Host ""
Write-Host "Done. Restart Qt Creator."
Write-Host "Tools -> External -> qt-llm"