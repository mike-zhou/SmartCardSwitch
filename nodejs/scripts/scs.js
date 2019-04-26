function updatePageStepper(stepper, data) {
    var elementId;

    elementId = stepper + "State";
    document.getElementById(elementId).innerHTML = data["state"].toString();
    elementId = stepper + "CurOffset";
    document.getElementById(elementId).innerHTML = data["homeOffset"].toString();
    elementId = stepper + "MaxOffset";
    document.getElementById(elementId).innerHTML = data["maximum"].toString();
    elementId = stepper + "LowClks";
    document.getElementById(elementId).value = data["lowClks"].toString();
    elementId = stepper + "HighClks";
    document.getElementById(elementId).value = data["highClks"].toString();
    elementId = stepper + "AccelerationBuffer";
    document.getElementById(elementId).value = data["accelerationBuffer"].toString();
    elementId = stepper + "AccelerationBufferDecrement";
    document.getElementById(elementId).value = data["accelerationBufferDecrement"].toString();
    elementId = stepper + "DecelerationBuffer";
    document.getElementById(elementId).value = data["decelerationBuffer"].toString();
    elementId = stepper + "DecelerationBufferIncrement";
    document.getElementById(elementId).value = data["decelerationBufferIncrement"].toString();
    elementId = stepper + "Locator";
    document.getElementById(elementId).selectedIndex = data["locatorIndex"];
    elementId = stepper + "LocatorLineNumberStart";
    document.getElementById(elementId).selectedIndex = data["locatorLineNumberStart"] - 1;
    elementId = stepper + "LocatorLineNumberTerminal";
    document.getElementById(elementId).selectedIndex = data["locatorLineNumberTerminal"] - 1;
}

function updatePageLocator(locator, data) {
    var elementId = locator + "Value";
    document.getElementById(elementId).innerHTML = data.toString();
}

function updatePageBdc(bdc, data) {
    var elementId;

    if (data == 0) {
        //forward
        elementId = bdc + "Forward";
        document.getElementById(elementId).checked = true;
    } else if (data == 1) {
        //reverse
        elementId = bdc + "Reverse";
        document.getElementById(elementId).checked = true;
    } else if (data == 2) {
        //release
        elementId = bdc + "Release";
        document.getElementById(elementId).checked = true;
    } else {
        console.log("Error: wrong bdc status: " + bdc + " " + data.toString());
    }
}

function updatePageDeviceConnected(bConnected) {
    document.getElementById("deviceConnected").checked = bConnected;
}

function updatePageDevicePowered(bPowered) {
    document.getElementById("devicePowered").checked = bPowered;
}

function updatePageDeviceFuseOk(bOk) {
    document.getElementById("deviceFuseOk").checked = bOk;
}

function updatePageOptPowered(bPowered) {
    document.getElementById("optPowered").checked = bPowered;
}

function updatePageBdcPowered(bPowered) {
    document.getElementById("bdcsPowered").checked = bPowered;
}

function updatePageStepperPowered(bPowered) {
    document.getElementById("steppersPowered").checked = bPowered;
}

function updatePageSmartCardSlowlyPlaceStartZ(offset) {
    document.getElementById("smartCardSlowlyPlaceStartZ").value = offset.toString();
}

function updatePageSmartCardSlowlyPlaceEndZ(offset) {
    document.getElementById("smartCardSlowlyPlaceEndZ").value = offset.toString();
}

function updatePageSmartCardFetchOffset(offset) {
    document.getElementById("smartCardFetchOffset").value = offset.toString();
}

function updatePageSmartCardReleaseOffsetZ(offset) {
    document.getElementById("smartCardReleaseOffsetZ").value = offset.toString();
}

function updatePageSmartCardInsertExtra(offset) {
    document.getElementById("smartCardInsertExtra").value = offset.toString();
}

function updatePageSmartCardReaderSlowInsertEndY(offset) {
    document.getElementById("smartCardReaderSlowInsertEndY").value = offset.toString();
}

