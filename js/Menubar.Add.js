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
	option.setTextContent( 'crytek-sponza' );
	option.onClick( function ()
	{
		editor.message("loadSponza");
		editor.delete("fluidsurface");
		editor.delete("crytek-sponza");

		// add a dummy object which represents the loaded model
		// this is just a stub and wont do anything apart from showing the imported model filename
		var object = editor.create("Model", "crytek-sponza");

		// now set the modelname attribute
		var attr_file = new Attribute( "file", Attribute.EType.EString, 1 );
		attr_file.setString("/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/crytek-sponza/sponza.obj");
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

	// Fluidsurface
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'fluidsurface' );
	option.onClick( function ()
	{
		editor.message("loadFluidsurface");
		editor.delete("fluidsurface");
		editor.delete("crytek-sponza");
		// add a dummy object which represents the loaded model
		// this is just a stub and wont do anything apart from showing the imported model filename
		var object = editor.create("Model", "fluidsurface");

		// now set the modelname attribute
		var attr_file = new Attribute( "file", Attribute.EType.EString, 1 );
		attr_file.setString("/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/fluidsurface/fluidsurface_final_0200.bobj.gz");
		object.attributes[attr_file.name()] = attr_file;

		// update camera...
		viewport.arcball.azimuth = 0.0;
		viewport.arcball.elevation = 0.0;
		viewport.arcball.distance = 1.0;
		viewport.arcball.lookat = vec3.fromValues(-0.979078, -0.98, -0.974673);
		viewport.arcball.sensitivity_pan = .01;
		viewport.arcball.sensitivity_zoom = 0.001;		
		viewport.arcball.sensitivity_rotate = 0.6;
		viewport.updateCamera();

	} );
	options.add( option );


	// ---
	options.add( new UI.HorizontalRule() );


	// Sun
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'Sun' );
	option.onClick( function ()
	{
		// create lightsource ---
		editor.create( "DirectionalLight", "sun" );

		// set attributes on lightsource ---
		var attr_color = new Attribute( "color", Attribute.EType.EC3f, 1 );
		attr_color.setC3f(1.0, 0.94, 0.88);
		var attr_direction = new Attribute( "direction", Attribute.EType.EV3f, 1 );
		attr_direction.setV3f( -0.3, -1.0, 0.1 );
		var attr_intensity = new Attribute( "intensity", Attribute.EType.EFloat, 1 );
		attr_intensity.setFloat( 12.0 );
		var attr_angularDiameter = new Attribute( "angularDiameter", Attribute.EType.EFloat, 1 );
		attr_angularDiameter.setFloat( 0.53 );

		var attr_list = new Array();
		attr_list.push(attr_color);
		attr_list.push(attr_direction);
		attr_list.push(attr_intensity);
		attr_list.push(attr_angularDiameter);
		editor.setAttr("sun", attr_list);
	} );
	options.add( option );

	// HDRI
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'HDRI' );
	option.onClick( function ()
	{
		// create lightsource ---
		editor.create( "HDRILight", "hdri light" );

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
		editor.setAttr("hdri light", attr_list);
	} );
	options.add( option );

	return container;
}