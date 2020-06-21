<?php
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);
if ( ($_GET['data'] == null) && $_GET["id"]) {
	$id = $_GET["id"];
	$sql = "SELECT ctrl FROM TIMER_CTRL WHERE id = ".$id;
	$result = $db->query($sql);
	$val = array();
	while ($row = $result->fetchArray()) {
		array_push($val, array($id=>$row[0]));
	}
	echo json_encode($val);
} else if ($_GET['data'] && $_GET["id"]) {
	if($_GET['data'] == 'false') {
		$data = 0;
	} else if ($_GET['data'] == 'true') {
		$data = 1;
	} else {
		die("데이터 오류");
	}
	$sql = "UPDATE TIMER_CTRL SET ctrl = '".$data."' WHERE id = ".$_GET["id"];
	$query = $db->exec($sql);
	if (!$query) {
		die("Database transaction failed: " . $db->lastErrorMsg() );
	}
	echo "success";
} else {
	die("error");
}
$db->close();
?>