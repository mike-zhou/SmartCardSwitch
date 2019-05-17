const http = require('http');
const fs = require('fs');

const hostname = '127.0.0.1';
const port = 80;

const scsHostName = "127.0.0.1";
const scsHostPort = 60002;

const iFingerHostName = "127.0.0.1"
const iFingerHostPort = 60004;

function appLog(str) {
    var d = new Date();
    var log = d.getFullYear() + "-" + d.getMonth() + "-" + d.getDay() + " " + d.getHours() + ":" + d.getMinutes() + ":" + d.getSeconds() + "." + d.getMilliseconds();

    log = log + " " + str;
    console.log(log);
}

function onRetrievingFile(fileName, fileType, response) {
    appLog("onRetrievingFile " + fileName);
    fs.stat(fileName, function(error, stats) {
        if (error) {
            appLog("onRetrievingFile Error: " + error.message);
            response.statusCode = 400;
            response.end();
            return;
        }

        if (stats.isFile()) {
            var stream = fs.createReadStream(fileName);
            response.statusCode = 200;
            response.setHeader('Content-Type', fileType);
            stream.pipe(response);
        } else {
            appLog("onRetrievingFile not a file: " + fileName);
            response.statusCode = 400;
            response.statusMessage = "not a file: " + fileName;
            response.end();
        }
    });
}

function onPostRequest(request, response) {
    let body = [];

    request.on('data', (chunk) => {
        body.push(chunk);
    }).on('end', () => {
        body = Buffer.concat(body).toString();
        appLog("onPostRequest " + request.url + " : " + body);

        //forward this request to SmartCardSwitch
        var scsOptions = {};
        scsOptions.hostname = scsHostName;
        scsOptions.port = scsHostPort;
        scsOptions.path = request.url;
        scsOptions.method = 'POST';
        scsOptions.headers = {};
        scsOptions.headers["Content-Type"] = "application/json";
        scsOptions.headers["Content-Length"] = Buffer.byteLength(body);

        var scsRequest = http.request(scsOptions, (scsResponse) => {
            let scsBody = [];

            scsResponse.on('data', (chunk) => {
                scsBody.push(chunk);
            }).on('end', () => {
                scsBody = Buffer.concat(scsBody).toString();
                appLog("onPostRequest SCS reply: " + scsBody);

                response.statusCode = scsResponse.statusCode;
                if ('headers' in response) {
                    if ('content-type' in response.headers) {
                        response.setHeader('Content-Type', scsResponse.headers['content-type']);
                    }
                }
                response.end(scsBody);
            });
        });
        scsRequest.on('error', (e) => {
            var msg = "onPostRequest error in SCS request: " + e;
            appLog(msg);
            //notify browser of error
            response.statusCode = 400;
            response.setHeader('Content-Type', 'text/plain');
            response.end(msg);
        });

        scsRequest.write(body);
        scsRequest.end();
    });
}

function onPostRequest_iFinger(request, response) {
    let body = [];

    request.on('data', (chunk) => {
        body.push(chunk);
    }).on('end', () => {
        body = Buffer.concat(body).toString();
        appLog("onPostRequest_iFinger " + request.url + " : " + body);

        //forward this request to SmartCardSwitch
        var iFingerOptions = {};
        iFingerOptions.hostname = iFingerHostName;
        iFingerOptions.port = iFingerHostPort;
        iFingerOptions.path = request.url;
        iFingerOptions.method = 'POST';
        iFingerOptions.headers = {};
        iFingerOptions.headers["Content-Type"] = "application/json";
        iFingerOptions.headers["Content-Length"] = Buffer.byteLength(body);

        var iFingerRequest = http.request(iFingerOptions, (iFingerResponse) => {
            let replyBody = [];

            iFingerResponse.on('data', (chunk) => {
                replyBody.push(chunk);
            }).on('end', () => {
                replyBody = Buffer.concat(replyBody).toString();
                appLog("onPostRequest_iFinger reply: " + replyBody);

                response.statusCode = iFingerResponse.statusCode;
                if ('headers' in response) {
                    if ('content-type' in response.headers) {
                        response.setHeader('Content-Type', iFingerResponse.headers['content-type']);
                    }
                }
                response.end(replyBody);
            });
        });
        iFingerRequest.on('error', (e) => {
            var msg = "onPostRequest_iFinger error in iFinger request: " + e;
            appLog(msg);
            //notify browser of error
            response.statusCode = 400;
            response.setHeader('Content-Type', 'text/plain');
            response.end(msg);
        });

        iFingerRequest.write(body);
        iFingerRequest.end();
    });
}

function onDefaultPage(request, response) {
    appLog("onDefaultPage ");
    const stream = fs.createReadStream('default.html');

    response.statusCode = 200;
    response.setHeader('Content-Type', 'text/html');
    stream.pipe(response);
}

function onGetCardSlotMappings(request, response) {
    appLog("onGetCardSlotMappings");

    fs.readFile("data/cardSlotMapping.json", function(err, contents) {
        if(err) {
            appLog("onGetCardSlotMappings ERROR: " + err);
            response.statusCode = 400;
            response.setHeader('Content-Type', 'text/json');
            response.end();
        }
        else {
            response.statusCode = 200;
            response.setHeader('Content-Type', 'text/json');
            response.write(contents);
            response.end();
        }
    });
}

function onHttpRequest(request, response) {
    appLog("onHttpRequest: " + request.url);

    var url = request.url;

    if (url === "/stepperMove") {
        onPostRequest(request, response);
    } else if (url === "/stepperConfigMovement") {
        onPostRequest(request, response);
    } else if (url === "/stepperConfigHome") {
        onPostRequest(request, response);
    } else if (url === "/query") {
        onPostRequest(request, response);
    } else if (url === "/bdc") {
        onPostRequest(request, response);
    } else if (url === "/saveCoordinate") {
        onPostRequest(request, response);
    } else if (url === "/toCoordinate") {
        onPostRequest(request, response);
    } else if (url === "/power") {
        onPostRequest(request, response);
    } else if( url === "/toSmartCardOffset") {
        onPostRequest(request, response);
    } else if (url === "/stepperConfigForwardClockwise") {
        onPostRequest(request, response);
    } else if (url === "/key") {
        onPostRequest_iFinger(request, response);
    } else if (url === "/getCardSlotMappings") {
        onGetCardSlotMappings(request, response);
    } else if (url === "/updateCardSlotMappings") {
        onSaveCardSlotMapping(request, response);
    } else if (url === "/") {
        onDefaultPage(request, response);
    } else if (url.indexOf("/subPages/") === 0) {
        onRetrievingFile(url.slice(1), "text/html", response);
    } else if (url.indexOf("/scripts/") === 0) {
        onRetrievingFile(url.slice(1), "application/javascript", response);
    } else if (url.indexOf("/css/") === 0) {
        onRetrievingFile(url.slice(1), "text/css", response);
    } else if (url.indexOf("/videos/") === 0) {
        onRetrievingFile(url.slice(1), "video/mp4", response);
    } else {
        var errorMsg = "onHttpRequest unsupported URL: " + request.url;

        appLog(errorMsg);
        response.statusCode = 400;
        response.setHeader('Content-Type', 'text/plain');
        response.end(errorMsg);
    }
}

const server = http.createServer((req, res) => {
    onHttpRequest(req, res);
});

server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
});
