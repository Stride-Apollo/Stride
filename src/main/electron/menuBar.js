function sliderToLabel(sliderId) {
	if (sliderId == "zoomedMinSlider") {
		return "zoomedMinLabel";
	} else if (sliderId == "zoomedMaxSlider") {
		return "zoomedMaxLabel";
	} else if (sliderId == "speedSlider") {
		return "speedLabel";
	} else if (sliderId == "opacitySlider") {
		return "opacityLabel";
	} else {
		return "";
	}
}

function sliderToPrefix(sliderId) {
	if (sliderId == "zoomedMinSlider") {
		return "Zoomed minimum: ";
	} else if (sliderId == "zoomedMaxSlider") {
		return "Zoomed maximum: ";
	} else if (sliderId == "speedSlider") {
		return "Animation speed (ms): ";
	} else if (sliderId == "opacitySlider") {
		return "Opacity: ";
	} else {
		return "";
	}
}



function setLabel(sliderId) {
	var prefix = sliderToPrefix(sliderId);
	var labelId = sliderToLabel(sliderId);
	document.getElementById(labelId).innerHTML = prefix + document.getElementById(sliderId).value;
	adjustSlider(sliderId);
}

function pickedColor(id) {
	var picker = document.getElementById(id);
	console.log("Picked color with ID: " + id + " Value: " + picker.value);
	console.log("Please check menuBar.js");
}

function clickSimButton(id) {
	var btn = document.getElementById(id);
	console.log("Clicked button with ID: " + id);
	console.log("Please check menuBar.js");
}

function adjustSlider(id) {
	var slider = document.getElementById(id);
	console.log("Slided slider with ID: " + id + " value: " + slider.value.toString() + " (there will probably be a lot of these logs stijn)");
	console.log("Please check menuBar.js");
}


