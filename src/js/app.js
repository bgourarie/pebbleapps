var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};
function locationSuccess(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=da5ab015f853c0537b10646cfdee92a1';
 // console.log(url);
  xhrRequest(url, 'get', function(responseText){
    //response text contains a JSON obj with weather info
    var json = JSON.parse(responseText);
    var temperature = Math.round((json.main.temp - 273.15) * 9/5 + 32);
    console.log('temperature is '+temperature);
    var conditions = json.weather[0].main;
    console.log('Conditions are '+conditions);
    // send updates to pebble:

    var dictionary = {
      'KEY_TEMPERATURE' : temperature,
      'KEY_CONDITIONS' : conditions
    };
    
    Pebble.sendAppMessage(dictionary, 
                         function(e){
                           console.log('Weather info sent successfully');
                         },
                         function(e){
                           console.log('Error sending weather info!');
                         });
    
  });
}
function locationError(err){
  console.log('Error requesting location!');
}
function getWeather(){
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when watchface is opened
Pebble.addEventListener('ready',
                       function(e) {
                         console.log('PebbleKit JS ready!');
                         getWeather();
                       });
//listen for app messages
Pebble.addEventListener('appmessage',
                       function(e) {
                         console.log('AppMessage Received!');
                         getWeather();
                       });


