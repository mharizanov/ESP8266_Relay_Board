var visibleFlag = 1;
var statusMsg = false;
var connected = false;
var doingsave = false;

var state = {
  relay1 : 0,
  relay1name : "Relay 1",
  relay1_therm_controlled : 0,
  relay2 : 0,
  relay2name : "Relay 2",
  relay2_therm_controlled : 0,
  relay3 : 0,
  relay3name : "Relay 3",
  relay3_therm_controlled : 0,
  relaytotal : 1,

};

function displayRelays(numRelays) {

  if (numRelays < 3) {
    $("#relay3tr").hide();
  } else {
    $("#relay3tr").show();
  }

  if (numRelays < 2) {
    $("#relay2tr").hide();
  } else {
    $("#relay2tr").show();
  }

  $("#relay1tr").show();
}

function update() {

  $("#relay1name").html(state.relay1name);
  $("#relay2name").html(state.relay2name);
  $("#relay3name").html(state.relay3name);

  if (state.relay1 == 1) {
    $("#relay1").html("ON");
    $("#relay1").css("background-color", "#ff9600");
  } else {
    $("#relay1").html("OFF");
    $("#relay1").css("background-color", "#555");
  }

  if (state.relay2 == 1) {
    $("#relay2").html("ON");
    $("#relay2").css("background-color", "#ff9600");
  } else {
    $("#relay2").html("OFF");
    $("#relay2").css("background-color", "#555");
  }

  if (state.relay3 == 1) {
    $("#relay3").html("ON");
    $("#relay3").css("background-color", "#ff9600");
  } else {
    $("#relay3").html("OFF");
    $("#relay3").css("background-color", "#555");
  }

  if (!state.relay1_therm_controlled) {
    relay1Enable();
  } else {
    $("#relay1").unbind('click');
  }

  if (!state.relay2_therm_controlled) {
    relay2Enable();
  } else {
    $("#relay2").unbind('click');
  }

  if (!state.relay3_therm_controlled) {
    relay3Enable();
  } else {
    $("#relay3").unbind('click');
  }

  displayRelays(state.relaytotal);
}

function relay1Enable() {

  $("#relay1").unbind('click');

  $("#relay1").click(function() {
    state.relay1++;
    if (state.relay1 > 1)
      state.relay1 = 0;

    if (state.relay1 == 1) {
      $(this).html("ON");
      $(this).css("background-color", "#ff9600");
    } else {
      $(this).html("OFF");
      $(this).css("background-color", "#555");
    }

    save("relay1", state.relay1);
  });
}

function relay2Enable() {

  $("#relay2").unbind('click');

  $("#relay2").click(function() {
    state.relay2++;
    if (state.relay2 > 1)
      state.relay2 = 0;

    if (state.relay2 == 1) {
      $(this).html("ON");
      $(this).css("background-color", "#ff9600");
    } else {
      $(this).html("OFF");
      $(this).css("background-color", "#555");
    }

    save("relay2", state.relay2);
  });
}

function relay3Enable() {

  $("#relay3").unbind('click');

  $("#relay3").click(function() {
    state.relay3++;
    if (state.relay3 > 1)
      state.relay3 = 0;

    if (state.relay3 == 1) {
      $(this).html("ON");
      $(this).css("background-color", "#ff9600");
    } else {
      $(this).html("OFF");
      $(this).css("background-color", "#555");
    }

    save("relay3", state.relay3);
  });
}
// function for checking if the page is visible or not
// (if not visible it will stop updating data)
function checkVisibility() {
  $(window).bind("focus", function(event) { visibleFlag = 1; });

  $(window).bind("blur", function(event) { visibleFlag = 0; });
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
  dur = dur * 1000;
  if (dur > 0) {
    setTimeout(function() {
      $("#statusView").hide(200);
      $("#statusView").html("");
      statusMsg = false
    }, dur)
  }
}

function save(param, payload) {
  doingsave = true;
  $.ajax({
    type : 'GET',
    url : "../control/relay.cgi?" + param + "=" + payload,
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

function relay_get() {
  var output = {};
  checkVisibility();
  if (visibleFlag) {
    $.ajax({
      url : "../control/relay.cgi",
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
            state = data;
            displayRelays(state.relaytotal);
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
  return;
}
