<?php
error_reporting(E_ALL);
set_time_limit(0);
ob_implicit_flush();

define('PACKLEN',	2500);
define('HASHSIZE',	32);
define('CLIENT',	"5e2b0202ee83fe5716f1796c7f4c3cd6e397e8bc6370f26520313af2a407ce4f");
define('MESSAGE',	"Hello world!");

$API_address = "localhost";
$API_port = 2122;

$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_connect($sock, $API_address, $API_port);

$find_req = "\x00" . sodium_hex2bin(CLIENT);

// Find request.
while (true) {
	socket_write($sock, $find_req, HASHSIZE + 1);
	$buff = socket_read($sock, PACKLEN);
	$str  = sodium_bin2hex($buff);
	$stat = (int)$str[HASHSIZE * 2 + 1];

	if ($stat != 0) {
		break;
	}

	sleep(3);
}

// Check status code.
if ($stat != 2) {
	echo "Error: client doesn't exist.";
	exit;
}

// Tunnel request.
$tunnel_req = "\x01" . sodium_hex2bin(CLIENT);

while (true) {
	socket_write($sock, $tunnel_req, HASHSIZE + 1);
	$buff = socket_read($sock, PACKLEN);

	if (strlen($buff) != HASHSIZE + 4) {
		echo "Incorrect response.\n";
		exit;
	}

	echo sodium_bin2hex($buff)."\n\n";

	// Write port bytes.
	$buff = intval($buff[HASHSIZE]) .
			intval($buff[HASHSIZE + 1]) .
			intval($buff[HASHSIZE + 2]) .
			intval($buff[HASHSIZE + 3]);
	$port = (int)$port;

echo $port."\n\n";


	if ($port == 0) {
		echo "Tunnel isn't created, wait.\n";
		sleep(2);
	}

	break;
}

socket_close($sock);

// Create new socket for data sending.
$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_connect($sock, $API_address, $port);

while (true) {
	// Status code, it will show us the readiness of the tunnel.
	$status = socket_read($sock, PACKLEN);

echo $status . "\n";exit;

	if ((int)$status == 1) {
		echo "Tunnel in creating process, wait.\n";
		sleep(1);
	}

	socket_write($sock, MESSAGE, strlen(MESSAGE));
	break;
}
