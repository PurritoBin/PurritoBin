/*************** CONFIGURATION ***************/
/*
   CORS attributes on the pastebin need to allow
   the pasting website in its headers.
   Look at the man page for an example.
*/
var PURRITOBIN_URL = location.protocol + "//" + location.host + ":42069";

/********************************************/
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

function submitPaste() {
  var request = makeHttpObject();
  request.open("POST", PURRITOBIN_URL, true);
  request.setRequestHeader("Content-Type", "text/plain; charset=UTF-8");
  request.send(document.getElementById("paste").value);
  request.onreadystatechange = function () {
    var decoded = request.responseText.trim();
    var data_div = document.getElementById("url");
    data_div.innerHTML = `<a href='${decoded}'>${decoded}</a>`;
    return;
  };
}

function submitEncrypted() {
  var request = makeHttpObject();
  request.open("POST", PURRITOBIN_URL, true);
  request.setRequestHeader("Content-Type", "text/plain; charset=UTF-8");
  var data = document.getElementById("paste").value;
  var random_key = CryptoJS.lib.WordArray.random(32).toString();
  var random_iv = CryptoJS.lib.WordArray.random(16).toString();
  var random_key__ = CryptoJS.enc.Hex.parse(random_key);
  var random_iv__ = CryptoJS.enc.Hex.parse(random_iv);
  var cipher = CryptoJS.AES.encrypt(data, random_key__, {
    iv: random_iv__,
    mode: CryptoJS.mode.CBC,
  });
  request.send(cipher);
  request.onreadystatechange = function () {
    var url_ = request.responseText.trim();
    var slug = url_.substring(url_.lastIndexOf("/") + 1, url.length);
    var base_url = url_.substring(0, url_.lastIndexOf("/") + 1);
    var decoded =
      base_url + "paste.html#" + slug + "_" + random_key + "_" + random_iv;
    var data_div = document.getElementById("url");
    data_div.innerHTML = `<a href='${decoded}'>${decoded}</a>`;
    return;
  };
}
