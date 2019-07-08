const FRAME_RETRIEVING_INTERVAL = 100;
const IMAGE_WIDTH = 1280;
const IMAGE_HEIGHT = 720;

var _firstFrameName = "";
var _currentFrameName = "";
var _lastFrameName = "";
var _frameDragged = false;

function frameUpdateTimer()
{
    var date = new Date();
    var frameInfo = {};
    
    if(_frameDragged) {

    }
    else {
        //query frame at current time
        frameInfo.milliseconds = date.getTime();
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/json";
    xhr.open('POST', '/frameQuery');

    xhr.onreadystatechange = function() {
        let DONE = 4; // readyState 4 means the request is done.
        let OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            if (xhr.status === OK) {
                let reply = xhr.response;
                reply = JSON.parse(reply);

                if(reply.pathFile !== _currentFrameName) {
                    _currentFrameName = reply.pathFile;
                    
                    let image = document.getElementById("videoFrame");
                    image.src = "/frames/" + reply.pathFile;
                    image.width = window.innerWidth;
                    image.height = image.width * IMAGE_HEIGHT / IMAGE_WIDTH;
                }

                _firstFrameName = reply.firstFile;
                _lastFrameName = reply.lastFile;
                let frameCtl = document.getElementById("framePosition");
                frameCtl.max = Number.parseInt(_lastFrameName);
                frameCtl.value = Number.parseInt(reply.queriedFile);
                frameCtl.min = Number.parseInt(_firstFrameName);
            }
        }
    };
    xhr.send(JSON.stringify(frameInfo));
}

function onFramePositionMouseDown()
{

}

function onFramePositionMouseUp()
{

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

    setInterval(frameUpdateTimer, FRAME_RETRIEVING_INTERVAL);
}

function onWindowSize()
{
    let image = document.getElementById("videoFrame");
    image.width = window.innerWidth;
    image.height = image.width * IMAGE_HEIGHT / IMAGE_WIDTH;
}

window.addEventListener("resize", onWindowSize);
document.addEventListener("DOMContentLoaded", initRecordPage);

