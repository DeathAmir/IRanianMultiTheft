$ErrorActionPreference = "Stop"
$root = Join-Path $PSScriptRoot ".."
$models = Join-Path $root "assets/models"
$fonts = Join-Path $root "assets/fonts"
New-Item -ItemType Directory -Force $models,$fonts | Out-Null
$downloads = @(
    @{ Url = "https://raw.githubusercontent.com/trebeljahr/quaternius-showcase/main/public/glb/modular_men/Swat.glb"; Path = (Join-Path $models "Swat.glb"); MinBytes = 100000 },
    @{ Url = "https://raw.githubusercontent.com/trebeljahr/quaternius-showcase/main/public/glb/modular_sci_fi_guns_pack/SMG_1.glb"; Path = (Join-Path $models "SMG_1.glb"); MinBytes = 50000 },
    @{ Url = "https://raw.githubusercontent.com/google/fonts/main/ofl/ibmplexsans/IBMPlexSans%5Bwdth%2Cwght%5D.ttf"; Path = (Join-Path $fonts "IBMPlexSans.ttf"); MinBytes = 100000 },
    @{ Url = "https://raw.githubusercontent.com/google/fonts/main/ofl/ibmplexsans/OFL.txt"; Path = (Join-Path $fonts "OFL-IBM-Plex-Sans.txt"); MinBytes = 1000 }
)
foreach ($item in $downloads) {
    if (!(Test-Path -LiteralPath $item.Path) -or (Get-Item -LiteralPath $item.Path).Length -lt $item.MinBytes) {
        Invoke-WebRequest -Uri $item.Url -OutFile $item.Path
    }
    if (!(Test-Path -LiteralPath $item.Path) -or (Get-Item -LiteralPath $item.Path).Length -lt $item.MinBytes) {
        throw "Asset download validation failed: $($item.Path)"
    }
    Write-Host "Downloaded and verified $($item.Path) ($((Get-Item -LiteralPath $item.Path).Length) bytes)"
}
