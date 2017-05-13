function hasClass(el, className) {
	if (el.classList)
		return el.classList.contains(className)
	else
		return !!el.className.match(new RegExp('(\\s|^)' + className + '(\\s|$)'))
}

function addClass(el, className) {
	if (el.classList)
		el.classList.add(className)
	else if (!hasClass(el, className)) el.className += " " + className
}

function removeClass(el, className) {
	if (el.classList)
		el.classList.remove(className)
	else if (hasClass(el, className)) {
		var reg = new RegExp('(\\s|^)' + className + '(\\s|$)')
		el.className=el.className.replace(reg, ' ')
	}
}

function parseCSVFile(file_text) {
	var cluster_data = {}
	cluster_data.type = "FeatureCollection";
	cluster_data.features = []
	var lines = file_text.split("\n")

	var format = lines[0].split(",");
	function getPropertyIndex(prop) {
		return format.indexOf(prop);
	}


	lines = lines.slice(1, lines.length-1);

	for (var i in lines) {
		var data_cluster = lines[i].split(",");
		cluster_data.features.push({});
		cluster_data.features[i].type = "Feature";

		cluster_data.features[i].geometry = {}
		cluster_data.features[i].geometry.type = "Point";
		cluster_data.features[i].geometry.coordinates = [parseFloat(data_cluster[getPropertyIndex("lon")]), parseFloat(data_cluster[getPropertyIndex("lat")])];

		cluster_data.features[i].properties = {}
		cluster_data.features[i].properties.id = parseInt(data_cluster[getPropertyIndex("id")]);
		cluster_data.features[i].properties.size = parseInt(data_cluster[getPropertyIndex("size")]);
		cluster_data.features[i].properties.infected = parseInt(data_cluster[getPropertyIndex("infected")]);
		cluster_data.features[i].properties.infected_percent = parseFloat(data_cluster[getPropertyIndex("infected_percent")]);
		cluster_data.features[i].properties.type = data_cluster[getPropertyIndex("type")];
	}
	return cluster_data;
}


function getClusterInfectedData(file_text, id) {
	var lines = file_text.split("\n")

	var format = lines[0].split(",");
	function getPropertyIndex(prop) {
		return format.indexOf(prop);
	}
	lines = lines.slice(1, lines.length-1);

	for (var i in lines) {
		var data_cluster = lines[i].split(",");
		if (id == parseInt(data_cluster[getPropertyIndex("id")])) {
			cluster_data = {
				size: parseInt(data_cluster[getPropertyIndex("size")]),
				infected: parseInt(data_cluster[getPropertyIndex("infected")])
			}
			return cluster_data;
		}
	}
	return undefined;
}
