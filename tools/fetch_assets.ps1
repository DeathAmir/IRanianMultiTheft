$ErrorActionPreference = "Stop"
$root = Join-Path $PSScriptRoot ".."
$models = Join-Path $root "assets/models"
$fonts = Join-Path $root "assets/fonts"
$audio = Join-Path $root "assets/audio"
New-Item -ItemType Directory -Force $models,$fonts,$audio | Out-Null
$downloads = @(
    @{ Url = "https://raw.githubusercontent.com/trebeljahr/quaternius-showcase/main/public/glb/modular_men/Swat.glb"; Path = (Join-Path $models "Swat.glb"); MinBytes = 100000 },
    @{ Url = "https://raw.githubusercontent.com/trebeljahr/quaternius-showcase/main/public/glb/modular_sci_fi_guns_pack/SMG_1.glb"; Path = (Join-Path $models "SMG_1.glb"); MinBytes = 50000 },
    @{ Url = "https://raw.githubusercontent.com/Seyamalam/blood-league-kickoff/main/public/assets/vendor/quaternius/night-striker.glb"; Path = (Join-Path $models "NightStriker.glb"); MinBytes = 500000 },
    @{ Url = "https://raw.githubusercontent.com/Seyamalam/blood-league-kickoff/main/public/assets/vendor/quaternius/universal-animation-library.glb"; Path = (Join-Path $models "UniversalAnimations.glb"); MinBytes = 200000 },
    @{ Url = "https://raw.githubusercontent.com/google/fonts/main/ofl/ibmplexsans/IBMPlexSans%5Bwdth%2Cwght%5D.ttf"; Path = (Join-Path $fonts "IBMPlexSans.ttf"); MinBytes = 100000 },
    @{ Url = "https://raw.githubusercontent.com/google/fonts/main/ofl/ibmplexsans/OFL.txt"; Path = (Join-Path $fonts "OFL-IBM-Plex-Sans.txt"); MinBytes = 1000 },
    @{ Url = "https://raw.githubusercontent.com/euuuuuuan/fatal-funnel-public/main/apps/game/public/sfx/smg_9mm.mp3"; Path = (Join-Path $audio "gunshot_smg.mp3"); MinBytes = 1000 },
    @{ Url = "https://raw.githubusercontent.com/euuuuuuan/moonshard-warden-public/main/src/assets/kenney-rpg-audio/footstep00.ogg"; Path = (Join-Path $audio "footstep.ogg"); MinBytes = 1000 },
    @{ Url = "https://opengameart.org/sites/default/files/clipload2.wav"; Path = (Join-Path $audio "reload.wav"); MinBytes = 10000 },
    @{ Url = "https://raw.githubusercontent.com/euuuuuuan/moonshard-warden-public/main/src/assets/kenney-rpg-audio/footstep02.ogg"; Path = (Join-Path $audio "footstep_alt.ogg"); MinBytes = 1000 },
    @{ Url = "https://raw.githubusercontent.com/euuuuuuan/moonshard-warden-public/main/src/assets/kenney-impact/impactMining_002.ogg"; Path = (Join-Path $audio "impact.ogg"); MinBytes = 1000 }
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
