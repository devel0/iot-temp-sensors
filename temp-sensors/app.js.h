F("\
var sensorDesc = [{\
id: \"28af8123070000e2\",\
description: \"test\"\
}];\
\
var debug = true;\
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
reloadall();\
}\
\
function reloadall() {\
if (!reload_enabled) return;\
$('.tempdev').each(function (idx) {\
let v = this.innerText;\
console.log('addr=[' + v + ']');\
reloadTemp(v);\
});\
}\
\
async function myfn() {\
\
const res = await $.ajax({\
url: baseurl + '/tempdevices',\
type: 'GET'\
});\
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
const restemp = await $.ajax({\
url: baseurl + '/temp/' + tempId,\
type: 'GET'\
});\
h += \"<td><span id='t\" + tempId + \"'>\";\
h += restemp;\
h += \"</span></td>\";\
\
h += \"<td><button class='btn btn-primary' onclick='reloadTemp(\\\"\" + res.tempdevices[i] + \"\\\")'>reload</button></td>\";\
\
h += \"</tr>\";\
}\
\
$('#tbody-temp')[0].innerHTML = h;\
}\
\
myfn();\
")
