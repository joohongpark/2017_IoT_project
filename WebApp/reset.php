<?php
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);
if($_GET['c'] == 'true') {
	$sql = "DELETE FROM POWER_INT; UPDATE sqlite_sequence SET seq = 0 WHERE name = 'POWER_INT';";
	$query = $db->exec($sql);
	if ($query) {
		die("success");
	}
}
$db->close();
?>