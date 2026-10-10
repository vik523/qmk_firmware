# Прошивка готового .hex в Pro Micro (загрузчик Caterina) без пересборки.
# Ждёт появления COM-порта загрузчика и сразу запускает avrdude.
# Нужен, когда `qmk flash` не успевает: перед прошивкой он пересобирает проект,
# а окно загрузчика — около 8 секунд.
#
#   1. qmk compile -kb handwired/dactyl_promicro -km 5x6
#   2. .\flash_caterina.ps1
#   3. перевести половину в загрузчик (QK_BOOT на слое ADJ или RST на GND)
#   4. повторить 2–3 для второй половины
param(
    [string]$Hex,                              # по умолчанию — .hex в корне qmk_firmware
    [string]$QmkMsys = 'C:\QMK_MSYS',          # откуда брать avrdude, если его нет в PATH
    [int]$WaitSeconds = 840
)

if (-not $Hex) {
    $root = Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..\..')
    $Hex  = Join-Path $root 'handwired_dactyl_promicro_5x6.hex'
}
if (-not (Test-Path $Hex)) { "Нет файла прошивки: $Hex"; exit 1 }

$avrdude = (Get-Command avrdude -ErrorAction SilentlyContinue).Source
if (-not $avrdude) { $avrdude = Join-Path $QmkMsys 'opt\qmk\bin\avrdude.exe' }
if (-not (Test-Path $avrdude)) { "Не найден avrdude: $avrdude"; exit 1 }
$conf = Join-Path (Split-Path $avrdude) 'avrdude.conf'

# Загрузчики Caterina: Arduino Leonardo/Micro, SparkFun Pro Micro 5 В / 3,3 В, Adafruit
$ids = 'VID_2341&PID_0036|VID_2341&PID_0037|VID_2A03&PID_0036|VID_2A03&PID_0037|VID_1B4F&PID_9205|VID_1B4F&PID_9203|VID_239A&PID_000C'

"$(Get-Date -Format HH:mm:ss) жду загрузчик..."
$deadline = (Get-Date).AddSeconds($WaitSeconds)
$port = $null
while ((Get-Date) -lt $deadline) {
    $dev = Get-CimInstance Win32_PnPEntity -Filter "PNPClass='Ports'" |
        Where-Object { $_.PNPDeviceID -match $ids -and $_.Name -match '\((COM\d+)\)' } |
        Select-Object -First 1
    if ($dev) {
        $null = $dev.Name -match '\((COM\d+)\)'
        $port = $Matches[1]
        break
    }
    Start-Sleep -Milliseconds 200
}
if (-not $port) { "$(Get-Date -Format HH:mm:ss) загрузчик не появился"; exit 2 }

"$(Get-Date -Format HH:mm:ss) загрузчик на $port, прошиваю"
Start-Sleep -Milliseconds 400   # драйвер должен успеть открыть порт
$avrArgs = @('-p', 'atmega32u4', '-c', 'avr109', '-P', $port, '-U', "flash:w:${Hex}:i")
if (Test-Path $conf) { $avrArgs = @('-C', $conf) + $avrArgs }
& $avrdude @avrArgs 2>&1 | ForEach-Object { "$_" }
"$(Get-Date -Format HH:mm:ss) код возврата avrdude: $LASTEXITCODE"
exit $LASTEXITCODE
