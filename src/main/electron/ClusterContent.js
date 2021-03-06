var app = angular.module('ClusterApp', []);

app.config(['$locationProvider', function($locationProvider) {
	$locationProvider.html5Mode(true);
}]);
app.controller('ClusterController', ['$scope', '$location', function($scope, $location) {

	console.log($location.url());
	console.log($location.search().data);
	console.log($location.search().cluster);

	var fs = require('fs');
	var content;
	var data = fs.readFileSync($location.search().data, 'utf8');
	content = parseCSVFile(data);
	for (var i in content.features) {
		if (content.features[i].properties.id == $location.search().cluster) {
			$scope.ID = content.features[i].properties.id;
			$scope.size = parseFloat(content.features[i].properties.size);
			$scope.infected = parseFloat(content.features[i].properties.infected);
			$scope.infected_percent = parseFloat(content.features[i].properties.infected_percent);
			$scope.cluster_type = content.features[i].properties.type;
			$scope.lat = parseFloat(content.features[i].geometry.coordinates[1]);
			$scope.lon = parseFloat(content.features[i].geometry.coordinates[0]);
			if ($scope.infected_percent == -1) {
				$scope.infected_percent = 0;
			}
		}
	}
	$scope.title = "Cluster " + $scope.ID;
	$scope.currentDay = $location.search().currentDay;

	var files = [];
	var filenames = [];
	var clusterEvolution = [];
	var dircontent = fs.readdirSync($location.search().directory);
	dircontent.forEach( function (file){
		var data = fs.readFileSync($location.search().directory + "/" + file, 'utf8');
		files.push(data);
		filenames.push(file);
	});

	clusterEvolution = getClusterInfectedCourse(files, $scope.ID);

	var myPlot = document.getElementById("graph");
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
		title: 'Cluster Evolution For Entire Simulation',
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

	Plotly.newPlot('graph', data, layout).then(function() {
		window.requestAnimationFrame(function() {
			window.requestAnimationFrame(function() {
				resizeWindow();

				document.getElementById("graph").style="margin: auto; width: " + plotWidth + "px; height: " + plotHeight + "px;";
			});
		});
	});
}]);