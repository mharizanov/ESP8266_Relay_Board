<html><head><title>Relay Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<meta http-equiv="Content-Type" content="text/html;charset=utf-8">
<link rel="stylesheet" type="text/css" href="../style.css">
</head>
<body>
<div id="main">
<p>
<b>Relay Settings</b>
</p>
<form name="relayform" action="relay.cgi" method="post">

<table>
<tr><td>Number of relays</td>
<td><select id="relay-total" name="relay-total">
<option value="1" %relay-total-1%>1</option>
<option value="2" %relay-total-2%>2</option>
<option value="3" %relay-total-3%>3</option>
</select>
<td> </td> <td> </td>
</tr>
<tr>
<td>Relay latching enabled?:</td><td><input type="checkbox" name="relay-latching-enable" id="relay-latching-enable" %relay-latching-enable% /></td>
<td></td><td></td>
</tr>
<tr id="relay1tr" hidden>
<td>Relay 1 name:</td><td><input type="text" name="relay1-name" id="relay1-name" value="%relay1-name%"/></td>
<td>Thermostat controlled ? <input type="checkbox" name="relay1-thermostat" id="relay1-thermostat" %relay1-thermostat%></input></td>
<td>GPIO: <input type="text" size=3 value="%relay1-gpio%" id="relay1-gpio" name="relay1-gpio"></td>
</tr>
<tr id="relay2tr" hidden>
<td>Relay 2 name:</td><td><input type="text" name="relay2-name" id="relay2-name" value="%relay2-name%"/></td>
<td>Thermostat controlled ? <input type="checkbox" name="relay2-thermostat" id="relay2-thermostat" %relay2-thermostat%></input></td>
<td>GPIO: <input type="text" size=3 value="%relay2-gpio%" id="relay2-gpio" name="relay2-gpio"></td>

</tr>
<tr id="relay3tr" hidden>
<td>Relay 3 name:</td><td><input type="text" name="relay3-name" id="relay3-name" value="%relay3-name%"/></td>
<td>Thermostat controlled ? <input type="checkbox" name="relay3-thermostat" id="relay3-thermostat" %relay3-thermostat%></input></td>
<td>GPIO: <input type="text" size=3 value="%relay3-gpio%" id="relay3-gpio" name="relay3-gpio"></td>

</tr>
<tr>
</tr>
</table>
<br/>
<button type="button" onClick="parent.location='/'">Back</button>
<input type="submit" name="save" value="Save">
</p>
</form>

<script src="http://code.jquery.com/jquery-1.9.0.min.js"></script>
<script>window.jQuery || document.write('<script src="../js/jquery-1.9.0.min.js"><\/script>')</script>
<script src="../js/relay.js"></script>
<script type="text/javascript">

$(document).ready(function() {
  relay_get();
  displayRelays($("#relay-total").find(":selected").val() );

});

$("#relay-total").change(function () {
      displayRelays($("#relay-total").find(":selected").val() );
});


</script>



</body>
</html>
