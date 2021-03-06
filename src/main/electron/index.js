const os = require('os')
const fs = require('fs')
var app = angular.module('VisualizationApp', []);
const random_seed = parseInt(Math.floor(Math.random()*100000+1).toFixed(6));
const random_temp_file = os.tmpdir() + "/visualization_data_" + random_seed;
app.controller('Controller', ['$scope', '$timeout', '$interval', function($scope, $timeout, $interval) {
	var tmpdata = "{\"windows\":[]}"
	fs.writeFileSync(random_temp_file, tmpdata)
	mapboxgl.accessToken = 'pk.eyJ1Ijoid29la2lraSIsImEiOiJjajJnNnhnOTcwMDBtNDBuMDltc3BreGZpIn0.kPsej_9LZ3cEaggCD8py9w';
	var map = new mapboxgl.Map({
		container: 'map',
		style: 'mapbox://styles/mapbox/streets-v9'
	});

	function formatFacilityData(data) {
		var facFeatures = [];
		console.log(data)
		for (var i = 0; i < data.facilities.length; ++i) {
			var facility = data.facilities[i];

			var decoratedFacility = {
				type: "Feature",
				geometry: {
					type: "Point",
					coordinates: [facility.location.lon, facility.location.lat]
				},
				properties: {
					"name": facility.name,
					"city": facility.city,
					"icon": "airport",
					"passengers_today": facility.passengers_today,
					"passengers_x_days": facility.passengers_x_days,
					"x_days": facility.x_days,
					"influence": facility.influence
				}
			};

			facFeatures.push(decoratedFacility);
		}

		return {
			"type": "FeatureCollection",
			"features": facFeatures
		};
	}

	function formatSphereOfInfluence(data) {
		var facFeatures = [];

		for (var i = 0; i < data.facilities.length; ++i) {
			var facility = data.facilities[i];
			facFeatures.push(createGeoJSONCircle(facility.location, facility.influence, 512));
		}
		result = {
			"type": "FeatureCollection",
			"features": facFeatures
		};
		console.log(result);

		return result;
	}
	var createGeoJSONCircle = function(center, radiusInKm, points) {
		if(!points) points = 64;

		var coords = {
			latitude: parseFloat(center.lat),
			longitude: parseFloat(center.lon)
		};

		var km = parseFloat(radiusInKm);

		var ret = [];
		var distanceX = km/(111.320*Math.cos(coords.latitude*Math.PI/180));
		var distanceY = km/110.574;

		var theta, x, y;
		for(var i=0; i<points; i++) {
			theta = (i/points)*(2*Math.PI);
			x = distanceX*Math.cos(theta);
			y = distanceY*Math.sin(theta);

			ret.push([coords.longitude+x, coords.latitude+y]);
		}
		ret.push(ret[0]);

		return {
				"type": "Feature",
				"geometry": {
					"type": "Polygon",
					"coordinates": [ret]
				}
		};
	};


	var files = [];	// file-text_of_file pairs
	var filenames = [];
	$scope.currentDay = 0;

	$scope.save = function() {
		console.log("Saving");
		var saveString = "{\"directory\": \"" + config.directory.toString()
			+ "\",\"population_directory\": \"" + $scope.population_directory
			+ "\",\"airport_directory\": \"" + $scope.airport_directory + "\",\"color_no_infected\": \""
			+ getHexColor($scope.no_infected_color).toString() + "\",\"color_min_infected\": \""
			+ getHexColor($scope.min_infected_color).toString() + "\",\"color_max_infected\": \""
			+ getHexColor($scope.max_infected_color).toString() + "\",\"circle_opacity\": "
			+ $scope.opacity + ",\"unzoomed_min_size\": " + $scope.unzoomed_min
			+ ",\"unzoomed_max_size\":  " + $scope.unzoomed_max
			+ ",\"animation_step\": " + $scope.animation_speed
			+ ",\"boundary\": " + $scope.boundary + "}";
		fs.writeFileSync(__dirname + "/data/config.json", saveString);
	}

	var config;
	var loaded = false;
	var block = false;
	var airport_files = [];
	var airport_filenames = [];
	var output_dir = decodeURI(global.location.search.substr(5));
	console.log(output_dir);

	fs.readFile(__dirname + "/data/config.json", 'utf8', function (err, data) {
		if (err) return console.log(err);
		config = setupConfig(data);
		var dircontent = fs.readdirSync(output_dir + "/" + config.directory);

		dircontent.forEach( function (file){
			var data = fs.readFileSync(output_dir + "/" + config.directory + "/" + file, 'utf8');
			files.push(data);
			filenames.push(file);
		});

		dircontent = fs.readdirSync(output_dir + "/" + config.airport_directory);

		dircontent.forEach( function (file){
			var data = fs.readFileSync(output_dir + "/" + config.airport_directory + "/" + file, 'utf8');
			airport_files.push(data);
			airport_filenames.push(file);
		});
		map.on('load', function() {
			makeClusters(parseCSVFile(files[$scope.currentDay]));
			drawAirports(JSON.parse(airport_files[$scope.currentDay]));
			loaded = true;
		});
	});

	function setupConfig(data) {
		config = JSON.parse(data);
		$scope.population_directory = config.population_directory;
		$scope.airport_directory = config.airport_directory;
		$scope.no_infected_color = config.color_no_infected;
		$scope.min_infected_color = config.color_min_infected;
		$scope.max_infected_color = config.color_max_infected;
		$scope.unzoomed_min = config.unzoomed_min_size;
		$scope.unzoomed_max = config.unzoomed_max_size;
		$scope.opacity = config.circle_opacity;
		$scope.animation_speed = config.animation_step;
		$scope.boundary = config.boundary;
		//$scope.$apply();
		addClass(document.getElementById("opacitySlider"), "is-dirty");
		removeClass(document.getElementById("opacitySlider"), "ng-untouched");
		addClass(document.getElementById("opacitySlider"), "ng-touched");
		addClass(document.getElementById("opacitySlider"), "is-upgraded");

		updateSlider("opacitySlider", $scope.opacity);
		updateSlider("zoomedMinSlider", $scope.unzoomed_min/9);
		updateSlider("zoomedMaxSlider", ($scope.unzoomed_max-1)/9);
		updateSlider("speedSlider", $scope.animation_speed/5000);
		componentHandler.upgradeDom();
		return config;
	}

	function updateSlider(id, grow) {
		if (document.getElementById(id).parentNode.lastChild.firstChild) {
			document.getElementById(id).parentNode.lastChild.firstChild.style.flexShrink = "1";
			document.getElementById(id).parentNode.lastChild.firstChild.style.flexGrow = grow.toString();
			document.getElementById(id).parentNode.lastChild.firstChild.style.flexBasis = "0%";
		}
		if (document.getElementById(id).parentNode.lastChild.lastChild) {
			document.getElementById(id).parentNode.lastChild.lastChild.style.flexShrink = "1";
			document.getElementById(id).parentNode.lastChild.lastChild.style.flexGrow = (1-grow).toString();
			document.getElementById(id).parentNode.lastChild.lastChild.style.flexBasis = "0%";
		}
	}

	$scope.simulation_run;
	$scope.runSimulation = function() {
		if ($scope.currentDay < files.length - 1) {
			$scope.nextDay();
		}
		if ($scope.simulation_run == undefined) {
			$scope.simulation_run = $interval($scope.nextDay, $scope.animation_speed);
			$interval.cancel($scope.simulation_rewind);
			$scope.simulation_rewind = undefined;
		} else {
			$interval.cancel($scope.simulation_run);
			$scope.simulation_run = undefined;
		}
	}

	$scope.simulation_rewind;
	$scope.rewindSimulation = function() {
		if ($scope.currentDay > 0) {
			$scope.previousDay();
		}
		if ($scope.simulation_rewind == undefined) {
			$scope.simulation_rewind = $interval($scope.previousDay, $scope.animation_speed);
			$interval.cancel($scope.simulation_run);
			$scope.simulation_run = undefined;
		} else {
			$interval.cancel($scope.simulation_rewind);
			$scope.simulation_rewind = undefined;
		}
	}

	$scope.nextDay = function() {
		if (++$scope.currentDay >= files.length) {
			$scope.currentDay--;
			$interval.cancel($scope.simulation_run);
			$scope.simulation_run = undefined;
		} else {
			updateMap(parseCSVFile(files[$scope.currentDay]), JSON.parse(airport_files[$scope.currentDay]));
		}
	}

	$scope.previousDay = function() {
		if (--$scope.currentDay < 0) {
			$scope.currentDay = 0;
			$interval.cancel($scope.simulation_rewind);
			$scope.simulation_rewind = undefined
		} else {
			updateMap(parseCSVFile(files[$scope.currentDay]), JSON.parse(airport_files[$scope.currentDay]));
		}
	}

	function updateMap(data, airportdata) {
		// Kind of sleep until map is done rendering, so you can update the data.
		if (block) {
			$timeout(updateMap(data, airportdata), 50);
		} else {
			var decoratedData = getDecoratedData(data);
			var neededData =
				{
					type: "FeatureCollection",
					features: []
				};

			if ($scope.clusterCheckBoxes[1] == true) {
				neededData.features = neededData.features.concat(decoratedData.getClusters(cluster_type.household).features);
			}
			if ($scope.clusterCheckBoxes[2] == true) {
				neededData.features = neededData.features.concat(decoratedData.getClusters(cluster_type.school).features);
			}
			if ($scope.clusterCheckBoxes[3] == true) {
				neededData.features = neededData.features.concat(decoratedData.getClusters(cluster_type.work).features);
			}
			if ($scope.clusterCheckBoxes[4] == true) {
				neededData.features = neededData.features.concat(decoratedData.getClusters(cluster_type.primary_community).features);
			}
			if ($scope.clusterCheckBoxes[5] == true) {
				neededData.features = neededData.features.concat(decoratedData.getClusters(cluster_type.secondary_community).features);
			}


			map.getSource("cluster_data").setData(neededData);

			decoratedData = airportdata;
			neededData = {
				type: "FeatureCollection",
				features: []
			};

			if ($scope.clusterCheckBoxes[0] == true) {
				neededData.features = neededData.features.concat(formatFacilityData(decoratedData).features);
			}

			map.getSource("airport_data").setData(neededData);


			decoratedData = airportdata;
			neededData = {
				type: "FeatureCollection",
				features: []
			};

			if ($scope.clusterCheckBoxes[0] == true) {
				neededData.features = neededData.features.concat(formatSphereOfInfluence(decoratedData).features);
			}

			map.getSource("sphere_data").setData(neededData);

			fitView(neededData);
		}
	}

	function getHexColor(color) {
		if (color[0] == "#") {
			return color;
		} else {
			return "#" + color;
		}
	};

	function updatePaint() {
		console.log("updating visuals!");
		if (loaded) {
			var no_infected_color = getHexColor($scope.no_infected_color);
			var min_infected_color = getHexColor($scope.min_infected_color);
			var max_infected_color = getHexColor($scope.max_infected_color);
			map.removeLayer("clusters");

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

			map.removeLayer("circle");
			map.addLayer({
				"id": "circle",
				"type": "line",
				"source": "sphere_data",
				"layout": {},
				"paint": {
					"line-color": "blue",
					"line-opacity": 1
				}
			});

			map.removeLayer("airports");
			map.addLayer({
				"id": "airports",
				"type": "symbol",
				"source": "airport_data",
				"layout": {
					"icon-image": "{icon}-15",
					"icon-size": 1.25,
					"text-field": "{name}",
					"text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
					"text-offset": [0, 0.6],
					"text-anchor": "top"
				}
			});
			block = true;
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

	$scope.$watch('animation_speed', function() {
		if ($scope.simulation_run != undefined) {
			$interval.cancel($scope.simulation_run);
			$scope.simulation_run = undefined;
			$scope.runSimulation();
		} else if ($scope.simulation_rewind != undefined) {
			$interval.cancel($scope.simulation_rewind);
			$scope.simulation_rewind = undefined;
			$scope.rewindSimulation();
		}
	});

	// TODO Make this better!
	$scope.$watch('[unzoomed_min, unzoomed_max]', function() {
		if ($scope.unzoomed_max <= $scope.unzoomed_min) {
			$scope.unzoomed_max = $scope.unzoomed_min+1;
			updateSlider("zoomedMinSlider", $scope.unzoomed_min/9);
			updateSlider("zoomedMaxSlider", ($scope.unzoomed_max-1)/9);
		}
	}, true);
	$scope.$watch('[no_infected_color, min_infected_color, max_infected_color, opacity, unzoomed_min, unzoomed_max]', updatePaint, true);

	//////////////////////////////////////
	// Checkbox stuff					//
	//////////////////////////////////////
	$scope.clusterCheckBoxes = [true, true, true, true, true, true];

	$scope.changeBox = function() {
		updateMap(parseCSVFile(files[$scope.currentDay]), JSON.parse(airport_files[$scope.currentDay]));
	}


	function drawAirports(airport_data) {
		$scope.airport_data = formatFacilityData(airport_data);
		$scope.sphere_data = formatSphereOfInfluence(airport_data);
		map.addSource("airport_data", {
			"type": "geojson",
			"data": $scope.airport_data
		});

		map.addSource("sphere_data", {
			"type": "geojson",
			"data": $scope.sphere_data
		});

		map.addLayer({
			"id": "airports",
			"type": "symbol",
			"source": "airport_data",
			"layout": {
				"icon-image": "{icon}-15",
				"icon-size": 1.25,
				"text-field": "{name}",
				"text-font": ["Open Sans Semibold", "Arial Unicode MS Bold"],
				"text-offset": [0, 0.6],
				"text-anchor": "top"
			}
		});

		map.addLayer({
			"id": "circle",
			"type": "line",
			"source": "sphere_data",
			"layout": {},
			"paint": {
				"line-color": "blue",
				"line-opacity": 1
			}
		});

		// Create a popup, but don't add it to the map yet.
		var popup = new mapboxgl.Popup({
			closeButton: false,
			closeOnClick: false
		});

		map.on('mouseenter', 'airports', function(e) {
			// Change the cursor style as a UI indicator.

			var coords = e.features[0].geometry.coordinates;
			map.getCanvas().style.cursor = 'pointer';

			var htmlText = "<h3 class='center'>" + e.features[0].properties.name + "</h3><h5 class='center'>" + e.features[0].properties.city + " Airport</h5><br>"
				+ "Passengers today: " + e.features[0].properties.passengers_today + "<br>"
				+ "Passengers in the last " + e.features[0].properties.x_days + " days: " + e.features[0].properties.passengers_x_days
				+ "<br>Radius of influence: " + e.features[0].properties.influence + " km"
				+ "<br>Coordinates: (" + coords[0] + ", " + coords[1] + ")";

			// Populate the popup and set its coordinates
			// based on the feature found.
			popup.setLngLat(coords)
				.setHTML(htmlText)
				.addTo(map);
		});

		map.on('mouseleave', 'airports', function() {
			map.getCanvas().style.cursor = '';
			popup.remove();
		});
	}

	const metersToPixelsAtMaxZoom = (meters, latitude) =>
	meters / 0.075 / Math.cos(latitude * Math.PI / 180)

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

			var htmlText = "<h5 class='center'>Cluster " + e.features[0].properties.id.toString()
				+ "</h5><br>Size: " + e.features[0].properties.size.toString()
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

		// If a map is rendered the style loading is done and we can unblock.
		map.on('render', function() {
			block = false;
		})

		fitView(cluster_data);
	}

	map.on('click', 'clusters', function(e) {
		for(var i in e.features){
			loadCluster(e.features[i].properties.id, config, filenames, $scope.currentDay, random_temp_file, output_dir);
		}
	});

	var overview = undefined;
	var overview_day = undefined;

	function loadOverview() {
		const electron = require('electron').remote;
		const BrowserWindow = electron.BrowserWindow;
		overview = new BrowserWindow({ width: 800, height: 600 });
		overview_day = $scope.currentDay;
		overview.webContents.on('did-finish-load', ()=>{
			overview.show();
			// overview.webContents.openDevTools();
			overview.focus();
		});
		overview.on('closed', function () {
			overview = undefined;
			overview_day = undefined;
		})
		var url = "file://" + __dirname + '/OverviewContent.html?data='
			+ output_dir + "/" + config.directory + "/" + filenames[$scope.currentDay]
			+ "&directory=" + output_dir + "/" + config.directory
			+ "&population=" + output_dir + "/" + config.population_directory
			+ "&currentDay=" + $scope.currentDay
			+ "&outputDir=" + output_dir
			+ "&randomFile=" + random_temp_file;
		overview.loadURL(url);
	}

	$scope.overview = function() {
		if (map.loaded()) {
			// console.log(e.features[0].properties.id);
			if (overview == undefined) {
				loadOverview();
			} else if (overview_day != $scope.currentDay) {
				overview_day = $scope.currentDay;
				var url = "file://" + __dirname + '/OverviewContent.html?data='
					+ output_dir + "/" + config.directory + "/" + filenames[$scope.currentDay]
					+ "&directory=" + output_dir + "/" + config.directory
					+ "&population=" + output_dir + "/" + config.population_directory
					+ "&currentDay=" + $scope.currentDay
					+ "&outputDir=" + output_dir
					+ "&randomFile=" + random_temp_file;
				overview.loadURL(url);
				overview.focus();
			} else {
				overview.focus();
			}
		}
	}

	function fitView(cluster_data) {
		var coords = [];
		for (var i in cluster_data.features) {
			coords.push(cluster_data.features[i].geometry.coordinates);
		};

		var min_lat = 90;
		var max_lat = -90;
		var min_lon = 180;
		var max_lon = -180;
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
		map.fitBounds([[min_lon-$scope.boundary, min_lat-$scope.boundary], [max_lon+$scope.boundary, max_lat+$scope.boundary]]);
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

		var tempresult = {};
		for (var i in zoom) {
			tempresult[i] = [];
			for (var j in sizes) {
				var size = {};
				size.zoom = zoom[i][0];
				size.value = sizes[j];
				var MAX_SIZE = orig_MAX_SIZE * zoom[i][1];
				var MIN_SIZE = orig_MIN_SIZE * zoom[i][1];
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
		return result;
	};

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

function loadCluster(id, config, filenames, currentDay, random_temp_file, output_dir) {
	var electron = require('electron').remote;
	const BrowserWindow = electron.BrowserWindow;
	var windows = JSON.parse(fs.readFileSync(random_temp_file)).windows;
	console.log("Load cluster!")
	if (!containsWin(windows, id)) {
		var win = new BrowserWindow({ width: 800, height: 600 });
		windows.push({id: id, window: win.id, day: currentDay});

		win.webContents.on('did-finish-load', ()=>{
			win.show();
			// win.webContents.openDevTools();
			win.focus();
		});

		var url = "file://" + __dirname + '/ClusterContent.html?data='
			+ output_dir + "/" + config.directory + "/" + filenames[currentDay]
			+ "&cluster=" + id
			+ "&directory=" + output_dir + "/" + config.directory
			+ "&currentDay=" + currentDay;
		win.loadURL(url);

		var content = "{\"windows\": " + JSON.stringify(windows) + "}";
		fs.writeFileSync(random_temp_file, content);
	} else {
		for (var i = windows.length-1; i >= 0; i--) {
			if (windows[i].id == id) {
				if (BrowserWindow.fromId(windows[i].window) == null || currentDay != windows[i].day) {
					if (currentDay != windows[i].day && BrowserWindow.fromId(windows[i].window) != null) {
						BrowserWindow.fromId(windows[i].window).close();
					}
					windows.splice(i, 1);
					var content = "{\"windows\": " + JSON.stringify(windows) + "}";
					fs.writeFileSync(random_temp_file, content);
					loadCluster(id, config, filenames, currentDay, random_temp_file, output_dir);
				} else {
					BrowserWindow.fromId(windows[i].window).focus();
				}
			}
		}
	}
};

function containsWin(array, obj) {
	var i = array.length;
	while (i--) {
		if (array[i].id == obj) {
			return true;
		}
	}
	return false;
}
