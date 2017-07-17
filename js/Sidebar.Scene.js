function createSidebarScene(editor)
{
	var container = new UI.Panel();
	container.setBorderTop( '0' );
	container.setPaddingTop( '20px' );


	// outliner  ---------------------------
	function buildOption( object, draggable )
	{
		var option = document.createElement( 'div' );
		option.draggable = draggable;
		option.innerHTML = buildHTML( object );
		option.value = object.id;
		return option;
	}

	function buildHTML( object )
	{
		var html = '<span class="type ' + object.type + '"></span> ' + object.name;
		return html;
	}

	var ignoreObjectSelectedSignal = false;

	var outliner = new UI.Outliner( editor );
	outliner.setId( 'outliner' );
	outliner.onChange( function ()
	{
		ignoreObjectSelectedSignal = true;

		editor.selectById( parseInt( outliner.getValue() ) );

		ignoreObjectSelectedSignal = false;

	} );



	container.add( outliner );
	container.add( new UI.Break() );







	function refreshUI()
	{
		var camera = editor.camera;
		//var scene = editor.scene;

		var options = [];
		options.push( buildOption( camera, false ) );
		//options.push( buildOption( scene, false ) );

		// now add all dynamic objects within the scene
		( function addObjects( objects, pad )
		{

			//for ( var i = 0, l = objects.length; i < l; ++i )
			for( i in objects )
			{
				var object = objects[i];

				var option = buildOption( object, true );
				//option.style.paddingLeft = ( pad * 10 ) + 'px';
				options.push( option );

				//addObjects( object.children, pad + 1 );
			}

		} )( editor.objects, 0 );




		outliner.setOptions( options );
	}


	refreshUI();

	// handle events ---
	editor.signals.sceneGraphChanged.add( refreshUI );

	return container;
}