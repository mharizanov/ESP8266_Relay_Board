<html>
<head><title>ESP8266 Relay/Thermostat board</title>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
<div id="main">
<h1>About</h1>
<p>
Three channel WiFi relay/thermostat board <br/>
(c) <a href="http://harizanov.com">Martin Harizanov</a> 2014-2015
</p>
<p>
Current firmware: <span style="font-weight: bold;" id="current-fw"></span>
<a href="https://github.com/alandpearson/ESP8266_Relay_Board">Alan Pearson fork<a></p>
<p>%freeheap%</p>

<div class="card" id="fw-upgrade" hidden>
<div id="messages"><div id="warning" hidden></div><div id="notification" hidden></div></div>
            <h3>Upgrade Firmware</h3>
            <form action="#" id="fw-form" class="pure-form" hidden>
              <div class="pure-form-stacked">
                <p>
                  Make sure you upload the file called: <span style="font-weight: bold;" id="fw-slot"></span>
                </p>
                <label>Firmware File</label>
                <input type="file" name="fw-file" id="fw-file"/>
		      <button id="fw-button" type="submit" class="pure-button button-primary">
                Update the firmware
              </button>
              </div>
            </form>

</div>

<h3>Upload New Webpage ESPFS</h3>

<form action="#" id="espfs-form" class="pure-form" >
<label>ESPFS File</label>
<input type="file" name="espfs-file" id="espfs-file"/>
<button id="espfs-button" type="submit" class="pure-button button-primary">
  Update Webpages 
</button>
</form>


<button onclick="location.href = '/';" class="float-left submit-button" >Back</button>
    

<script src="./js/ui.js"></script>
<script src="./js/flash.js"></script>
<script type="text/javascript">

window.onload=function(e) {
  fetchFlash();
  bnd($("#fw-form"), "submit", flashFirmware);
  bnd($("#espfs-form"), "submit", flashEspfs);
};

</script>

</body></html>
