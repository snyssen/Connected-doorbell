var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var bodyParser = require('body-parser');
var morgan = require('morgan');
var fs = require('fs');

// config serveur et démarrage
app.use(bodyParser.json())
	.use(morgan('combined'));
http.listen(1337, function () {
	console.log('Server is listening on *:1337');
}); 


/* ===== routage ===== */

// Depuis sonnette
app.get('/ring', function (req, res) { // Quelqu'un est en train de sonner à la porte !
		console.log('Someone rang at the door !');
		var timestamp = getCurrentDate();
		console.log('current time is ' + timestamp);
		io.sockets.emit('ringing', timestamp);
		res.json(timestamp);
		res.end();
	});

// Depuis client web
app.get('/', function (req, res) { // Racine du serveur, initie une connexion au web socket
		fs.readFile('./index.html', 'utf-8', function (error, content) {
			if (error) {
				res.writeHead(500, { "Content-Type": "text/html" });
				res.write(error.code + " " + error.message);
				res.end();
			}
			else {
				res.writeHead(200, { "Content-Type": "text/html" });
				res.end(content);
			}
		});
	});

/* ===== Fonctions ===== */

// Renvoie un timestamp du moment actuel
function getCurrentDate() {
	var currentDate = new Date();
	var day = (currentDate.getDate() < 10 ? '0' : '') + currentDate.getDate();
	var month = ((currentDate.getMonth() + 1) < 10 ? '0' : '') + (currentDate.getMonth() + 1);
	var year = currentDate.getFullYear();
	var hour = (currentDate.getHours() < 10 ? '0' : '') + currentDate.getHours();
	var minute = (currentDate.getMinutes() < 10 ? '0' : '') + currentDate.getMinutes();
	var second = (currentDate.getSeconds() < 10 ? '0' : '') + currentDate.getSeconds();
	return year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
}

/* ===== Web sockets ===== */

io.on('connection', function (socket) {
	console.log('client connected !');

	socket.on('GetTime', function () {
		socket.emit('GetTime', getCurrentDate());
	});
});