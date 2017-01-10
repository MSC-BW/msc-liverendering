function createMenubarFile( editor )
{
	var container = new UI.Panel();
	container.setClass( 'menu' );

	var title = new UI.Panel();
	title.setClass( 'title' );
	title.setTextContent( 'File' );
	container.add( title );

	var options = new UI.Panel();
	options.setClass( 'options' );
	container.add( options );


	// New
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'New' );
	option.onClick( function ()
	{
		//if ( confirm( 'Any unsaved data will be lost. Are you sure?' ) )
		//{
		//	editor.clear();
		//}
		//editor.execute(message("test"));
		//editor.message("test!");
		var attr_map = new Attribute( "map", Attribute.EType.EString, 1 );
		attr_map.setString("texture2d:/zhome/academic/HLRS/zmc/zmcdkoer/ospr/scenes/lightProbes/rnl_probe.pfm");

		var attr_list = new Array();
		attr_list.push(attr_map);

		editor.setAttr("hdri light", attr_list);
	} );
	options.add( option );

	return container;
}