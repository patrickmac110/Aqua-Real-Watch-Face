/* global Pebble, navigator, localStorage, XMLHttpRequest */

'use strict';

var config = require('./config');
var CACHE_KEY = 'aqua-real-weather-v1';
var REFRESH_MS = 60 * 60 * 1000;

function prefersFahrenheit() {
  var locale = (navigator.language || '').toLowerCase();
  return /^en-(us|bs|bz|ky|lr|fm|mh|pw)/.test(locale);
}

function sendWeather(weather) {
  Pebble.sendAppMessage({
    WEATHER_TEMP: weather.temperature,
    WEATHER_CODE: weather.code,
    WEATHER_IS_DAY: weather.isDay ? 1 : 0,
    WEATHER_UNIT: weather.unit === 'F' ? 70 : 67,
    WEATHER_SKY_PHASE: typeof weather.skyPhase === 'number'
      ? weather.skyPhase : (weather.isDay ? 1 : 3)
  }, function() {}, function(error) {
    console.log('Weather delivery failed: ' + JSON.stringify(error));
  });
}

function minutesFromIso(value) {
  if (typeof value !== 'string' || value.length < 16) {
    return null;
  }
  var hour = parseInt(value.substring(11, 13), 10);
  var minute = parseInt(value.substring(14, 16), 10);
  if (!isFinite(hour) || !isFinite(minute)) {
    return null;
  }
  return hour * 60 + minute;
}

function skyPhaseFor(response) {
  var current = response.current;
  if (!current || current.is_day !== 1) {
    return 3;
  }
  var daily = response.daily || {};
  var now = minutesFromIso(current.time);
  var sunrise = daily.sunrise && minutesFromIso(daily.sunrise[0]);
  var sunset = daily.sunset && minutesFromIso(daily.sunset[0]);
  if (now === null || sunrise === null || sunset === null || sunset <= sunrise) {
    return now !== null && now < 11 * 60 ? 0 : (now !== null && now >= 17 * 60 ? 2 : 1);
  }
  var daylight = sunset - sunrise;
  var progress = (now - sunrise) / daylight;
  if (progress < 0.28) {
    return 0;
  }
  if (progress < 0.72) {
    return 1;
  }
  return 2;
}

function cachedWeather() {
  try {
    var value = JSON.parse(localStorage.getItem(CACHE_KEY));
    if (value && typeof value.temperature === 'number' &&
        typeof value.code === 'number') {
      return value;
    }
  } catch (error) {
    console.log('Weather cache was unreadable: ' + error.message);
  }
  return null;
}

function requestForecast(latitude, longitude) {
  var useFahrenheit = prefersFahrenheit();
  var url = 'https://api.open-meteo.com/v1/forecast' +
      '?latitude=' + encodeURIComponent(latitude.toFixed(4)) +
      '&longitude=' + encodeURIComponent(longitude.toFixed(4)) +
      '&current=temperature_2m,weather_code,is_day' +
      '&daily=sunrise,sunset&timezone=auto&forecast_days=1' +
      '&temperature_unit=' + (useFahrenheit ? 'fahrenheit' : 'celsius');
  var request = new XMLHttpRequest();
  request.open('GET', url, true);
  request.timeout = 15000;
  request.onload = function() {
    if (request.status < 200 || request.status >= 300) {
      console.log('Weather HTTP status: ' + request.status);
      return;
    }
    try {
      var response = JSON.parse(request.responseText);
      var current = response.current;
      var weather = {
        temperature: Math.round(current.temperature_2m),
        code: current.weather_code,
        isDay: current.is_day === 1,
        skyPhase: skyPhaseFor(response),
        unit: useFahrenheit ? 'F' : 'C',
        fetchedAt: Date.now()
      };
      localStorage.setItem(CACHE_KEY, JSON.stringify(weather));
      sendWeather(weather);
    } catch (error) {
      console.log('Weather response was invalid: ' + error.message);
    }
  };
  request.onerror = function() {
    console.log('Weather request failed');
  };
  request.ontimeout = function() {
    console.log('Weather request timed out');
  };
  request.send();
}

function refreshWeather() {
  navigator.geolocation.getCurrentPosition(function(position) {
    requestForecast(position.coords.latitude, position.coords.longitude);
  }, function(error) {
    console.log('Location unavailable: ' + error.message);
  }, {
    timeout: 15000,
    maximumAge: 30 * 60 * 1000
  });
}

Pebble.addEventListener('ready', function() {
  config.send(config.load(), function() {
    var cached = cachedWeather();
    if (cached) {
      sendWeather(cached);
    }
    if (!cached || Date.now() - cached.fetchedAt > REFRESH_MS / 2) {
      refreshWeather();
    }
    setInterval(refreshWeather, REFRESH_MS);
  });
});

Pebble.addEventListener('showConfiguration', config.showConfiguration);
Pebble.addEventListener('webviewclosed', config.handleWebviewClosed);
