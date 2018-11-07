/* GENERATED FILE BY gen-h UTIL - DO NOT EDIT THIS */
F("<html>\
\
<head>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
</head>\
\
<link href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\"\
crossorigin=\"anonymous\">\
<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.4.1/css/all.css\" integrity=\"sha384-5sAR7xN1Nv6T6+dT2mhtzEpVJvfS3NScPQTrOxhwjIuvcA67KV2R5Jz6kr4abQsz\"\
crossorigin=\"anonymous\">\
\
<body>\
<div class=\"container\">\
<div class=\"row mt-3\">\
<div class=\"col-auto\">\
<h1>Temperature sensors</h1>\
</div>\
<div class=\"col\"><i class=\"fas fa-spin fa-spinner j-spin collapse\"></i></div>\
</div>\
<div class=\"row\">\
<div class=\"col\">\
<div class=\"table-container\">\
<div class=\"table table-striped\">\
<table class=\"table\">\
<thead>\
<tr>\
<th scope=\"col\"><b>Address</b></th>\
<th scope=\"col\"><b>Description</b></th>\
<th scope=\"col\"><b>Value (C)</b></th>\
<th scope=\"col\"><b>Action</b></th>\
</tr>\
</thead>\
<tbody id=\"tbody-temp\"></tbody>\
</table>\
</div>\
</div>\
</div>\
</div>\
<div class=\"row\">\
<div class=\"col-auto\">\
<button class=\"btn btn-primary\" onclick='reloadall()'>reload all</button>\
</div>\
<div class=\"col\">\
<button class=\"btn btn-default\" onclick='reload_enabled=true;'>autoreload</button>\
</div>\
</div>\
<h3 class=\"mt-3\">Api</h3>\
<div class=\"row\">\
<div class=\"col\">\
<code>/temp/address</code> ( read temperature of sensor by given 8 hex address )<br /></div>\
</div>\
<div class=\"row mt-2\">\
<div class=\"col\">\
<code id='freeram'></code><br />\
</div>\
</div>\
</div>\
\
<script src=\"https://code.jquery.com/jquery-3.3.1.min.js\" integrity=\"sha256-FgpCb/KJQlLNfOu91ta32o/NMZxltwRo8QtmkMRdAu8=\"\
crossorigin=\"anonymous\"></script>\
<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js\"></script>\
\
<script src=\"app.js\"></script>\
</body>\
\
</html>\
")
