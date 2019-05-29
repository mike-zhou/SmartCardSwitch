var globalCardSlotMappings;
var globalTouchScreenMappings;



function getSelectedMappingName()
{
    var index =  document.getElementById("mappingActivation_selection").selectedIndex;
    var mappingName = document.getElementById("mappingActivation_selection").options[index].text;

    return mappingName;
}

function updateCardSlotMappingTable(mapping) {
    var html = "<table class=\"cardSlotAccessTable\">";

    //header
    html = html + "<tr><th>Card Name</th><th>Slot</th><th>Action</th></tr>";

    //mappings
    for (var i = 0; i < mapping.length; i++) {
        var row;

        row = "<tr>";
        row = row + "<td>" + mapping[i].cardName + "</td>";
        row = row + "<td align=\"center\">" + mapping[i].slotNumber + "</td>";
        row = row + "<td>";
        row = row +     "<button id=\"" + "cardAccess_insert_" + i + "\"> Insert </button>" + " ";
        row = row +     "<button id=\"" + "cardAccess_extract_" + i + "\"> Extract </button>" + " ";
        row = row +     "<button id=\"" + "cardAccess_swipe_" + i + "\"> Swipe </button>" + " ";
        row = row +     "<button id=\"" + "cardAccess_tapContactless_" + i + "\"> Tap Contactless </button>" + " ";
        row = row +     "<button id=\"" + "cardAccess_tapBarcode_" + i + "\"> Tap Barcode </button>";
        row = row + "</td>";
        row = row + "</tr>";

        html = html + row;
    }

    html = html + "</table>";

    document.getElementById("cardAccess").innerHTML = html;
}

function loadMapping(mappingName)
{
    for(var i=0; i<globalCardSlotMappings.length; i++) {
        var element = globalCardSlotMappings[i];
        if(element.name == mappingName) {
            updateCardSlotMappingTable(element.mapping);
            break;
        }
    }
}

function activateMapping(mappingName)
{
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', '/saveCardSlotMappings');

    var foundMapping = false;

    for(var i=0; i<globalCardSlotMappings.length; i++) {
        var element = globalCardSlotMappings[i];
        if(element.name == mappingName) {
            element.active = true;
            foundMapping = true;
        }
        else {
            element.active = false;
        }
    }

    if(!foundMapping) {
        alert(mappingName + " is not found");
        return;
    }

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;

                console.log("mapping is saved");
                loadMapping(mappingName);
                console.log(mappingName + " has been activated");

                alert(mappingName + " has been activated");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(globalCardSlotMappings));
}

