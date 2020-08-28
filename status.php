<?php
error_reporting(E_ALL); set_time_limit(0); ob_implicit_flush();

$role_list = array(
	0 => "UDP_NONE", 1 => "UDP_NODE", 2 => "UDP_USER"
);

define("HASHSIZE", 32);

$req = "\x01";
$API_address = "network_1";
$API_port = 1122;

$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_connect($sock, $API_address, $API_port);

socket_write($sock, $req, strlen($req));
$buff = socket_read($sock, HASHSIZE + 2);

if (strlen($buff) != HASHSIZE + 2) {
	echo "Incorrect response.\n";
	exit;
}

$str = sodium_bin2hex($buff);

if (isset($role_list[$str[1]])) {
	echo "Current role: {$role_list[$str[1]]}\n";
	echo "Father status: {$str[3]}\n";
}
