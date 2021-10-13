function onElementClicked() 
{
    //element id is in the format of group_action_XXX
    var elementId = document.activeElement.id;
    var paraArray = elementId.split("_");

    var group = paraArray[0];
    if (group === "adjustStepperW") 
    {
        var action = paraArray[1];

        if(action === "pullUpSmartCard") 
        {
            var xhr = new XMLHttpRequest();
            xhr.responseType = "text/plain";
            xhr.open('POST', '/adjustStepperW');
        
            var command = {};
            command["command"] = "pullUpCard";
            command["index"] = 0;
        
            xhr.onreadystatechange = function() {
                var DONE = 4; // readyState 4 means the request is done.
                var OK = 200; // status 200 is a successful return.
                if (xhr.readyState === DONE) {
                    console.log("response is available");
                    console.log("response type: " + xhr.responseType);
        
                    if (xhr.status === OK) {
                        console.log(cardName + "was pulled up");
                    } else {
                        alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
                    }
                }
            };
            xhr.send(JSON.stringify(command));
        }
        else if(action === "decreaseOffset") 
        {
            var offset = parseInt(document.getElementById("adjustStepperW_offsest").innerText);
            offset = offset - 1;

            var xhr = new XMLHttpRequest();
            xhr.responseType = "text/plain";
            xhr.open('POST', '/adjustStepperW');
        
            var command = {};
            command["command"] = "setOffset";
            command["offset"] = offset;
        
            xhr.onreadystatechange = function() {
                var DONE = 4; // readyState 4 means the request is done.
                var OK = 200; // status 200 is a successful return.
                if (xhr.readyState === DONE) {
                    console.log("response is available");
                    console.log("response type: " + xhr.responseType);
        
                    if (xhr.status === OK) {
                        document.getElementById("adjustStepperW_offsest").innerText = offset;
                        console.log("current stepper W offset: " + offset);
                    } else {
                        alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
                    }
                }
            };
            xhr.send(JSON.stringify(command));
        }
        else if(action === "increaseOffset") 
        {
            var offset = parseInt(document.getElementById("adjustStepperW_offsest").innerText);
            offset = offset + 1;

            var xhr = new XMLHttpRequest();
            xhr.responseType = "text/plain";
            xhr.open('POST', '/adjustStepperW');
        
            var command = {};
            command["command"] = "setOffset";
            command["offset"] = offset;
        
            xhr.onreadystatechange = function() {
                var DONE = 4; // readyState 4 means the request is done.
                var OK = 200; // status 200 is a successful return.
                if (xhr.readyState === DONE) {
                    console.log("response is available");
                    console.log("response type: " + xhr.responseType);
        
                    if (xhr.status === OK) {
                        document.getElementById("adjustStepperW_offsest").innerText = offset;
                        console.log("current stepper W offset: " + offset);
                    } else {
                        alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
                    }
                }
            };
            xhr.send(JSON.stringify(command));
        }
        else if(action === "putBackSmartCard") 
        {
            var xhr = new XMLHttpRequest();
            xhr.responseType = "text/plain";
            xhr.open('POST', '/adjustStepperW');
        
            var command = {};
            command["command"] = "putBackCard";
            command["index"] = 0;
        
            xhr.onreadystatechange = function() {
                var DONE = 4; // readyState 4 means the request is done.
                var OK = 200; // status 200 is a successful return.
                if (xhr.readyState === DONE) {
                    console.log("response is available");
                    console.log("response type: " + xhr.responseType);
        
                    if (xhr.status === OK) {
                        console.log(cardName + "was put back");
                    } else {
                        alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
                    }
                }
            };
            xhr.send(JSON.stringify(command));
        }
        else if(action === "finish") 
        {
            var xhr = new XMLHttpRequest();
            xhr.responseType = "text/plain";
            xhr.open('POST', '/adjustStepperW');
        
            var command = {};
            command["command"] = "finish";
        
            xhr.onreadystatechange = function() {
                var DONE = 4; // readyState 4 means the request is done.
                var OK = 200; // status 200 is a successful return.
                if (xhr.readyState === DONE) {
                    console.log("response is available");
                    console.log("response type: " + xhr.responseType);
        
                    if (xhr.status === OK) {
                        console.log("finished stepper W adjusting");
                    } else {
                        alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
                    }
                }
            };
            xhr.send(JSON.stringify(command));
        }
        else 
        {
            alert("Error: unknown action: " + action);
        }
    }
}
