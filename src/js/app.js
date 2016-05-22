
Pebble.addEventListener('appmessage',
    function (e) {
      console.log('AppMessage received!');
      getWeather();
    }
);

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
    function (e) {
      console.log('PebbleKit JS ready!');
    }
);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  console.log('locationSuccess called!');
  var myAPIKey = '';
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + myAPIKey;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log('Conditions are ' + conditions);

      var dictionary = {
		'KEY_TEMPERATURE': temperature,
		'KEY_CONDITIONS': conditions
	  };

	  Pebble.sendAppMessage(dictionary,
		  function (e) {
			console.log('Weather info sent to Pebble succesfully!')
		  },
		  function (e) {
			console.log('Error sending weather info to Pebble!')
		  }
	  );

    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when an AppMessage is received
Pebble.addEventListener('ready',
    function (e) {
      console.log('AppMessage received!');

      // Get the initial weather
      getWeather();
    }
);
