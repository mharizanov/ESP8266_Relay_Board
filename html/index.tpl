<html>
<head><title>ESP8266 Relay Board</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
<div id="main">
<h1>Harizanov 3Ch Relay/Thermostat</h1>
<p>
<h2>Settings</h2>
<a href="config/wifi/wifi.tpl">WiFi</a><BR>
<a href="config/mqtt.tpl">MQTT</a><BR>
<a href="config/httpd.tpl">HTTP Daemon</a><BR>
<a href="config/ntp.tpl">NTP</a><BR>
<a href="config/sensor.tpl">Sensor</a><BR>
<a href="config/broadcastd.tpl">Broadcast Daemon</a><BR>
<a href="config/relay.tpl">Relays</a><BR>
<a href="config/thermostat.tpl">Thermostat</a>

<h2>Control / View</h2>
<a href="control/ui.tpl">Overview Page</a><BR>
<a href="control/thingspeak.html">Thingspeak</a><BR>
<a href="control/dht22.tpl">DHT22 Readings</a><BR>
<a href="control/ds18b20.tpl">DS18B20 Readings</a><BR>
<a href="control/relay.html">Relays</a><BR>
<a href="control/thermostat.html">Thermostat</a>

<h2>System</h2>
<a href="control/reset.cgi" onclick="return confirm('Are you sure you want to restart?')">Restart the system</a><BR>
<a href="about.tpl">About</a>

</p>
</div>
</body></html>
