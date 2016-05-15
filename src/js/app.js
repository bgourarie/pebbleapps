var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};
function locationSuccess(pos){
  locationSuccessHelper( pos.coords.latitude,pos.coords.longitude);
}
function locationSuccessHelper(x,y) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
     x + '&lon=' + y + '&appid=da5ab015f853c0537b10646cfdee92a1';
 // console.log(url);
  xhrRequest(url, 'get', function(responseText){
    //response text contains a JSON obj with weather info
    var json = JSON.parse(responseText);
    var temperature = Math.round((json.main.temp - 273.15) * 9/5 + 32);
    console.log('temperature is '+temperature);
    var allIcons = ['01d','01n',
                    '02d','02n',
                    '03d','03n',
                    '04d','04n',
                    '09d','09n',
                    '10d','10n',
                    '11d','11n',
                    '13d','13n',
                    '50d','50n',
                   ];
    var conditions = allIcons.indexOf(json.weather[0].icon);
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
  // use hardcoded defaults!
  var lat = 47.620805;
  var long =  -122.349725;
  locationSuccessHelper(lat, long);
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


