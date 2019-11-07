const BARCODE_RETRIEVING_INTERVAL = 100;

function barcodeUpdateTimer() 
{
    let xhr = new XMLHttpRequest();
    xhr.responseType = "json";
    xhr.open('POST', '/barcodeImageQuery');

    xhr.onreadystatechange = function() {
        let DONE = 4; // readyState 4 means the request is done.
        let OK = 200; // status 200 is a successful return.
        if (xhr.readyState === DONE) {
            if (xhr.status === OK) {
                let reply = xhr.response;
                let image = document.getElementById("barcode");
                image.src = reply["barcodeName"];
            }
        }
    }

    xhr.send();
}

function initPage()
{
    setInterval(barcodeUpdateTimer, BARCODE_RETRIEVING_INTERVAL);
}

document.addEventListener("DOMContentLoaded", initPage);
