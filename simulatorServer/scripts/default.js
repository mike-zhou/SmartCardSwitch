
let counter = 0;
let frameUpdateTimer = setInterval(frameUpdate, 1000);
let clientIpUpdateTimer = setInterval(clientUpdate, 1000);

function onElementClicked() 
{
    //element id is in the format of group_action_XXX
    var elementId = document.activeElement.id;
    var paraArray = elementId.split("_");

    var group = paraArray[0];
    if (group === "nozzle") 
    {
        let index = paraArray[1];
        let action = paraArray[2];
        let command = {};
    
        command.index = parseInt(index);
        command.action = parseInt(action);

        let xhr = new XMLHttpRequest();
        xhr.responseType = "json";
        xhr.open('POST', '/nozzle');
    
        xhr.onreadystatechange = function() {
            var DONE = 4; // readyState 4 means the request is done.
            var OK = 200; // status 200 is a successful return.
            if (xhr.readyState === DONE) {
                console.log("response is available");
                console.log("response type: " + xhr.responseType);
    
                if (xhr.status === OK) {
                    console.log(elementId + " is processed successfully");
                } else {
                    console.log(elementId + " is not processed successfully");
                }
            }
        };
        xhr.send(JSON.stringify(command));
    }
    else if(group === "fps") {
        let selector = document.getElementById("fps");
        let fps = parseInt(selector.value);
        let period = 1000/fps;

        //update frequency
        clearInterval(frameUpdateTimer);
        peroid = Math.floor(period);
        console.log("new frame update interval: " + period);
        frameUpdateTimer = setInterval(frameUpdate, peroid);
    }
}

function frameUpdate()
{
    let image = document.getElementById("videoFrame");
    image.src = "/frames/lastFrame" + counter;
    counter++;
}

function clientUpdate()
{
    let xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', '/clientIp');

    xhr.onreadystatechange = function() {
        let DONE = 4; // readyState 4 means the request is done.
        let OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                let obj = xhr.response;
                let ip = obj.clientIp;

                document.getElementById("currentUser").textContent = ip;
            } else {
                document.getElementById("currentUser").textContent = "Unknown RS23ETH Server Status";
            }
        }
    };
    xhr.send();
}
