
// HTML webpage
const char savedPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Config Musmet MIDI Bridge</title><style>*{box-sizing:border-box;font-family:Arial,sans-serif}body{padding:15px;text-align:center}.container{max-width:500px;margin:0 auto;border-radius:8px}</style></head><body><div class="container"><h2>Musmet Midi Bridge</h2><p>Configuration saved successfully!</p><br><p>SSID: %CONFIG_ssid%</p><p>Password: %CONFIG_pw%</p><p>IP Address: %CONFIG_ip%</p><p>Port: %CONFIG_port%</p><br><p>Press the physical restart button on the MIDI bridge</p><p>or go back to<a href="/">config page</a></p></div></body></html>
)rawliteral";