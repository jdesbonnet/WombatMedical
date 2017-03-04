/**
 * This is a jQuery plugin which implements ECG chart
 * window.
 * 
 * To initialize:
 * $("#myECGChartId").preview ( {
 *  ... config options and callback functions ...
 * } );
 *
 * Configuration options:

 *
 * Callback functions:

 * 
 * Commands:
 * 
 *
 */
(function( $ ) {
	
	// Debug: total time used in animation function
	var at = 0;
	var options;
	var wrapperEl;
	var svgEl;


	// Circular buffer
	var dataLen = 0;
	var dataStartPtr = 0;
	var dataPtEls = [];
	var dataBufSize = 100;

	var chartX0, chartY0, chartWidth, chartHeight;

	var scrollInterval;

	var methods = {
		init : function( opts ) {
			options = opts;
			wrapperEl = this.get(0);
			dataBufSize = options.bufferSize;
			create();
		},
		addData : function (v) {

			if ( ! isNumber(v) ) {
				return;
			}

			var y = (chartY0 + chartHeight/2) - v*200 ;
			if (y<chartY0) y=chartY0;
			if (y>(chartY0+chartHeight)) y=chartY0+chartHeight;


			var endPtr = (dataStartPtr + dataLen) % dataBufSize;
			var el = dataPtEls[endPtr];
			if (el==null) {
				dataPtEls[endPtr] = createDataPointEl (chartX0+chartWidth,y);
				svgEl.appendChild(dataPtEls[endPtr]);
				dataLen++;
			} else {
				dataPtEls[endPtr].setAttribute("x",chartX0+chartWidth-4);
				dataPtEls[endPtr].setAttribute("y",y);
			}
			
			if (dataLen === dataBufSize) {
				dataStartPtr++;
			}

		},
		start : function () {
			console.log("start");
			scrollInterval = setInterval(function() {
				scroll();
			}, 20);
		},
		stop: function () {
			clearInterval(scrollInterval);
		},
		scroll : function () {
			scroll();
		},
					
	};
	 
	$.fn.ecg_chart = function(method) {
		// Method calling logic
		if ( methods[method] ) {
			return methods[ method ].apply( this, Array.prototype.slice.call( arguments, 1 ));
		} else if ( typeof method === 'object' || ! method ) {
			return methods.init.apply( this, arguments );
		} else {
			$.error( 'Method ' +  method + ' does not exist on jQuery.tooltip' );
		}
	};

	/**
	 * Time scroll chart by moving all points left.
	 */
	function scroll () {
		var i;
		for (i = 0; i < dataBufSize; i++) {
			el = dataPtEls[i];
			if (el==null) {
				continue;
			}
			x = el.getAttribute("x");
			x -= (chartWidth/dataBufSize);
			el.setAttribute("x",x);
		}
	}

	function create () {
		// Create <SVG> element and append to wrapperEl (the HTML DIV place holder)
		svgEl = document.createElementNS("http://www.w3.org/2000/svg", "svg");
		svgEl.id = "svgCanvas";
		svgEl.setAttribute("width",options.width);
		svgEl.setAttribute("height",options.height);	
		//svgEl.setAttribute("viewBox","0 0 1280 720"); // TODO: use WIDTH, HEIGHT
		wrapperEl.appendChild(svgEl);
		
		// White background, so if all layers have overlapping transparent regions, 
		// 'white' will be the default color.
		var svgBgEl = document.createElementNS("http://www.w3.org/2000/svg", "rect");
		svgBgEl.setAttribute("x",0);
		svgBgEl.setAttribute("y",0); 
		svgBgEl.setAttribute("width",options.width); 
		svgBgEl.setAttribute("height",options.height); 
		svgBgEl.setAttribute("fill","white");
		svgBgEl.setAttribute("stroke","none");
		svgEl.appendChild(svgBgEl);

		chartX0 = options.padding;
		chartY0 = options.padding;
		chartWidth = options.width - options.padding*2;
		chartHeight = options.height - options.padding*2;

		// Draw chart area
		
		var drawEl = document.createElementNS("http://www.w3.org/2000/svg", "rect");
		drawEl.setAttribute("x",chartX0);
		drawEl.setAttribute("y",chartY0);
		drawEl.setAttribute("width",chartWidth); 
		drawEl.setAttribute("height",chartHeight);
		drawEl.setAttribute("fill","none");
		drawEl.setAttribute("stroke","black");
		svgEl.appendChild(drawEl);

		// Draw y=0 line
		var zeroLineEl = document.createElementNS("http://www.w3.org/2000/svg", "line");
		zeroLineEl.setAttribute("x1",chartX0);
		zeroLineEl.setAttribute("y1",chartY0 + chartHeight/2);
		zeroLineEl.setAttribute("x2",chartX0+chartWidth);
		zeroLineEl.setAttribute("y2",chartY0 + chartHeight/2);
		zeroLineEl.setAttribute("stroke","black");
		svgEl.appendChild(zeroLineEl);

	}

	function createDataPointEl (x,y) {
		var ptEl = document.createElementNS("http://www.w3.org/2000/svg", "rect");
		ptEl.setAttribute("x",x-4);
		ptEl.setAttribute("y",y); 
		ptEl.setAttribute("width","4"); 
		ptEl.setAttribute("height","4"); 
		ptEl.setAttribute("fill","red");
		ptEl.setAttribute("stroke","none");
		return ptEl;
	}


	function isNumber(n) {
		return !isNaN(parseFloat(n)) && isFinite(n);
	}


})( jQuery );

