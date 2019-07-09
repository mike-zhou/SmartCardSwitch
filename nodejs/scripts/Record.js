const FRAME_RETRIEVING_INTERVAL = 100;
const IMAGE_WIDTH = 1280;
const IMAGE_HEIGHT = 720;

var _firstFrameName = "";
var _currentFrameName = "";
var _lastFrameName = "";
var _isDragging = false;
var _speed = 1;
var _anchorPosition = 0;
var _anchorTime = new Date();

function frameUpdateTimer()
{
    var date = new Date();
    var frameInfo = {};
    
    if(_anchorPosition == 0) {
        //query frame at current time
        frameInfo.milliseconds = date.getTime();
    }
    else if(_isDragging) {
        frameInfo.milliseconds = _anchorPosition;
    }
    else {
        let anchor = _anchorPosition;
        let offset = _speed * (date.getTime() - _anchorTime.getTime());
        frameInfo.milliseconds = anchor + offset;
        console.log("_anchorPosition: " + _anchorPosition + " offset: " + offset);
        console.log("frameInfo.milliseconds: " + frameInfo.milliseconds);
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', '/frameQuery');

    xhr.onreadystatechange = function() {
        let DONE = 4; // readyState 4 means the request is done.
        let OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            if (xhr.status === OK) {
                let reply = xhr.response;

                if(reply.pathFile !== _currentFrameName) {
                    _currentFrameName = reply.pathFile;
                    
                    let image = document.getElementById("videoFrame");
                    image.src = "/frames/" + reply.pathFile;
                    image.width = window.innerWidth;
                    image.height = image.width * IMAGE_HEIGHT / IMAGE_WIDTH;
                }

                _firstFrameName = reply.firstFile;
                _lastFrameName = reply.lastFile;
                if(!_isDragging) {
                    let frameCtl = document.getElementById("framePosition");

                    console.log("returned: " + reply.queriedFile);
                    frameCtl.max = Number.parseInt(_lastFrameName);
                    frameCtl.value = Number.parseInt(reply.queriedFile);
                    frameCtl.min = Number.parseInt(_firstFrameName);

                    if(_anchorPosition == 1) {
                        _anchorPosition = Number.parseInt(reply.queriedFile);
                        _anchorTime = new Date();
                    }
                }
            }
        }
    };
    xhr.send(JSON.stringify(frameInfo));
}

function onFramePositionMouseDown()
{
    _isDragging = true;
    _anchorPosition = Number.parseInt(document.getElementById("framePosition").value);
    _anchorTime = new Date(); 
}

function onFramePositionMouseUp()
{
    _anchorPosition = Number.parseInt(document.getElementById("framePosition").value);
    _anchorTime = new Date(); 
    _isDragging = false;
}

function onFramePositionMouseMove()
{
    if(_isDragging) {
        _anchorPosition = Number.parseInt(document.getElementById("framePosition").value);
        _anchorTime = new Date();    
    }
}

function initRecordPage()
{
    let html;

    html = "<img id=\"videoFrame\">";
    document.getElementById("videoFrameContainer").innerHTML = html;
    document.getElementById("videoFrame").width = IMAGE_WIDTH;
    document.getElementById("videoFrame").height = IMAGE_HEIGHT;
    document.getElementById("framePosition").width =window.innerWidth;
    document.getElementById("framePosition").addEventListener("mousedown", onFramePositionMouseDown);
    document.getElementById("framePosition").addEventListener("mouseup", onFramePositionMouseUp);
    document.getElementById("framePosition").addEventListener("mousemove", onFramePositionMouseMove);

    setInterval(frameUpdateTimer, FRAME_RETRIEVING_INTERVAL);
}

function onWindowSize()
{
    let image = document.getElementById("videoFrame");
    image.width = window.innerWidth;
    image.height = image.width * IMAGE_HEIGHT / IMAGE_WIDTH;
}

function onElementClicked()
{
    let elementId = document.activeElement.id;

    if(elementId === "toFirstFrame") {
        _anchorPosition = 1; //to get the earliest frame
        _anchorTime = new Date();
        _speed = 1;
   }
    else if(elementId === "fastBackWard") {
        _speed = _speed - 1;
    }
    else if(elementId === "fastForward") {
        _speed = _speed + 1;
    }
    else if(elementId === "toLastFrame") {
        _anchorPosition = 0; //to get frame at this time
        _anchorTime = new Date();
        _speed = 1;
    }

    let speedStr;
    
    if(_speed > 0) {
        speedStr = "+" + _speed.toString();
    }
    else if(_speed == 0) {
        speedStr = "0";
    }
    else {
        speedStr = _speed.toString();
    }

    document.getElementById("speed").innerText = speedStr;
}

window.addEventListener("resize", onWindowSize);
document.addEventListener("DOMContentLoaded", initRecordPage);

