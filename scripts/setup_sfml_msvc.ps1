$ErrorActionPreference = "Stop"

$sfmlName = "SFML-2.6.2-windows-vc17-64-bit"
$sfmlUrl = "https://www.sfml-dev.org/files/$sfmlName.zip"

$projectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$installRoot = Resolve-Path (Join-Path $projectRoot "..")
$sfmlRoot = Join-Path $installRoot $sfmlName
$sfmlConfig = Join-Path $sfmlRoot "lib\cmake\SFML\SFMLConfig.cmake"
$genericExtractedRoot = Join-Path $installRoot "SFML-2.6.2"
$genericExtractedConfig = Join-Path $genericExtractedRoot "lib\cmake\SFML\SFMLConfig.cmake"
$zipPath = Join-Path $installRoot "$sfmlName.zip"

if (Test-Path $sfmlConfig) {
    Write-Host "SFML already installed: $sfmlRoot"
    exit 0
}

if ((Test-Path $genericExtractedConfig) -and (-not (Test-Path $sfmlRoot))) {
    Write-Host "Found generic SFML folder. Renaming to: $sfmlRoot"
    Rename-Item -Path $genericExtractedRoot -NewName $sfmlName
    exit 0
}

Write-Host "SFML was not found at: $sfmlConfig"
Write-Host "Downloading $sfmlName..."

Invoke-WebRequest -Uri $sfmlUrl -OutFile $zipPath

Write-Host "Extracting SFML to: $installRoot"
Expand-Archive -Path $zipPath -DestinationPath $installRoot -Force

if ((Test-Path $genericExtractedConfig) -and (-not (Test-Path $sfmlRoot))) {
    Write-Host "Renaming extracted folder to: $sfmlRoot"
    Rename-Item -Path $genericExtractedRoot -NewName $sfmlName
}

if (-not (Test-Path $sfmlConfig)) {
    throw "SFML setup failed. Expected config file was not created: $sfmlConfig"
}

Write-Host "SFML installed successfully: $sfmlRoot"
