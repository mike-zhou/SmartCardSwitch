const http = require('http');
const fs = require('fs');
var net = require('net');

//IP address and port number clients can connect to.
const hostname = '0.0.0.0';
const port = 80;

//IP address and port number SmartCardSwitch listens to.
const scsHostName = "127.0.0.1";
const scsUserProxyPort = 60001;
const scsHostPort = 60002;

//IP address and port number iFinger listens to.
const iFingerHostName = "127.0.0.1";
const iFingerHostPort = 60004;

//log relevant setting
const logFolder = "/home/user1/Temp/logs/nodejs";
const logFileName = "log";
const logFileAmount = 10;
const logFileSize = 5000000; // 5M bytes
var logBuffer = "";
var logMaintancePeriod = 60000; //60000 milliseconds

//frames root folder
const framesRootFolder = "data/frames/";

const _cardSlotMappingFile = "data/cardSlotMapping.json";
const _touchScreenMappingFile = "data/touchScreenMapping.json";

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

function appLog(str) 
{
    var d = new Date();
    var logLine = d.getFullYear() + "-" + (d.getMonth() + 1) + "-" + d.getDate() + " " + d.getHours() + ":" + d.getMinutes() + ":" + d.getSeconds() + "." + d.getMilliseconds();

    logLine = logLine + " " + str;
    console.log(logLine);
    logBuffer = logBuffer + logLine + "\r\n";
}

