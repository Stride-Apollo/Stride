


var app = angular.module('ClusterApp', []);

app.config(['$locationProvider', function($locationProvider) {
	$locationProvider.html5Mode(true);
}]);
app.controller('ClusterController', ['$scope', '$location', function($scope, $location) {

	console.log($location.search().data);
	console.log($location.search().cluster);

	var fs = require('fs');
	var content;
	fs.readFile($location.search().data, 'utf8', function (err, data) {
		content = parseCSVFile(data);
		console.log(content);
		for (var i in content.features) {
			if (content.features[i].properties.id == $location.search().cluster) {
				$scope.ID = content.features[i].properties.id;
				$scope.size = parseFloat(content.features[i].properties.size);
				$scope.infected = parseFloat(content.features[i].properties.infected);
				$scope.infected_percent = parseFloat(content.features[i].properties.infected_percent);
				if ($scope.infected_percent == -1) {
					$scope.infected_percent = 0;
				}
			}
		}
		$scope.title = "Cluster " + $scope.ID;
		$scope.$apply();
	});
}]);
