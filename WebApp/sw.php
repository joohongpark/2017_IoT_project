<?php
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);
if (!$_GET['data'] || !$_GET["id"]) {
	die("No Data Available");
}


if($_GET['data'] == 'false') {
	$data = 0;
} else if ($_GET['data'] == 'true') {
	$data = 1;
} else {
	die("데이터 오류");
}
$sql = "UPDATE ONOFF SET onoff = '".$data."' WHERE id = ".$_GET["id"];
$query = $db->exec($sql);
if (!$query) {
	die("Database transaction failed: " . $db->lastErrorMsg() );
}
$db->close();
?>