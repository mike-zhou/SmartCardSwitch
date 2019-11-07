const BARCODE_RETRIEVING_INTERVAL = 100;



function barcodeUpdateTimer() 
{
    
    let image = document.getElementById("barcode");
    image.src = "/barcodeImage"
}

function initPage()
{
    setInterval(barcodeUpdateTimer, BARCODE_RETRIEVING_INTERVAL);
}

document.addEventListener("DOMContentLoaded", initPage);
