var globalCardSlotMappings;

function updateCardSlotMapping(mapping)
{
    var html = "<table class=\"cardSlotMappingTable\">";

    //header
    html = html + "<tr><th>Card Name<th><th>Slot number</th><th>Action</th></tr>";

    //mappings
    for(var i=0; i<mapping.length; i++) 
    {
        var row;
        
        row = "<tr>";
        row = row + "<td>" + mapping[i].cardName + "</td>";
        row = row + "<td>" + mapping[i].slotNumber + "</td>";
        row = row + "<td><button id=\"" + "mappingContent_delete_" + i + "\"> Delete </button></td>";
        row = row + "</tr>";

        html = html + row;
    }

    html = html + "</table>";

    document.getElementById("mappingContent_details").innerHTML = html;
}

function onCardSlotMappingArrived(mappings)
{
    globalCardSlotMappings = mappings;
    var names = [];
    var activeMapping;

    if((globalCardSlotMappings === undefined) ||
        (!Array.isArray(globalCardSlotMappings)) ||
        (globalCardSlotMappings.lenth == 0)) 
    {
        console.log("onCardSlotMappingArrived illegal mapping: " + mappings);
        return;
    }

    for(var i=0; i<globalCardSlotMappings.length; i++) 
    {
        var cur = globalCardSlotMappings[i];
        names[i] = cur.name;
        if(cur.active == true) {
            activeMapping = cur.name;
            updateCardSlotMapping(cur.mapping);
        }
    }


    document.getElementById("mappingSelect_activeMapping").text = activeMapping;

    var selection = document.getElementById("mappingSelect_mappingSet");
    for(var i=0; i<selection.options.length; ) 
    {
        selection.options.remove(0);
    }
    for(var i=0; i<names.length; i++) 
    {
        var option = document.createElement("option");
        option.text = names[i];
        selection.options.add(option, i+1);
    }
}

function askForCardSlotMapping()
{
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', '/getCardSlotMappings');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                onCardSlotMappingArrived(jsonObj);
                console.log("getCardSlotMappings succeeded");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };
    xhr.send();
}

document.addEventListener("DOMContentLoaded", askForCardSlotMapping);

function onElementClicked() {
    var elementId = document.activeElement.id;
    var paraArray = elementId.split("_");

    var device = paraArray[0];
    if (device === "stepper") {
        var index = paraArray[1];
        var action = paraArray[2];

        if (action === "decrease") {
            onStepperDecrease(device + index);
        } else if (action === "decrease128") {
            moveStepper(device + index, false, 128);
        } else if (action === "decrease64") {
            moveStepper(device + index, false, 64);
        } else if (action === "decrease32") {
            moveStepper(device + index, false, 32);
        } else if (action === "decrease16") {
            moveStepper(device + index, false, 16);
        } else if (action === "decrease8") {
            moveStepper(device + index, false, 8);
        } else if (action === "decrease4") {
            moveStepper(device + index, false, 4);
        } else if (action === "decrease1") {
            moveStepper(device + index, false, 1);
        } else if (action === "increase") {
            onStepperIncrease(device + index);
        } else if (action === "increase128") {
            moveStepper(device + index, true, 128);
        } else if (action === "increase64") {
            moveStepper(device + index, true, 64);
        } else if (action === "increase32") {
            moveStepper(device + index, true, 32);
        } else if (action === "increase16") {
            moveStepper(device + index, true, 16);
        } else if (action === "increase8") {
            moveStepper(device + index, true, 8);
        } else if (action === "increase4") {
            moveStepper(device + index, true, 4);
        } else if (action === "increase1") {
            moveStepper(device + index, true, 1);
        } else if (action === "configMovement") {
            onStepperConfigMovement(index);
        } else if (action === "configHome") {
            onStepperConfigHome(index);
        } else if (action === "configForwardClockwise") {
            onStepperConfigForwardClockwise(index);
        } else {
            alert("onElementClicked unknown action type: " + elementId);
        }
    } else if (device === "bdc") {
        var index = parseInt(paraArray[1]);
        var action = paraArray[2];

        if (action === "Forward") {
            setBdc(index, 0);
        } else if (action === "Reverse") {
            setBdc(index, 1);
        } else if (action === "Release") {
            setBdc(index, 2);
        } else {
            console.log("Error: wrong bdc action: " + action);
        }
    } else if (device === "coordinateItem") {
        var type = paraArray[1];
        var index = parseInt(paraArray[2]);
        onCoordinateItem(type, index);
    } else if (device === "smartCardOffset") {
        var index;
        var type = paraArray[1];

        if ((type === "index") || (type === "value")) {
            index = parseInt(paraArray[2]);
        }
        onSmartCardOffset(type, index);
    } else if (device === "power") {
        var target = paraArray[1];
        var action = paraArray[2];
        onPower(target, action === "on");
    } else if (device === "iFinger") {
        var keyIndex = paraArray[2];
        onKey(keyIndex);
    } else if (elementId === "coordinate_save") {
        saveCoordinate();
    } else if (elementId === "deviceQuery") {
        queryDevice();
    }

}
