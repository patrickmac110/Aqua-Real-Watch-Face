/* global Pebble, localStorage */

'use strict';

var STORAGE_KEY = 'aqua-real-settings-v1';
var DEFAULTS = {
  timeSize: 100,
  timeStyle: 3,
  timeFormat: 0,
  timeFont: 13,
  goldBrightness: 135,
  dateSize: 14,
  dateBold: 0,
  dateFormat: 0,
  weatherSize: 18,
  weatherBackground: 0,
  weatherForeground: 0
};

function integer(value, fallback) {
  var parsed = parseInt(value, 10);
  return isFinite(parsed) ? parsed : fallback;
}

function oneOf(value, allowed, fallback) {
  return allowed.indexOf(value) >= 0 ? value : fallback;
}

function normalize(value) {
  value = value || {};
  var timeSize = integer(value.timeSize, DEFAULTS.timeSize);
  timeSize = Math.max(80, Math.min(220, Math.round(timeSize / 5) * 5));
  var goldBrightness = integer(value.goldBrightness, DEFAULTS.goldBrightness);
  goldBrightness = Math.max(70, Math.min(180,
      Math.round(goldBrightness / 5) * 5));
  return {
    timeSize: timeSize,
    timeStyle: oneOf(integer(value.timeStyle, DEFAULTS.timeStyle),
        [0, 1, 2, 3], DEFAULTS.timeStyle),
    timeFormat: oneOf(integer(value.timeFormat, DEFAULTS.timeFormat),
        [0, 1, 2], DEFAULTS.timeFormat),
    timeFont: Math.max(0, Math.min(27,
        integer(value.timeFont, DEFAULTS.timeFont))),
    goldBrightness: goldBrightness,
    dateSize: oneOf(integer(value.dateSize, DEFAULTS.dateSize),
        [14, 18, 24, 28, 42], DEFAULTS.dateSize),
    dateBold: integer(value.dateBold, DEFAULTS.dateBold) ? 1 : 0,
    dateFormat: oneOf(integer(value.dateFormat, DEFAULTS.dateFormat),
        [0, 1, 2, 3, 4, 5, 6], DEFAULTS.dateFormat),
    weatherSize: oneOf(integer(value.weatherSize, DEFAULTS.weatherSize),
        [14, 18, 24], DEFAULTS.weatherSize),
    weatherBackground: oneOf(integer(value.weatherBackground,
        DEFAULTS.weatherBackground), [0, 1, 2, 3, 4],
        DEFAULTS.weatherBackground),
    weatherForeground: oneOf(integer(value.weatherForeground,
        DEFAULTS.weatherForeground), [0, 1, 2, 3],
        DEFAULTS.weatherForeground)
  };
}

function load() {
  try {
    return normalize(JSON.parse(localStorage.getItem(STORAGE_KEY)));
  } catch (error) {
    console.log('Settings cache was unreadable: ' + error.message);
    return normalize(null);
  }
}

function save(settings) {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(normalize(settings)));
}

function dictionary(settings) {
  settings = normalize(settings);
  return {
    SETTINGS_TIME_SCALE: settings.timeSize,
    SETTINGS_TIME_STYLE: settings.timeStyle,
    SETTINGS_TIME_FORMAT: settings.timeFormat,
    SETTINGS_TIME_FONT: settings.timeFont,
    SETTINGS_GOLD_BRIGHTNESS: settings.goldBrightness,
    SETTINGS_DATE_SIZE: settings.dateSize,
    SETTINGS_DATE_BOLD: settings.dateBold,
    SETTINGS_DATE_FORMAT: settings.dateFormat,
    SETTINGS_WEATHER_SIZE: settings.weatherSize,
    SETTINGS_WEATHER_BG: settings.weatherBackground,
    SETTINGS_WEATHER_FG: settings.weatherForeground
  };
}

function send(settings, complete) {
  Pebble.sendAppMessage(dictionary(settings), function() {
    if (complete) {
      complete();
    }
  }, function(error) {
    console.log('Settings delivery failed: ' + JSON.stringify(error));
    if (complete) {
      complete();
    }
  });
}

function selected(value, current) {
  return value === current ? ' selected' : '';
}

