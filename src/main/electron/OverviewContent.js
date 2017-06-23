var app = angular.module('OverviewApp', []);

app.config(['$locationProvider', function($locationProvider) {
	$locationProvider.html5Mode(true);
}]);

app.controller('OverViewController', ['$scope', '$location', function($scope, $location) {

	console.log($location.url());
	console.log($location.search().data);

	var fs = require('fs');
	var content;
	var data = fs.readFileSync($location.search().data, 'utf8');

	$scope.currentDay = $location.search().currentDay;
	content = parseCSVFile(data);
	$scope.clusters = []
	for (var i in content.features) {
		var cluster = {};
		cluster.ID = content.features[i].properties.id;
		cluster.size = parseFloat(content.features[i].properties.size);
		cluster.infected = parseFloat(content.features[i].properties.infected);
		cluster.infected_percent = parseFloat(content.features[i].properties.infected_percent);
		cluster.cluster_type = content.features[i].properties.type;
		cluster.lat = parseFloat(content.features[i].geometry.coordinates[1]);
		cluster.lon = parseFloat(content.features[i].geometry.coordinates[0]);
		if (cluster.infected_percent == -1) {
			cluster.infected_percent = 0;
		}
		$scope.clusters.push(cluster);
	}


	// Load clusters
	$scope.clusterClick = function(ID) {
		var dir = $location.search().directory;
		var config = {directory: dir.slice(__dirname.length)};
		loadCluster(ID, config, filenames, $location.search().currentDay);
	}

	var files = [];
	var filenames = [];
	var dircontent = fs.readdirSync($location.search().directory);
	dircontent.forEach( function (file){
		var data = fs.readFileSync($location.search().directory + "/" + file, 'utf8');
		files.push(data);
		filenames.push(file);
	});

	var clusterEvolution = getTotalInfectedCourse(files);

	// Load population files
	var pop_files = [];
	var pop_filenames = [];
	var pop_dir = $location.search().population;
	dircontent = fs.readdirSync(pop_dir);
	dircontent.forEach( function (file){
		var data = fs.readFileSync(pop_dir + "/" + file, 'utf8');
		pop_files.push(data);
		pop_filenames.push(file);
	});

	console.log(pop_files);
	console.log(pop_filenames);


	$scope.population_size = clusterEvolution[$scope.currentDay].size;
	$scope.infected = clusterEvolution[$scope.currentDay].infected;
	$scope.total_cluster = $scope.clusters.length;

	var myPlot = document.getElementById("graphInfected");
	Plotly.purge(myPlot);

	var infected = scatterplot('Infection Count');
	var sizes = scatterplot('Cluster Sizes');

	for (i=0; i < clusterEvolution.length; i++) {
		infected.x.push(i);
		sizes.x.push(i);
		infected.y.push(clusterEvolution[i].infected);
		sizes.y.push(clusterEvolution[i].size);
	}
	console.log(infected.y);

	var data = [sizes,infected];

	var plotWidth = 800;
	var plotHeight = 450;
	var layout = {
		title: 'Evolution Of Population Illness',
		width: plotWidth,
		height: plotHeight,
		paper_bgcolor: "rgba(255,0,0,0)",
		plot_bgcolor: "rgba(255,0,0,0)",
		legend: {
			y: 0.5,
			traceorder: 'reversed',
			font: {size: 16},
			yref: 'paper'
		}};

	Plotly.newPlot('graphInfected', data, layout).then(function() {
		window.requestAnimationFrame(function() {
			window.requestAnimationFrame(function() {
				resizeWindow();

				document.getElementById("graphInfected").style="margin: auto; width: " + plotWidth + "px; height: " + plotHeight + "px;";
			});
		});
	});
}]);
