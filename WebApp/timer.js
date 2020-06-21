var timerobj = new Object();
function show_timer( ele_id, id, data) {
	var element = document.getElementById(ele_id);
		var ele_canvaswrap = document.createElement("div");
			ele_canvaswrap.setAttribute("class", "canvaswrap");
			var ele_canvas = document.createElement("canvas");
				ele_canvas.setAttribute("id", "timer_" + id);
				ele_canvas.setAttribute("class", "canvas_timer");
				ele_canvas.setAttribute("width", p_width);
				ele_canvas.setAttribute("height", p_height);
			ele_canvaswrap.appendChild(ele_canvas);
		var ele_listswrap = document.createElement("div");
			ele_listswrap.setAttribute("style", "width: " + p_width + "px;");
			ele_listswrap.setAttribute("class", "listswrap");
			ele_listswrap.setAttribute("id", "timelist_" + id);
			var ele_wrap_small = document.createElement("div");
				ele_wrap_small.setAttribute("class", "wrap_small");
				var ele_list_button1 = document.createElement("button");
					ele_list_button1.setAttribute("class", "list_button");
					ele_list_button1.setAttribute("onclick", "timer_alerm(timerobj[" + id + "], " + id + ")");
					ele_list_button1.innerHTML = "시간 추가";
				var ele_list_button2 = document.createElement("button");
					ele_list_button2.setAttribute("class", "list_button");
					ele_list_button2.setAttribute("onclick", "save_timer_data_to_server(" + id + ",timerobj[" + id + "].export())");
					ele_list_button2.innerHTML = "적용";
				ele_wrap_small.appendChild(ele_list_button1);
				ele_wrap_small.appendChild(ele_list_button2);
			ele_listswrap.appendChild(ele_wrap_small);
		element.appendChild(ele_canvaswrap);
		element.appendChild(ele_listswrap);
	timerobj[id] = new draw_timer({id : ("timer_" + id), times : data});
	show_timer_list(timerobj[id].export(), id);
}
function create_timer_node( id, no, start, stop, s_no ) {
	var element = document.getElementById(id);
		var ul_lists = document.createElement("ul");
			ul_lists.setAttribute("class", "wrap_small");
			var li_lists = document.createElement("li");
				li_lists.setAttribute("class", "inner_text");
				var span_start = document.createElement("span");
					span_start.setAttribute("id", ("start" + no) );
					span_start.innerHTML = start;
				var span_center = document.createElement("span");
					span_center.innerHTML = " ~ ";
				var span_stop = document.createElement("span");
					span_stop.setAttribute("id", ("stop" + no) );
					span_stop.innerHTML = stop;
				li_lists.appendChild(span_start);
				li_lists.appendChild(span_center);
				li_lists.appendChild(span_stop);
			var li_del = document.createElement("li");
				li_del.setAttribute("class", "del_button_space");
				var button = document.createElement("button");
					button.setAttribute("class", "del_button");
					button.setAttribute("value", no);
					button.setAttribute("onclick", "kill_timer_alerm(timerobj[" + s_no + "],this.value, " + s_no + ")");
					button.innerHTML = "X";
				li_del.appendChild(button);
			ul_lists.appendChild(li_lists);
			ul_lists.appendChild(li_del);
		element.appendChild(ul_lists);
}
function kill_timer_alerm(obj, i, id) {
	obj.del(i);
	show_timer_list(obj.export(), id);
}
function timer_alerm(obj, id) {
	obj.addtime();
	show_timer_list(obj.export(), id);
}
function show_timer_list(lists, id) {
	var parentsnode = document.getElementById("timelist_" + id);
	if(!parentsnode.firstChild.className) {
		var child_saved = parentsnode.childNodes[1];
	} else {
		var child_saved = parentsnode.firstChild;
	}
	var grandpanode = parentsnode.parentNode;
	grandpanode.removeChild(parentsnode);
	var lists_el = document.createElement("div");
		lists_el.setAttribute("style", "width: " + p_width + "px;");
		lists_el.setAttribute("class", "listswrap");
		lists_el.setAttribute("id", "timelist_" + id);
		lists_el.appendChild(child_saved);
	grandpanode.appendChild(lists_el);
	for(i = 0; i<lists.length; i++) {
		var h1 = Math.floor( Math.ceil(lists[i].start_time) / 60 );
		var m1 = Math.ceil(lists[i].start_time) % 60;
		var h2 = Math.floor( Math.ceil(lists[i].stop_time) / 60 );
		var m2 = Math.ceil(lists[i].stop_time) % 60;
		create_timer_node(("timelist_" + id), i,(h1 + "시 " + m1 + "분"),(h2 + "시 " + m2 + "분"), id);
	}
}
function draw_timer(cfg) {
	var canvas = document.getElementById(cfg.id);
	var border = 5;
	var rad = (canvas.width / 2) - border; //  반지름 (캔버스 크기에 의해 자동으로 정해짐)
	var handler = this
	var times_obj = cfg.times;
	var times_n = cfg.times.length;
	var start_times = new Array();
	var stop_times = new Array();
	var nowcontrol = 0;

	var cursorAnimReady = true; // 원활한 프레임 구현
	var scope = new Object; // 선택자 객체
	var wheel = new Object; // 원 객체
	var select_start = new Object; // 선택버튼 (시작) 객체
	var select_stop = new Object; // 선택버튼 (시작) 객체
	var events_arr = new Array();
	var events_arr1 = new Array();

	for(i = 0; i < times_n; i++) {
		start_times[i] = times_obj[i].start_time;
		stop_times[i] = times_obj[i].stop_time;
	}

	select_start.draw = function (c,n) {
		var obj = c.getContext('2d');
		var pos = timetopos(start_times[n]);
		obj.beginPath();
		obj.arc(pos.x, pos.y, pos.rad, 0, 2 * Math.PI, false);
		// left top radius start end 시계방향
		obj.fillStyle = "rgba(255,255,255,0)";
		obj.fill();	
		//obj.strokeStyle = "black";
		obj.lineWidth = border;
		//obj.stroke();
		obj.closePath();
	};

	select_stop.draw = function (c,n) {
		var obj = c.getContext('2d');
		var pos = timetopos(stop_times[n]);
		obj.beginPath();
		obj.arc(pos.x, pos.y, pos.rad, 0, 2 * Math.PI, false);
		// left top radius start end 시계방향
		obj.fillStyle = "rgba(255,255,255,0)";
		obj.fill();	
		//obj.strokeStyle = "black";
		obj.lineWidth = border;
		//obj.stroke();
		obj.closePath();
	};

	scope.draw = function (c,n) {
		var obj = c.getContext('2d');
		//wheel.draw(c); // 원 다시 그림
		obj.beginPath();
		obj.moveTo(c.width / 2, c.height / 2);
		var start = (Math.PI/180) * -90 + (start_times[n] / 1440) * (2 * Math.PI);
		var stop = (Math.PI/180) * -90 + (stop_times[n] / 1440) * (2 * Math.PI);
		obj.arc(c.width / 2, c.height / 2, (c.width / 2) - border, start , stop, false);
		// left top radius start end 시계방향
		obj.fillStyle = "rgba(153,228,255,0.3)";
		obj.fill();		
		obj.strokeStyle = "#61d6ff";	
		obj.lineWidth = 1;
		obj.closePath();
		obj.stroke();		
	}

	wheel.draw = function (c) {
		var obj = c.getContext('2d');
		obj.beginPath();
		obj.arc(canvas.width / 2, canvas.height / 2, rad, 0, 2 * Math.PI, false);
		// left top radius start end 시계방향
		obj.strokeStyle = "black";
		obj.lineWidth = border;
		obj.stroke();
		obj.closePath();
	};
	function getAngle(newDot) { // 마우스 포인터가 가리키는 각을 구해줌 (입력변수 : 좌표)
		var angle;
		var atan = Math.atan2(newDot.x, newDot.y);
		if(atan < 0) {
			atan += 2 * Math.PI;
		}
		return atan;
	}

	function timetopos(time) { // 각도 (시간값)를 position 값으로 변환
		var theta = (time / 1440) * 2 * Math.PI;
		var canvas_posx = (canvas.width - border) / 2; // center
		var canvas_posy = (canvas.height - border) / 2; // center
		var posx = ( Math.sin(theta) + 1) * canvas_posx;
		var posy = (-Math.cos(theta) + 1) * canvas_posy;
		var rads = (rad / 10);
		return {x: posx, y: posy, rad: rads};
	}

	function timetopos_abs(time) { // 각도 (시간값)를 position 값으로 변환
		var theta = (time / 1440) * 2 * Math.PI;
		var canvas_posx = (canvas.width - border) / 2; // center
		var canvas_posy = (canvas.height - border) / 2; // center
		var posx = Math.sin(theta) * canvas_posx;
		var posy = Math.cos(theta) * canvas_posy;
		var rads = (rad / 10);
		return {x: posx, y: posy, rad: rads};
	}

	function getMousePosition(e) {
		e = e || window.event;
		var x, y;
		var scrollX = document.body.scrollLeft + document.documentElement.scrollLeft;
		var scrollY = document.body.scrollTop + document.documentElement.scrollTop;
		var posx = canvas.width / 2; // center
		var posy = canvas.height / 2; // center

		if (e.touches) {
			x = e.touches[0].clientX + scrollX;
			y = e.touches[0].clientY + scrollY;
		} else {
			// e.pageX e.pageY e.x e.y bad for cross-browser
			x = e.clientX + scrollX;
			y = e.clientY + scrollY;
		}

		var rect = canvas.getBoundingClientRect();
		x -= rect.left + scrollX;
		y -= rect.top + scrollY;

		// 2차원 좌표계로 변환
		x = x - posx;
		y = posy - y;

		return {x: x, y: y};
	}

	wheel.isDotIn = function (nowDot, point) {
		// is dot in circle
		if (Math.pow(nowDot.x - point.x, 2) + Math.pow(nowDot.y - point.y, 2) < Math.pow(point.rad, 2)) {
			return true;
		}
		return false;
	};

	function event_click() {
		addEventListner(canvas, "mousedown", function (e) {handler.mouseDown(e)});
		addEventListner(canvas, "touchstart", function (e) {handler.mouseDown(e)});
	}
	function addEventListner(object, event, callback) {
		if (!object)
			return false;

		events_arr[event] = callback;

		if (!object.addEventListener) {
			object.attachEvent('on' + event, events_arr[event]);
		} else {
			object.addEventListener(event, events_arr[event]);
		}

		return true;
	}
	function removeEventListener(object, event) {
		if (!object)
			return false;

		if (!events_arr[event])
			return false;

		if (!object.removeEventListener) {
			object.detachEvent('on' + event, events_arr[event]);
		} else {
			object.removeEventListener(event, events_arr[event]);
		}

		events_arr[event] = null;
		return true;
	}

	this.mouseDown = function (e) {
		var is_range = false;
		var is_start = false;
		var p = 0;
		var moves, end = false;
		for(i = 0; i < times_n; i++) {
			var s1 = wheel.isDotIn(getMousePosition(e),timetopos_abs(start_times[i]));
			var s2 = wheel.isDotIn(getMousePosition(e),timetopos_abs(stop_times[i]));
			if(s1 == true || s2 == true) {
				is_range = true;
				p = i;
				if(s1 == true)
					is_start = true;
			}
		}
		if(is_range == false) {
			console.log("범위 밖");
			return false;
		}
		if( events_arr1["mouseUp"] == true ) {
			events_arr1["mouseUp"] = false;
		}
		removeEventListener(canvas, "mousedown");
		removeEventListener(canvas, "touchstart");
		if(e.cancelable == true) {
			e.preventDefault();
		}
		if( is_start == true ) {
			events_arr1["mouseDown"] = true;
			events_arr1["mouseDown_Start"] = true;	
			events_arr1[p] = true;
			moves = function (o) {
				if(o.type === "mousemove") {
					handler.mouseMove(o);
				} else if(o.cancelable == true) {
					o.preventDefault();
					handler.mouseMove(o);
				}
			}
		} else {
			events_arr1["mouseDown"] = true;
			events_arr1["mouseDown_Stop"] = true;
			events_arr1[p] = true;		
			moves = function (o) {
				if(o.type === "mousemove") {
					handler.mouseMove(o);
				} else if(o.cancelable == true) {
					o.preventDefault();
					handler.mouseMove(o);
				}
			}
		}
		if(moves) {
			console.log("이벤트리스너");
			addEventListner(canvas, "touchmove", moves);
			addEventListner(canvas, "mousemove", moves);
			addEventListner(canvas, "mouseup", handler.mouseUp);
			addEventListner(canvas, "touchend", handler.mouseUp);
			addEventListner(canvas, "mouseout", handler.mouseUp);
		}

	}

	this.mouseMove = function (e) {
		events_arr1["mouseMove"] = true;
		var n = 0;
		for(i = 0; i < times_n; i++) {
			if(events_arr1[i] == true) {
				n = i;
			}
		}
		if (!cursorAnimReady) {
			return;
		}
		cursorAnimReady = false;

		if (events_arr1["mouseUp"] == true) {
			return;
		}
		if(events_arr1["mouseDown_Start"] == true) {
			start_times[n] = getAngle(getMousePosition(e)) / (2 * Math.PI);
			start_times[n] = start_times[n] * 1440;
			change_inhtml("start",start_times[n],n);
			draw_All();
		} else if(events_arr1["mouseDown_Stop"] == true) {
			stop_times[n] = getAngle(getMousePosition(e)) / (2 * Math.PI);
			stop_times[n] = stop_times[n] * 1440;
			change_inhtml("stop",stop_times[n],n);
			draw_All();
		}
		requestAnimationFrame(function () {
			cursorAnimReady = true;
		});
		
	}

	this.mouseUp = function (e) {
		for(i = 0; i < times_n; i++) {
			events_arr1[i] = false;
		}
		events_arr1["mouseUp"] = true;
		events_arr1["mouseDown_Start"] = false;
		events_arr1["mouseDown_Stop"] = false;
		events_arr1["mouseMove"] = false;
		removeEventListener(canvas, "touchmove");
		removeEventListener(canvas, "mousemove");
		removeEventListener(canvas, "mouseup");
		removeEventListener(canvas, "touchend");
		event_click();
	}

	this.mouseOver = function (e) {
		for(i = 0; i < times_n; i++) {
			events_arr1[i] = false;
		}
		events_arr1["mouseUp"] = false;
		events_arr1["mouseDown_Start"] = false;
		events_arr1["mouseDown_Stop"] = false;
		events_arr1["mouseMove"] = false;
		removeEventListener(canvas, "touchmove");
		removeEventListener(canvas, "mousemove");
		removeEventListener(canvas, "mouseup");
		removeEventListener(canvas, "touchend");
		event_click();
	}
	
	function change_inhtml(status, value, id) {
		var h = Math.floor( Math.ceil(value) / 60 );
		var m = Math.ceil(value) % 60;
		document.getElementById(status + id).innerHTML = h + "시 " + m + "분";
	}
	function draw_All () {
		canvas.getContext('2d').clearRect(0,0,canvas.width,canvas.height); // 화면 비우기	
		wheel.draw(canvas);	
		for(i = 0; i < times_n; i++) {
			scope.draw(canvas, i);
			select_start.draw(canvas, i);
			select_stop.draw(canvas, i);
		}	
	}
	this.addtime = function () {
		start_times.push(180);
		stop_times.push(300);
		times_n++;
		draw_All();

	};
	this.export = function () {
		var obj = new Array();
		for(i = 0; i < times_n; i++) {
			var tobj = new Object();
			tobj.start_time = Math.floor(start_times[i]);
			tobj.stop_time = Math.floor(stop_times[i]);
			obj.push(tobj);
			//obj[i]["stop_times"] = stop_times[i];
		}
		console.log(obj);
		return obj;

	};
	this.del = function (n) {
		if(times_n < n) {
			return false;
		}
		times_n--;
		start_times.splice(n, 1);
		stop_times.splice(n, 1);
		draw_All();
	};
	draw_All();
	event_click();
	//enableEvents();
}