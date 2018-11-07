F("\
var sensorDesc = [{\
id: \"28af8123070000e2\",\
description: \"test\"\
}];\
\
var debug = false;\
\
\
var baseurl = '';\
if (debug) baseurl = 'http://10.10.4.111';\
\
function reloadTemp(addr) {\
$('.j-spin').removeClass('collapse');\
$.get(baseurl + '/temp/' + addr, function (data) {\
$('.j-spin').addClass('collapse');\
$('#t' + addr)[0].innerText = data;\
});\
}\
var reload_enabled = false;\
setInterval(autoreload, 3000);\
\
function autoreload() {\
if (!reload_enabled) return;\
reloadall();\
}\
\
async function reloadall() {\
$('.tempdev').each(function (idx) {\
let v = this.innerText;\
console.log('addr=[' + v + ']');\
reloadTemp(v);\
});\
\
const res = await $.ajax({\
url: baseurl + \"/temphistory\",\
type: 'GET'\
});\
\
var colors = ['orange', 'yellow', 'green', 'blue', 'violet', 'black', 'red']\
var ctx = document.getElementById(\"myChart\").getContext('2d');\
\
var i = 0;\
var dss = [];\
$.each(res, function (idx, data) {\
id = Object.keys(data)[0];\
desc = id;\
q = $.grep(sensorDesc, (el, idx) => el.id == id);\
if (q.length > 0) desc = q[0].description;\
\
if (i > colors.length - 1) color = 'brown';\
else color = colors[i];\
dss.push({\
borderColor: color,\
label: desc,\
data: [{\
x: 0,\
y: 1\
}, {\
x: 1,\
y: 2\
\
}]\
});\
\
++i;\
});\
\
var myChart = new Chart(ctx, {\
type: 'line',\
data: {\
datasets: dss\
}\
});\
}\
\
async function myfn() {\
\
$('.j-spin').removeClass('collapse');\
const res = await $.ajax({\
url: baseurl + '/tempdevices',\
type: 'GET'\
});\
$('.j-spin').addClass('collapse');\
\
var h = \"\";\
\
for (i = 0; i < res.tempdevices.length; ++i) {\
let tempId = res.tempdevices[i];\
\
h += \"<tr>\";\
\
h += \"<td><span class='tempdev'>\";\
h += tempId;\
h += \"</span></td>\";\
\
h += \"<td>\";\
q = $.grep(sensorDesc, (el, idx) => el.id == tempId);\
if (q.length > 0) h += q[0].description;\
h += \"</td>\";\
\
h += \"<td><span id='t\" + tempId + \"'>\";\
h += \"</span></td>\";\
\
h += \"<td><button class='btn btn-primary' onclick='reloadTemp(\\\"\" + res.tempdevices[i] + \"\\\")'>reload</button></td>\";\
\
h += \"</tr>\";\
}\
\
$('#tbody-temp')[0].innerHTML = h;\
\
const res2 = await $.ajax({\
url: baseurl + '/info',\
type: 'GET'\
});\
$('#info')[0].innerHTML = JSON.stringify(res2, null, 2);\
\
reloadall();\
}\
\
myfn();\
")
