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
		var config = {directory: dir.slice($location.search().outputDir.length)};
		loadCluster(ID, config, filenames, $location.search().currentDay, $location.search().randomFile, $location.search().outputDir);
	}

	$scope.loadFromInput = function() {
		var dir = $location.search().directory;
		var config = {directory: dir.slice($location.search().outputDir.length)};
		for (var i in $scope.clusters) {
			if ($scope.clusters[i].ID == $scope.inputID) {
				loadCluster($scope.inputID, config, filenames, $location.search().currentDay, $location.search().randomFile, $location.search().outputDir);
			}
		}
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

	var currentGraph = 0;
	var graphs = [];
	var titles = ['Evolution Of Population Illness', 'Age distribution', 'Household cluster sizes',
					'School cluster sizes', 'Work cluster sizes', 'Primary community cluster sizes', 'Secondary community cluster sizes'];

	function drawGraph(resize = false) {
		var plotWidth = 800;
		var plotHeight = 450;

		var layout = {
			title: titles[currentGraph],
			width: plotWidth,
			height: plotHeight,
			paper_bgcolor: "rgba(255,0,0,0)",
			plot_bgcolor: "rgba(255,0,0,0)",
			legend: {
				y: 0.5,
				traceorder: 'reversed',
				font: {size: 16},
				yref: 'paper'
			}
		};

		var myPlot = document.getElementById("graphInfected");
		Plotly.purge(myPlot);

		console.log("PRINTING");
		console.log(graphs[currentGraph]);

		Plotly.newPlot('graphInfected', graphs[currentGraph], layout).then(function() {
			window.requestAnimationFrame(function() {
				window.requestAnimationFrame(function() {

					if (resize) {
						resizeWindow();
					}

					document.getElementById("graphInfected").style="margin: auto; width: " + plotWidth + "px; height: " + plotHeight + "px;";
				});
			});
		});
	}

	$scope.nextGraph = function() {
		currentGraph += 1;
		if (currentGraph == graphs.length) {
			currentGraph = 0;
		}

		drawGraph();
	}

	$scope.previousGraph = function() {
		currentGraph -= 1;
		if (currentGraph == -1) {
			currentGraph = graphs.length - 1;
		}

		drawGraph();
	}

	function makeBarChart(data) {
		var xData = [];
		var yData = [];
		for (var attr in data) {
			if (data.hasOwnProperty(attr)) {
				xData.push(parseInt(attr));
				yData.push(parseInt(data[attr]));
			}
		}

		var chartData = [
			{
				x: xData,
				y: yData,
				type: "bar"
			}
		];

		console.log(chartData);

		return chartData;
	}

	function makeEvolutionGraph() {
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

		return data;
	}

	graphs.push(makeEvolutionGraph());

	var popDataJSON = JSON.parse(pop_files[$scope.currentDay])
	var cluster_sizes = popDataJSON.cluster_sizes;

	var age_map = popDataJSON.age_map;
	var household_map = popDataJSON.cluster_sizes.household;
	var school_map = popDataJSON.cluster_sizes.school;
	var work_map = popDataJSON.cluster_sizes.work;
	var primary_community_map = popDataJSON.cluster_sizes.primary_community;
	var secondary_community_map = popDataJSON.cluster_sizes.secondary_community;

	var graphData = [age_map, household_map, school_map, work_map, primary_community_map, secondary_community_map];

	for (var i = 0; i < graphData.length; ++i) {
		graphs.push(makeBarChart(graphData[i]));
	}

	drawGraph(true);

	$scope.population_size = clusterEvolution[$scope.currentDay].size;
	$scope.infected = clusterEvolution[$scope.currentDay].infected;
	$scope.total_cluster = $scope.clusters.length;

	$scope.household_density = Number((parseFloat(popDataJSON.densities.household)).toFixed(2));
	$scope.school_density = Number((parseFloat(popDataJSON.densities.school)).toFixed(2));
	$scope.work_density = Number((parseFloat(popDataJSON.densities.work)).toFixed(2));
	$scope.primary_community_density = Number((parseFloat(popDataJSON.densities.primary_community)).toFixed(2));
	$scope.secondary_community_density = Number((parseFloat(popDataJSON.densities.secondary_community)).toFixed(2));

	document.getElementById('inputID').addEventListener('keypress', function(e) {
		if(e.keyCode === 13) {
			e.preventDefault();
			$scope.loadFromInput();
		}
	})
}]);
