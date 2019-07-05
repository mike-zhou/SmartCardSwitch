const frameRetrievingInterval = 100;
const imageWidth = 1280;
const imageHeight = 720;

var currentFrameName = "";

function frameUpdateTimer()
{
    var date = new Date();
    var frameInfo = {};
    
    frameInfo.milliseconds = date.getTime();

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/json";
    xhr.open('POST', '/frameQuery');

    xhr.onreadystatechange = function() {
        let DONE = 4; // readyState 4 means the request is done.
        let OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            if (xhr.status === OK) {
                let reply = xhr.response;
                document.getElementById("videoFrame").src = "/frames/" + reply.pathFile;
            }
        }
    };
    xhr.send(JSON.stringify(frameInfo));
}

function initRecordPage()
{
    let html;

    html = "<img id=\"videoFrame\" src=\"/frames/123.jpg\">";
    document.getElementById("videoFrameContainer").innerHTML = html;
    document.getElementById("videoFrame").width = imageWidth;
    document.getElementById("videoFrame").height = imageHeight;

    setInterval(frameUpdateTimer, frameRetrievingInterval);
}

document.addEventListener("DOMContentLoaded", initRecordPage);

