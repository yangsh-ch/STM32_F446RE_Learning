<#
.SYNOPSIS
    從 portfolio repo 挑一個 Day 資料夾，複製進純 ASCII 路徑的 mbed 編譯 workspace，
    編譯出 .bin（放到 workspace\output\），並可自動燒錄到板子。

.PARAMETER Day
    要編譯的資料夾名稱，例如 "Day06_OLED_SPI"（必填）。

.PARAMETER MainFile
    該 Day 底下有多個 "*_main.cpp" 時，指定要編哪一個，例如 "02_main.cpp"。

.PARAMETER PortfolioPath
    portfolio repo 路徑，可以含中文，只是讀取來源、不會被寫入。

.PARAMETER WorkspacePath
    純 ASCII 路徑的編譯 workspace，預設 C:\mbed-build-workspace。
    Arm GCC 連結器在非 ASCII 路徑下會出錯，所以一定要在這裡編譯。

.PARAMETER Flash
    編譯成功後是否自動燒錄，預設 $true。

.EXAMPLE
    .\build-day-from-portfolio.ps1 -Day Day06_OLED_SPI
#>

param(
    [Parameter(Mandatory = $true)][string]$Day,
    [string]$MainFile,
    [string]$PortfolioPath = "D:\洋升\自學\MCU-NUCLEO-F446RE\STM32_F446RE_Learning",
    [string]$WorkspacePath = "C:\mbed-build-workspace",
    [string]$Target = "NUCLEO_F446RE",
    [string]$Toolchain = "GCC_ARM",
    [bool]$Flash = $true,
    [string]$DriveLabelPattern = "NOD*F446RE*"  # 實測這片板子顯示 "NOD_F446RE"（少一個 E），用萬用字元同時吃到 "NODE_F446RE"
)

$ErrorActionPreference = "Stop"
$AppTarget = "portfolio_build"
$DayPath = Join-Path $PortfolioPath $Day
$OutputDir = Join-Path $WorkspacePath "output"

if (-not (Test-Path $DayPath)) {
    throw "找不到資料夾：$DayPath（檢查 -Day 拼字或 -PortfolioPath）"
}

