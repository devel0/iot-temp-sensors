F(" \
var sensorDesc = [{ \
id: \"28b5742407000084\", \
description: \"external\" \
}, \
{ \
id: \"2833bf3a050000ec\", \
description: \"bedroom\" \
}, \
{ \
id: \"28cc5d3a050000e3\", \
description: \"bathroom\" \
}, \
{ \
id: \"288aef140500008d\", \
description: \"lab\" \
} \
]; \
 \
var debug = false; \
 \
var infoUpdateIntervalMs = 5000; \
var tempUpdateIntervalMs = 10000; \
 \
 \
requirejs.config({ \
\"moment\": \"://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.6/require.min.js\" \
}); \
 \
function showSpin() { \
$('.j-spin').removeClass(\"collapse\"); \
} \
 \
function hideSpin() { \
$('.j-spin').addClass(\"collapse\"); \
} \
 \
var history_interval_sec = 10; \
 \
var baseurl = ''; \
if (debug) baseurl = 'http://10.10.2.5'; \
 \
async function reloadInfo() { \
showSpin(); \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/info', \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
hideSpin(); \
 \
$('.freeram')[0].innerText = res.freeram; \
$('.freeramMin')[0].innerText = res.freeram; \
$('.temperatureHistoryIntervalMin')[0].innerText = (res.history_interval_sec / 60.0).toFixed(1); \
$('.temperatureBacklogHours')[0].innerText = res.history_backlog_hours; \
} \
async function reloadTemp(addr) { \
showSpin(); \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/temp/' + addr, \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
 \
} \
} \
hideSpin(); \
$('#t' + addr)[0].innerText = res; \
} \
 \
var infoLastLoad; \
var tempLastLoad; \
var chartLastLoad; \
var autorefreshInProgress = false; \
 \
async function autorefresh() { \
if (autorefreshInProgress) return; \
 \
autorefreshInProgress = true; \
var dtnow = new Date(); \
if (infoLastLoad === undefined || (dtnow - infoLastLoad) > infoUpdateIntervalMs) { \
await reloadInfo(); \
infoLastLoad = new Date(); \
} \
if (tempLastLoad === undefined || (dtnow - tempLastLoad) > tempUpdateIntervalMs) { \
await reloadAllTemp(); \
tempLastLoad = new Date(); \
} \
if (chartLastLoad === undefined || (dtnow - chartLastLoad) > history_interval_sec * 1000) { \
await reloadCharts(); \
chartLastLoad = new Date(); \
} \
autorefreshInProgress = false; \
} \
 \
function sleep(ms) { \
return new Promise(resolve => setTimeout(resolve, ms)); \
} \
 \
async function reloadAllTemp() { \
$('.tempdev').each(async function (idx) { \
let v = this.innerText; \
await reloadTemp(v); \
}); \
} \
 \
async function reloadCharts() { \
let finished = false; \
 \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + \"/temphistory\", \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
 \
} \
} \
 \
var colors = ['orange', 'yellow', 'green', 'blue', 'violet', 'black', 'red']; \
var ctx = document.getElementById(\"myChart\").getContext('2d'); \
 \
var dtnow = moment(); \
console.log('moment now = ' + dtnow.format()); \
 \
var i = 0; \
var dss = []; \
$.each(res, function (idx, data) { \
id = Object.keys(data)[0]; \
desc = id; \
q = $.grep(sensorDesc, (el, idx) => el.id == id); \
if (q.length > 0) desc = q[0].description; \
 \
if (i > colors.length - 1) color = 'brown'; \
else color = colors[i]; \
 \
valcnt = data[id].length; \
 \
dts = []; \
$.each(data[id], function (idx, val) { \
secbefore = (valcnt - idx - 1) * history_interval_sec; \
tt = moment(dtnow).subtract(secbefore, 'seconds'); \
dts.push({ \
t: tt, \
y: val \
}); \
}); \
 \
dss.push({ \
borderColor: color, \
label: desc, \
data: dts, \
pointRadius: 0 \
}); \
 \
++i; \
}); \
 \
var myChart = new Chart(ctx, { \
type: 'line', \
data: { \
datasets: dss \
}, \
options: { \
animation: false, \
scales: { \
xAxes: [{ \
type: 'time', \
time: { \
displayFormats: { \
'hour': 'HH:mm' \
} \
}, \
position: 'bottom' \
}] \
} \
} \
}); \
} \
 \
async function myfn() { \
setInterval(autorefresh, 1000); \
 \
showSpin(); \
const res = await $.ajax({ \
url: baseurl + '/tempdevices', \
type: 'GET' \
}); \
const resnfo = await $.ajax({ \
url: baseurl + '/info', \
type: 'GET' \
}); \
history_interval_sec = resnfo.history_interval_sec; \
console.log('history_interval_sec = ' + history_interval_sec); \
hideSpin(); \
 \
var h = \"\"; \
 \
for (i = 0; i < res.tempdevices.length; ++i) { \
let tempId = res.tempdevices[i]; \
 \
h += \"<tr>\"; \
 \
h += \"<td><span class='tempdev'>\"; \
h += tempId; \
h += \"</span></td>\"; \
 \
h += \"<td>\"; \
q = $.grep(sensorDesc, (el, idx) => el.id == tempId); \
if (q.length > 0) h += q[0].description; \
h += \"</td>\"; \
 \
h += \"<td><span id='t\" + tempId + \"'>\"; \
h += \"</span></td>\"; \
 \
h += \"</tr>\"; \
} \
 \
$('#tbody-temp')[0].innerHTML = h; \
} \
 \
$(document).ready(function () { \
myfn(); \
}); \
")
