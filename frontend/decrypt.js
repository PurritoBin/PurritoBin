function makeHttpObject() {
  try {
    return new XMLHttpRequest();
  } catch (error) {}
  try {
    return new ActiveXObject("Msxml2.XMLHTTP");
  } catch (error) {}
  try {
    return new ActiveXObject("Microsoft.XMLHTTP");
  } catch (error) {}
  throw new Error("Could not create HTTP request object.");
}
// get location without
var location_full = window.location.href;
var location_path = location_full.split("#")[0];
location_path = location_path.substring(0, location_path.length - 10);
// first we get the needed values from the hash
// this is still secure cuz nothing is sent to the server
var hash = window.location.hash.substr(1);
var hashv = hash.split("_");
if (hashv.length > 0 && hash != "") {
  var pasteurl = location_path + hashv[0];
  var key = "",
    default_iv = "";
  if (hashv.length > 1) {
    key = CryptoJS.enc.Hex.parse(hashv[1]);
  }
  if (hashv.length > 2) {
    default_iv = CryptoJS.enc.Hex.parse(hashv[2]);
  } else {
    default_iv = CryptoJS.enc.Hex.parse("00000000000000000000000000000000");
  }

  // this part is async, so we need to define the
  // decryption calls after the request has been fullfilled
  // the joys of async, damn
  var request = makeHttpObject();
  request.open("GET", pasteurl, true);
  request.send(null);
  request.onreadystatechange = function () {
    if (request.readyState == 4) {
      var decoded = request.responseText;
      if (hashv.length > 1) {
        var decrypted = CryptoJS.AES.decrypt(decoded, key, {
          iv: default_iv,
          mode: CryptoJS.mode.CBC,
        });
        decoded = decrypted.toString(CryptoJS.enc.Utf8);
      }
      var data_div = document.getElementById("dec");
      data_div.innerText = decoded;
      return;
    }
  };
}
