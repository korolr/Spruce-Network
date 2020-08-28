<?php
error_reporting(E_ALL); set_time_limit(0); ob_implicit_flush();

define('PACKLEN',	2500);
define('HASHSIZE',	32);
define('CLIENT',	"5e2b0202ee83fe5716f1796c7f4c3cd6e397e8bc6370f26520313af2a407ce4f");

$statuslist = array(
	1 => "user doesn't exist",
	2 => "found"
);

$req = "\x02" . sodium_hex2bin(CLIENT);
$API_address = "network_1";
$API_port = 1122;

$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_connect($sock, $API_address, $API_port);

while (true) {
	socket_write($sock, $req, HASHSIZE + 1);
	$buff = socket_read($sock, PACKLEN);

	if (strlen($buff) != HASHSIZE + 1) {
		echo "Incorrect response.\n";
		break;
	}

	$str = sodium_bin2hex($buff)."\n";

	if (isset($statuslist[$str[1]])) {
		echo "Status: {$statuslist[$str[1]]}.\n";
		break;
	}

	sleep(3);
}