function createCoordinateLineWithIndex(type, x, y, z, w, index) {
    var html;
    var radioId = "coordinateItem_" + type + "_" + index.toString();

    html = "<input name=\"coordinateSelection\" id=\"" + radioId + "\" type=\"radio\"></input>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\">" + type + "_" + index.toString() + "</label>";
    html += "<label>: </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_x\">" + x.toString() + "</label>";
    html += "<label>, </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_y\">" + y.toString() + "</label>";
    html += "<label>, </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_z\">" + z.toString() + "</label>";
    html += "<label>, </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_w\">" + w.toString() + "</label>";
    html += "<br>";

    return html;
}

function createCoordinateLine(type, x, y, z, w) {
    var html;
    var radioId = "coordinateItem_" + type;

    html = "<input name=\"coordinateSelection\" id=\"" + radioId + "\" type=\"radio\"></input>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\">" + type + "</label>";
    html += "<label>: </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_x\">" + x.toString() + "</label>";
    html += "<label>, </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_y\">" + y.toString() + "</label>";
    html += "<label>, </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_z\">" + z.toString() + "</label>";
    html += "<label>, </label>";
    html += "<label class=\"coordinateItem\" for=\"" + radioId + "\" id=\"" + radioId + "_w\">" + w.toString() + "</label>";
    html += "<br>";

    return html;
}

