//----------------------------------------------------------------------
// webvars.h
// ---------
// We put the HTML and Javascript in a separate file.
// Partly for tidiness, and partly because there is an obscure bug in
// the Arduino IDE that can corrupt Javascript if you put it in the .ino
// file.
//----------------------------------------------------------------------

// Simple home page
const String homepage = R"(
<!DOCTYPE html>
<html>
  <head>
    <title>Web sockets test</title>
    <script src="script.js"></script>
  </head>
  <body onload="onPageLoad() ">
    <h1>Web sockets test</h1>
    <p>This is a very simple web sockets test page that just updates a single number.</p>
    <p><span id="foo">Foo</span></p>
  </body>
</html>
)";

// Javascript for home page
const String script = R"(
var gateway = "ws://" + window.location.hostname + "/ws";
var websocket;

// When the page loads we initialise the WebSocket code
function onPageLoad(event) {
  initWebSocket();
}

// Initialise the WebSocket
function initWebSocket() {
  console.log("Trying to open a WebSocket connection");
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;

  // The client requests data using a timer
  setInterval(getReadings, 100); // Refresh rate is 10Hz
}

// To request a reading send a message to the server
function getReadings(){
  websocket.send("getReadings");
}

// Just log connection open and close
function onOpen(event) {
  console.log("Connection opened");
  getReadings();
}

function onClose(event) {
  console.log("Connection closed");
  setTimeout(initWebSocket, 2000);
}

// Called when we receive data from the server
function onMessage(event) {
  // Parse the message as JSON
  var myObj = JSON.parse(event.data);
  // Set the page element to the data
  document.getElementById("foo").innerHTML = myObj["foo"];
})"
;