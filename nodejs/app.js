const http = require('http');
const fs = require('fs');
var net = require('net');

const hostname = '0.0.0.0';
const port = 80;

const scsHostName = "127.0.0.1";
const scsUserProxyPort = 60001;
const scsHostPort = 60002;

const iFingerHostName = "127.0.0.1"
const iFingerHostPort = 60004;

const _cardSlotMappingFile = "data/cardSlotMapping.json";
var _isAccessingCard = false;
var _isPressingkey = false;
var _commandIdNumber = 0;

function newCommandId()
{
    var cmdId;

    _commandIdNumber = _commandIdNumber + 1;
    cmdId = "unique command id " + _commandIdNumber;

    return cmdId;
}

function getCommandId() 
{
    return "unique command id " + _commandIdNumber;
}

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

function onPostRequest_SCS(request, response) {
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

    fs.readFile(_cardSlotMappingFile, function(err, contents) {
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

function onSaveCardSlotMapping(request, response)
{
    appLog("onSaveCardSlotMapping");
    let mappings = [];

    request.on('data', (chunk) => {
        mappings.push(chunk);
    }).on('end', () => {
        mappings = Buffer.concat(mappings).toString();
        appLog("onSaveCardSlotMapping " + request.url + " : " + mappings);

        fs.writeFile(_cardSlotMappingFile, mappings, function(err) {
            if(err) {
                appLog("onSaveCardSlotMapping ERROR: " + err);
                response.statusCode = 400;
                response.setHeader('Content-Type', 'text/plain');
                response.write("failed to save mapping, error: " + err);
                response.end();
            }
            else {
                appLog("onSaveCardSlotMapping mapping is saved");
                response.statusCode = 200;
                response.setHeader('Content-Type', 'text/plain');
                response.write("mapping is saved successfully");
                response.end();
            }
        });
    });
}

function packageCommand(command)
{
    var cmdLength = command.length;
    var pkg = new Uint8Array(cmdLength + 8);

    //TLV structure
    //tag
    pkg[0] = 0xAA;
    pkg[1] = 0xBB;

    //length
    var contentLength = cmdLength + 4;
    pkg[2] = Math.floor(contentLength / 256);
    pkg[3] = contentLength - pkg[2]*256;

    //version
    pkg[4] = 0;
    pkg[5] = 0;

    //command
    for(var i=0; i<cmdLength; i++) {
        pkg[6 + i] = command.charAt(i);
    }

    //tail

    pkg[6 + cmdLength] = 0xCC;
    pkg[6 +  cmdLength + 1] = 0xDD;

    return pkg;
}

function retrieveReply(pkg) 
{
    var reply = "";

    if(pkg.length > 8) 
    {
        //check tag
        if((pkg[0] == 0xAA) &&
            (pkg[1] == 0xBB) &&
            (pkg[pkg.length - 2] == 0xCC) &&
            (pkg[pkg.length -1 ] == 0xDD))
        {
            var length = pkg[2] * 256 + pkg[3];
            if(length === (pkg.length - 4)) 
            {
                for(var i=0; i<(pkg.length -8); i++)
                {
                    reply = reply + String.fromCharCode(pkg[6+i]);
                }
            }
        }
    }

    if(reply.length < 1) {
        reply = "{}";
    }
    return reply;
}

function sendSCSCommand(command, response)
{
    var client = new net.Socket();
    let replySegments = [];

    client.on('connect', () => {
        appLog("sendSCSCommand connected, send command to SCS: " + command);
        var cmdPkg = packageCommand(command);
        client.end(Buffer.from(cmdPkg.buffer));
    }).on('data', (content) => {
        replySegments.push(content);
    }).on('end', () => {
        var replyPkg = Buffer.concat(replySegments); //reply changes to UInt8Array
        appLog("sendSCSCommand reply received " + replyPkg.length + " bytes");
        var cmdReply = retrieveReply(replyPkg);
        appLog("sendSCSCommand reply: " + cmdReply);
        var replyObj = JSON.parse(cmdReply);
        if(replyObj["commandId"] === "invalid") 
        {
            response.setHeader('Content-Type', 'text/plain');
            response.statusCode = 400;
            response.write(replyObj["errorInfo"]);
            response.end();
        }
        else if(replyObj["commandId"] === getCommandId()) 
        {
            response.setHeader('Content-Type', 'text/plain');

            if(replyObj["result"] === "failed") {
                response.statusCode = 400;
                response.write(replyObj["errorInfo"]);
            }
            else if(replyObj["result"] === "succeeded") {
                response.statusCode = 200;
            }
            else {
                response.statusCode = 400;
                response.write("internal error: unknown SCS reply");
            }

            response.end();
        }
        else 
        {
            appLog("sendSCSCommand reply not match");
            response.statusCode = 400;
            response.setHeader('Content-Type', 'text/plain');
            response.write("SCS reply doesn't match command");
            response.end();
        }
    }).on('error', (err) => {
        appLog("sendSCSCommand Error: " + err);
        response.statusCode = 400;
        response.setHeader('Content-Type', 'text/plain');
        response.write("Error: " + err);
        response.end();
    }).on('close', (hadError) => {
        appLog("sendSCSCommand socket closed with error: " + hadError);
        _isAccessingCard = false;
    });

    client.connect(scsUserProxyPort, scsHostName);
}

function onCardAccess(request, response)
{
    appLog("onCardAccess");

    if(_isAccessingCard == true) {
        response.statusCode = 400;
        response.setHeader('Content-Type', 'text/plain');
        response.write("a card is being accessed");
        response.end();
        return;
    }
    else {
        _isAccessingCard = true;
    }

    let command = [];

    request.on('data', (chunk) => {
        command.push(chunk);
    }).on('end', () => {
        command = Buffer.concat(command).toString(); //command changes to a string object.
        appLog("onCardAccess " + request.url + " : " + command);

        fs.readFile(_cardSlotMappingFile, function(err, contents) {
            if(err) 
            {
                appLog("onCardAccess failed to read mapping file ERROR: " + err);
                response.statusCode = 400;
                response.setHeader('Content-Type', 'text/plain');
                response.write("failed to read mapping file");
                response.end();
                _isAccessingCard = false;
            }
            else 
            {
                var mappings=JSON.parse(contents);
                var cmd = JSON.parse(command);
                var slotNumber;
                
                //find card's slot number in the active mapping
                for(var i=0; i<mappings.length; i++)
                {
                    if(mappings[i].active == true) 
                    {
                        for(var j=0; j<mappings[i].mapping.length; j++) {
                            if(mappings[i].mapping[j].cardName === cmd.name) {
                                slotNumber = mappings[i].mapping[j].slotNumber;
                                break;
                            }
                        }
                        break;
                    }
                }

                if(isNaN(slotNumber)) {
                    appLog("onCardAccess cannot find card name in active mapping: " + cmd.name);
                    response.statusCode = 400;
                    response.setHeader('Content-Type', 'text/plain');
                    response.write("failed to read mapping file");
                    response.end();
                    _isAccessingCard = false;
                }
                else {
                    if(cmd.command === "insert") {
                        var scsCommand = {};
                        
                        scsCommand["userCommand"] = "insert smart card";
                        scsCommand["commandId"] = newCommandId();
                        scsCommand["smartCardNumber"] = slotNumber;

                        sendSCSCommand(JSON.stringify(scsCommand), response);
                    }
                    else if(cmd.command === "extract") {

                    }
                    else if(cmd.command === "swipe") {

                    }
                    else if(cmd.command === "tapContactless") {

                    }
                    else if(cmd.command === "tapBarcode") {

                    }
                    else {
                        appLog("onCardAccess unsupported command: " + command);
                        response.statusCode = 400;
                        response.setHeader('Content-Type', 'text/plain');
                        response.write("onCardAccess unsupported command: " + command);
                        response.end();
                        _isAccessingCard = false;
                        return;
                    }
                }
            }
        });
    });

}

function onHttpRequest(request, response) 
{
    appLog("onHttpRequest: " + request.url);

    var url = request.url;

    if (url === "/stepperMove") {
        onPostRequest_SCS(request, response);
    } else if (url === "/stepperConfigMovement") {
        onPostRequest_SCS(request, response);
    } else if (url === "/stepperConfigHome") {
        onPostRequest_SCS(request, response);
    } else if (url === "/query") {
        onPostRequest_SCS(request, response);
    } else if (url === "/bdc") {
        onPostRequest_SCS(request, response);
    } else if (url === "/saveCoordinate") {
        onPostRequest_SCS(request, response);
    } else if (url === "/toCoordinate") {
        onPostRequest_SCS(request, response);
    } else if (url === "/power") {
        onPostRequest_SCS(request, response);
    } else if( url === "/toSmartCardOffset") {
        onPostRequest_SCS(request, response);
    } else if (url === "/stepperConfigForwardClockwise") {
        onPostRequest_SCS(request, response);
    } else if (url === "/key") {
        onPostRequest_iFinger(request, response);
    } else if (url === "/getCardSlotMappings") {
        onGetCardSlotMappings(request, response);
    } else if (url === "/saveCardSlotMappings") {
        onSaveCardSlotMapping(request, response);
    } else if (url === "/cardAccess") {
        onCardAccess(request, response);
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
