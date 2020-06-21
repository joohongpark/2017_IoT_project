<?php
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);
if (!$_GET['data'] || !$_GET["id"]) {
	die("No Data Available");
}
$sql = "UPDATE NAME SET dev_name = '".$_GET['data']."' WHERE id = ".$_GET["id"];
$query = $db->exec($sql);
if (!$query) {
	die("Database transaction failed: " . $db->lastErrorMsg() );
}
$db->close();
?>