function onKey(keyIndex) {
    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/key');

    var command = {};
    command["index"] = keyIndex;

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log("key was pressed: " + keyIndex);
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function getCardName(index)
{
    var cardName = "";

    for(var i=0; i<globalCardSlotMappings.length; i++) 
    {
        var element = globalCardSlotMappings[i];
        if(element.active == true) 
        {
            cardName = element.mapping[index].cardName;
            break;
        }
    }

    return cardName;
}

function onCardInsert(index)
{
    var cardName = getCardName(index);
    if(cardName === "") {
        alert("No card was found");
        return;
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/cardAccess');

    var command = {};
    command["command"] = "insert";
    command["name"] = cardName;

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log(cardName + "was inserted");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onCardExtract(index)
{
    var cardName = getCardName(index);
    if(cardName === "") {
        alert("No card was found");
        return;
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/cardAccess');

    var command = {};
    command["command"] = "extract";
    command["name"] = cardName;

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log(cardName + "was extracted");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onCardSwipe(index)
{
    var cardName = getCardName(index);
    if(cardName === "") {
        alert("No card was found");
        return;
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/cardAccess');

    var command = {};
    command["command"] = "swipe";
    command["name"] = cardName;
    command["downPeriod"] = 4000;

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log(cardName + "was swipped");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onCardTapContactless(index)
{
    var cardName = getCardName(index);
    if(cardName === "") {
        alert("No card was found");
        return;
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/cardAccess');

    var command = {};
    command["command"] = "tapContactless";
    command["name"] = cardName;
    command["downPeriod"] = 4000;

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log(cardName + "was tapped on contactless reader");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onCardTapBarcode(index)
{
    var cardName = getCardName(index);
    if(cardName === "") {
        alert("No card was found");
        return;
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/cardAccess');

    var command = {};
    command["command"] = "tapBarcode";
    command["name"] = cardName;
    command["downPeriod"] = 4000;

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log(cardName + "was tapped on barcode reader");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onAddTouchScreenArea()
{
    var areaName = document.getElementById("touchScreen_areas").value;
    if(areaName) {
        var selectedAreas = document.getElementById("touchScreen_selectedAreas");
        var html = selectedAreas.innerHTML;
        
        html = html + "<option>" + areaName + "</option>";
        selectedAreas.innerHTML = html;
        selectedAreas.size = selectedAreas.length;        
    }
}

function onDeleteTouchScreenArea()
{
    var selectedAreas = document.getElementById("touchScreen_selectedAreas");
    selectedAreas.remove(selectedAreas.selectedIndex);
    selectedAreas.size = selectedAreas.length;
}

function onPressTouchScreenArea()
{
    var xhr = new XMLHttpRequest();
    xhr.responseType = "text/plain";
    xhr.open('POST', '/touchScreen');

    var command = [];
    var selectedAreas = document.getElementById("touchScreen_selectedAreas").options;
    if(selectedAreas.length < 1) 
    {
        alert('Error: no touch screen area is selected');
        return;
    }
    for(var i=0; i<selectedAreas.length; i++)
    {
        command[i] = {};
        command[i]["areaName"] = selectedAreas[i].value;
        command[i]["order"] = i;
    }

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                console.log("onPressTouchScreenArea succeeded");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText + ":" + xhr.response); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onActivateTouchScreenMapping()
{
    var html = "";
    var itemAmount = 0;
    var selectedMappingName = document.getElementById("touchScreen_mappingSelection").value;

    for(var i=0; i<globalTouchScreenMappings.length; i++) 
    {
        var mapping = globalTouchScreenMappings[i];

        mapping.active = false;
        if(mapping.name == selectedMappingName) 
        {
            mapping.active = true;
            document.getElementById("touchScreen_currentMapping").innerText = selectedMappingName;

            for(var j=0; j<mapping.mapping.length; j++) 
            {
                html = html + "<option>" + mapping.mapping[j]["areaName"] + "</option>";
                itemAmount = j + 1;
            }

            document.getElementById("touchScreen_areas").innerHTML = html;
            document.getElementById("touchScreen_areas").size = document.getElementById("touchScreen_areas").length;
            document.getElementById("touchScreen_selectedAreas").innerHTML = "";
        }
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', '/saveTouchScreenMappings');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                console.log("onActivateTouchScreenMapping succeeded");
                alert("Touch screen mapping is activated");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(globalTouchScreenMappings));
}

function onElementClicked() 
{
    //element id is in the format of group_action_XXX
    var elementId = document.activeElement.id;
    var paraArray = elementId.split("_");

    var group = paraArray[0];
    if (group === "mappingActivation") 
    {
        var action = paraArray[1];
        if(action === "activate") {
            var mappingName = getSelectedMappingName();
            console.log("onElementClicked activate mapping: " + mappingName);
            activateMapping(mappingName);
        }
    }
    else if(group === "cardAccess") 
    {
        var action = paraArray[1];
        var index = paraArray[2];
        
        if(action === "insert") {
            onCardInsert(index);
        }
        else if(action === "extract") {
            onCardExtract(index);
        }
        else if(action === "swipe") {
            onCardSwipe(index);
        }
        else if(action === "tapContactless") {
            onCardTapContactless(index);
        }
        else if(action === "tapBarcode") {
            onCardTapBarcode(index);
        }
        else {
            alert("unknown action: " + action);
        }
    }
    else if(group === "touchScreen") {
        var action = paraArray[1];

        if(action === "add") {
            onAddTouchScreenArea();
        }
        else if(action === "delete") {
            onDeleteTouchScreenArea();
        }
        else if(action === "press") {
            onPressTouchScreenArea();
        }
        else if(action === "mappingActivate") {
            onActivateTouchScreenMapping();
        }
    }
    else if(group === "iFinger") {
        onKey(paraArray[2]);
    }
}

function onCardSlotMappingArrived(mappings) {
    globalCardSlotMappings = mappings; //save to global variable.
    var names = [];
    var activeMapping;

    if ((globalCardSlotMappings === undefined) ||
        (!Array.isArray(globalCardSlotMappings)) ||
        (globalCardSlotMappings.lenth == 0)) {
        console.log("onCardSlotMappingArrived illegal mapping: " + mappings);
        return;
    }

    for (var i = 0; i < globalCardSlotMappings.length; i++) {
        var cur = globalCardSlotMappings[i];
        names[i] = cur.name;
        if (cur.active == true) {
            activeMapping = cur.name;
            updateCardSlotMappingTable(cur.mapping);
        }
    }

    document.getElementById("mappingActivation_currentMapping").innerText = activeMapping;

    var selection = document.getElementById("mappingActivation_selection");
    for (var i = 0; i < selection.options.length;) {
        selection.options.remove(0);
    }
    for (var i = 0; i < names.length; i++) {
        var option = document.createElement("option");
        option.text = names[i];
        selection.options.add(option, i);
    }
}

function onTouchScreenMappingArrived(mappings)
{
    globalTouchScreenMappings = mappings;
    var html = "";
    var activeMappingName = "";
    var itemAmount = 0;
    var names = [];

    for(var i=0; i<mappings.length; i++)
    {
        var mapping = mappings[i];

        names[i] = mapping.name;
        if(mapping.active == true) 
        {
            activeMappingName = mapping.name;
            for(var j=0; j<mapping.mapping.length; j++) 
            {
                html = html + "<option>" + mapping.mapping[j]["areaName"] + "</option>";
                itemAmount = j + 1;
            }
        }
    }

    var selection = document.getElementById("touchScreen_mappingSelection");
    for (var i = 0; i < names.length; i++) {
        var option = document.createElement("option");
        option.text = names[i];
        selection.options.add(option, i);
    }

    document.getElementById("touchScreen_currentMapping").innerText = activeMappingName;
    document.getElementById("touchScreen_areas").innerHTML = html;
    document.getElementById("touchScreen_areas").size = itemAmount;
}

function askForMappings() {
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

    var xhr2 = new XMLHttpRequest();
    xhr2.responseType = "json";
    xhr2.open('POST', '/getTouchScreenMappings');

    xhr2.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr2.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr2.responseType);

            if (xhr2.status === OK) {
                var jsonObj = xhr2.response;
                onTouchScreenMappingArrived(jsonObj);
                console.log("getTouchScreenMapping succeeded");
            } else {
                alert('Error: ' + xhr2.status + ":" + xhr2.statusText); // An error occurred during the request.
            }
        }
    };
    xhr2.send();
}
document.addEventListener("DOMContentLoaded", askForMappings);
