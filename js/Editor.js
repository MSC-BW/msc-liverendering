class Editor
{
	constructor()
	{
		this.idcounter = 0;
		var Signal = signals.Signal;

		this.signals =
		{
			objectSelected: new Signal(),
			imageReceived:new Signal(),
			sceneGraphChanged: new Signal()
		};


		this.objects = {};
		this.objects_handle = {};


		this.camera = new PerspectiveCamera();
		this.camera.id = this.idcounter++;
		this.objects_handle["camera"] = this.camera;


		this.selected = null;

		this.websocket = null;
	}

	connect( url )
	{
		var editor = this;
		this.websocket = new WebSocket(url, "protocolOne");
		this.websocket.binaryType = "arraybuffer";
		this.websocket.onmessage =
		function (event)
		{
			// convert received data from arraybuffer to blob
			var mimeString = "";
			var blob = new Blob([new DataView(event.data)], { type: mimeString });

			// set image in renderview
			//renderview.loadImageFromBlob( blob );
			editor.signals.imageReceived.dispatch( blob );
		}
		this.websocket.onopen =
		function()
		{
			// initialize the default camera which has been created by ospray remotely
			var attr_fovy = editor.camera.getAttr("fovy");
			var attr_apertureRadius = editor.camera.getAttr("apertureRadius");
			var attr_focusDistance = editor.camera.getAttr("focusDistance");
			var attr_xform = editor.camera.getAttr("xform");

			var attr_list = new Array();
			attr_list.push(attr_fovy);
			attr_list.push(attr_apertureRadius);
			attr_list.push(attr_focusDistance);
			attr_list.push(attr_xform);
			editor.setAttr("camera", attr_list);
		}
	}


	selectById( id )
	{
		if ( id === this.camera.id )
		{
			this.select( this.camera );
			return;
		}
		//this.select( this.scene.getObjectById( id, true ) );
		this.select( this.objects[id] );
	}

	select( object )
	{
		if ( this.selected === object ) return;
		this.selected = object;

		//var uuid = null;
		//if ( object !== null )
		//{
		//	uuid = object.uuid;
		//}
		//this.config.setKey( 'selected', uuid );

		this.signals.objectSelected.dispatch( object );
	}

	execute( command )
	{
		if( this.websocket !== null )
		{
			// 1 means connection is open
			if( this.websocket.readyState == 1 )
			{
				this.websocket.send(command);
			}
		}
	}



	// rsi interface ---------------------------------------------
	message( text )
	{
		var command = message( text );
		this.execute(command);
	}

	setAttr( object_handle, attr_list )
	{
		// TODO: set attribute on object
		var object = this.objects_handle[object_handle];
		if(object)
		{
			for( var i=0, length = attr_list.length; i<length;++i )
			{
				var attr = attr_list[i];
				object.attributes[attr.name()] = attr;
			}
		}

		// all scene edits will be applied to the remote scene as well
		var command = setAttr( object_handle, attr_list );
		this.execute(command);
	}

	create( type, object_handle )
	{
		var object = null;
		// TODO: reflect object creation locally
		if( type == "SphereLight" )
		{
			object = new SphereLight();
		}else
		if( type == "DirectionalLight" )
		{
			object = new DirectionalLight();
		}else
		if( type == "HDRILight" )
		{
			object = new HDRILight();
		}else
		if( type == "Model" )
		{
			object = new Model();
		}

		if(object)
		{
			object.name = object_handle;
			object.id = this.idcounter++;
			this.objects[object.id] = object;
			this.objects_handle[object_handle] = object;
		}

		// all scene edits will be applied to the remote scene as well
		var command = create( type, object_handle );
		this.execute(command);

		// fire event to update gui ---
		this.signals.sceneGraphChanged.dispatch();

		return object;
	}

	delete( object_handle )
	{
		if( object_handle in this.objects_handle )
		{
			var object = this.objects_handle[object_handle];

			if( object == this.selected )
				this.select(null);

			delete this.objects_handle[object_handle];
			delete this.objects[object.id];
		}

		// all scene edits will be applied to the remote scene as well
		var command = rsiDelete( object_handle );
		this.execute(command);

		// fire event to update gui ---
		this.signals.sceneGraphChanged.dispatch();
	}

};