<?php
date_default_timezone_set('Asia/Seoul');
$now_time = floor((time() - strtotime(date('Y/m/d', time()))) / 60);
//echo strftime("%H%M", strtotime("12/28/2002 08:40"));
//echo date('Y-m-d', time());
$db = new SQLite3('dbdb.db',SQLITE3_OPEN_READWRITE);
$sql = "SELECT POWER_NOW.id, POWER_NOW.pow, POWER_NOW.energy, ONOFF.onoff, NAME.dev_name FROM POWER_NOW INNER JOIN NAME, ONOFF ON POWER_NOW.id = NAME.id AND POWER_NOW.id = ONOFF.id WHERE POWER_NOW.id IN (SELECT id FROM IS_EXISTS WHERE exist = 1)";
$sql_timer_exists = "SELECT id FROM TIMER GROUP BY id";


$data = array(); // $data라는 공간 생성
$name = array();
$timer_exists_arr = array();

$result = $db->query($sql);
$timer_exists = $db->query($sql_timer_exists);
while ($row = $timer_exists->fetchArray()) {
	$timer_exists_arr[$row[0]] = true;
}
while ($row = $result->fetchArray()) {
	//$sql_sw_result = $db->query($sql_sw);
	//echo $sql_sw_result;
	array_push($data, array('id'=>$row[0], 'energy'=>$row[1], 'pow'=>$row[2], 'onoff'=>$row[3]?true:false, 'has_timer'=>$timer_exists_arr[$row[0]]?true:false));
	array_push($name, array('name'=>$row[4]));
}
echo json_encode(array("dev"=>$data, "dev_name"=>$name));
//$r = $result->fetchArray();
//echo $r[3];
$db->close();
?>