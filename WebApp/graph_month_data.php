<?php
date_default_timezone_set('Asia/Seoul');
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);

if (!$_GET["id"]) {
	die("return");
}
if (!$_GET["month"]) {
	$month = date('ym', time());
} else {
	$month = $_GET["month"];
}

$id = $_GET["id"];
$pow_arr = array();
$pow_result_arr = array();
$pow_result_sum_arr = array();
$stack = 0;
$yy = "20".substr($month, 0,2);
$mm = substr($month, 2,2);
$sql = "SELECT time_var, SUM(energy) FROM (SELECT energy, SUBSTR(time_data, 1,6) time_var FROM POWER_INT WHERE id = ".$id." AND time_data LIKE '".$month."%') WHERE 1 GROUP BY time_var";
$result = $db->query($sql);
while ($row = $result->fetchArray()) {
	$pow_arr[($row[0] - $month * 100)] = $row[1];
	//array_push($pow_arr, array(($row[0] - $month * 100) => $row[1]));
	/*
	$t = str_pad($i++, 2, "0", STR_PAD_LEFT);
	$j += $row[0];
	array_push($pow_arr, array($t.":00",$row[0]));
	array_push($pow_sum_arr, array($t.":00",$j));
	*/
}
for($i = 1; $i <= 31; $i++) {
	if(!$pow_arr[$i]) {
		$pow_arr[$i] = 0;
	}

	if (checkdate($mm, $i, $yy)) {
		$stack += $pow_arr[$i];
		array_push($pow_result_arr, array($yy."-".$mm."-".str_pad($i, 2, "0", STR_PAD_LEFT), $pow_arr[$i]));
		array_push($pow_result_sum_arr, array($yy."-".$mm."-".str_pad($i, 2, "0", STR_PAD_LEFT), $stack));
	}
}
/*
if(!$i) {
	array_push($pow_arr, array("00:00",0));
	array_push($pow_sum_arr, array("00:00",0));
}
echo json_encode(array("pow"=>$pow_arr, "sum"=>$pow_sum_arr));
*/
echo json_encode(array("pow"=>$pow_result_arr, "sum"=>$pow_result_sum_arr));
$query = $db->close();
?>