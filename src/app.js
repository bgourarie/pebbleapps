/**
 * Welcome to Pebble.js!
 *
 */

var UI = require('ui');
var ajax = require('ajax');
var Vector2 = require('vector2');

var splashWindow = new UI.Window();

var text = new UI.Text({
  position: new Vector2(0,0),
  size: new Vector2(144,168),
  text: 'Downloading Weather data...',
  font: 'GOTHIC_28_BOLD',
  color: 'black',
  textOverflow: 'wrap',
  textAlign: 'center',
  backgroundColor: 'white'
});

splashWindow.add(text);
splashWindow.show();
/*var card = new UI.Card({
  title: 'Weather',
  subtitle: 'Fetching...'
});
// card.show();
*/
var parseFeed =  function(data, quantity) {
  var items = [];
  for(var i=0; i< quantity; i++) {
    var title = data.list[i].weather[0].main;
    title = title.charAt(0).toUpperCase() + title.substring(1);
    var time = data.list[i].dt_txt;
    time = time.substring(time.indexOf('-')+1, time.indexOf(':') + 3);
    items.push({
      title:title,
      subtitle:time
    });
  }
  return items;
}
// fetch teh weather info

var lat = 47.620805;
var long =  -122.349725;
var myAPIKey = 'da5ab015f853c0537b10646cfdee92a1';
var URL = 'http://api.openweathermap.org/data/2.5/forecast?lat='+lat+'&lon='+long +'&appid='+myAPIKey;
var URL2 = 'http://api.openweathermap.org/data/2.5/weather?lat='+lat+'&lon='+long +'&appid='+myAPIKey;

// make the req:
ajax(
{
  url: URL,
  type: 'json',
},
  function(data){
    //success:
    var menuItems = parseFeed(data,10);
    // just check they extracted okay..
    for(var i =0; i<menuItems.length; i++){
      console.log(menuItems[i].title+ ' | '+menuItems[i].subtitle);
  }   
  var resultsMenu = new UI.Menu({
    sections: [{
      title: 'Current Forecast',
      items: menuItems
    }]
  });
  
    resultsMenu.show();
    splashWindow.hide();
  /*  
    console.log('Successfully fetched the weaher');
    var location = data.name;
    var temp = Math.round(data.main.temp - 273.15) + 'C';
    
    var description = data.weather[0].description;
    description = description.charAt(0).toUpperCase() + description.substring(1);
    card.subtitle(location+', '+temp);
    card.body(description);
    */
  },
  function (err){
    console.log('Failed fetching weather data ' + err);
  }
);