// Listen for when the watchface is opened
Pebble.addEventListener('ready',
    function (e) {
      console.log('PebbleKit JS ready!');
    }
);

function locationSuccess(pos) {
  // We will request the weather here
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
