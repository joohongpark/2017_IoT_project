var iType = [];
var options = {
	title: {
		text: "24시간 동안의 전력 사용량"
	},
            animationEnabled: true,
	data: [
	{
		type: "line", //change it to line, area, column, pie, etc
		dataPoints: [
			{ x: 0 , y: 5},
			{ x: 1, y: 10 },
			{ x: 2, y: 12 },
			{ x: 3, y: 8 },
			{ x: 4, y: 14 },
			{ x: 5, y: 6 },
			{ x: 6, y: 24 },
			{ x: 7, y: 1 },
			{ x: 8, y: 10 },
			{ x: 9 , y: 5},
			{ x: 10, y: 10 },
			{ x: 11, y: 10 },
			{ x: 12, y: 12 },
			{ x: 13, y: 8 },
			{ x: 14, y: 14 },
			{ x: 15, y: 6 },
			{ x: 16, y: 24 },
			{ x: 17, y: 1 },
			{ x: 18, y: 10 },
			{ x: 19 , y: 5},
			{ x: 20 , y: 5},
			{ x: 21, y: 10 },
			{ x: 22, y: 12 },
			{ x: 23, y: 8 }
		]
	}
	]
};
$(document).ready(function(){
	$(".lists").click(function(){
		if( iType[this.id] == null ) {
			$( this ).animate( {height: 400 }, 1000, function() {
				$(this).append($('<div class="info"></div>'));
				$(".info").CanvasJSChart(options)
			} );
			iType[this.id] = 1;
	} else {
		$( this ).animate( {height: 80}, 1000, function() {
			$(this).find(".info").remove();
		} );
		iType[this.id] = null;
    }
	});
	intv = setInterval(function() {
			$("#2").find(".pow").html(Math.floor(Math.random(5) * 100) + "KWh");
			$("#1").find(".pow").html(Math.floor(Math.random(5) * 100) + "KWh");

	}, 1000);
});