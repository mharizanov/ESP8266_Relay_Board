var now = new Date();
var timenow = now.getHours() + (now.getMinutes() / 60);
var days = {0 : 'sun', 1 : 'mon', 2 : 'tue', 3 : 'wed', 4 : 'thu', 5 : 'fri', 6 : 'sat', 7 : 'sun'};
var today = days[now.getDay()];

//=================================================
// DATA
//=================================================
var roomTempTimeout = 0;
var relayRestPeriod = 0;
var visibleFlag = 1;
var setpoint = 21;
var unit = "&deg;C";
var statusMsg = false;
var connected = false;
var doingsave = false;
var tid;

var thermostat = {
  room_temp : -9999,
  humidity : "50",
  humidistat : 0,
  thermostat_relay_active : 0,
  zone_name : "Zone Name",
  opmode : 0,
  enable : 0,
  manual_setpoint : 21,
  current_setpoint : -9999,
  thermostat1_input : 0,
  schedule_mode : 0,
  therm_high_temp_colour_deg : 24,
  therm_low_temp_colour_deg : 10,
  mqtthost : "Undefined"
};

var schedule = {};

var day1 = [
  {s : 0, e : 6, sp : 8}, {s : 6, e : 9, sp : 18}, {s : 9, e : 17, sp : 12}, {s : 17, e : 22, sp : 18},
  {s : 22, e : 24, sp : 8}
];

schedule['mon'] = JSON.parse(JSON.stringify(day1));
schedule['tue'] = JSON.parse(JSON.stringify(day1));
schedule['wed'] = JSON.parse(JSON.stringify(day1));
schedule['thu'] = JSON.parse(JSON.stringify(day1));
schedule['fri'] = JSON.parse(JSON.stringify(day1));
schedule['sat'] = JSON.parse(JSON.stringify(day1));
schedule['sun'] = JSON.parse(JSON.stringify(day1));

schedule = server_get2("thermostat_schedule"); // all data * 100 to avoid floating point on the ESP8266 side
for (var d in schedule) {
  for (var z in schedule[d]) {
    schedule[d][z].s /= 100;
    schedule[d][z].e /= 100;
    schedule[d][z].sp /= 10; // except setpoint, we only mult this x10 on both sides
  }
}

// Colour map variables 0 colourmap function changed
// to read the thermostat strut directly
// var maxc = thermostat.therm_high_temp_colour_deg;
// var minc = thermostat.therm_low_temp_colour_deg;

// ================================================
// State variables
// ================================================
var editmode = 'move';
$("#mode-move").css("background-color", "#ff9600");
var key = 1;
var day = "mon";
var mousedown = 0;
var slider_width = $(".slider").width();
var slider_height = $(".slider").height();
var changed = 0;

thermostat = server_get2("state");
thermostat.manual_setpoint /= 10;

if (thermostat.humidistat) {
  unit = "%";
  $("#heating_thermostat").html("Humidify");
  $("#cooling_thermostat").html("Dry");
}
$("#unit").html(unit);

setpoint = thermostat.manual_setpoint;
$(".zone-setpoint").html(setpoint.toFixed(1) + unit);

update();
updateclock();
setInterval(server_get, 5000);
setInterval(updateclock, 1000);
$("#upload_sched").click(uploadSchedule);
$("#schedule_json").hide();
$("#schedule_export").click(function() { $("#schedule_json").toggle(); });