function updatePageCoordinates(serverResponse) {
    var html = "";
    var i;

    var coorSmartCardGate = serverResponse["coordinateSmartCardGate"];
    html = createCoordinateLine("SmartCardGate", coorSmartCardGate.x, coorSmartCardGate.y, coorSmartCardGate.z, coorSmartCardGate.w);

    var coorSmartCards = serverResponse["coordinateSmartCards"];
    for (i = 0; i < coorSmartCards.length; i++) {
        var index = coorSmartCards[i].index;
        var x = coorSmartCards[i].x;
        var y = coorSmartCards[i].y;
        var z = coorSmartCards[i].z;
        var w = coorSmartCards[i].w;
        html += createCoordinateLineWithIndex("SmartCard", x, y, z, w, index);
    }

    var coorPedKeyGate = serverResponse["coordinatePedKeyGate"];
    html += createCoordinateLine("PedKeyGate", coorPedKeyGate.x, coorPedKeyGate.y, coorPedKeyGate.z, coorPedKeyGate.w);

    var coorPedKeys = serverResponse["coordinatePedKeys"];
    var coorPedKeysPressed = serverResponse["coordinatePedKeysPressed"];
    for (i = 0;
        (i < coorPedKeys.length) || (i < coorPedKeysPressed.length); i++) {
        if (i < coorPedKeys.length) {
            var index = coorPedKeys[i].index;
            var x = coorPedKeys[i].x;
            var y = coorPedKeys[i].y;
            var z = coorPedKeys[i].z;
            var w = coorPedKeys[i].w;
            if (typeof index === 'undefined') {
                continue;
            }
            if (typeof x === 'undefined') {
                continue;
            }
            if (typeof y === 'undefined') {
                continue;
            }
            if (typeof z === 'undefined') {
                continue;
            }
            if (typeof w === 'undefined') {
                continue;
            }
            html += createCoordinateLineWithIndex("PedKey", x, y, z, w, index + 1);
        }
        if (i < coorPedKeysPressed.length) {
            var index = coorPedKeysPressed[i].index;
            var x = coorPedKeysPressed[i].x;
            var y = coorPedKeysPressed[i].y;
            var z = coorPedKeysPressed[i].z;
            var w = coorPedKeysPressed[i].w;
            if (typeof index === 'undefined') {
                continue;
            }
            if (typeof x === 'undefined') {
                continue;
            }
            if (typeof y === 'undefined') {
                continue;
            }
            if (typeof z === 'undefined') {
                continue;
            }
            if (typeof w === 'undefined') {
                continue;
            }
            html += createCoordinateLineWithIndex("PedKeyPressed", x, y, z, w, index + 1);
        }
    }

    var coorSoftKeyGate = serverResponse["coordinateSoftKeyGate"];
    html += createCoordinateLine("SoftKeyGate", coorSoftKeyGate.x, coorSoftKeyGate.y, coorSoftKeyGate.z, coorSoftKeyGate.w);

    var coorSoftKeys = serverResponse["coordinateSoftKeys"];
    var coorSoftKeysPressed = serverResponse["coordinateSoftKeysPressed"];
    for (i = 0;
        (i < coorSoftKeys.length) || (i < coorSoftKeysPressed.length); i++) {
        if (i < coorSoftKeys.length) {
            var index = coorSoftKeys[i].index;
            var x = coorSoftKeys[i].x;
            var y = coorSoftKeys[i].y;
            var z = coorSoftKeys[i].z;
            var w = coorSoftKeys[i].w;
            html += createCoordinateLineWithIndex("SoftKey", x, y, z, w, index);
        }
        if (i < coorSoftKeysPressed.length) {
            var index = coorSoftKeysPressed[i].index;
            var x = coorSoftKeysPressed[i].x;
            var y = coorSoftKeysPressed[i].y;
            var z = coorSoftKeysPressed[i].z;
            var w = coorSoftKeysPressed[i].w;
            html += createCoordinateLineWithIndex("SoftKeyPressed", x, y, z, w, index);
        }
    }

    var coorAssistKeyGate = serverResponse["coordinateAssistKeyGate"];
    html += createCoordinateLine("AssistKeyGate", coorAssistKeyGate.x, coorAssistKeyGate.y, coorAssistKeyGate.z, coorAssistKeyGate.w);

    var coorAssistKeys = serverResponse["coordinateAssistKeys"];
    var coorAssistKeysPressed = serverResponse["coordinateAssistKeysPressed"];
    for (i = 0;
        (i < coorAssistKeys.length) || (i < coorAssistKeysPressed.length); i++) {
        if (i < coorAssistKeys.length) {
            var index = coorAssistKeys[i].index;
            var x = coorAssistKeys[i].x;
            var y = coorAssistKeys[i].y;
            var z = coorAssistKeys[i].z;
            var w = coorAssistKeys[i].w;
            html += createCoordinateLineWithIndex("AssistKey", x, y, z, w, index);
        }
        if (i < coorAssistKeysPressed.length) {
            var index = coorAssistKeysPressed[i].index;
            var x = coorAssistKeysPressed[i].x;
            var y = coorAssistKeysPressed[i].y;
            var z = coorAssistKeysPressed[i].z;
            var w = coorAssistKeysPressed[i].w;
            html += createCoordinateLineWithIndex("AssistKeyPressed", x, y, z, w, index);
        }
    }

    var coorTouchScreenKeyGate = serverResponse["coordinateTouchScreenKeyGate"];
    html += createCoordinateLine("TouchScreenKeyGate", coorTouchScreenKeyGate.x, coorTouchScreenKeyGate.y, coorTouchScreenKeyGate.z, coorTouchScreenKeyGate.w);

    var coorTouchScreenKeys = serverResponse["coordinateTouchScreenKeys"];
    var coorTouchScreenKeysPressed = serverResponse["coordinateTouchScreenKeysPressed"];
    for (i = 0;
        (i < coorTouchScreenKeys.length) || (i < coorTouchScreenKeysPressed.length); i++) {
        if (i < coorTouchScreenKeys.length) {
            var index = coorTouchScreenKeys[i].index;
            var x = coorTouchScreenKeys[i].x;
            var y = coorTouchScreenKeys[i].y;
            var z = coorTouchScreenKeys[i].z;
            var w = coorTouchScreenKeys[i].w;
            html += createCoordinateLineWithIndex("TouchScreenKey", x, y, z, w, index);
        }
        if (i < coorAssistKeysPressed.length) {
            var index = coorAssistKeysPressed[i].index;
            var x = coorAssistKeysPressed[i].x;
            var y = coorAssistKeysPressed[i].y;
            var z = coorAssistKeysPressed[i].z;
            var w = coorAssistKeysPressed[i].w;
            html += createCoordinateLineWithIndex("coorTouchScreenKeysPressed", x, y, z, w, index);
        }
    }

    var coorSmartCardReaderGate = serverResponse["coordinateSmartCardReaderGate"];
    html += createCoordinateLine("SmartCardReaderGate", coorSmartCardReaderGate.x, coorSmartCardReaderGate.y, coorSmartCardReaderGate.z, coorSmartCardReaderGate.w);

    var coorSmartCardReader = serverResponse["coordinateSmartCardReader"];
    html += createCoordinateLine("SmartCardReader", coorSmartCardReader.x, coorSmartCardReader.y, coorSmartCardReader.z, coorSmartCardReader.w);

    var coorBarCodeReaderGate = serverResponse["coordinateBarCodeReaderGate"];
    html += createCoordinateLine("BarCodeReaderGate", coorBarCodeReaderGate.x, coorBarCodeReaderGate.y, coorBarCodeReaderGate.z, coorBarCodeReaderGate.w);

    var coorBarCodeReader = serverResponse["coordinateBarCodeReader"];
    html += createCoordinateLine("BarCodeReader", coorBarCodeReader.x, coorBarCodeReader.y, coorBarCodeReader.z, coorBarCodeReader.w);

    var coorContactlessReaderGate = serverResponse["coordinateContactlessReaderGate"];
    html += createCoordinateLine("ContactlessReaderGate", coorContactlessReaderGate.x, coorContactlessReaderGate.y, coorContactlessReaderGate.z, coorContactlessReaderGate.w);

    var coorContactlessReader = serverResponse["coordinateContactlessReader"];
    html += createCoordinateLine("ContactlessReader", coorContactlessReader.x, coorContactlessReader.y, coorContactlessReader.z, coorContactlessReader.w);

    var coorSafe = serverResponse["coordinateSafe"];
    html += createCoordinateLine("Safe", coorSafe.x, coorSafe.y, coorSafe.z, coorSafe.w);

    document.getElementById("coordinateList").innerHTML = html;
}

