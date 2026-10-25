<#
.SYNOPSIS
    監聽 ST-LINK 虛擬 COM port，把板子 printf 的輸出即時印在終端機；按 Ctrl+C 結束。

.PARAMETER Baud
    鮑率，預設 115200（Phase 2-03 起）；Phase 2-02 以前的程式用 9600。

.PARAMETER Port
    COM 埠，例如 "COM5"。不指定時自動找 ST-LINK（VID 0483、PID 374B）。

.EXAMPLE
    .\serial-monitor.ps1
    .\serial-monitor.ps1 -Baud 9600
    .\serial-monitor.ps1 -Port COM7
#>

param(
    [int]$Baud = 115200,
    [string]$Port
)

$ErrorActionPreference = "Stop"

if (-not $Port) {
    $dev = Get-CimInstance Win32_PnPEntity |
        Where-Object { $_.PNPDeviceID -like "USB\VID_0483&PID_374B*" -and $_.Name -match "\((COM\d+)\)" } |
        Select-Object -First 1
    if (-not $dev) { throw "找不到 ST-LINK 的 COM port：確認板子 USB 有接上，或用 -Port 指定" }
    $null = $dev.Name -match "\((COM\d+)\)"
    $Port = $Matches[1]
}

# 同一個 COM port 一次只能給一個程式用；用邏輯分析儀擷取前要先停掉這裡（Ctrl+C）
$p = New-Object System.IO.Ports.SerialPort $Port, $Baud, None, 8, One
$p.Open()
Write-Host "監聽 $Port @ $Baud，按板子 RESET；要結束請按 Ctrl+C"
try {
    while ($true) {
        $s = $p.ReadExisting()
        if ($s) { Write-Host -NoNewline $s }
        Start-Sleep -Milliseconds 100
    }
} finally {
    $p.Close()
}