var TIME_FONT_GROUPS = [
  ['Gothic', [
    [0, 'Gothic 09'], [1, 'Gothic 14'], [2, 'Gothic 14 Bold'],
    [3, 'Gothic 18'], [4, 'Gothic 18 Bold'], [5, 'Gothic 24'],
    [6, 'Gothic 24 Bold'], [7, 'Gothic 28'], [8, 'Gothic 28 Bold']
  ]],
  ['Bitham', [
    [9, 'Bitham 18 Light'], [10, 'Bitham 30 Black'],
    [11, 'Bitham 34 Light'], [12, 'Bitham 34 Medium Numbers'],
    [13, 'Bitham 42 Light'], [14, 'Bitham 42 Bold'],
    [15, 'Bitham 42 Medium Numbers']
  ]],
  ['Roboto', [
    [16, 'Roboto Condensed 21'], [17, 'Roboto Bold 49']
  ]],
  ['Droid Serif', [
    [18, 'Droid Serif 28 Bold']
  ]],
  ['Leco', [
    [19, 'Leco 20 Bold Numbers'], [20, 'Leco 26 Bold Numbers'],
    [21, 'Leco 28 Light Numbers'], [22, 'Leco 32 Bold Numbers'],
    [23, 'Leco 36 Bold Numbers'], [24, 'Leco 38 Bold Numbers'],
    [25, 'Leco 42 Numbers'], [26, 'Leco 60 Numbers'],
    [27, 'Leco 60 Bold Numbers']
  ]]
];

function buildTimeFontOptions(current) {
  var html = '';
  TIME_FONT_GROUPS.forEach(function(group) {
    html += '<optgroup label="' + group[0] + '">';
    group[1].forEach(function(font) {
      html += '<option value="' + font[0] + '"' + selected(font[0], current) +
          '>' + font[1] + '</option>';
    });
    html += '</optgroup>';
  });
  return html;
}

