<?php
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);

$is_write = $_GET["write"];
$id = $_GET["id"];
$jsondata = $_GET["data"];

if(!$is_write) {
	$sql = "SELECT start_time, stop_time FROM TIMER WHERE id = ".$id;

	$time = array();

	$result = $db->query($sql);

	while ($row = $result->fetchArray()) {
		array_push($time, array('start_time'=>$row[0], 'stop_time'=>$row[1]));
	}
	echo json_encode(array("times"=>$time));
} else {
	$sql = "DELETE FROM TIMER WHERE id = ".$id;
	$query = $db->exec($sql);
	$sql = "";
	if($query) {
		$jsonArray = json_decode(urldecode($jsondata));
		$json_length = count($jsonArray);

		foreach ($jsonArray as $value) {
			$sql .= "INSERT INTO TIMER VALUES(".$id.",".$value -> start_time.",".$value -> stop_time.");";
		}
		$query = $db->exec($sql);
	}
}
$query = $db->close();
?>