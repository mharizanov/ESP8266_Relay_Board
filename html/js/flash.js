//===== FLASH cards

function flashFirmware(e) {
  e.preventDefault();
  var fw_data = document.getElementById('fw-file').files[0];

  $("#fw-form").setAttribute("hidden", "");
  showNotification("Firmware is being updated ...");

  ajaxReq("POST", "/flash/upload", function(resp) {
    ajaxReq("GET", "/flash/reboot", function(resp) {
      showNotification("Firmware has been successfully updated!");
      setTimeout(function() { window.location.reload() }, 4000);

      $("#fw-form").removeAttribute("hidden");
      $("#fw-form").removeAttribute("hidden");
    });
  }, null, fw_data)
}
function flashEspfs(e) {
  e.preventDefault();
  var fw_data = document.getElementById('espfs-file').files[0];

  $("#espfs-form").setAttribute("hidden", "");
  showNotification("Webpage ESPFS is being updated ...");

  ajaxReq("POST", "/flash/upload-espfs", function(resp) {
    setTimeout(function() { window.location.reload() }, 2000);
    showNotification("Webpages have been successfully updated!");
    $("#espfs-form").removeAttribute("hidden");
    $("#espfs-form").removeAttribute("hidden");
  }, null, fw_data)
}

function fetchFlash() {
  // Only show flash upload dialogue if flash map >1
  // i.e. Flash size > 512Kb
  ajaxReq("GET", "/flash/flash-size", function(resp) {
    if (resp > 1) {
      $("#fw-upgrade").removeAttribute("hidden");
      $("#fw-form").removeAttribute("hidden");
    } else {
      $("#fw-upgrade").setAttribute("hidden");
      $("#fw-form").setAttribute("hidden");
    }
  });
  ajaxReq("GET", "/flash/next", function(resp) {
    $("#fw-slot").innerHTML = resp;
  });
  ajaxReq("GET", "/flash/version", function(resp) {
    var v = $("#current-fw");
    if (v != null) {
      v.innerHTML = resp;
    } else {
      v.innerHTML = "Unknown";
    }
  });
}
