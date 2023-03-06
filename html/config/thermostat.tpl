<html><head><title>Thermostat settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="../style.css">

</head>
<body>
<div id="main">
<p>
<b>Thermostat Settings</b>
</p>
<form name="thermostatform" action="thermostat_config.cgi" method="post">

<table>
<tr><td>Thermostat input:</td><td>

<select name="therm1-input" id="therm1-input">
  <option value="0" %selected-ds18b20% >DS18B20 sensor #1</option>
  <option value="1" %selected-dht22t% >DHT22 temperature</option>
  <option value="2" %selected-dht22h% >DHT22 humidity</option>
  <option value="3" %selected-mqtt% >MQTT</option>
  <option value="4" %selected-serial% >Serial input</option>  
  <option value="5" %selected-fixed% >Fixed value (10)</option>    
</select><td> </td>
<tr><td>Thermostat hysteresis high:</td><td><input type="text" name="therm1-hysteresis-high" id="therm1-hysteresis-high" value="%therm1-hysteresis-high%" /> </td><td>(in tenths of a degree, 5 means 0.5 degrees)</td></tr>
<tr><td>Thermostat hysteresis low:</td><td><input type="text" name="therm1-hysteresis-low" id="therm1-hysteresis-low" value="%therm1-hysteresis-low%" /> </td><td>(in tenths of a degree, 5 means 0.5 degrees)</td></tr>

<tr><td>Room Temperature timeout secs:</td><td><input type="text" name="therm-room-temp-timeout-secs" id="therm-room-temp-timeout-secs" value="%therm-room-temp-timeout-secs%">     </td>
<td>Timeout for thermostat room temperature reading. Configured sensor should provide readings every timeout seconds or thermostat turns off</td></tr>

<tr><td>Low temperature colour degrees </td><td><input type="text" name="therm-low-temp-colour-deg" id="therm-low-temp-colour-deg" value="%therm-low-temp-colour-deg%">     </td>
<td>Thermostat schedule display lowest temp starting point for colour gradient</td></tr>

<tr><td>High temperature colour degrees </td><td><input type="text" name="therm-high-temp-colour-deg" id="therm-high-temp-colour-deg" value="%therm-high-temp-colour-deg%">     </td>
<td>Thermostat schedule display lowest temp starting point for colour gradient</td></tr>

<td>Thermostat Relay Rest Minutes:</td><td><input type="text" name="therm-relay-rest-min" id="therm-relay-rest-min" value="%therm-relay-rest-min%"/></td>
<td>To avoid relay cycling, this the minimum OFF period - only applicable to relays configured for thermostat operation.</td>
</table>
<br/>
<button type="button" onClick="parent.location='/'">Back</button>
<input type="submit" name="save" value="Save">
</p>
</form>

</body>
</html>
