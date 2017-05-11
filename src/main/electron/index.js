var app = angular.module('VisualizationApp', []);

app.controller('Controller', ['$scope', '$interval', function($scope, $interval) {
	mapboxgl.accessToken = 'pk.eyJ1Ijoid29la2lraSIsImEiOiJjajJnNnhnOTcwMDBtNDBuMDltc3BreGZpIn0.kPsej_9LZ3cEaggCD8py9w';
	var map = new mapboxgl.Map({
		container: 'map',
		style: 'mapbox://styles/mapbox/streets-v9'
	});
	var files = [];	// file-text_of_file pairs
	var filenames = [];
	$scope.currentDay = 0;

	var config;
	var fs = require('fs');
	fs.readFile(__dirname + "/data/config.json", 'utf8', function (err, data) {
		if (err) return console.log(err);
		config = setupConfig(data);
		fs.readdir(__dirname + "/" + config.directory, function(err, content){
			if (err) {
				return console.error(err);
			}

			content.forEach( function (file){
				var data = fs.readFileSync(__dirname + "/" + config.directory + "/" + file, 'utf8');
				files.push(data);
				filenames.push(file);
			});
			map.on('load', function() {
				makeClusters(parseCSVFile(files[$scope.currentDay]));
				// makeClusters(JSON.parse(files[$scope.currentDay]));
			});
		});

	});

	function setupConfig(data) {
		config = JSON.parse(data);
		$scope.no_infected_color = config.color_no_infected;
		$scope.min_infected_color = config.color_min_infected;
		$scope.max_infected_color = config.color_max_infected;
		$scope.unzoomed_min = config.unzoomed_min_size;
		$scope.unzoomed_max = config.unzoomed_max_size;
		$scope.opacity = config.circle_opacity;
		$scope.animation_speed = config.animation_step;
		//$scope.$apply();
		addClass(document.getElementById("opacitySlider"), "is-dirty");
		removeClass(document.getElementById("opacitySlider"), "ng-untouched");
		addClass(document.getElementById("opacitySlider"), "ng-touched");
		addClass(document.getElementById("opacitySlider"), "is-upgraded");

		updateSlider("opacitySlider", $scope.opacity);
		updateSlider("zoomedMinSlider", $scope.unzoomed_min/10);
		updateSlider("zoomedMaxSlider", $scope.unzoomed_max/10);
		updateSlider("speedSlider", $scope.animation_speed/5000);
		componentHandler.upgradeDom();
		return config;
	}

	function updateSlider(id, grow) {
		document.getElementById(id).parentNode.lastChild.firstChild.style.flexShrink = "1";
		document.getElementById(id).parentNode.lastChild.firstChild.style.flexGrow = grow.toString();
		document.getElementById(id).parentNode.lastChild.firstChild.style.flexBasis = "0%";
		document.getElementById(id).parentNode.lastChild.lastChild.style.flexShrink = "1";
		document.getElementById(id).parentNode.lastChild.lastChild.style.flexGrow = (1-grow).toString();
		document.getElementById(id).parentNode.lastChild.lastChild.style.flexBasis = "0%";
	}

	$scope.simulation_run;
	$scope.runSimulation = function() {
		if ($scope.currentDay < files.length - 1) {
			$scope.nextDay();
		}
		if ($scope.simulation_run == undefined) {
			$scope.simulation_run = $interval($scope.nextDay, $scope.animation_speed);
		} else {
			$interval.cancel($scope.simulation_run);
			$scope.simulation_run = undefined;
		}
	}

	$scope.nextDay = function() {
		if (++$scope.currentDay >= files.length) {
			$scope.currentDay--;
			clearInterval($scope.simulation_run);
			$scope.simulation_run = undefined;
		} else {
			updatePaint();
			updateMap(parseCSVFile(files[$scope.currentDay]))
		}
		console.log($scope.currentDay);
	}

	$scope.previousDay = function() {
		if (--$scope.currentDay < 0) {
			$scope.currentDay = 0;
		} else {
			updatePaint();
			updateMap(parseCSVFile(files[$scope.currentDay]))
		}
	}

	function updateMap(data) {
		map.getSource("cluster_data").setData(data);
		fitView(data);
	}

	// function parseData(cluster_data) {
	// 	for (var i in cluster_data.features) {
	// 		cluster_data.features[i].properties.infected_percent = parseFloat(cluster_data.features[i].properties.infected_percent);
	// 		cluster_data.features[i].properties.infected = parseFloat(cluster_data.features[i].properties.infected);
	// 		cluster_data.features[i].properties.size = parseFloat(cluster_data.features[i].properties.size);
	// 		cluster_data.features[i].geometry.coordinates = [parseFloat(cluster_data.features[i].geometry.coordinates[0]), parseFloat(cluster_data.features[i].geometry.coordinates[1])];
	// 	}
	// 	return cluster_data;
	// }

	function updatePaint() {
		if (map.loaded()) {
			var no_infected_color;
			var min_infected_color;
			var max_infected_color;
			map.removeLayer("clusters");
			if ($scope.no_infected_color[0] == "#") {
				no_infected_color = $scope.no_infected_color;
			} else {
				no_infected_color = "#" + $scope.no_infected_color;
			}
			if ($scope.min_infected_color[0] == "#") {
				min_infected_color = $scope.min_infected_color;
			} else {
				min_infected_color = "#" + $scope.min_infected_color;
			}
			if ($scope.max_infected_color[0] == "#") {
				max_infected_color = $scope.max_infected_color;
			} else {
				max_infected_color = "#" + $scope.max_infected_color;
			}

			map.addLayer({
				"id": "clusters",
				"type": "circle",
				"source": "cluster_data",
				"paint": {
					'circle-radius': {
						property: "size",
						type: "interval",
						stops: getSize($scope.cluster_data)
					},
					"circle-color": {
						property: "infected_percent",
						stops: [[-1, no_infected_color],[0, min_infected_color],[1, max_infected_color]]
					},
					"circle-opacity": $scope.opacity
				}
			});
		}
	};

	function updateSize() {
		var circle_radius_style = {
			property: "size",
			type: "interval",
			stops: getSize(cluster_data)
		};
		map.setPaintProperty("clusters", 'circle-radius', circle_radius_style);
	}

	$scope.$watch('[no_infected_color, min_infected_color, max_infected_color, opacity, unzoomed_min, unzoomed_max]', updatePaint, true);

	// TODO Make this better!
	$scope.$watch('[unzoomed_min, unzoomed_max]', function() {
		if ($scope.unzoomed_max <= $scope.unzoomed_min) {
			$scope.unzoomed_max = $scope.unzoomed_min+1;
			updateSlider("zoomedMinSlider", $scope.unzoomed_min/10);
			updateSlider("zoomedMaxSlider", $scope.unzoomed_max/10);
		}
	}, true);

	function makeClusters(cluster_data) {
		$scope.cluster_data = cluster_data;
		map.addSource("cluster_data", {
			"type": "geojson",
			"data": cluster_data
		});

		map.addLayer({
			"id": "clusters",
			"type": "circle",
			"source": "cluster_data",
			"paint": {
				'circle-radius': {
					property: "size",
					type: "interval",
					stops: getSize(cluster_data)
				},
				"circle-color": {
					property: "infected_percent",
					stops: [[-1, $scope.no_infected_color],[0, $scope.min_infected_color],[1, $scope.max_infected_color]]
				},
				"circle-opacity": $scope.opacity
			}
		});
		// Create a popup, but don't add it to the map yet.
		var popup = new mapboxgl.Popup({
			closeButton: false,
			closeOnClick: false
		});

		map.on('mouseenter', 'clusters', function(e) {
			// Change the cursor style as a UI indicator.

			var coords = e.features[0].geometry.coordinates;
			map.getCanvas().style.cursor = 'pointer';

			var htmlText = "Cluster " + e.features[0].properties.id.toString()
				+ "<br>Size: " + e.features[0].properties.size.toString()
				+ "<br>Infected: " + e.features[0].properties.infected.toString()
				+ "<br>Coordinates: (" + coords[0] + ", " + coords[1] + ")";

			// Populate the popup and set its coordinates
			// based on the feature found.
			popup.setLngLat(coords)
				.setHTML(htmlText)
				.addTo(map);
		});

		map.on('mouseleave', 'clusters', function() {
			map.getCanvas().style.cursor = '';
			popup.remove();
		});

		var windows = []
		map.on('click', 'clusters', function(e) {
			// console.log(e.features[0].properties.id);
			const electron = require('electron').remote;
			const BrowserWindow = electron.BrowserWindow;

			if (!containsWin(windows, e.features[0].properties.id)) {
				var win = new BrowserWindow({ width: 800, height: 600 });

				windows.push({id:e.features[0].properties.id, win: win });
				win.webContents.on('did-finish-load', ()=>{
					win.show();
				//win.webContents.openDevTools();
				win.focus();
			});

				win.on('closed', function () {
					for (var i in windows) {
						if (windows[i].id == e.features[0].properties.id) {

							windows.splice(i, 1);
						}
					}
				})
				var url = "file://" + __dirname + '/ClusterContent.html?data='
					+ __dirname + "/" + config.directory + "/" + filenames[$scope.currentDay]
					+ "&cluster=" + e.features[0].properties.id
					+ "&directory=" + __dirname + "/" + config.directory
					+ "&$scope.currentDay=" + $scope.currentDay;
				win.loadURL(url);
			} else {
				for (var i in windows) {
					if (windows[i].id == e.features[0].properties.id) {
						windows[i].win.focus();
					}
				}
			}
		});

		fitView(cluster_data);
	}

	function fitView(cluster_data) {
		var coords = [];
		for (var i in cluster_data.features) {
			coords.push(cluster_data.features[i].geometry.coordinates);
		};

		var min_lat = Number.MAX_VALUE;
		var max_lat = Number.MIN_VALUE;
		var min_lon = Number.MAX_VALUE;
		var max_lon = Number.MIN_VALUE;
		for (var i in coords) {
			if (coords[i][0] > max_lon) {
				max_lon = coords[i][0];
			};
			if (coords[i][0] < min_lon) {
				min_lon = coords[i][0];
			};
			if (coords[i][1] > max_lat) {
				max_lat = coords[i][1];
			};
			if (coords[i][1] < min_lat) {
				min_lat = coords[i][1];
			};
		};
		// console.log("(" + min_lon + ", " + min_lat + ")" + "(" + max_lon + ", " + max_lat + ")");
		map.fitBounds([[min_lon-2, min_lat-2], [max_lon+2, max_lat+2]]);
	}

	function getSize(cluster_data) {
		var result = [[{zoom: 0, value:0}, 0]];
		//var result = [[0, 0]];
		var orig_MIN_SIZE = $scope.unzoomed_min;
		var orig_MAX_SIZE = $scope.unzoomed_max;
		var min_value = Number.MAX_VALUE;
		var max_value = Number.MIN_VALUE;

		for (var i in cluster_data.features) {
			var size = cluster_data.features[i].properties.size;
			if (size > max_value) {
				max_value = size;
			}
			if (size < min_value) {
				min_value = size;
			}
		};

		var zoom = [[1,1], [4,1], [7,2], [13, 4], [15, 7], [18, 10]];
		var sizes = [];
		for (var i in cluster_data.features) {
			sizes.push(cluster_data.features[i].properties.size);
		};

		sizes.sort();

		/*for (var i in sizes) {
		 result.push([sizes[i], ((sizes[i] - min_value)*(orig_MAX_SIZE - orig_MIN_SIZE)/(max_value - min_value) + orig_MIN_SIZE)]);
		 }
		 console.log(result);
		 return result;*/

		var tempresult = {};
		for (var i in zoom) {
			tempresult[i] = [];
			for (var j in sizes) {
				var size = {};
				size.zoom = zoom[i][0];
				size.value = sizes[j];
				var MAX_SIZE = orig_MAX_SIZE * zoom[i][1];
				var MIN_SIZE = orig_MIN_SIZE * zoom[i][1];
				// console.log(size);
				// console.log(((sizes[j] - min_value)*(MAX_SIZE - MIN_SIZE)/(max_value - min_value) + MIN_SIZE));
				tempresult[i].push([size.value, ((sizes[j] - min_value)*(MAX_SIZE - MIN_SIZE)/(max_value - min_value) + MIN_SIZE)]);
			}
		};
		for (var i in tempresult) {
			tempresult[i].sort(function(a,b) {return a[1] - b[1]});
			for (var j in tempresult[i]) {
				var leftStop = {};
				leftStop.zoom = parseInt(i);
				leftStop.value = tempresult[i][j][0];
				result.push([leftStop, tempresult[i][j][1]]);
			}
		}
		// console.log(result);
		return result;
	};

	function containsWin(array, obj) {
		var i = array.length;
		while (i--) {
			if (array[i].id === obj) {
				return true;
			}
		}
		return false;
	}

	// TODO Set colors automatically on init
	$scope.openMenu = function() {
		var drawer = document.getElementById("conf_drawer");
		var btn = document.getElementById("conf_button");

		if (document.getElementById("conf_button").getAttribute("class") == "material-icons") {
			btn.setAttribute("class", "material-icons is_clicked");
			drawer.setAttribute("class", "expanded");
			btn.innerHTML = "navigate_next";
		} else {
			btn.setAttribute("class", "material-icons");
			drawer.setAttribute("class", "");
			btn.innerHTML = "navigate_before";
		}
	}
}]);