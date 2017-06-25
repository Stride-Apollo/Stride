const electron = require('electron')
// Module to control application life.
const electronapp = electron.app
// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow

const path = require('path')
const url = require('url')



// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let mainWindow

const os = require('os')
const fs = require('fs')

if (process.argv[1] == undefined) {
	console.log("No output directory specified\nPlease specify a directory where the visualization data is located\nrelative from the installed dir");
	electronapp.quit();
}

const output_dir = __dirname.substr(0, __dirname.indexOf("vis") - 1) + "/" + process.argv[1];

function createWindow() {
	// Create the browser window.
	const {width, height} = electron.screen.getPrimaryDisplay().workAreaSize
	mainWindow = new BrowserWindow({width, height})

	// and load the index.html of the app.
	var url = "file://" + __dirname + '/index.html?dir=' + output_dir;
	console.log(url)
	mainWindow.loadURL(url);

	// Open the DevTools.
	 mainWindow.webContents.openDevTools()
	// Emitted when the window is closed.
	mainWindow.on('closed', function () {
		// Dereference the window object, usually you would store windows
		// in an array if your app supports multi windows, this is the time
		// when you should delete the corresponding element.
		mainWindow = null
	})
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
electronapp.on('ready', createWindow)

// Quit when all windows are closed.
electronapp.on('window-all-closed', function () {
	// On OS X it is common for applications and their menu bar
	// to stay active until the user quits explicitly with Cmd + Q
	if (process.platform !== 'darwin') {
		electronapp.quit()
	}
})

electronapp.on('activate', function () {
	// On OS X it's common to re-create a window in the app when the
	// dock icon is clicked and there are no other windows open.
	if (mainWindow === null) {
		createWindow()
	}
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.