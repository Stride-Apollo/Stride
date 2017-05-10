

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
	}
	return cluster_data;
}

