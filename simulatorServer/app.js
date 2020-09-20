const http = require('http');
const fs = require('fs');
var net = require('net');
const formidable = require('formidable');

const config = require("./config.json");

//IP address and port number clients can connect to.
const hostname = '0.0.0.0';
const port = 80;


//log relevant setting
const logFolder = "/home/user1/Temp/logs/simulatorServer";
const logFileName = "log";
const logFileAmount = 10;
const logFileSize = 5000000; // 5M bytes
var logBuffer = "";
var logMaintancePeriod = 60000; //60000 milliseconds

//frames root folder
const framesRootFolder = "/media/frameDisk/";
let frameUploadFailed;

//client ip
const RS232ETH_SERVER_IP = "127.0.0.1";
const RS232ETH_SERVER_PORT = 8082;
const RS232ETH_URL = "/clientIp";
const DEFAULT_CLIENT_IP = "no user";
const clientIpUpdateInterval = 1000; //1000 ms
let clientIp = DEFAULT_CLIENT_IP;

//set up log
setInterval(maintainLog, logMaintancePeriod);
//client ip retrieval
setInterval(updateClientIp, clientIpUpdateInterval);
//create web server.
const server = http.createServer((req, res) => {
    onHttpRequest(req, res);
});
//start web server
server.listen(port, hostname, () => {
    appLog(`Server running at http://${hostname}:${port}/`);
});

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

