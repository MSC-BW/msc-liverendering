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