# AC controller

Control Electrolux ChillFlex portable air conditioner with ESPHome.

The ESPHome Climate integration or Arduino HeatpumpIR lib don't directly support
this using, but [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266/)
does, so it can be wired to ESPHome Climate component.

## Init

```
python3 -m venv env
source env/bin/activate
pip3 install esphome
```

## Compile

`esphome compile cooler.yaml`

## Upload

`esphome upload cooler.yaml --device 192.168.42.48`

## Notes

- https://esphome.io/components/remote_transmitter.html
- https://esphome.io/components/climate/climate_ir.html
- https://github.com/crankyoldgit/IRremoteESP8266/tree/master
  - https://github.com/crankyoldgit/IRremoteESP8266/wiki/ESP01-Send-&-Receive-Circuit
- Parts
  - https://www.partco.fi/en/development-boards/esp/22438-esp32-nodemcu.html
  - https://www.partco.fi/en/electronic-components/actives/opto-components/ir_modules/6862-tsop31238.html
  - https://www.partco.fi/en/leds/special-purpose-leds/17474-tsal6200.html
- Adapted from:
  - https://github.com/Hagakurje/ESPHome_Gree_AC
  - https://github.com/esphome/feature-requests/issues/444#issuecomment-765089230