function maintainLog()
{
    function persistLog(fileName) 
    {
        fs.appendFile(logFolder + "/" + fileName, logBuffer, function(error) {
            if(error) {
                console.log("ERROR: persistLog error: " + error);
            }
        });
        logBuffer = ""; //clear logBuffer
    }

    fs.stat(logFolder, function(error, stats) {
        if(error) {
            console.log("Error in retrieveing folder information: " + logFolder);
            fs.mkdir(logFolder, function(err) {
                if(err) {
                    console.log("ERROR: maintainLog failed to create folder: " + err);
                    logBuffer = "";
                }
                else {
                    //save log to the first log file.
                    console.log("maintainLog created log folder: " + logFolder);
                    persistLog(logFileName + "_0");
                }
            });
        }
        else 
        {
            fs.readdir(logFolder, function(err, files) {
                if(err) {
                    console.log("ERROR: maintainLog failed to read files in: " + logFolder);
                    logBuffer = "";
                }
                else 
                {
                    if(files.length < 1) {
                        persistLog(logFileName + "_0");
                    }
                    else
                    {
                        let min_index;
                        let max_index;

                        //find the minimum and maximum index
                        let nameElements = files[0].split('_');
                        min_index = parseInt(nameElements[1]);
                        max_index = min_index;
                        for(let i=0; i<files.length; i++) {
                            nameElements = files[i].split('_');
                            let index = parseInt(nameElements[1]);
                            if(min_index > index) {
                                min_index = index;
                            }
                            if(max_index < index) {
                                max_index = index;
                            }
                        }

                        if(files.length > logFileAmount) {
                            //log files need to be deleted
                            for(let i=0; i<(files.length - logFileAmount); i++) {
                                let index = min_index + i;
                                let fileName = logFolder + "/" + logFileName + "_" + index;
                                fs.unlink(fileName, function(err) {
                                    if(err) {
                                        console.log("ERROR: maintainLog failed to delete: " + fileName);
                                    }
                                })
                            }
                        }

                        {
                            let filePathName = logFolder + "/" + logFileName + "_" + max_index;
                            //get file size
                            fs.stat(filePathName, function(err, stats) {
                                if(err) {
                                    console.log("ERROR: failed to get file size of " + filePathName);
                                }
                                else {
                                    if(stats["size"] < logFileSize) {
                                        persistLog(logFileName + "_" + max_index); //save log to current file
                                    }
                                    else {
                                        persistLog(logFileName + "_" + (max_index + 1)); //save log to new file
                                    }
                                }
                            });
                        }
                    }
                }
            });  
        }
    });
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

    if(_isPressingkey == true) 
    {
        appLog("onPostRequest_iFinger key is being pressed");
        response.statusCode = 400;
        response.setHeader('Content-Type', 'text/plain');
        response.end("key is being pressed");
        return;
    }
    else {
        _isPressingkey = true;
    }

    request.on('data', (chunk) => {
        body.push(chunk);
    }).on('end', () => {
        body = Buffer.concat(body).toString();
        appLog("onPostRequest_iFinger " + request.url + " : " + body);

        //forward this request to iFinger
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
                appLog("onPostRequest_iFinger reply: " + JSON.stringify(replyBody));

                response.statusCode = iFingerResponse.statusCode;
                if ('headers' in response) {
                    if ('content-type' in response.headers) {
                        response.setHeader('Content-Type', iFingerResponse.headers['content-type']);
                    }
                }
                response.end(replyBody);
                _isPressingkey = false;
            });
        });
        iFingerRequest.on('error', (e) => {
            var msg = "onPostRequest_iFinger error in iFinger request: " + e;
            appLog(msg);
            //notify browser of error
            response.statusCode = 400;
            response.setHeader('Content-Type', 'text/plain');
            response.end(msg);
            _isPressingkey = false;
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
    for(var i=0; i<cmdLength; i++) 
    {
        var v = command.charCodeAt(i);
        pkg[6 + i] = v;
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
        client.write(Buffer.from(cmdPkg.buffer));
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

    if(_isAccessingCard == true) 
    {
        appLog("onCardAccess card is being accessed");
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
                
                if(cmd.command === "return card") 
                {
                    var scsCommand = {};
                    
                    scsCommand["userCommand"] = "return smart card";
                    scsCommand["commandId"] = newCommandId();
                    
                    sendSCSCommand(JSON.stringify(scsCommand), response);
                }
                else 
                {
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
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "remove smart card";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;

                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "swipe") {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "swipe smart card";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            scsCommand["downPeriod"] = cmd.downPeriod;

                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "tapContactless") {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "tap smart card";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            scsCommand["downPeriod"] = cmd.downPeriod;

                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "tapBarcode") {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "tap bar code";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            scsCommand["downPeriod"] = cmd.downPeriod;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from bay to smartCardGate")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from bay to smartCardGate";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from smartCardGate to smartCardReaderGate")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from smartCardGate to smartCardReaderGate";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from smartCardReaderGate to smartCardReader")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from smartCardReaderGate to smartCardReader";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from smartCardReader to smartCardReaderGate")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from smartCardReader to smartCardReaderGate";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from smartCardReaderGate to smartCardGate")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from smartCardReaderGate to smartCardGate";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from smartCardGate to barcodeReaderGate")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from smartCardGate to barcodeReaderGate";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from barcodeReaderGate to barcodeReader")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from barcodeReaderGate to barcodeReader";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from barcodeReader to barcodeReaderGate")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from barcodeReader to barcodeReaderGate";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from barcodeReaderGate to smartCardGate")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from barcodeReaderGate to smartCardGate";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
                        }
                        else if(cmd.command === "move card from smartCardGate to bay")
                        {
                            var scsCommand = {};
                            
                            scsCommand["userCommand"] = "move card from smartCardGate to bay";
                            scsCommand["commandId"] = newCommandId();
                            scsCommand["smartCardNumber"] = slotNumber;
                            
                            sendSCSCommand(JSON.stringify(scsCommand), response);
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
            }
        });
    });
}