function updatePage(serverResponse) {
    for (key in serverResponse) {
        if (key === "stepper0") {
            updatePageStepper(key, serverResponse[key]);
        } else if (key === "stepper1") {
            updatePageStepper(key, serverResponse[key]);
        } else if (key === "stepper2") {
            updatePageStepper(key, serverResponse[key]);
        } else if (key === "stepper3") {
            updatePageStepper(key, serverResponse[key]);
        } else if (key === "stepper4") {
            updatePageStepper(key, serverResponse[key]);
        } else if (key === "locator0") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "locator1") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "locator2") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "locator3") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "locator4") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "locator5") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "locator6") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "locator7") {
            updatePageLocator(key, serverResponse[key]);
        } else if (key === "bdc0") {
            updatePageBdc("bdc_0_", serverResponse[key]);
        } else if (key === "bdc1") {
            updatePageBdc("bdc_1_", serverResponse[key]);
        } else if (key === "bdc2") {
            updatePageBdc("bdc_2_", serverResponse[key]);
        } else if (key === "bdc3") {
            updatePageBdc("bdc_3_", serverResponse[key]);
        } else if (key === "bdc4") {
            updatePageBdc("bdc_4_", serverResponse[key]);
        } else if (key === "bdc5") {
            updatePageBdc("bdc_5_", serverResponse[key]);
        } else if (key === "deviceConnected") {
            updatePageDeviceConnected(serverResponse[key]);
        } else if (key === "devicePowered") {
            updatePageDevicePowered(serverResponse[key]);
        } else if (key === "deviceFuseOk") {
            updatePageDeviceFuseOk(serverResponse[key]);
        } else if (key === "optPowered") {
            updatePageOptPowered(serverResponse[key]);
        } else if (key === "bdcPowered") {
            updatePageBdcPowered(serverResponse[key]);
        } else if (key === "stepperPowered") {
            updatePageStepperPowered(serverResponse[key]);
        } else if (key === "smartCardSlowlyPlaceStartZ") {
            updatePageSmartCardSlowlyPlaceStartZ(serverResponse[key]);
        } else if (key === "smartCardSlowlyPlaceEndZ") {
            updatePageSmartCardSlowlyPlaceEndZ(serverResponse[key]);
        } else if (key === "smartCardFetchOffset") {
            updatePageSmartCardFetchOffset(serverResponse[key]);
        } else if (key === "smartCardReleaseOffset") {
            updatePageSmartCardReleaseOffsetZ(serverResponse[key]);
        } else if (key === "smartCardInsertExtra") {
            updatePageSmartCardInsertExtra(serverResponse[key]);
        } else if (key === "smartCardReaderSlowInsertEndY") {
            updatePageSmartCardReaderSlowInsertEndY(serverResponse[key]);
        }
    }

    updatePageCoordinates(serverResponse);
}

