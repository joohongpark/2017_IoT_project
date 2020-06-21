<?php
date_default_timezone_set('Asia/Seoul');
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);

if (!$_GET["id"]) {
	die("return");
}
if (!$_GET["day"]) {
	$now_time = date('ymd', time());
} else {
	$now_time = $_GET["day"];
}

$id = $_GET["id"];
$pow_arr = array();
$pow_result_arr = array();
$pow_result_sum_arr = array();
$i = 0;
$stack = 0;
$sql = "SELECT time_data, energy FROM POWER_INT WHERE ".$now_time."00 <= time_data AND ".($now_time + 1)."00 >=time_data AND id = ".$id;
$result = $db->query($sql);
while ($row = $result->fetchArray()) {
	$pow_arr[($row[0] - $now_time * 100)] = $row[1];
}
for($i = 0; $i <= 23; $i++) {
	if(!$pow_arr[$i]) {
		$pow_arr[$i] = 0;
	}
	$stack += $pow_arr[$i];
	array_push($pow_result_arr, array(str_pad($i, 2, "0", STR_PAD_LEFT).":00", $pow_arr[$i]));
	array_push($pow_result_sum_arr, array(str_pad($i, 2, "0", STR_PAD_LEFT).":00", $stack));

}

echo json_encode(array("pow"=>$pow_result_arr, "sum"=>$pow_result_sum_arr));

$query = $db->close();
?>