function buildPage(settings) {
  settings = normalize(settings);
  var html = '<!doctype html><html><head>' +
      '<meta charset="utf-8">' +
      '<meta name="viewport" content="width=device-width,initial-scale=1,' +
      'maximum-scale=1,user-scalable=no">' +
      '<title>Aqua Real Settings</title>' +
      '<style>' +
      '*{box-sizing:border-box}body{margin:0;background:#071725;color:#eefcff;' +
      'font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}' +
      '.hero{padding:24px 18px 18px;background:linear-gradient(160deg,#073c59,' +
      '#071725);border-bottom:1px solid #17677d}.hero h1{margin:0;font-size:25px}' +
      '.hero p{margin:6px 0 0;color:#a7dce8;font-size:14px}' +
      'main{padding:14px}.card{background:#0d2636;border:1px solid #17465b;' +
      'border-radius:13px;margin-bottom:13px;overflow:hidden}.card h2{font-size:14px;' +
      'letter-spacing:.08em;text-transform:uppercase;color:#66d4e8;margin:0;' +
      'padding:13px 14px 8px}.row{padding:11px 14px 14px;border-top:1px solid ' +
      '#15394a}.row:first-of-type{border-top:0}.row.hidden{display:none}label{display:flex;' +
      'justify-content:space-between;align-items:center;font-weight:600;' +
      'margin-bottom:8px}.value{color:#78e4ef;font-variant-numeric:tabular-nums}' +
      'select,input[type=range]{width:100%}select{height:42px;border-radius:9px;' +
      'border:1px solid #2d6072;background:#071b28;color:#fff;padding:0 10px;' +
      'font-size:16px}input[type=range],input[type=checkbox]{accent-color:#36c8dd}' +
      '.checkLabel{justify-content:flex-start;gap:10px;margin:0}.checkLabel input{' +
      'width:22px;height:22px;margin:0}.clockPreview{height:82px;margin:3px 14px 15px;' +
      'border-radius:9px;background:linear-gradient(#032c42,#00121e);display:flex;' +
      'align-items:center;justify-content:center;font-size:34px;font-weight:800;' +
      'letter-spacing:2px;color:#bfffff;text-shadow:2px 3px #00364f;overflow:hidden}' +
      '.clockPreview.smooth{font-weight:300;letter-spacing:1px}' +
      '.clockPreview.stencil{font-family:Impact,"Arial Narrow",sans-serif;' +
      'letter-spacing:4px}.clockPreview.gold{font-family:Georgia,serif;color:#ffc629;' +
      'text-shadow:1px 1px #5b2900,2px 3px #1b0b00}' +
      '.previewWrap{padding:5px 14px 15px}.previewLabel{font-size:12px;' +
      'color:#8eb9c7;margin-bottom:7px}.floatPreview{height:52px;border-radius:8px;' +
      'display:flex;align-items:center;justify-content:center;gap:10px;' +
      'font-weight:700;border-top:2px solid #ffe45c;box-shadow:0 8px 18px #0008}' +
      '.weatherIcon{font-size:24px}.save{display:block;width:100%;height:52px;' +
      'border:0;border-radius:12px;background:#2bc2d6;color:#001820;font-size:18px;' +
      'font-weight:800;margin:17px 0 8px}.note{text-align:center;color:#759dab;' +
      'font-size:12px;margin:0 8px 18px}.note a{color:#78e4ef}</style></head><body>' +
      '<div class="hero"><h1>Aqua Real</h1><p>Display, legibility, and weather sky</p>' +
      '</div><main>' +
      '<section class="card"><h2>Clock</h2><div class="row">' +
      '<label for="timeStyle">Time style</label><select id="timeStyle">' +
      '<option value="3"' + selected(3, settings.timeStyle) + '>Smooth glass</option>' +
      '<option value="0"' + selected(0, settings.timeStyle) + '>Pixel glass — original</option>' +
      '<option value="1"' + selected(1, settings.timeStyle) + '>Nautical stencil</option>' +
      '<option value="2"' + selected(2, settings.timeStyle) + '>Gold doubloons</option>' +
      '</select></div><div class="row" id="timeFontRow">' +
      '<label for="timeFont">Pebble system font</label><select id="timeFont">' +
      buildTimeFontOptions(settings.timeFont) + '</select></div>' +
      '<div class="row" id="goldBrightnessRow">' +
      '<label for="goldBrightness">Gold brightness <span class="value" ' +
      'id="goldBrightnessValue">' + settings.goldBrightness + '%</span></label>' +
      '<input id="goldBrightness" type="range" min="70" max="180" step="5" value="' +
      settings.goldBrightness + '"></div><div class="row">' +
      '<label for="timeSize">Floating time <span class="value" id="timeValue">' +
      settings.timeSize + '%</span></label>' +
      '<input id="timeSize" type="range" min="80" max="220" step="5" value="' +
      settings.timeSize + '"></div><div class="row"><label for="timeFormat">Time format</label>' +
      '<select id="timeFormat"><option value="0"' + selected(0, settings.timeFormat) +
      '>Follow Pebble preference</option><option value="1"' +
      selected(1, settings.timeFormat) + '>12-hour</option><option value="2"' +
      selected(2, settings.timeFormat) + '>24-hour</option></select></div>' +
      '<div class="clockPreview" id="clockPreview">10:08</div></section>' +
      '<section class="card"><h2>Date</h2><div class="row">' +
      '<label for="dateFormat">Date display</label><select id="dateFormat">' +
      '<option value="0"' + selected(0, settings.dateFormat) + '>Full — Tuesday, Sep 1</option>' +
      '<option value="1"' + selected(1, settings.dateFormat) + '>Long — Tuesday, September 1</option>' +
      '<option value="2"' + selected(2, settings.dateFormat) + '>Short — Tue, Sep 1</option>' +
      '<option value="3"' + selected(3, settings.dateFormat) + '>Month and day — September 1</option>' +
      '<option value="4"' + selected(4, settings.dateFormat) + '>Weekday only — Tuesday</option>' +
      '<option value="5"' + selected(5, settings.dateFormat) + '>Numeric — 09/01/2026</option>' +
      '<option value="6"' + selected(6, settings.dateFormat) + '>Hidden</option>' +
      '</select></div><div class="row"><label for="dateSize">Date size</label>' +
      '<select id="dateSize"><option value="14"' + selected(14, settings.dateSize) +
      '>Small</option><option value="18"' + selected(18, settings.dateSize) +
      '>Medium</option><option value="24"' + selected(24, settings.dateSize) +
      '>Large</option><option value="28"' + selected(28, settings.dateSize) +
      '>Extra large</option><option value="42"' + selected(42, settings.dateSize) +
      '>Maximum</option></select></div><div class="row"><label class="checkLabel" ' +
      'for="dateBold"><input id="dateBold" type="checkbox"' +
      (settings.dateBold ? ' checked' : '') + '> Bold date text</label></div></section>' +
      '<section class="card"><h2>Weather float</h2><div class="row">' +
      '<label for="weatherSize">Temperature size</label><select id="weatherSize">' +
      '<option value="14"' + selected(14, settings.weatherSize) + '>Small</option>' +
      '<option value="18"' + selected(18, settings.weatherSize) + '>Medium</option>' +
      '<option value="24"' + selected(24, settings.weatherSize) + '>Large</option>' +
      '</select></div><div class="row"><label for="weatherBackground">Float color</label>' +
      '<select id="weatherBackground"><option value="0"' +
      selected(0, settings.weatherBackground) + '>Deep navy</option>' +
      '<option value="1"' + selected(1, settings.weatherBackground) + '>Black</option>' +
      '<option value="2"' + selected(2, settings.weatherBackground) + '>Deep teal</option>' +
      '<option value="3"' + selected(3, settings.weatherBackground) + '>Pearl white</option>' +
      '<option value="4"' + selected(4, settings.weatherBackground) + '>Burgundy</option>' +
      '</select></div><div class="row"><label for="weatherForeground">Text and icon</label>' +
      '<select id="weatherForeground"><option value="0"' +
      selected(0, settings.weatherForeground) + '>White</option>' +
      '<option value="1"' + selected(1, settings.weatherForeground) + '>Yellow</option>' +
      '<option value="2"' + selected(2, settings.weatherForeground) + '>Black</option>' +
      '<option value="3"' + selected(3, settings.weatherForeground) + '>Ice blue</option>' +
      '</select></div><div class="previewWrap"><div class="previewLabel">Contrast preview</div>' +
      '<div class="floatPreview" id="floatPreview"><span class="weatherIcon">&#9729;</span>' +
      '<span id="previewTemp">73&deg;F</span></div></div></section>' +
      '<section class="card"><h2>Above-water scenery</h2><div class="row" ' +
      'style="color:#b8d8e1;line-height:1.45">Automatically follows local sunrise and ' +
      'sunset through morning, noon, evening, and night. Clear, cloud, fog, rain, ' +
      'snow, and storm conditions each alter the sky and atmospheric details.</div></section>' +
      '<button class="save" id="save">Save to watch</button>' +
      '<p class="note">Settings are stored on both your phone and watch.<br>' +
      'Weather data by <a href="https://open-meteo.com/">Open-Meteo</a>.</p></main>' +
      '<script>(function(){var bg=["#001b44","#000000","#004f55","#f3ffff",' +
      '"#720d32"],fg=["#ffffff","#ffe45c","#000000","#aaffff"];' +
      'function n(id){return parseInt(document.getElementById(id).value,10)}' +
      'function update(){var ts=n("timeSize"),style=n("timeStyle"),font=n("timeFont"),' +
      'gold=n("goldBrightness"),ws=n("weatherSize"),b=n("weatherBackground"),' +
      'f=n("weatherForeground");' +
      'document.getElementById("timeValue").textContent=ts+"%";' +
      'document.getElementById("goldBrightnessValue").textContent=gold+"%";' +
      'document.getElementById("timeFontRow").className="row"+' +
      '(style===3?"":" hidden");document.getElementById("goldBrightnessRow").className=' +
      '"row"+(style===2?"":" hidden");' +
      'var c=document.getElementById("clockPreview");c.className="clockPreview"+' +
      '(style===1?" stencil":style===2?" gold":style===3?" smooth":"");' +
      'c.style.fontSize=Math.min(62,Math.round(34*ts/100))+"px";' +
      'c.style.filter=style===2?"brightness("+(gold/100)+")":"none";' +
      'c.style.fontFamily=font===18?"Georgia,serif":font>=19?' +
      '"Arial Narrow,Arial,sans-serif":font>=16?"Roboto Condensed,Arial,sans-serif":' +
      '"Arial,Helvetica,sans-serif";' +
      'c.style.fontWeight=[2,4,6,8,10,12,14,15,17,18,19,20,22,23,24,27].' +
      'indexOf(font)>=0?"800":font===9||font===11||font===13||font===21?"300":"500";' +
      'var p=document.getElementById("floatPreview");p.style.background=bg[b];' +
      'p.style.color=fg[f];document.getElementById("previewTemp").style.fontSize=' +
      '(ws+4)+"px"}' +
      '["timeSize","timeStyle","timeFont","goldBrightness","weatherSize",' +
      '"weatherBackground","weatherForeground"].' +
      'forEach(function(id){document.getElementById(id).onchange=update;' +
      'document.getElementById(id).oninput=update});update();' +
      'document.getElementById("save").onclick=function(){var options={' +
      'timeSize:n("timeSize"),timeStyle:n("timeStyle"),timeFormat:n("timeFormat"),' +
      'timeFont:n("timeFont"),goldBrightness:n("goldBrightness"),' +
      'dateSize:n("dateSize"),dateBold:document.getElementById("dateBold").checked?1:0,' +
      'dateFormat:n("dateFormat"),weatherSize:n("weatherSize"),' +
      'weatherBackground:n("weatherBackground"),weatherForeground:n("weatherForeground")};' +
      'var result="pebblejs://close#"+encodeURIComponent(JSON.stringify(options));' +
      'document.location=result};})();</script></body></html>';
  return html;
}

function showConfiguration() {
  var page = buildPage(load());
  Pebble.openURL('data:text/html;charset=utf-8,' + encodeURIComponent(page));
}

function handleWebviewClosed(event) {
  if (!event || !event.response) {
    return;
  }
  try {
    var settings = normalize(JSON.parse(decodeURIComponent(event.response)));
    save(settings);
    send(settings);
  } catch (error) {
    console.log('Settings response was invalid: ' + error.message);
  }
}

module.exports = {
  load: load,
  send: send,
  showConfiguration: showConfiguration,
  handleWebviewClosed: handleWebviewClosed
};
