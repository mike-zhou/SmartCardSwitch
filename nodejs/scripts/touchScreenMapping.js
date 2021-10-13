var globalTouchScreenMappings;

function updateTouchScreenMappingTable(mapping) {
    var html = "<table class=\"touchScreenMappingTable\">";

    //header
    html = html + "<tr><th>AreaName</th><th>Index</th><th>Action</th></tr>";

    //mappings
    for (var i = 0; i < mapping.length; i++) {
        var row;

        row = "<tr>";
        row = row + "<td>" + mapping[i].areaName + "</td>";
        row = row + "<td align=\"center\">" + mapping[i].index + "</td>";
        row = row + "<td><button id=\"" + "mappingContent_delete_" + i + "\"> Delete </button></td>";
        row = row + "</tr>";

        html = html + row;
    }

    html = html + "</table>";

    document.getElementById("mappingContent_details").innerHTML = html;
}

function onTouchScreenMappingArrived(mappings) {
    globalTouchScreenMappings = mappings; //save to global variable.
    var names = [];
    var currentMapping = "";

    if ((globalTouchScreenMappings === undefined) ||
        (!Array.isArray(globalTouchScreenMappings)) ||
        (globalTouchScreenMappings.lenth == 0)) {
        console.log("onTouchScreenMappingArrived illegal mapping: " + mappings);
        return;
    }

    //find and show active mapping
    for (var i = 0; i < globalTouchScreenMappings.length; i++) {
        var cur = globalTouchScreenMappings[i];
        names[i] = cur.name;
        if (cur.active == true) {
            currentMapping = cur.name;
            updateTouchScreenMappingTable(cur.mapping);
        }
    }
    //update mapping name
    document.getElementById("mappingSelect_currentMapping").innerText = currentMapping;

    //update mapping selection list
    var selection = document.getElementById("mappingSelect_mappingSet");
    for (var i = 0; i < names.length; i++) {
        var option = document.createElement("option");
        option.text = names[i];
        selection.options.add(option, i);
    }
 
    //set mapping index list
    var slotSelection = document.getElementById("mappingModify_indexSelection");
    for (var i = 0; i < 50; i++) {
        var option = document.createElement("option");
        option.text = i;
        slotSelection.options.add(option, i);
    }
}

function askForTouchScreenMapping() {
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', '/getTouchScreenMappings');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                onTouchScreenMappingArrived(jsonObj);
                console.log("getTouchScreenMappings succeeded");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };
    xhr.send();
}

document.addEventListener("DOMContentLoaded", askForTouchScreenMapping);

function loadMapping(mappingName)
{
    for(var i=0; i<globalTouchScreenMappings.length; i++) {
        var element = globalTouchScreenMappings[i];
        if(element.name == mappingName) {
            document.getElementById("mappingSelect_currentMapping").innerText = mappingName;
            updateTouchScreenMappingTable(element.mapping);
            break;
        }
    }
}

function addTouchScreenMapping(mappingName, areaName, index)
{
    for(var i=0; i<globalTouchScreenMappings.length; i++) 
    {
        var element = globalTouchScreenMappings[i];
        if(element.name != mappingName) {
            continue;
        }
        var areaExist = false;

        for(var j=0; j<element.mapping.length; j++)
        {
            if(element.mapping[j].areaName == areaName) {
                areaExist = true;
                break;
            }
        }

        if(areaExist) {
            console.log("ERROR: " + areaName + " has already in the mapping");
            alert(areaName + " cannot be added\nIt has already in the mapping!");
        }
        else
        {
            var item = {};
            item["areaName"] = areaName;
            item["index"] = index;
            element.mapping.push(item);
            updateTouchScreenMappingTable(element.mapping);
        }
        break;
    }
}

function getCurrentMappingName()
{
    var index =  document.getElementById("mappingSelect_mappingSet").selectedIndex;
    var mappingName = document.getElementById("mappingSelect_mappingSet").options[index].text;

    return mappingName;
}

function saveTouchScreenMapping()
{
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
                console.log("saveTouchScreenMappings succeeded");
                alert("Touch screen mapping is saved");
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
            var mappingName = getCurrentMappingName();

            for(var i=0; i<globalTouchScreenMappings.length; i++)
            {
                var element = globalTouchScreenMappings[i];
                if(element.name != mappingName) {
                    continue;
                }
        
                element.mapping.splice(index, 1);
                loadMapping(mappingName);
            }
        }
    }
    else if(group === "mappingModify")
    {
        var action = paraArray[1];
        if(action === "add") {
            var areaName = document.getElementById("mappingModify_areaName").value;
            var index = document.getElementById("mappingModify_indexSelection").selectedIndex;
            var mappingName = document.getElementById("mappingSelect_currentMapping").innerText;

            console.log("onElementClicked add " + areaName + ":" + index + " to " + mappingName);
            addTouchScreenMapping(mappingName, areaName, index);
        }
        else if(action === "save") {
            saveTouchScreenMapping();
        }
    }
}
