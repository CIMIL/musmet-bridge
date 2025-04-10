
// HTML webpage
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>PicoW Config</title>
</head>
<body>
    <h2>Modify Config.txt</h2>
    <form action="/save" method="POST">
        <p>SSID:</p>
        <textarea name="config_ssid" rows="1" cols="30">%CONFIG_ssid%</textarea><br>
        <p>PWD: </p>
        <textarea name="config_pw"   rows="1" cols="30">%CONFIG_pw%</textarea><br>
        <p>IP:  </p>
        <textarea name="config_ip"   rows="1" cols="30">%CONFIG_ip%</textarea><br>
        <p>PORT:</p>
        <textarea name="config_port" rows="1" cols="30">%CONFIG_port%</textarea><br>
        <input type="submit" value="Save">
    </form>
    <button onclick="fetch('/print').then(response => alert('Printed to Serial!'));">Print Config</button>
</body>
</html>
)rawliteral";