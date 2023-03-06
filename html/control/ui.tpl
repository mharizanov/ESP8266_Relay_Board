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
    <link rel="stylesheet" href="ui.css" />
    <link rel="stylesheet" href="style.css" />
  </head>
  <body>
  
    <div data-role="page" data-title="Relay Board" id="root">
    
     <div data-theme="d" data-role="header">
       <h1>  ESP8266 Relay Board  </h1>
     </div><br>
     
        <input type="hidden" value="">

        <div id="relay1tr" class="relay-row">
        <button id="relay1" style="float:right">OFF</button>
        <div class="relay-label" id="relay1name">Relay 1</div>
        <br/>
        </div>
        
        <div id="relay2tr" class="relay-row">
        <button id="relay2" class="relay-button">OFF</button>
        <div class="relay-label" id="relay2name">Relay 2</div>
        <br/>
        </div>

        <div id="relay3tr" class="relay-row"> 
        <button id="relay3" style="float:right">OFF</button>
        <div class="relay-label" id="relay3name">Relay 3</div>
        <br/>
         </div>
        
              
        <div class="content-secondary" %ds18b20-enable% >
          <div class="s-title">DS18B20 Temperature Sensors</div>
          <ul data-role="listview" data-inset="true" >
            <li id="ds18b20_0" class="ui-li">
                    <span id="ds18b20_id_0"> N/A</span>
                    <span class="inputvalue" id="ds18b20_temperature_0"> --.--&#8451;</span>
            </li>
            <li id="ds18b20_1" class="ui-li">
                    <span id="ds18b20_id_1"> N/A</span>
                    <span class="inputvalue" id="ds18b20_temperature_1" > --.--&#8451;</span>
            </li>
            <li id="ds18b20_2" class="ui-li">
                    <span id="ds18b20_id_2"> N/A</span>
                    <span class="inputvalue" id="ds18b20_temperature_2" > --.--&#8451;</span>
            </li>
            <li id="ds18b20_3" class="ui-li">
                  <span id="ds18b20_id_3"> N/A</span>
                    <span class="inputvalue" id="ds18b20_temperature_3" > --.--&#8451;</span>
            </li>
        </div>
     
        <div class="content-secondary"  %dht22-enable% >
          <div class="s-title">DHT22 Sensor</div>
          <ul data-role="listview" data-inset="true" >
            <li class="ui-li">
                    <b>DHT22 Temperature</b>
                    <span class="inputvalue" name="DHT22temperature" id="DHT22temperature">N/A</span>
            </li>
            <li class="ui-li">
                    <b>DHT22 Humidity</b>
                    <span class="inputvalue" name="DHT22humidity" id="DHT22humidity">N/A</span>
            </li>
        </div>

          <div class="content-secondary"  %thermostat-enable% >
          <div class="s-title" >Thermostat</div>
          <ul data-role="listview" data-inset="true" >
            <li class="ui-li">
                    <b>Thermostat</b>
                    <span class="inputvalue" name="thermostat1_enable" id="thermostat_enable">N/A</span>
            </li>
            <li id="room_temp_li" class="ui-li">
                    <b>Thermostat Room Temperature</b>
                    <span class="inputvalue" name="thermostat1_room_temperature" id="thermostat1_room_temperature">N/A</span>
            </li>
            <li id="set_point_li" class="ui-li">
                    <b>Thermostat Set Point</b>
                    <span class="inputvalue" name="thermostat1_setpoint" id="thermostat1_setpoint">N/A</span>
            </li>
             <li id="call_for_heat_li" class="ui-li">
                    <b>Thermostat call for heat</b>
                    <span class="inputvalue" name="thermostat1_on" id="thermostat1_on">N/A</span>
            </li>
        </div>
   
     </div><br>
      
     <div data-theme="d" data-role="footer">
 
     </div>
      
    </div>

<script type="text/javascript" src="../js/relay.js"></script>
<script src="../js/ui_tpl.js"></script>


<script  type="text/javascript">
$(document).ready(function() {
  relay_get();
  therm_get();
  sensor_get();
  update();
  setInterval(relay_get, 5000);
  setInterval('therm_get()', 5000);
  setInterval('sensor_get()', 30000);

  checkVisibility();
});
</script>
           
</body>
  
</html>