function moveStepper(stepper, forward, steps) {
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'stepperMove');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                console.log(JSON.stringify(jsonObj)); // 'This is the output.'
                updatePage(jsonObj);
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };

    var parameters = {};
    //stepper index
    if (stepper === "stepper0") {
        parameters["index"] = 0;
    } else if (stepper === "stepper1") {
        parameters["index"] = 1;
    } else if (stepper === "stepper2") {
        parameters["index"] = 2;
    } else if (stepper === "stepper3") {
        parameters["index"] = 3;
    } else {
        alert("Wrong stepper: " + stepper);
        return;
    }
    //direction
    if (forward == true) {
        parameters["forward"] = true;
    } else {
        parameters["forward"] = false;
    }
    parameters["steps"] = steps;

    xhr.send(JSON.stringify(parameters));
}

function setBdc(index, action) {
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'bdc');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                console.log(JSON.stringify(jsonObj)); // 'This is the output.'
                updatePage(jsonObj);
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };

    var parameters = {};
    parameters["index"] = index;
    parameters["action"] = action;

    xhr.send(JSON.stringify(parameters));
}

function queryDevice() {
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'query');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                console.log(JSON.stringify(jsonObj)); // 'This is the output.'
                updatePage(jsonObj);
            } else {
                console.log('Error: ' + xhr.status); // An error occurred during the request.
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };

    xhr.send();
}


function onStepperDecrease(stepper) {
    var stepsId = stepper + "Steps";
    var steps = document.getElementById(stepsId).value;
    moveStepper(stepper, false, steps);
}

function onStepperIncrease(stepper) {
    var stepsId = stepper + "Steps";
    var steps = document.getElementById(stepsId).value;
    moveStepper(stepper, true, steps);
}

function onStepperConfigMovement(index) {
    var lowClks = document.getElementById("stepper" + index + "LowClks").value;
    var highClks = document.getElementById("stepper" + index + "HighClks").value;
    var accelerationBuffer = document.getElementById("stepper" + index + "AccelerationBuffer").value;
    var accelerationBufferDecrement = document.getElementById("stepper" + index + "AccelerationBufferDecrement").value;
    var decelerationBuffer = document.getElementById("stepper" + index + "DecelerationBuffer").value;
    var decelerationBufferIncrement = document.getElementById("stepper" + index + "DecelerationBufferIncrement").value;

    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'stepperConfigMovement');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                console.log(JSON.stringify(jsonObj)); // 'This is the output.'
                updatePage(jsonObj);
            } else {
                alert('Error: ' + xhr.status + ": " + xhr.statusText); // An error occurred during the request.
            }
        }
    };

    var parameters = {};
    parameters["index"] = parseInt(index);
    parameters["lowClks"] = parseInt(lowClks);
    parameters["highClks"] = parseInt(highClks);
    parameters["accelerationBuffer"] = parseInt(accelerationBuffer);
    parameters["accelerationBufferDecrement"] = parseInt(accelerationBufferDecrement);
    parameters["decelerationBuffer"] = parseInt(decelerationBuffer);
    parameters["decelerationBufferIncrement"] = parseInt(decelerationBufferIncrement);

    xhr.send(JSON.stringify(parameters));
}

