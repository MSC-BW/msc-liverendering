
function pad(n, width, z)
{
  z = z || '0';
  n = n + '';
  return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
}

function createMenubarAdd( editor )
{
	var container = new UI.Panel();
	container.setClass( 'menu' );

	var title = new UI.Panel();
	title.setClass( 'title' );
	title.setTextContent( 'Add' );
	container.add( title );

	var options = new UI.Panel();
	options.setClass( 'options' );
	container.add( options );





	// Sponza
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'sponza' );
	option.onClick( function ()
	{
		//var model_file = "/lustre/cray/ws8/ws/zmcdkoer-ospraydemo/scenes/crytek-sponza/sponza.msg";
		var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/crytek-sponza/sponza.msg";

		//editor.message("loadSponza");
		editor.message("loadModel:"+model_file);
		editor.delete("fluidsurface");
		editor.delete("sponza");
		editor.delete("san-miguel");

		// add a dummy object which represents the loaded model
		// this is just a stub and wont do anything apart from showing the imported model filename
		var object = editor.create("Model", "sponza");

		// now set the modelname attribute
		var attr_file = new Attribute( "file", Attribute.EType.EString, 1 );
		attr_file.setString(model_file);
		
		object.attributes[attr_file.name()] = attr_file;

		// set camera view 
		viewport.arcball.azimuth = -433.0;
		viewport.arcball.elevation = -5.12;
		viewport.arcball.distance = 252.0;
		viewport.arcball.lookat = vec3.fromValues(85, 231.0, -56.0);
		viewport.arcball.sensitivity_pan = 1.0;
		viewport.arcball.sensitivity_zoom = 0.005;
		viewport.arcball.sensitivity_rotate = 1.0;
		viewport.updateCamera();
	} );
	options.add( option );

	// San-Miguel
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'san-miguel' );
	option.onClick( function ()
	{
		//var model_file = "/lustre/cray/ws8/ws/zmcdkoer-ospraydemo/scenes/san-miguel/sanMiguel.msg";
		var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/san-miguel/sanMiguel.msg";

		//editor.message("loadSanMiguel");
		editor.message("loadModel:"+model_file);
		editor.delete("fluidsurface");
		editor.delete("sponza");
		editor.delete("san-miguel");

		// add a dummy object which represents the loaded model
		// this is just a stub and wont do anything apart from showing the imported model filename
		var object = editor.create("Model", "san-miguel");

		// now set the modelname attribute
		var attr_file = new Attribute( "file", Attribute.EType.EString, 1 );
		attr_file.setString(model_file);
		
		object.attributes[attr_file.name()] = attr_file;

		// set camera view 
		viewport.arcball.azimuth = 213.16;
		viewport.arcball.elevation = 29.8;
		viewport.arcball.distance = 13.3;
		viewport.arcball.lookat = vec3.fromValues(-7.8, -3.1, 13.3);
		viewport.arcball.sensitivity_pan = .08;
		viewport.arcball.sensitivity_zoom = 0.0025;
		viewport.arcball.sensitivity_rotate = .1;
		viewport.updateCamera();
	} );
	options.add( option );

	// Fluidsurface
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'fluidsurface' );
	option.onClick( function ()
	{
		//var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.bobj.gz";
		//var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.msg";
		//var model_file = "/lustre/cray/ws8/ws/zmcdkoer-ospraydemo/scenes/fluidsurface/fluidsurface_final_0200.msg";
		//var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface2/fluidsurface.0018.obj";
		//var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface3/tmp/fluidsurface_final_0052.bobj.gz";
		var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface3/fluidsurface.0052.msg";

		//editor.message("loadFluidsurface");
		//editor.message("loadModel:"+model_file);
		editor.delete("fluidsurface");
		editor.delete("sponza");
		editor.delete("san-miguel");
		// add a dummy object which represents the loaded model
		// this is just a stub and wont do anything apart from showing the imported model filename
		var object = editor.create("Model", "fluidsurface");

		// now set the modelname attribute
		var attr_file = new Attribute( "file", Attribute.EType.EString, 1 );
		attr_file.setString(model_file);
		
		object.attributes[attr_file.name()] = attr_file;

		// update camera...
		/*
		// old fluidsurface from Ari:
		viewport.arcball.azimuth = -49.0;
		viewport.arcball.elevation = 31.0;
		viewport.arcball.distance = 2.0;
		viewport.arcball.lookat = vec3.fromValues(0.349, -0.245, -0.337);
		viewport.arcball.sensitivity_pan = .01;
		viewport.arcball.sensitivity_zoom = 0.001;		
		viewport.arcball.sensitivity_rotate = 0.6;
		viewport.updateCamera();
		*/
		/*
		// houdini animation:
		viewport.arcball.azimuth = -242;
		viewport.arcball.elevation = 45.3;
		viewport.arcball.distance = 36.6;
		viewport.arcball.lookat = vec3.fromValues(-0.324, -2.732, 1.428);
		viewport.arcball.sensitivity_pan = .01;
		viewport.arcball.sensitivity_zoom = 0.001;		
		viewport.arcball.sensitivity_rotate = 0.6;
		viewport.updateCamera();
		*/
		// houdini animation:
		viewport.arcball.azimuth = 0;
		viewport.arcball.elevation = 45.0;
		viewport.arcball.distance = 2.2;
		viewport.arcball.lookat = vec3.fromValues(-0.0038611299823969603, -0.02474219910800457, 0.9438909888267517);
		viewport.arcball.sensitivity_pan = .01;
		viewport.arcball.sensitivity_zoom = 0.001;		
		viewport.arcball.sensitivity_rotate = 0.6;
		viewport.updateCamera();
		editor.setDuration(150);
		editor.signals.timeChanged.add( function ( time )
		{
			var frame = Math.trunc(time);
			console.log(frame, time)
			//var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface3/tmp/fluidsurface_final_"+pad(frame.toString(), 4)+".bobj.gz";
			var model_file = "/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface3/fluidsurface."+pad(frame.toString(), 4)+".msg";
			console.log(model_file);
			editor.message("loadModel:"+model_file);
		} );

		editor.setTime(editor.time);

	} );
	options.add( option );


	// ---
	options.add( new UI.HorizontalRule() );

	// Sphere
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'Sphere' );
	option.onClick( function ()
	{
		// create lightsource ---
		var handle = "sphere light";
		editor.create( "SphereLight", handle );

		// set attributes on lightsource ---
		var attr_color = new Attribute( "color", Attribute.EType.EC3f, 1 );
		attr_color.setC3f(1.0, 0.94, 0.88);
		var attr_position = new Attribute( "position", Attribute.EType.EV3f, 1 );
		//attr_position.setV3f( 0.0, 0.0, 0.0 );
		attr_position.setV3f( 0.64, 2.26, 18.34 );
		var attr_intensity = new Attribute( "intensity", Attribute.EType.EFloat, 1 );
		attr_intensity.setFloat( 1.0 );
		var attr_radius = new Attribute( "radius", Attribute.EType.EFloat, 1 );
		attr_radius.setFloat( 0.1 );

		var attr_list = new Array();
		attr_list.push(attr_color);
		attr_list.push(attr_position);
		attr_list.push(attr_intensity);
		attr_list.push(attr_radius);
		editor.setAttr(handle, attr_list);
	} );
	options.add( option );


	// Directional
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'Directional' );
	option.onClick( function ()
	{
		// create lightsource ---
		var handle = "directional light";
		editor.create( "DirectionalLight", handle );

		// set attributes on lightsource ---
		var attr_color = new Attribute( "color", Attribute.EType.EC3f, 1 );
		attr_color.setC3f(1.0, 0.94, 0.88);
		var attr_direction = new Attribute( "direction", Attribute.EType.EV3f, 1 );
		attr_direction.setV3f( -0.3, -1.0, 0.1 );
		var attr_intensity = new Attribute( "intensity", Attribute.EType.EFloat, 1 );
		attr_intensity.setFloat( 1.0 );
		var attr_angularDiameter = new Attribute( "angularDiameter", Attribute.EType.EFloat, 1 );
		attr_angularDiameter.setFloat( 0.53 );

		var attr_list = new Array();
		attr_list.push(attr_color);
		attr_list.push(attr_direction);
		attr_list.push(attr_intensity);
		attr_list.push(attr_angularDiameter);
		editor.setAttr(handle, attr_list);
	} );
	options.add( option );

	// HDRI
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'HDRI' );
	option.onClick( function ()
	{
		// create lightsource ---
		var handle = "hdri light";
		editor.create( "HDRILight", handle );

		// set attributes on lightsource ---
		/*
		var attr_up = new Attribute( "up", Attribute.EType.EV3f, 1 );
		attr_up.setV3f( 0.0, 1.0, 0.0 );
		var attr_dir = new Attribute( "dir", Attribute.EType.EV3f, 1 );
		attr_dir.setV3f( 1.0, 0.0, 0.0 );
		*/

		var attr_intensity = new Attribute( "intensity", Attribute.EType.EFloat, 1 );
		attr_intensity.setFloat( 30.0 );
		var attr_map = new Attribute( "map", Attribute.EType.EString, 1 );
		attr_map.setString("texture2d:/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/lightProbes/rnl_probe.pfm");


		var attr_list = new Array();
		attr_list.push(attr_intensity);
		attr_list.push(attr_map);
		editor.setAttr(handle, attr_list);
	} );
	options.add( option );

	return container;
}