function updateclock() {

  servertime = 0;

  if (thermostat.time != 0) {
    now = new Date(parseInt(thermostat.time) * 1000);
    servertime = 1;
  } else {
    now = new Date(0);
    servertime = 0;
  }
  //Use UTC methods to avoid DST conversions
  //as thermostat time display needs to use same time,timezone and DST
  //as server
  timenow = now.getUTCHours() + (now.getUTCMinutes() / 60);
  today = days[now.getUTCDay()];

  checkVisibility();

  if (servertime) {
    $("#datetime").html("<center>" + today.toUpperCase() + " " + format_time(timenow) + "</center>");
  } else {
    $("#datetime").html("<center><font color=\"red\">?Current Server Date Time?</font></center>");
  }
  //$("#datetime").html("Time Now:" + thermostat.time);

  if (thermostat.schedule_mode == 0) {
    setpoint = thermostat.manual_setpoint;
    $(".zone-setpoint").html(setpoint.toFixed(1) + unit);
  }

  var current_key = 0;
  for (var z in schedule[today]) {
    if (schedule[today][z].s <= timenow && schedule[today][z].e > timenow) {
      if (thermostat.schedule_mode == 1) {
        setpoint = schedule[today][z].sp * 1;
        $(".zone-setpoint").html(setpoint.toFixed(1) + unit);
        current_key = z;
      }
    }
  }

  var sx = $(".slider[day=" + today + "]")[0].offsetLeft;
  var y = $(".slider[day=" + today + "]")[0].offsetTop;
  var x1 = sx + slider_width * (timenow / 24.0);
  var x2 = sx + slider_width * (schedule[today][current_key].s / 24.0);

  x2 = sx;
  $("#timemarker").css('top', y + "px");
  $("#timemarker").css('left', x2 + "px");
  $("#timemarker").css('width', (x1 - x2) + "px");
}

function setStatus(msg, dur, pri) { // show msg on status bar

  if (statusMsg == true) {
    return
  };
  statusMsg = true;
  if (pri > 0) {
    $("#statusView").toggleClass("statusViewAlert", true);
    $("#statusView").toggleClass("statusView", false);
  } else {
    $("#statusView").toggleClass("statusView", true);
    $("#statusView").toggleClass("statusViewAlert", false);
  }
  $("#statusView").show();
  $("#statusView").html(msg);
  window.scrollTo(0, 0);

  dur = dur * 1000;
  if (dur > 0) {
    statusTimeout = setTimeout(function() {
      $("#statusView").hide(200);
      $("#statusView").html("");
      statusMsg = false
    }, dur)
  }
}

function update() {

  $(".zone-title").html(thermostat.zone_name);

  if (!isNaN((Number(thermostat.humidity)).toFixed(1)))
    $('.humidity').show();

  if (Number(thermostat.room_temp) == -9999 && thermostat.thermostat1_input == 3) {
    $(".zone-temperature").html("--.- &deg;C");
    roomTempTimeout = 1;
    setStatus("Room temperature timeout, check remote temperature sender.", 0, 1);
  } else if (Number(thermostat.room_temp) == -9999) {
    $(".zone-temperature").html("--.- &deg;C");
    setStatus("Room temperature timeout, check sensor.", 0, 1);
    roomTempTimeout = 1;

  } else {
    $(".zone-temperature").html((Number(thermostat.room_temp / 10)).toFixed(1) + "&deg;C");
    if (roomTempTimeout) {
      setStatus("Room temperature sensor restored.", 2, 0);
      roomTempTimeout = 0;
    }
  }
  $("#zone-humidity").html((Number(thermostat.humidity)).toFixed(1) + "%");

  if (thermostat.thermostat_relay_active === 0) {
    $(".zone-setpoint").css("color", "#000000");
    if (relayRestPeriod) {
      setStatus("Relay cycle timer cleared.", 2, 0);
      relayRestPeriod = 0;
    }
  } else if (thermostat.thermostat_relay_active === 1) {
    $(".zone-setpoint").css("color", "#f00000");
    if (relayRestPeriod) {
      setStatus("Thermostat cycle timer cleared.", 2, 0);
      relayRestPeriod = 0;
    }
  } else {
    // some sort of issue, turn blue (e.g. relay in rest mode)
    $(".zone-setpoint").css("color", "#6495ED");
    setStatus("Thermostat resting to avoid cycling", 0, 0);
    relayRestPeriod = 1;
  }

  if (thermostat.enable == 1) {
    $("#toggle").html("ON");
    $("#toggle").css("background-color", "#ff9600");
  } else {
    $("#toggle").html("OFF");
    $("#toggle").css("background-color", "#555");
  }

  if (thermostat.schedule_mode == 0) {
    // manual
    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#ff9600");
    $("#scheduled_thermostat").css("background-color", "#555");
  } else if (thermostat.schedule_mode == 1) {
    // auto
    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#555");
    $("#scheduled_thermostat").css("background-color", "#ff9600");
  } else {
    // override
    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#555");
    $("#scheduled_thermostat").css("background-color", "#af9600");
  }

  if (thermostat.opmode == 0) {
    $(".thermostatopmode").css("background-color", "#555");
    $("#heating_thermostat").css("background-color", "#c00000");
    $("#cooling_thermostat").css("background-color", "#555");
  } else {
    $(".thermostatopmode").css("background-color", "#555");
    $("#heating_thermostat").css("background-color", "#555");
    $("#cooling_thermostat").css("background-color", "#0000c0");
  }
  document.title = thermostat.zone_name;
}