# WorkspacePath 不能疊到 PortfolioPath 裡，不然清空 workspace 會把 portfolio repo 也清掉
$portfolioFull = [System.IO.Path]::GetFullPath($PortfolioPath).TrimEnd('\')
$workspaceFull = [System.IO.Path]::GetFullPath($WorkspacePath).TrimEnd('\')
if ($workspaceFull -ieq $portfolioFull -or $portfolioFull.StartsWith("$workspaceFull\", [StringComparison]::OrdinalIgnoreCase)) {
    throw "WorkspacePath 不能等於或包住 PortfolioPath：`n  $workspaceFull`n  $portfolioFull"
}

function Write-Step($msg) { Write-Host "`n==> $msg" -ForegroundColor Cyan }

# 每次執行獨立一份 log（帶時間戳記），收在 output\logs\，不會佔用根目錄空間
$LogDir = Join-Path $OutputDir "logs"
New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
$LogPath = Join-Path $LogDir "$Day`_$(Get-Date -Format 'yyyyMMdd_HHmmss').log"
Start-Transcript -Path $LogPath -IncludeInvocationHeader | Out-Null

try {

# 1. 準備 workspace + mbed-os（只有第一次會跑到）
if (-not (Test-Path $WorkspacePath)) {
    Write-Step "建立編譯 workspace 並抓 mbed-os（第一次比較久）"
    New-Item -ItemType Directory -Path (Split-Path $WorkspacePath -Parent) -Force -ErrorAction SilentlyContinue | Out-Null
    Push-Location (Split-Path $WorkspacePath -Parent)
    mbed-tools new (Split-Path $WorkspacePath -Leaf)
    Pop-Location
} elseif (-not (Test-Path (Join-Path $WorkspacePath "mbed-os"))) {
    Write-Step "mbed-os 不見了，補抓回來"
    Push-Location $WorkspacePath
    mbed-tools deploy
    Pop-Location
}
Set-Location $WorkspacePath

# 2. 清掉上一輪的原始碼，換成這次要編的 Day
Write-Step "清空舊原始碼，複製 $Day"
Remove-Item main.cpp, Core -Recurse -Force -ErrorAction SilentlyContinue

$mainCandidates = Get-ChildItem $DayPath -Filter "*_main.cpp" -File
if ($mainCandidates.Count -eq 0) { throw "$DayPath 底下找不到任何 *_main.cpp" }

$chosenMain = if ($MainFile) {
    $mainCandidates | Where-Object Name -eq $MainFile
} elseif ($mainCandidates.Count -eq 1) {
    $mainCandidates[0]
}
if (-not $chosenMain) {
    Write-Host "$Day 底下有多個 main 檔，請用 -MainFile 指定：" -ForegroundColor Yellow
    $mainCandidates.Name | ForEach-Object { Write-Host "  $_" }
    throw "請加上 -MainFile 參數重跑"
}

Write-Host "使用：$($chosenMain.Name)" -ForegroundColor Green
Copy-Item $chosenMain.FullName main.cpp -Force

Get-ChildItem $DayPath -File | Where-Object {
    $_.Extension -in ".cpp", ".h" -and $_.Name -ne $chosenMain.Name -and $_.Name -notlike "*_main.cpp"
} | ForEach-Object { Copy-Item $_.FullName -Destination $WorkspacePath -Force }

if (Test-Path (Join-Path $DayPath "Core")) {
    Copy-Item (Join-Path $DayPath "Core") Core -Recurse -Force
}

# 3. 產生 CMakeLists.txt（自動掃 workspace 裡的 .cpp/.h，排除 mbed-os/cmake_build）
@"
cmake_minimum_required(VERSION 3.19.0)
set(MBED_PATH `${CMAKE_CURRENT_SOURCE_DIR}/mbed-os CACHE INTERNAL "")
set(MBED_CONFIG_PATH `${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(APP_TARGET $AppTarget)
include(`${MBED_PATH}/tools/cmake/app.cmake)
project(`${APP_TARGET})
add_subdirectory(`${MBED_PATH})
add_executable(`${APP_TARGET})

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "`${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
list(FILTER APP_SOURCES EXCLUDE REGEX "`${CMAKE_CURRENT_SOURCE_DIR}/(mbed-os|cmake_build)/.*")
target_sources(`${APP_TARGET} PRIVATE `${APP_SOURCES})

file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "`${CMAKE_CURRENT_SOURCE_DIR}/*.h")
list(FILTER APP_HEADERS EXCLUDE REGEX "`${CMAKE_CURRENT_SOURCE_DIR}/(mbed-os|cmake_build)/.*")
set(APP_INCLUDE_DIRS "")
foreach(_hdr `${APP_HEADERS})
    get_filename_component(_dir `${_hdr} DIRECTORY)
    list(APPEND APP_INCLUDE_DIRS `${_dir})
endforeach()
list(REMOVE_DUPLICATES APP_INCLUDE_DIRS)
target_include_directories(`${APP_TARGET} PRIVATE `${APP_INCLUDE_DIRS})

target_link_libraries(`${APP_TARGET} mbed-os)
mbed_set_post_build(`${APP_TARGET})
"@ | Set-Content CMakeLists.txt -Encoding utf8

# 4. 編譯（沒改到程式碼時 ninja 會直接沿用上次的 .bin，這是正常的，不用強迫重編）
$builtBin = "cmake_build\$Target\develop\$Toolchain\$AppTarget.bin"

Write-Step "編譯：mbed-tools compile -m $Target -t $Toolchain"
mbed-tools compile -m $Target -t $Toolchain
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $builtBin)) {
    throw "編譯失敗，往上捲看錯誤訊息（預期產出：$builtBin）。"
}

# 5. 複製到 output\，檔名跟著 Day 走，不用去 cmake_build 深處挖
$outBin = Join-Path $OutputDir "$Day.bin"
Copy-Item $builtBin $outBin -Force

Write-Host ""
Write-Host "===================================================================" -ForegroundColor Green
Write-Host " 編譯成功！($Day / $($chosenMain.Name))" -ForegroundColor Green
Write-Host " 產出檔案：$outBin" -ForegroundColor Green
Write-Host "===================================================================" -ForegroundColor Green

# 6. 自動燒錄
if ($Flash) {
    Write-Step "尋找板子隨身碟（磁碟區標籤 '$DriveLabelPattern'）"
    $boardDrive = Get-Volume -ErrorAction SilentlyContinue |
        Where-Object { $_.DriveType -eq 'Removable' -and $_.FileSystemLabel -like $DriveLabelPattern } |
        Select-Object -First 1

    if ($boardDrive) {
        Copy-Item $outBin "$($boardDrive.DriveLetter):\" -Force
        Write-Host "已自動燒錄到 $($boardDrive.DriveLetter): (標籤: $($boardDrive.FileSystemLabel))" -ForegroundColor Green
    } else {
        Write-Host "沒偵測到符合 '$DriveLabelPattern' 的隨身碟，確認板子已接上，或手動把這個檔案拖進去：" -ForegroundColor Yellow
        Write-Host "  $outBin" -ForegroundColor Yellow
    }
} else {
    Write-Host "已設定 -Flash:`$false，不自動燒錄。手動燒錄：把 $outBin 拖進板子的隨身碟。" -ForegroundColor Yellow
}

} finally {
    Stop-Transcript | Out-Null
    Write-Host "本次紀錄：$LogPath" -ForegroundColor Magenta
}
