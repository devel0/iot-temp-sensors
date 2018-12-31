/* GENERATED FILE BY gen-h UTIL - DO NOT EDIT THIS */
F("<html> \
 \
<head> \
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \
</head> \
 \
<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.1.3/css/bootstrap.min.css\" \
integrity=\"sha256-eSi1q2PG6J7g7ib17yAaWMcrr5GrtohYChqibrV7PBE=\" crossorigin=\"anonymous\" /> \
<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.5.0/css/all.css\" integrity=\"sha384-B4dIYHKNBt8Bc12p+WXckhzcICo0wtJAoU8YZTY5qE0Id1GSseTk6S+L3BlXeVIU\" \
crossorigin=\"anonymous\"> \
 \
<body> \
<div class=\"container-fluid\"> \
<div class=\"row mt-3\"> \
<div class=\"col-auto\"> \
<h1>Temperature sensors</h1> \
</div> \
<div class=\"col\"> \
<button class=\"btn btn-link\" onclick=\"window.open('https://github.com/devel0/iot-temp-sensors')\">About</button> \
<i class=\"fas fa-spin fa-spinner j-spin collapse\"></i> \
</div> \
</div> \
<div class=\"row\"> \
<div class=\"col col-sm-12 col-lg-7\"> \
<h2>Chart</h2> \
<canvas id=\"myChart\" height=\"80\"></canvas> \
</div> \
<div class=\"col col-sm-12 col-lg-5\"> \
<h2>Tabular</h2> \
<div class=\"table-container\"> \
<div class=\"table table-striped\"> \
<table class=\"table\"> \
<thead> \
<tr> \
<th scope=\"col\"><b>Address</b></th> \
<th scope=\"col\"><b>Description</b></th> \
<th scope=\"col\"><b>Value (C)</b></th> \
</tr> \
</thead> \
<tbody id=\"tbody-temp\"></tbody> \
</table> \
</div> \
</div> \
</div> \
<div class=\"col col-sm-12 col-lg-5\"> \
<h2>Stats</h2> \
<div class=\"table-container\"> \
<div class=\"table table-striped\"> \
<table class=\"table\"> \
<thead> \
<tr> \
<th scope=\"col\"><b>Item</b></th> \
<th scope=\"col\"><b>Value</b></th> \
</tr> \
</thead> \
<tbody> \
<tr> \
<td>freeram current (bytes)</td> \
<td><span class=\"freeram\"></span></td> \
</tr> \
<tr> \
<td>freeram min (bytes)</td> \
<td><span class=\"freeramMin\"></span></td> \
</tr> \
<tr> \
<td>temperature chart sample freq (min)</td> \
<td><span class=\"temperatureHistoryIntervalMin\"></span></td> \
</tr> \
<tr> \
<td>temperature backlog (hours)</td> \
<td><span class=\"temperatureBacklogHours\"></span></td> \
</tr> \
</tbody> \
</table> \
</div> \
</div> \
</div> \
</div> \
</div> \
 \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js\" integrity=\"sha256-FgpCb/KJQlLNfOu91ta32o/NMZxltwRo8QtmkMRdAu8=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.1.3/js/bootstrap.min.js\" integrity=\"sha256-VsEqElsCHSGmnmHXGQzvoWjWwoznFSZc6hs7ARLRacQ=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment.min.js\" integrity=\"sha256-CutOzxCRucUsn6C6TcEYsauvvYilEniTXldPa6/wu0k=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.3/Chart.min.js\" integrity=\"sha256-oSgtFCCmHWRPQ/JmR4OoZ3Xke1Pw4v50uh6pLcu+fIc=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.6/require.min.js\" integrity=\"sha256-1fEPhSsRKlFKGfK3eO710tEweHh1fwokU5wFGDHO+vg=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"app.js\"></script> \
</body> \
 \
</html>")
