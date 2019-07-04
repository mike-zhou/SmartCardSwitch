import { setInterval } from "timers";

const frameRetrievingInterval = 100;
const imageWidth = 1280;
const imageHeight = 720;

var currentFrameName = "";

function frameUpdateTimer()
{
    var date = new Date();
    var frameInfo = {};
    
    frameInfo.year = date.getFullYear();
    frameInfo.month = date.getMonth() + 1;
    frameInfo.day = date.getDate();
    frameInfo.hour = date.getHours();
    frameInfo.minute = date.getMinutes();
    frameInfo.second = date.getSeconds();
    frameInfo.milliSecond = date.getMilliseconds();

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/frameName');

    xhr.onreadystatechange = function() {
        let DONE = 4; // readyState 4 means the request is done.
        let OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                document.getElementById("videoFrame").src = "/frames/" + xhr.response;
            }
        }
    };
    xhr.send(JSON.stringify(frameInfo));
}

function initRecordPage()
{
    let html;

    html = "<img id=\"videoFrame\">";
    document.getElementById("videoFrameContainer").innerHTML = html;
    document.getElementById("videoFrame").width = imageWidth;
    document.getElementById("videoFrame").height = imageHeight;

    setInterval(frameUpdateTimer, frameRetrievingInterval);
}

document.addEventListener("DOMContentLoaded", initRecordPage);

