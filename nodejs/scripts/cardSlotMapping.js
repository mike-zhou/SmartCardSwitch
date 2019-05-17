var globalCardSlotMappings;

function updateCardSlotMappingTable(mapping) {
    var html = "<table class=\"cardSlotMappingTable\">";

    //header
    html = html + "<tr><th>Card Name</th><th>Slot</th><th>Action</th></tr>";

    //mappings
    for (var i = 0; i < mapping.length; i++) {
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

    document.getElementById("mappingSelect_activeMapping").innerText = activeMapping;

    var selection = document.getElementById("mappingSelect_mappingSet");
    for (var i = 0; i < selection.options.length;) {
        selection.options.remove(0);
    }
    for (var i = 0; i < names.length; i++) {
        var option = document.createElement("option");
        option.text = names[i];
        selection.options.add(option, i);
    }

    var slotSelection = document.getElementById("mappingModify_slotSelection");
    for (var i = 0; i < 60; i++) {
        var option = document.createElement("option");
        option.text = i;
        slotSelection.options.add(option, i);
    }
}

function askForCardSlotMapping() {
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

function addCardSlotMapping(mappingName, cardName, slotNumber)
{
    for(var i=0; i<globalCardSlotMappings.length; i++) 
    {
        var element = globalCardSlotMappings[i];
        if(element.name != mappingName) {
            continue;
        }
        var cardExist = false;

        for(var j=0; j<element.mapping.length; j++)
        {
            if(element.mapping[j].cardName == cardName) {
                cardExist = true;
                break;
            }
        }

        if(cardExist) {
            console.log("ERROR: " + cardName + " has already in the mapping");
            alert(cardName + " cannot be added\nIt has already in the mapping!");
        }
        else
        {
            var item = {};
            item["cardName"] = cardName;
            item["slotNumber"] = slotNumber;
            element.mapping.push(item);
            updateCardSlotMappingTable(element.mapping);
        }
    }
}

function getCurrentMappingName()
{
    var index =  document.getElementById("mappingSelect_mappingSet").selectedIndex;
    var mappingName = document.getElementById("mappingSelect_mappingSet").options[index].text;

    return mappingName;
}

function onElementClicked() 
{
    //element id is in the format of group_action_XXX
    var elementId = document.activeElement.id;
    var paraArray = elementId.split("_");

    var group = paraArray[0];
    if (group === "mappingSelect") 
    {
        var action = paraArray[1];
        if(action === "choose") {
            var mappingName = getCurrentMappingName();
            console.log("onElementClicked load mapping: " + mappingName);
            loadMapping(mappingName);
        }
    }
    else if(group === "mappingContent") 
    {
        var action = paraArray[1];
        if(action === "delete") {
            var index = paraArray[2];

        }
    }
    else if(group === "mappingModify")
    {
        var action = paraArray[1];
        if(action === "add") {
            var cardName = document.getElementById("mappingModify_cardName").value;
            var slotNumber = document.getElementById("mappingModify_slotSelection").selectedIndex;
            var mappingName = document.getElementById("mappingSelect_activeMapping").innerText;

            console.log("onElementClicked add " + cardName + ":" + slotNumber + " to " + mappingName);
            addCardSlotMapping(mappingName, cardName, slotNumber);
        }
        else if(action === "save") {

        }
    }
}
