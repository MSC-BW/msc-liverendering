function createTimeline( editor )
{
	var signals = editor.signals;

	var container = new UI.Panel();
	container.setId("timeline");






	// some globals...
	var scale = 32;
	var prevScale = scale;



	// TIMELINE CONTROL PANEL ===================================
	var panel = new UI.Panel();
	panel.setPosition( 'absolute' );
	panel.setWidth( '150px' );
	panel.setHeight( '100%' );
	panel.dom.style.background = '#555';
	container.add( panel );

	var controls = new UI.Panel();
	controls.setPosition( 'absolute' );
	controls.setWidth( '100%' );
	controls.setPadding( '5px 0px' );
	controls.setBackground( '#666' );
	panel.add( controls );

	var timeText = new UI.Text();
	timeText.setColor( '#bbb' );
	timeText.setMarginLeft( '5px' );
	timeText.setValue( '0:00.00' );
	controls.add( timeText );

	var updateTimeText = function ( time )
	{
		var minutes = Math.floor( time / 60 );
		var seconds = time % 60;
		var padding = seconds < 10 ? '0' : '';
		timeText.setValue( minutes + ':' + padding + seconds.toFixed( 2 ) );
	};


	// TIMELINE ===================================

	// the timeline ui panel...
	var timeline = new UI.Panel();
	timeline.setPosition( 'absolute' );
	timeline.setLeft( '150px' );
	timeline.setRight( '0px');
	timeline.setTop( '0px' );
	timeline.setBottom( '0px' );
	timeline.setOverflow( 'hidden' );
	container.add( timeline );


	// marks -------------------------------------------------------
	// add the marks within the timeline
	var marks = document.createElementNS( 'http://www.w3.org/2000/svg', 'svg' );
	marks.style.position = 'absolute';
	marks.setAttribute( 'width', '100%' );
	marks.setAttribute( 'height', '32px' );

	var marksPath = document.createElementNS( 'http://www.w3.org/2000/svg', 'path' );
	marksPath.setAttribute( 'style', 'stroke: #888; stroke-width: 1px; fill: none;' );
	marks.appendChild( marksPath );

	marks.addEventListener( 'mousedown', function ( event )
	{
		var onMouseMove = function ( event )
		{
			var new_time = ( event.offsetX + scroller.scrollLeft ) / scale;
			editor.setTime( new_time )
		};
		var onMouseUp = function ( event )
		{
			onMouseMove( event );

			marks.removeEventListener( 'mousemove', onMouseMove );
			document.removeEventListener( 'mouseup', onMouseUp );
		};

		marks.addEventListener( 'mousemove', onMouseMove, false );
		document.addEventListener( 'mouseup', onMouseUp, false );
	}, false );

	timeline.dom.appendChild( marks );


	// redraw marks callback
	var updateMarks = function ()
	{
		var drawing = '';
		var scale4 = scale / 4;
		var offset = - scroller.scrollLeft % scale;
		var width = marks.getBoundingClientRect().width || 1024;

		for ( var i = offset, l = width; i <= l; i += scale ) {

			drawing += 'M ' + i + ' 8 L' + i + ' 24';
			drawing += 'M ' + ( i + ( scale4 * 1 ) ) + ' 12 L' + ( i + ( scale4 * 1 ) ) + ' 20';
			drawing += 'M ' + ( i + ( scale4 * 2 ) ) + ' 12 L' + ( i + ( scale4 * 2 ) ) + ' 20';
			drawing += 'M ' + ( i + ( scale4 * 3 ) ) + ' 12 L' + ( i + ( scale4 * 3 ) ) + ' 20';

		}

		marksPath.setAttribute( 'd', drawing );
	};


	// timeMark -------------------------------------------------------
	var timeMark = document.createElement( 'div' );
	timeMark.style.position = 'absolute';
	timeMark.style.top = '0px';
	timeMark.style.left = '-8px';
	timeMark.style.width = '16px';
	timeMark.style.height = '100%';
	timeMark.style.background = 'url(' + ( function ()
	{
		var canvas = document.createElement( 'canvas' );
		canvas.width = 16;
		canvas.height = 1;

		var context = canvas.getContext( '2d' );
		context.fillStyle = '#f00';
		context.fillRect( 8, 0, 1, 1 );
		
		return canvas.toDataURL();
	}() ) + ')';
	timeMark.style.pointerEvents = 'none';
	timeline.dom.appendChild( timeMark );

	var updateTimeMark = function()
	{
		timeMark.style.left = ( editor.time * scale ) - scroller.scrollLeft - 8 + 'px';
	};

	// bottom scrollbar -------------------------------------------------------
	var scroller = document.createElement( 'div' );
	scroller.id = "scroller";
	scroller.style.position = 'absolute';
	scroller.style.top = '32px';
	scroller.style.bottom = '10px'
	scroller.style.width = '100%';
	scroller.style.overflow = 'auto';
	scroller.addEventListener( 'scroll', function ( event )
	{
		updateMarks();
		updateTimeMark();
	}, false );
	timeline.dom.appendChild( scroller );


	// add something to the scroller which reflects the duration
	var scroller_containers = new UI.Panel();
	scroller_containers.setHeight( '100%' );
	scroller_containers.setId("gaga");
	scroller.appendChild( scroller_containers.dom );


	var updateContainers = function ()
	{
		var width = editor.duration * scale;
		scroller_containers.setWidth( width + 'px' );
	};

	// initial update of the timeline
	updateMarks();


	// register signal callbacks --------------------------
	signals.timeChanged.add( function ( time )
	{
		updateTimeText( time );
		updateTimeMark();
	} );

	signals.durationChanged.add( function( duration )
	{
		updateContainers();
	} );

	return container;
}
