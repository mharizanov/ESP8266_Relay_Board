<!DOCTYPE html>
<html>
  <head>

    <title>Relay Board</title>
    <meta http-equiv="Content-Type" content="text/html">
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <meta name="apple-mobile-web-app-capable" content="yes" />
    <meta name="apple-mobile-web-app-status-bar-style" content="black" />
    <link rel="stylesheet" href="http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.css" />
    <script src="https://code.jquery.com/jquery-2.2.4.js"></script>
    <script src="https://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.js"></script>
    <script src="../js/ui_tpl.js"></script>
    <script src="../js/relay.js"></script>
    <script type="text/javascript">
      $(document).ready(function() {
        server_get();
      });
      </script>

    <link rel="stylesheet" href="ui.css" />
  </head>

  <body>
  
    <div data-role="page" data-title="Relay Board" id="root">
    
     <div data-theme="d" data-role="header">
       <h1>
         ESP8266 Relay Board
       </h1>
     </div><br>
     
     <div data-role="content">

        <div class="content-primary">
          <div class="s-title"><center>Relay Control</center></div>
          <ul data-role="listview" data-inset="true" >
            <li id="relay1tr">
                    <label for="relay1"><b>%relay1name%</b></label>
                    <select name="relay1" id="relay1" data-mini="false" data-role="slider">
                      <option value="0">Off</option>
                      <option value="1">On</option>
                    </select>
            </li>
            <li id="relay2tr">
                    <label for="relay2"><b>%relay2name%</b></label>
                    <select name="relay2" id="relay2" data-mini="false" data-role="slider">
                      <option value="0">Off</option>
                      <option value="1">On</option>
                    </select>
            </li>
            <li id=relay3tr>
                    <label for="relay3"><b>%relay3name%</b></label>
                    <select name="relay3" id="relay3" data-mini="false" data-role="slider">
                      <option value="0">Off</option>
                      <option value="1">On</option>
                    </select>
            </li>
          </ul>
        </div><br>
        
        <div class="content-secondary" %ds18b20-enable% >
          <div class="s-title"><center>DS18B20 Temperature Sensors</center></div>
          <ul data-role="listview" data-inset="true" >
            <li>
                    <b>DS18B20</b>
                    <span class="inputvalue" name="DS18B20temperature" id="DS18B20temperature">N/A</span>
            </li>
        </div>
     
        <div class="content-secondary" %dht22-enable% >
          <div class="s-title"><center>DHT22 Sensor</center></div>
          <ul data-role="listview" data-inset="true" >
            <li>
                    <b>DHT22 Temperature</b>
                    <span class="inputvalue" name="DHT22temperature" id="DHT22temperature">N/A</span>
            </li>
            <li>
                    <b>DHT22 Humidity</b>
                    <span class="inputvalue" name="DHT22humidity" id="DHT22humidity">N/A</span>
            </li>
        </div>

          <div class="content-secondary" %thermostat-enable% >
          <div class="s-title"><center>Thermostat</center></div>
          <ul data-role="listview" data-inset="true" >
            <li>
                    <b>Thermostat</b>
                    <span class="inputvalue" name="thermostat1_enable" id="thermostat_enable">N/A</span>
            </li>
            <li>
                    <b>Thermostat Room Temperature</b>
                    <span class="inputvalue" name="thermostat1_room_temperature" id="thermostat1_room_temperature">N/A</span>
            </li>
            <li>
                    <b>Thermostat Set Point</b>
                    <span class="inputvalue" name="thermostat1_setpoint" id="thermostat1_setpoint">N/A</span>
            </li>
             <li>
                    <b>Thermostat call for heat</b>
                    <span class="inputvalue" name="thermostat1_on" id="thermostat1_on">N/A</span>
            </li>
        </div>
   
     </div><br>
      
     <div data-theme="d" data-role="footer">
 
     </div>
      
    </div>

  </body>
  
</html>
