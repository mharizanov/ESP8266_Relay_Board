curl 'http://heating.pearson.uk/control/thermostat.cgi?param=thermostat_schedule' \
	-X 'POST' \
	-H 'Accept: text/plain, */*; q=0.01' \
	-H 'Origin: http://heating.pearson.uk' \
	-H 'Content-Type: application/x-www-form-urlencoded; charset=UTF-8' \
	-H 'Referer: http://heating.pearson.uk/control/thermostat.html' \
	-H 'Content-Length: 201' \
	-H 'Host: heating.pearson.uk' \
	-H 'Accept-Language: en-gb' \
	-H 'User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/15.6.1 Safari/605.1.15' \
	-H 'Accept-Encoding: gzip, deflate' \
	-H 'Connection: keep-alive' \
	-H 'X-Requested-With: XMLHttpRequest' \
	--data '{"sun":[{"s":0,"e":100,"sp":100},{"s":100,"e":300,"sp":205},{"s":300,"e":645,"sp":150},{"s":645,"e":945,"sp":205},{"s":945,"e":1730,"sp":100},{"s":1730,"e":2130,"sp":205},{"s":2130,"e":2400,"sp":100}]}'



#Don't forget to change --length parameter to curl above