$("#toggle").click(function() {
  thermostat.enable++;
  if (thermostat.enable > 1)
    thermostat.enable = 0;
  if (thermostat.enable == 1) {
    $("#toggle").html("ON");
    $(this).css("background-color", "#ff9600");
  }
  if (thermostat.enable === 0) {
    $("#toggle").html("OFF");
    $(this).css("background-color", "#555");
  }

  // save("tx/heating",thermostat.enable+","+parseInt(setpoint*100));
  save("thermostat_enable", thermostat.enable.toString());
});

$("#zone-setpoint-dec").click(function() {
  clearTimeout(tid);
  if (thermostat.schedule_mode == 1) {
    // only go into override mode if in automode
    thermostat.schedule_mode = 2;
    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#555");
  }

  //$(".thermostatmode").css("background-color", "#555");
  //$("#manual_thermostat").css("background-color", "#555");
  thermostat.manual_setpoint -= 0.5;
  setpoint = thermostat.manual_setpoint;
  $(".zone-setpoint").html(setpoint.toFixed(1) + unit);
  tid = setTimeout(function() {
    save("thermostat_schedule_mode", (thermostat.schedule_mode).toString());
    save("thermostat_manual_setpoint", ((thermostat.manual_setpoint.toFixed(1)) * 10).toString());
  }, 1000);
});

$("#zone-setpoint-inc").click(function() {
  clearTimeout(tid);
  if (thermostat.schedule_mode == 1) {
    // only go into override mode if in automode
    thermostat.schedule_mode = 2;
    $(".thermostatmode").css("background-color", "#555");
    $("#manual_thermostat").css("background-color", "#555");
  }
  // $(".thermostatmode").css("background-color", "#555");
  // $("#manual_thermostat").css("background-color", "#555");
  thermostat.manual_setpoint += 0.5;
  setpoint = thermostat.manual_setpoint;
  $(".zone-setpoint").html(setpoint.toFixed(1) + unit);

  tid = setTimeout(function() {
    save("thermostat_schedule_mode", (thermostat.schedule_mode).toString());
    save("thermostat_manual_setpoint", ((thermostat.manual_setpoint.toFixed(1)) * 10).toString());
  }, 1000);
});

// ============================================
// SCHEDULER

for (day in schedule)
  draw_day_slider(day);

