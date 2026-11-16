board         := "arduino:avr:nano"
cpu           := "atmega328old"
monitor_speed := "115200"
port          := "/dev/ttyUSB0"
path          := "jhenn_encoder.ino"

run: build upload monitor

alias b := build
build:
    arduino-cli compile \
      --fqbn {{board}} \
      --board-options "cpu={{cpu}}" \
      -v {{path}}

alias u := upload
upload:
    arduino-cli upload \
      --port {{port}} \
      --fqbn {{board}} \
      --board-options "cpu={{cpu}}" \
      -v {{path}}

alias m := monitor
monitor:
    arduino-cli monitor \
      --config baudrate={{monitor_speed}} \
      --port {{port}} \
      --fqbn {{board}} \
      {{path}}

alias i := info
info:
    arduino-cli board details --fqbn {{board}}
