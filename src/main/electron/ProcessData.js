const cluster_type = {
	household: "household",
	work: "work",
	school: "school",
	primary_community: "primary_community",
	secondary_community: "secondary_community",
};

function getDecoratedData(data) {
	return {
		parsed_data: data,

		getTotalPopulation: function() {
			// Loop over the households, they always contain everybody (except travellers)
			var pop_count = 0;
			for(var i = 0; i < this.parsed_data.features.length; i++) {
				if (this.parsed_data.features[i].properties.type == cluster_type.household) {
					pop_count += this.parsed_data.features[i].properties.size;
				}
				
			}
			// TODO add the things below
			// pop_count += travellers_amount;
			return pop_count;
		},

		getTotalInfected: function() {
			// Loop over the households, they always contain everybody (except travellers)
			var infected_count = 0;
			for(var i = 0; i < this.parsed_data.features.length; i++) {
				if (this.parsed_data.features[i].properties.type == cluster_type.household) {
					infected_count += this.parsed_data.features[i].properties.infected;
				}
			}

			// TODO add the things below
			// infected_count += this.clusters.travellers_infected;
			return infected_count;
		},

		getInfectedFraction: function() {
			return this.getTotalInfected() / this.getTotalPopulation();
		},

		getHealthyFraction: function() {
			return 1.0 - this.getInfectedFraction();
		},

		getCluster: function(id, type) {
			for (var i = 0; i < this.parsed_data.features.length; i++) {
				if (this.parsed_data.features[i].properties.type == type && this.parsed_data.features[i].properties.id == id) {
					return this.parsed_data.features[i];
				}
			}
			return undefined;
		},

		getClusters: function(type) {
			var wanted_clusters = [];
			for (var i = 0; i < this.parsed_data.features.length; i++) {
				if (this.parsed_data.features[i].properties.type == type) {
					wanted_clusters.push(this.parsed_data.features[i]);
				}
			}
			return 	{
						type: "FeatureCollection",
						features: wanted_clusters
					};
		}
	};
}

function getClusterInfectedCourse(data, type, id) {
	/// data is iterable
	/// type is the type of cluster you want to inspect
	/// id is the id of the cluster you want to inspect
	var timeline = [];

	for (var i = 0; i < data.length; i++) {
		var current_data = getDecoratedData(data[i]);
		var current_cluster = current_data.getCluster(type, id);

		timeline.push({
			size: current_cluster.properties.size,
			infected: current_cluster.properties.infected
		});
	}

	return timeline;
}

function getTotalInfectedCourse(data, type, id) {
	/// data is iterable
	/// type is the type of cluster you want to inspect
	/// id is the id of the cluster you want to inspect
	var timeline = [];

	for (var i = 0; i < data.length; i++) {
		var current_data = getDecoratedData(data[i]);
		var current_cluster = current_data.getCluster(type, id);

		timeline.push({
			size: current_data.getTotalPopulation(),
			infected: current_data.getTotalInfected()
		});
	}

	return timeline;
}