function updateClientIp()
{
    let options = {};
    let body = "{}";

    options.hostname = RS232ETH_SERVER_IP;
    options.port = RS232ETH_SERVER_PORT;
    options.path = RS232ETH_URL;
    options.method = 'POST';
    options.headers = {};
    options.headers["Content-Type"] = "application/json";
    options.headers["Content-Length"] = Buffer.byteLength(body);

    let request = http.request(options, (response) => {
        let replyBody = [];

        response.on('data', (chunk) => {
            replyBody.push(chunk);
        }).on('end', () => {
            if(response.statusCode == 200) {
                let reply = Buffer.concat(replyBody).toString();
                reply = JSON.parse(reply); //change string to object

                if(reply.clientIp === "") {
                    clientIp = DEFAULT_CLIENT_IP;
                }
                else {
                    clientIp = reply.clientIp;
                }
            }
            else {
                appLog("RS232ETH server replied: " + response.statusCode);
                clientIp = "Error in RS232ETH server";
            }
        });
    });
    request.on('error', (e) => {
        var msg = "updateClientIp failed to retrieve client ip: " + e;
        appLog(msg);
        clientIp = "RS232ETH server is not running";
    });

    request.write(body);
    request.end();
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


function onNozzle(request, response) {
    let body = [];

    request.on('data', (chunk) => {
        body.push(chunk);
    }).on('end', () => {
        body = Buffer.concat(body).toString();
        appLog("onNozzle " + request.url + " : " + body);

        //forward this request to simulator
        var simulatorOptions = {};
        simulatorOptions.hostname = "127.0.0.1";
        simulatorOptions.port = 8080;
        simulatorOptions.path = "/nozzle";
        simulatorOptions.method = 'POST';
        simulatorOptions.headers = {};
        simulatorOptions.headers["Content-Type"] = "application/json";
        simulatorOptions.headers["Content-Length"] = Buffer.byteLength(body);

        var simulatorRequest = http.request(simulatorOptions, (simulatorResponse) => {
            let replyBody = [];

            simulatorResponse.on('data', (chunk) => {
                replyBody.push(chunk);
            }).on('end', () => {
                replyBody = Buffer.concat(replyBody).toString();
                appLog("onNozzle reply: " + JSON.stringify(replyBody));

                response.statusCode = simulatorResponse.statusCode;
                if ('headers' in response) {
                    if ('content-type' in response.headers) {
                        response.setHeader('Content-Type', simulatorResponse.headers['content-type']);
                    }
                }
                response.end(replyBody);
            });
        });
        simulatorRequest.on('error', (e) => {
            var msg = "onNozzle error in simulator request: " + e;
            appLog(msg);
            //notify browser of error
            response.statusCode = 400;
            response.setHeader('Content-Type', 'text/plain');
            response.end(msg);
        });

        simulatorRequest.write(body);
        simulatorRequest.end();
    });
}

function onDefaultPage(request, response) 
{
    let fileName = "default_" + config.currentManufacture + ".html"
    appLog("onDefaultPage ");
    const stream = fs.createReadStream(fileName);

    response.statusCode = 200;
    response.setHeader('Content-Type', 'text/html');
    stream.pipe(response);
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
        let clientFolder = framesRootFolder + cmdObj.client + "/";

        fs.readdir(clientFolder, function(err, frameFolders) {
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
            fs.readdir(clientFolder + "/" + frameFolders[0] + "/", function(err, earliestFiles) {
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
                fs.readdir(clientFolder + "/" + frameFolders[frameFolders.length-1] + "/", function(err, latestFiles) {
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
                    fs.readdir(clientFolder + "/" + folder + "/", function(err, files) {
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
                        reply.pathFile = "/frames/" + cmdObj.client + "/" + folder + "/" + file;
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
    var fileName = "/media/frameDisk/lastFrame";
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

function onFrameUpload(req, res)
{
    let form = new formidable.IncomingForm();
    
    form.uploadDir = framesRootFolder;
    form.keepExtensions = true;
    form.type = "multipart";
    form.maxFileSize = 10*1024*1024;
    form.parse(req, function(err, fields, files) {
        if(err) {
            if(frameUploadFailed != 1) {
                appLog("onUpload failed in form parsing: " + err);
                frameUploadFailed = 1;
            }
            res.statusCode = 400;
            res.statusMessage = err;
            res.end();
        }
        else
        {
            let oldPath = files.file.path;
            let newPath = framesRootFolder + "lastFrame";
            fs.rename(oldPath, newPath, function(err) {
                if(err) {
                    res.statusCode = 400;
                    res.statusMessage = err;
                    res.end();                    
                    if(frameUploadFailed != 2) {
                        appLog("onUpload failed in renaming: " + err);
                        frameUploadFailed = 2;
                    }
                }
                else {
                    res.statusCode = 200;
                    res.end();
                    if(frameUploadFailed != 0) {
                        appLog("onUpload: received file: " + newPath);
                        frameUploadFailed = 0;
                    }
                }
            });
        }
    });
}

function onRetrieveClientIp(request, response)
{
    let obj = {};

    obj.clientIp = clientIp;
    response.statusCode = 200;
    response.setHeader('Content-Type', 'json');
    response.end(JSON.stringify(obj));
}

function onHttpRequest(request, response) 
{
    var url = request.url;
    
    if(url === "/frameUpload") {
        onFrameUpload(request, response);
        return;
    }
    else if(url.indexOf("/frames/") === 0) {
        // URL: /frames/...
        //appLog("onHttpRequest: " + request.url);
        onFrameRetrive(request, response);
        return;
    }
    else if (url.indexOf("/clientIp") === 0) {
        onRetrieveClientIp(request, response);
        return;
    }

    appLog("onHttpRequest: " + request.url);

    if (url === "/nozzle") {
        onNozzle(request, response);
    } else if (url === "/") {
        onDefaultPage(request, response);
    } else if (url.indexOf("/scripts/") === 0) {
        onRetrievingFile(url.slice(1), "application/javascript", response);
    } else if (url.indexOf("/css/") === 0) {
        onRetrievingFile(url.slice(1), "text/css", response);
    } else {
        var errorMsg = "onHttpRequest unsupported URL: " + request.url;

        appLog(errorMsg);
        response.statusCode = 400;
        response.setHeader('Content-Type', 'text/plain');
        response.end(errorMsg);
    }
}