function onStepperConfigHome(index) {
    var locatorIndex = parseInt(document.getElementById("stepper" + index + "Locator").selectedIndex);
    var locatorLineNumberStart = parseInt(document.getElementById("stepper" + index + "LocatorLineNumberStart").selectedIndex) + 1;
    var locatorLineNumberTerminal = parseInt(document.getElementById("stepper" + index + "LocatorLineNumberTerminal").selectedIndex) + 1;

    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'stepperConfigHome');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                var jsonObj = xhr.response;
                console.log(JSON.stringify(jsonObj)); // 'This is the output.'
                updatePage(jsonObj);
            } else {
                alert('Error: ' + xhr.status + ": " + xhr.statusText); // An error occurred during the request.
            }
        }
    };

    var parameters = {};
    parameters["index"] = parseInt(index);
    parameters["locator"] = locatorIndex;
    parameters["locatorLineNumberStart"] = locatorLineNumberStart;
    parameters["locatorLineNumberTerminal"] = locatorLineNumberTerminal;

    xhr.send(JSON.stringify(parameters));
}

function saveCoordinate() {
    console.log(">>saveCoordinate()");
    //find out which coordinate radio is selected
    var radioId = {};
    var selectedRadio = {};
    var elements = document.getElementsByName("coordinate");
    for (var i = 0; i < elements.length; i++) {
        if (elements[i].checked == true) {
            radioId = elements[i].id;
            break;
        }
    }
    if (radioId.length < 1) {
        alert("a coordinate type has to be selected");
        return;
    }
    console.log(radioId + " is selected"); {
        var items = radioId.split("_");
        selectedRadio["signature"] = items[0];
        selectedRadio["coordinateType"] = items[1];
        selectedRadio["index"] = parseInt(items[2]);
    }

    if (selectedRadio.signature !== "coordinate") {
        console.log("Error: wrong radio Id");
        return;
    }

    //set command
    var command = {};
    command["coordinateType"] = selectedRadio.coordinateType;
    command["data"] = 0;
    switch (selectedRadio.coordinateType) {
        case "safe":
        case "smartCardGate":
        case "smartCardReader":
        case "smartCardReaderGate":
        case "pedKeyGate":
        case "softKeyGate":
        case "touchScreenGate":
        case "assistKeyGate":
        case "contactlessReader":
        case "contactlessReaderGate":
        case "barcodeReader":
        case "barcodeReaderGate":
            break;

        case "smartCard":
            command["data"] = selectedRadio["index"];
            break;

        case "pedKey":
        case "softKey":
        case "assistKey":
        case "touchScreenKey:":
            {
                var keyUp = document.getElementById("coordinate_keyState_0").checked;
                var keyDown = document.getElementById("coordinate_keyState_1").checked;

                if (!keyUp && !keyDown) {
                    alert("UP or DOWN has to be selected");
                    return;
                }
                //update coordinate type
                if (keyDown) {
                    command["coordinateType"] = command["coordinateType"] + "Pressed";
                }
                command["data"] = selectedRadio.index;
                break;
            }

        case "smartCardSlowlyPlaceStartZ":
        case "smartCardSlowlyPlaceEndZ":
        case "smartCardFetchOffset":
        case "smartCardReleaseOffsetZ":
        case "smartCardInsertExtra":
        case "smartCardReaderSlowInsertEndY":
            command["data"] = parseInt(document.getElementById(selectedRadio.coordinateType).value);
            break;

        case "maximum":
            command["data"] = selectedRadio.index;
            break;

        default:
            alert("unknown coordinate type");
            return;
    }

    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'saveCoordinate');

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log("saveCoordinate succeeded");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onCoordinateItem(type, index) {
    var updateDest = true;
    var x, y, z, w;

    if ((type === "SmartCardGate") ||
        (type === "PedKeyGate") ||
        (type === "SoftKeyGate") ||
        (type === "AssistKeyGate") ||
        (type === "TouchScreenKeyGate") ||
        (type === "SmartCardReaderGate") ||
        (type === "SmartCardReader") ||
        (type === "BarCodeReaderGate") ||
        (type === "BarCodeReader") ||
        (type === "ContactlessReaderGate") ||
        (type === "ContactlessReader") ||
        (type === "Safe")) {
        x = document.getElementById("coordinateItem_" + type + "_x").innerText;
        y = document.getElementById("coordinateItem_" + type + "_y").innerText;
        z = document.getElementById("coordinateItem_" + type + "_z").innerText;
        w = document.getElementById("coordinateItem_" + type + "_w").innerText;
    } else if ((type === "SmartCard") ||
        (type === "PedKey") ||
        (type === "PedKeyPressed") ||
        (type === "SoftKey") ||
        (type === "SoftKeyPressed") ||
        (type === "AssistKey") ||
        (type === "AssistKeyPressed") ||
        (type === "TouchScreenKey") ||
        (type === "TouchScreenKeyPressed")) {
        x = document.getElementById("coordinateItem_" + type + "_" + index.toString() + "_x").innerText;
        y = document.getElementById("coordinateItem_" + type + "_" + index.toString() + "_y").innerText;
        z = document.getElementById("coordinateItem_" + type + "_" + index.toString() + "_z").innerText;
        w = document.getElementById("coordinateItem_" + type + "_" + index.toString() + "_w").innerText;
    } else if (type === "to") {
        updateDest = false;

        var command = {};
        command["x"] = document.getElementById("destCoordinateX").innerText;
        command["y"] = document.getElementById("destCoordinateY").innerText;
        command["z"] = document.getElementById("destCoordinateZ").innerText;
        command["w"] = document.getElementById("destCoordinateW").innerText;
        command["direct"] = true;

        var xhr = new XMLHttpRequest();
        xhr.responseType = "json";
        xhr.open('POST', 'toCoordinate');

        xhr.onreadystatechange = function() {
            var DONE = 4; // readyState 4 means the request is done.
            var OK = 200; // status 200 is a successful return.
            if (xhr.readyState === DONE) {
                console.log("response is available");
                console.log("response type: " + xhr.responseType);

                if (xhr.status === OK) {
                    console.log("toCoordinate succeeded");
                } else {
                    alert('Error: failed to go to: ' + JSON.stringify(command)); // An error occurred during the request.
                }
            }
        };
        xhr.send(JSON.stringify(command));
    } else if (type === "indirectTo") {
        updateDest = false;

        var command = {};
        command["x"] = document.getElementById("destCoordinateX").innerText;
        command["y"] = document.getElementById("destCoordinateY").innerText;
        command["z"] = document.getElementById("destCoordinateZ").innerText;
        command["w"] = document.getElementById("destCoordinateW").innerText;
        command["direct"] = false;

        var xhr = new XMLHttpRequest();
        xhr.responseType = "json";
        xhr.open('POST', 'toCoordinate');

        xhr.onreadystatechange = function() {
            var DONE = 4; // readyState 4 means the request is done.
            var OK = 200; // status 200 is a successful return.
            if (xhr.readyState === DONE) {
                console.log("response is available");
                console.log("response type: " + xhr.responseType);

                if (xhr.status === OK) {
                    console.log("toCoordinate succeeded");
                } else {
                    alert('Error: failed to go to: ' + JSON.stringify(command)); // An error occurred during the request.
                }
            }
        };
        xhr.send(JSON.stringify(command));
    } else {
        console.log("Error: unknown coordinateItem type: " + type);
        updateDest = false;
    }

    if (updateDest) {
        document.getElementById("destCoordinateX").innerText = x;
        document.getElementById("destCoordinateY").innerText = y;
        document.getElementById("destCoordinateZ").innerText = z;
        document.getElementById("destCoordinateW").innerText = w;
    }
}

function onPower(target, on) {
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'power');

    var command = {};
    command["target"] = target;
    command["on"] = on;

    xhr.onreadystatechange = function() {
        var DONE = 4; // readyState 4 means the request is done.
        var OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            console.log("response is available");
            console.log("response type: " + xhr.responseType);

            if (xhr.status === OK) {
                console.log("saveCoordinate succeeded");
            } else {
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

function onKey(keyIndex) {
    var xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', 'key');

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
                alert('Error: ' + xhr.status + ":" + xhr.statusText); // An error occurred during the request.
            }
        }
    };
    xhr.send(JSON.stringify(command));
}

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