function draw_day_slider(day) {
  var out = "";
  var key = 0;
  for (var z in schedule[day]) {
    var left = (schedule[day][z].s / 24.0) * 100;
    var width = ((schedule[day][z].e - schedule[day][z].s) / 24.0) * 100;
    var color = color_map(schedule[day][z].sp);

    out += "<div class='slider-segment' style='left:" + left + "%; width:" + width + "%; background-color:" + color +
           "' key=" + key + " title='" + schedule[day][z].sp + unit + "'></div>";

    if (key > 0) {
      out += "<div class='slider-button' style='left:" + left + "%;' key=" + key + "></div>";
    }
    key++;
  }
  out += "<div class='slider-label'>" + day.toUpperCase() + "</div>";
  $(".slider[day=" + day + "]").html(out);
}
$("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));

$("body").on("mousedown", ".slider-button", function(e) {
  mousedown = 1;
  key = $(this).attr('key');
});
$("body").mouseup(function(e) {
  mousedown = 0;
  if (changed) {
    save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    changed = 0;
  }
});

$("body").on("mousemove", ".slider", function(e) {
  if (mousedown && editmode == 'move') {
    day = $(this).attr('day');
    slider_update(e);
  }
});

$("body").on("touchstart", ".slider-button", function(e) {
  mousedown = 1;
  key = $(this).attr('key');
});
$("body").on("touchend", ".slider-button", function(e) {
  mousedown = 0;
  if (changed) {
    save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    changed = 0;
  }
});

$("body").on("touchmove", ".slider", function(e) {
  var event = window.event;
  e.pageX = event.touches[0].pageX;
  if (mousedown && editmode == 'move') {
    day = $(this).attr('day');
    slider_update(e);
  }
});

// MERGE
$("body").on("click", ".slider-button", function() {
  if (editmode == 'merge') {
    day = $(this).parent().attr("day");
    key = parseInt($(this).attr("key"), 10);
    schedule[day][key - 1].e = schedule[day][key].e;
    schedule[day].splice(key, 1);
    draw_day_slider(day);
    // editmode = 'move';
    save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
  }
});

$("body").on("click", ".slider-segment", function(e) {
  day = $(this).parent().attr("day");
  key = parseInt($(this).attr("key"), 10);

  if (editmode == 'split') {
    var x = e.pageX - $(this).parent()[0].offsetLeft;
    var prc = x / slider_width;
    var hour = prc * 24.0;
    hour = Math.round(hour / 0.5) * 0.5;

    if (hour > schedule[day][key].s + 0.5 && hour < schedule[day][key].e - 0.5) {
      var end = parseFloat(schedule[day][key].e);
      schedule[day][key].e = hour;

      schedule[day].splice(key + 1, 0, {s : hour, e : end, sp : schedule[day][key].sp});

      draw_day_slider(day);
      $("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));
      save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
    }
    // editmode = 'move';
  } else if (editmode == 'move') {
    $("#slider-segment-temperature").val((schedule[day][key].sp * 1).toFixed(1));
    $("#slider-segment-start").val(format_time(schedule[day][key].s));
    $("#slider-segment-end").val(format_time(schedule[day][key].e));

    $("#slider-segment-block").show();
    $("#slider-segment-block-movepos").hide();
  }
});

function slider_update(e) {
  $("#slider-segment-block-movepos").show();
  $("#slider-segment-block").hide();

  if (key !== undefined) {
    var x = e.pageX - $(".slider[day=" + day + "]")[0].offsetLeft;

    var prc = x / slider_width;
    var hour = prc * 24.0;
    hour = Math.round(hour / 0.5) * 0.5;

    if (hour > schedule[day][key - 1].s && hour < schedule[day][key].e) {
      schedule[day][key - 1].e = hour;
      schedule[day][key].s = hour;
      update_slider_ui(day, key);
      changed = 1;
    }
    $("#slider-segment-time").val(format_time(schedule[day][key].s));
  }
  // $("#average_temperature").html(calc_average_schedule_temperature().toFixed(1));
}

$("body").on("click", "#slider-segment-ok", function() {
  schedule[day][key].sp = $("#slider-segment-temperature").val();
  var color = color_map(schedule[day][key].sp);
  $(".slider[day=" + day + "]").find(".slider-segment[key=" + key + "]").css("background-color", color);

  var time = decode_time($("#slider-segment-start").val());
  if (time != -1 && key > 0 && key < schedule[day].length) {
    if (time >= (schedule[day][key - 1].s + 0.5) && time <= (schedule[day][key].e - 0.5)) {
      schedule[day][key - 1].e = time;
      schedule[day][key].s = time;
    }
  }
  $("#slider-segment-start").val(format_time(schedule[day][key].s));
  update_slider_ui(day, key);

  time = decode_time($("#slider-segment-end").val());
  if (time != -1 && key > 0 && key < (schedule[day].length - 1)) {
    if (time >= (schedule[day][key].s + 0.5) && time <= (schedule[day][key + 1].e - 0.5)) {
      schedule[day][key].e = time;
      schedule[day][key + 1].s = time;
    }
  }
  $("#slider-segment-end").val(format_time(schedule[day][key].e));
  update_slider_ui(day, key + 1);
  save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
  updateclock();
});

$("#slider-segment-movepos-ok").click(function() {
  var time = decode_time($("#slider-segment-time").val());
  if (time != -1 && key > 0) {
    if (time >= (schedule[day][key - 1].s + 0.5) && time <= (schedule[day][key].e - 0.5)) {
      schedule[day][key - 1].e = time;
      schedule[day][key].s = time;
    }
  }
  $("#slider-segment-time").val(format_time(schedule[day][key].s));
  update_slider_ui(day, key);
  save("thermostat_schedule", "{\"" + day + "\":" + JSON.stringify(calc_schedule_esp(schedule[day])) + "}");
});

$("#mode-split").click(function() {
  editmode = 'split';
  $(".editmode").css("background-color", "#555");
  $(this).css("background-color", "#ff9600");
});

$("#mode-move").click(function() {
  editmode = 'move';
  $(".editmode").css("background-color", "#555");
  $(this).css("background-color", "#ff9600");
});

$("#mode-merge").click(function() {
  editmode = 'merge';
  $(".editmode").css("background-color", "#555");
  $(this).css("background-color", "#ff9600");
});

$("#manual_thermostat").click(function() {
  $(".thermostatmode").css("background-color", "#555");
  $(this).css("background-color", "#ff9600");
  thermostat.schedule_mode = 0;

  setpoint = thermostat.manual_setpoint;
  $(".zone-setpoint").html(setpoint.toFixed(1) + unit);

  save("thermostat_schedule_mode", (thermostat.schedule_mode).toString());
  updateclock();
});

$("#scheduled_thermostat").click(function() {
  $(".thermostatmode").css("background-color", "#555");
  $(this).css("background-color", "#ff9600");
  thermostat.schedule_mode = 1;
  save("thermostat_schedule_mode", (thermostat.schedule_mode).toString());
  updateclock();
});

$("#heating_thermostat").click(function() {
  $(".thermostatopmode").css("background-color", "#555");
  $(this).css("background-color", "#c00000");
  opmode = 0;
  save("thermostat_opmode", opmode.toString());
  updateclock();
});

$("#cooling_thermostat").click(function() {
  $(".thermostatopmode").css("background-color", "#555");
  $(this).css("background-color", "#0000c0");
  opmode = 1;
  save("thermostat_opmode", opmode.toString());
  updateclock();
});

function color_map(temperature) {
  /*
  // http://www.particleincell.com/blog/2014/colormap/
  // rainbow short
  var f=(temperature-minc)/(maxc-minc);	//invert and group
      var a=(1-f)/0.25;	//invert and group
      var X=Math.floor(a);	//this is the integer part
      var Y=Math.floor(255*(a-X)); //fractional part from 0 to 255
      switch(X)
      {
              case 0: r=255;g=Y;b=0;break;
              case 1: r=255-Y;g=255;b=0;break;
              case 2: r=0;g=255;b=Y;break;
              case 3: r=0;g=255-Y;b=255;break;
              case 4: r=0;g=0;b=255;break;
      }

      */
  var f = (temperature - thermostat.therm_low_temp_colour_deg) /
          (thermostat.therm_high_temp_colour_deg - thermostat.therm_low_temp_colour_deg);
  // var f = (temperature - minc) / (maxc - minc);
  var a = (1 - f);
  var Y = Math.floor(255 * a);
  r = 255;
  g = Y;
  b = 0;

  return "rgb(" + r + "," + g + "," + b + ")";
}

function update_slider_ui(day, key) {
  if (schedule[day] !== undefined && key < schedule[day].length) {
    var slider = $(".slider[day=" + day + "]");
    if (key > 0) {
      var width = ((schedule[day][key - 1].e - schedule[day][key - 1].s) / 24.0) * 100;
      slider.find(".slider-segment[key=" + (key - 1) + "]").css("width", width + "%");
    }

    var left = (schedule[day][key].s / 24.0) * 100;
    var width = ((schedule[day][key].e - schedule[day][key].s) / 24.0) * 100;
    slider.find(".slider-segment[key=" + key + "]").css("width", width + "%");
    slider.find(".slider-segment[key=" + key + "]").css("left", left + "%");
    slider.find(".slider-button[key=" + key + "]").css("left", left + "%");
  }
}

function format_time(time) {
  var hour = Math.floor(time);
  var mins = Math.round((time - hour) * 60);
  if (mins < 10)
    mins = "0" + mins;
  if (hour < 10)
    hour = "0" + hour;
  return hour + ":" + mins;
}

function decode_time(timestring) {
  var time = -1;
  if (timestring.indexOf(":") != -1) {
    var parts = timestring.split(":");
    var hour = parseInt(parts[0], 10);
    var mins = parseInt(parts[1], 10);

    if (mins >= 0 && mins < 60 && hour >= 0 && hour < 25) {
      if (hour == 24 && mins !== 0) {
      } else {
        time = hour + (mins / 60);
      }
    }
  }
  return time;
}

function calc_average_schedule_temperature() {
  var sum = 0;
  for (var d in schedule) {
    for (var z in schedule[d]) {
      var hours = (schedule[d][z].e - schedule[d][z].s)
      sum += (schedule[d][z].sp * hours);
    }
  }
  return sum / (24 * 7.0);
}

function calc_schedule_esp(sched) {
  var fixsched = JSON.parse(JSON.stringify(sched));
  for (var d in fixsched) {
    fixsched[d].s *= 100;
    fixsched[d].e *= 100;
    fixsched[d].sp *= 10;
  }
  return fixsched;
}

// function for checking if the page is visible or not
// (if not visible it will stop updating data)
function checkVisibility() {
  $(window).bind("focus", function(event) { visibleFlag = 1; });

  $(window).bind("blur", function(event) { visibleFlag = 0; });
}

function save(param, payload) {
  //  if( doingsave ) {
  //    return;
  //}
  doingsave = true;
  $.ajax({
    type : 'POST',
    url : "thermostat.cgi?param=" + param,
    data : payload,
    dataType : 'text',
    cache : false,
    async : true,
    timeout : 3000,
    tryCount : 0,
    retryLimit : 3,
    success : function(data) {
      statusMsg = false;
      if (!connected)
        setStatus("Connected", 2, 0);
      connected = true;
      doingsave = false;
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
      doingsave = false;
    }
  });
}

function server_get() {
  var output = {};
  if (visibleFlag) {
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
            output = data;
            thermostat = data;
            thermostat.manual_setpoint /= 10;
            update();
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
  }
  return output;
}

function server_get2(param) {
  var output = {};
  if (visibleFlag) {
    $.ajax({
      url : "thermostat.cgi?param=" + param,
      dataType : 'json',
      async : false,
      timeout : 3000,
      tryCount : 0,
      retryLimit : 3,
      success : function(data) {
        if (data.length !== 0)
          output = data;
        statusMsg = false;
        if (!connected)
          setStatus("Connected", 2, 0);
        connected = true;
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
  }
  return output;
}

function uploadSchedule() {

  var sched_data = document.getElementById('sched_file').files[0];

  $.ajax({
    type : "POST",
    url : "../control/thermostat.cgi?param=thermostat_schedule",
    data : sched_data,
    processData : false, // tell jQuery not to process the data
    dataType : "text",
    contentType : "application/x-www-form-urlencoded",
    success : function() {
      setTimeout(function() { window.location.reload() }, 2000);
      setStatus("Schedule has been successfully uploaded.", 2, 0);
    },
    error : function(xhr, textStatus, errorThrown) {
      setStatus("Schedule not uploaded - Upload one day at a time.", 0, 1);
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
}