function onGetTouchScreenMappings(request, response)
{
    appLog("onGetTouchScreenMappings");

    fs.readFile(_touchScreenMappingFile, function(err, contents) {
        if(err) {
            appLog("onGetTouchScreenMapping ERROR: " + err);
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

function onSaveTouchScreenMappings(request, response)
{
    appLog("onSaveTouchScreenMappings");
    let mappings = [];

    request.on('data', (chunk) => {
        mappings.push(chunk);
    }).on('end', () => {
        mappings = Buffer.concat(mappings).toString();
        appLog("onSaveTouchScreenMappings " + request.url + " : " + mappings);

        fs.writeFile(_touchScreenMappingFile, mappings, function(err) {
            if(err) {
                appLog("onSaveTouchScreenMappings ERROR: " + err);
                response.statusCode = 400;
                response.setHeader('Content-Type', 'text/plain');
                response.write("failed to save mapping, error: " + err);
                response.end();
            }
            else {
                appLog("onSaveTouchScreenMappings mapping is saved");
                response.statusCode = 200;
                response.setHeader('Content-Type', 'text/plain');
                response.write("mapping is saved successfully");
                response.end();
            }
        });
    });
}

function onTouchScreen(request, response)
{
    appLog("onTouchScreen");
    let command = [];
    
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

    request.on('data', (chunk) => {
        command.push(chunk);
    }).on('end', () => {
        command = Buffer.concat(command).toString(); //command changes to a string object.
        appLog("onTouchScreen " + request.url + " : " + command);

        fs.readFile(_touchScreenMappingFile, function(err, contents) {
            if(err) 
            {
                appLog("onTouchScreen failed to read mapping file ERROR: " + err);
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
                var scsCmd = {};
                var errorInfo = "";

                scsCmd["userCommand"] = "touch screen";
                scsCmd["commandId"] = newCommandId();
                scsCmd["downPeriod"] = 4000;
                scsCmd["upPeriod"] = 4000;
                scsCmd["keys"] = [];
                //find touch screen area in the active mapping
                for(var i=0; i<mappings.length; i++)
                {
                    //find the active mapping.
                    if(mappings[i].active == true) 
                    {
                        var activeMapping = mappings[i].mapping;

                        //iterate elements in cmd
                        for(var j=0; j<cmd.length; j++)
                        {
                            var areaName = cmd[j].areaName;
                            var found = false;

                            scsCmd["keys"][j] = {};
                            scsCmd["keys"][j].index = cmd[j].order;

                            //find the key index for the area name
                            for(var k=0; k<activeMapping.length; k++)
                            {
                                if(areaName === activeMapping[k].areaName) 
                                {
                                    found = true;
                                    scsCmd["keys"][j].keyNumber = activeMapping[k].index;
                                    break;
                                }
                            }

                            if(found == false) {
                                errorInfo = "Error: no key is defined for " + areaName;
                                break;
                            }
                        }
                        break;
                    }
                }

                if(errorInfo.length > 0) {
                    appLog("onTouchScreen " + errorInfo);
                    response.statusCode = 400;
                    response.setHeader('Content-Type', 'text/plain');
                    response.write(errorInfo);
                    response.end();
                    _isAccessingCard = false;
                }
                else {
                    sendSCSCommand(JSON.stringify(scsCmd), response);
                }
            }
        });
    });
}

function onAdjustStepperW(request, response)
{
    appLog("onAdjustStepperW");
    let command = [];
    
    request.on('data', (chunk) => {
        command.push(chunk);
    }).on('end', () => {
        command = Buffer.concat(command).toString();
        appLog("onAdjustStepperW " + request.url + " : " + command);

        var cmd = JSON.parse(command);

        //direct the command to SCS
        if(cmd.command === "pullUpCard") 
        {
            var scsCommand = {};
            
            scsCommand["userCommand"] = "move card from bay to smartCardGate";
            scsCommand["commandId"] = newCommandId();
            scsCommand["smartCardNumber"] = cmd.index;

            sendSCSCommand(JSON.stringify(scsCommand), response);
        }
        else if(cmd.command === "setOffset") 
        {
            var scsCommand = {};
            
            scsCommand["userCommand"] = "adjust stepper w";
            scsCommand["commandId"] = newCommandId();
            scsCommand["adjustment"] = cmd.offset;

            sendSCSCommand(JSON.stringify(scsCommand), response);
        }
        else if(cmd.command === "putBackCard") 
        {
            var scsCommand = {};
            
            scsCommand["userCommand"] = "move card from smartCardGate to bay";
            scsCommand["commandId"] = newCommandId();
            scsCommand["smartCardNumber"] = cmd.index;

            sendSCSCommand(JSON.stringify(scsCommand), response);
        }
        else if(cmd.command === "finish") 
        {
            var scsCommand = {};
            
            scsCommand["userCommand"] = "finish stepper w adjustment";
            scsCommand["commandId"] = newCommandId();

            sendSCSCommand(JSON.stringify(scsCommand), response);
        }
        else
        {
            appLog("onAdjustStepperW unsupported command: " + command);
            response.statusCode = 400;
            response.setHeader('Content-Type', 'text/plain');
            response.write("onAdjustStepperW unsupported command: " + command);
            response.end();
        }
    });   
}

function onFrameQuery(request, response)
{
    let body = [];

    request.on('data', (chunk) => {
        body.push(chunk);
    }).on('end', () => {
        body = Buffer.concat(body).toString();
        let cmdObj = JSON.parse(body);
        
        let milliseconds = cmdObj.milliseconds;

        fs.readdir(framesRootFolder, function(err, frameFolders) {
            if(err) {
                response.statusCode = 400;
                response.end();
                return;
            }
            if(frameFolders.length < 1) {
                //empty folder
                response.statusCode = 400;
                response.end();
                return;
            }

            frameFolders = frameFolders.sort();

            //find the earliest frame
            fs.readdir(framesRootFolder + "/" + frameFolders[0] + "/", function(err, earliestFiles) {
                let earliestFile;

                if(err) {
                    response.statusCode = 400;
                    response.end();
                    return;
                }
                if(earliestFiles.length < 1) {
                    //empty folder
                    response.statusCode = 400;
                    response.end();
                    return;
                }

                earliestFiles.sort();
                earliestFile = earliestFiles[0];

                //find the latest file
                fs.readdir(framesRootFolder + "/" + frameFolders[frameFolders.length-1] + "/", function(err, latestFiles) {
                    let latestFile;

                    if(err) {
                        response.statusCode = 400;
                        response.end();
                        return;
                    }
                    if(earliestFiles.length < 1) {
                        //empty folder
                        response.statusCode = 400;
                        response.end();
                        return;
                    }
                    
                    latestFiles.sort();
                    latestFile = latestFiles[latestFiles.length-1];

                    //find the designated file
                    let minutes = milliseconds / 60000;
                    let folder = "";
                    for(var i=frameFolders.length; i>0; i--)
                    {
                        if(Number.parseInt(frameFolders[i-1]) < minutes) {
                            folder = frameFolders[i-1];
                            break;
                        }
                    }
                    if(folder === "") {
                        folder = frameFolders[0];
                    }
                    fs.readdir(framesRootFolder + "/" + folder + "/", function(err, files) {
                        if(err) {
                            response.statusCode = 400;
                            response.end();
                            return;
                        }
                        if(files.length < 1) {
                            //empty folder
                            response.statusCode = 400;
                            response.end();
                            return;
                        }
            
                        files = files.sort();
                        let file = "";
                        for(var i = files.length; i>0; i--) {
                            if(Number.parseInt(files[i-1]) < milliseconds) {
                                file = files[i-1];
                                break;
                            }
                        }
                        if(file === "") {
                            file = files[0];
                        }
        
                        let reply = {};
        
                        reply.firstFile = earliestFile;
                        reply.queriedFile = file;
                        reply.lastFile = latestFile;
                        reply.pathFile = folder + "/" + file;
                        response.statusCode = 200;
                        response.end(JSON.stringify(reply));
                    });                    
                });
            });
        });
    });
}

function onFrameRetrive(request, response)
{
    var fileName = "data" + request.url;
    fs.stat(fileName, function(error, stats) {
        if (error) {
            response.statusCode = 400;
            response.end();
            return;
        }

        if (stats.isFile()) {
            var stream = fs.createReadStream(fileName);
            response.statusCode = 200;
            //response.setHeader('Content-Type', fileType);
            stream.pipe(response);
        } else {
            response.statusCode = 400;
            response.end();
        }
    });
}

function onHttpRequest(request, response) 
{
    var url = request.url;
    
    if(url === "/frameQuery") {
        onFrameQuery(request, response);
        return;
    }
    else if(url.indexOf("/frames/") === 0) {
        // URL: /frames/...
        //appLog("onHttpRequest: " + request.url);
        onFrameRetrive(request, response);
        return;
    }

    appLog("onHttpRequest: " + request.url);

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
    } else if (url === "/toCoordinateItem") {
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
    } else if (url === "/getTouchScreenMappings") {
        onGetTouchScreenMappings(request, response);
    } else if (url === "/saveTouchScreenMappings") {
        onSaveTouchScreenMappings(request, response);
    } else if (url === "/touchScreen") {
        onTouchScreen(request, response);
    } else if (url === "/adjustStepperW") {
        onAdjustStepperW(request, response);
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

//set up log
setInterval(maintainLog, logMaintancePeriod);
//create web server.
const server = http.createServer((req, res) => {
    onHttpRequest(req, res);
});
//start web server
server.listen(port, hostname, () => {
    appLog(`Server running at http://${hostname}:${port}/`);
});
