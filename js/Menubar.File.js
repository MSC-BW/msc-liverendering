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
		
	} );
	options.add( option );

	return container;
}