/*
Based on Luca Soltoggio's work
http://arduinoelettronica.wordpress.com/

*/

var visibleFlag = 1;

var thermostat = {
  room_temp : -9999,
  humidity : "N/A",
  humidistat : 0,
  thermostat_relay_active : 0,
  relay1name : "N/A",
  opmode : 0,
  enable : 0,
  manual_setpoint : -9999,
  current_setpoint : -9999,
  thermostat1_input : 0,
  schedule_mode : 0,
  mqtthost : "N/A"
};

// get and parse json data from ESP8266 and update HTML page
function get_esp_data() {
  // if (visibleFlag) {
  var jqxhr = $.getJSON('state.cgi?random=' + Math.random(), function(data) {
                 // call value_update function passing json variable
                 value_update(data);
                 $("[data-role='slider']").slider('enable');
                 $('label').css('color', '#333');
                 $('.ui-bar-d').css('color', '#333');
                 $('.inputvalue').css('color', 'coral');
                 $('.s-title').css('color', '#333');
               }).error(function() {
    $("[data-role='slider']").slider('disable');
    $('label').css('color', '#BBBBBB');
    $('.ui-bar-d').css('color', '#BBBBBB');
    $('.inputvalue').css('color', '#BBBBBB');
    $('.s-title').css('color', '#BBBBBB');
  });
  //}
}

function set_esp_data(pname, pvalue) {
  $.getJSON('relay.cgi?' + pname + '=' + pvalue + '&random=' + Math.random(), function(data) { value_update(data); });
}

// if a slider changed, this funciont will call the function above
function detect_changes() {
  $("[data-role='slider']").change(function() { set_esp_data($(this).attr("name"), $(this).val()); });
}

// function for parsing json data and updating HTML page
// this will update slider status and label value
function value_update(data) {
  $.each(data, function(index, value) {
    if (index.match(/relay.*/)) {
      $('#' + index).val(value).slider("refresh");
    } else {
      $('#' + index).text(value);
    }
  });
}

// function for checking if the page is visible or not
// (if not visible it will stop updating data)
function checkVisibility() {
  $(window).bind("focus", function(event) { visibleFlag = 1; });

  $(window).bind("blur", function(event) { visibleFlag = 0; });
}

function sensor_get() {
  var output = {};
  // if (visibleFlag) {
  $.ajax({
    url : "../control/state.cgi",
    dataType : 'json',
    async : true,
    timeout : 3000,
    tryCount : 0,
    retryLimit : 3,
    success : function(data) {
      if (data.length !== 0) {
        statusMsg = false;
        if (!connected)
          setStatus("Connected", 2, 0);
        connected = true;
        if (!doingsave) {
          sensors = data;
          sensor_update();
        }
      }
    },
    error : function(xhr, textStatus, errorThrown) {
      alert(errorThrown);
      if (textStatus == 'timeout') {
        this.tryCount++;
        if (this.tryCount <= this.retryLimit) {
          // try again
          $.ajax(this);
          return;
        }
        return;
      }
      if (connected)
        setStatus("No connection to server!", 0, 1);
      connected = false;
    }
  });
  // }
  return output;
}

function therm_get() {
  var output = {};

  // if (visibleFlag) {
  $.ajax({
    url : "thermostat.cgi?param=state",
    dataType : 'json',
    async : true,
    timeout : 3000,
    tryCount : 0,
    retryLimit : 3,
    success : function(data) {
      if (data.length !== 0) {
        statusMsg = false;
        if (!connected)
          setStatus("Connected", 2, 0);
        connected = true;
        if (!doingsave) {
          thermostat = data;
          therm_update();
        }
      }
    },
    error : function(xhr, textStatus, errorThrown) {
      if (textStatus == 'timeout') {
        this.tryCount++;
        if (this.tryCount <= this.retryLimit) {
          // try again
          $.ajax(this);
          return;
        }
        return;
      }
      if (connected)
        setStatus("No connection to server!", 0, 1);
      connected = false;
    }
  });
  // }
  return output;
}

function sensor_update() {

  if (sensors.ds18b20.length > 0) {
    $("#ds18b20_0").show();
    $("#ds18b20_temperature_0").html(sensors.ds18b20[0].temperature + "&#8451;");
    $("#ds18b20_id_0").html(sensors.ds18b20[0].id);
  } else {
    $("#ds18b20_0").hide();
    $("#ds18b20_1").hide();
    $("#ds18b20_2").hide();
    $("#ds18b20_3").hide();
  }

  if (sensors.ds18b20.length > 1) {
    $("#ds18b20_1").show();
    $("#ds18b20_temperature_1").html(sensors.ds18b20[1].temperature + "&#8451;");
    $("#ds18b20_id_1").html(sensors.ds18b20[1].id);
  } else {
    $("#ds18b20_1").hide();
    $("#ds18b20_2").hide();
    $("#ds18b20_3").hide();
  }

  if (sensors.ds18b20.length > 2) {
    $("#ds18b20_2").show();
    $("#ds18b20_temperature_2").html(sensors.ds18b20[2].temperature + "&#8451;");
    $("#ds18b20_id_2").html(sensors.ds18b20[2].id);
  } else {
    $("#ds18b20_2").hide();
    $("#ds18b20_3").hide();
  }

  if (sensors.ds18b20.length > 3) {
    $("#ds18b20_3").show();
    $("#ds18b20_temperature_3").html(sensors.ds18b20[3].temperature + "&#8451;");
    $("#ds18b20_id_3").html(sensors.ds18b20[3].id);
  } else {
    $("#ds18b20_3").hide();
  }
}

function therm_update() {

  if (thermostat.enable == 0) {
    var therm_state = "Off";
    $("#thermostat_enable").html("Disabled");
    $("#room_temp_li").hide();
    $("#set_point_li").hide();
    $("#call_for_heat_li").hide();

  } else if (thermostat.enable == 1) {
    $("#thermostat_enable").html("Enabled");
    $("#room_temp_li").show();
    $("#set_point_li").show();
    $("#call_for_heat_li").show();
  }
  if (thermostat.room_temp == -9999) {
    $("#thermostat1_room_temperature").html("--.-");
  } else {
    $("#thermostat1_room_temperature").html(thermostat.room_temp / 10);
  }

  if (thermostat.current_setpoint == -9999) {
    $("#thermostat1_setpoint").html("--.-");
  } else {
    $("#thermostat1_setpoint").html(thermostat.current_setpoint / 10);
  }

  if (thermostat.thermostat_relay_active == 0) {
    $("#thermostat1_on").html("No");
  } else if (thermostat.thermostat_relay_active == 1) {
    $("#thermostat1_on").html("Yes");
  } else if (thermostat.thermostat_relay_active == 2) {
    $("#thermostat1_on").html("Relay Rest Period");
  } else {
    $("#thermostat1_on").html("Unknown State");
  }
}
