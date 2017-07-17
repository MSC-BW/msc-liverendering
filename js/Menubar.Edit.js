function createMenubarEdit( editor )
{
	var container = new UI.Panel();
	container.setClass( 'menu' );

	var title = new UI.Panel();
	title.setClass( 'title' );
	title.setTextContent( 'Edit' );
	container.add( title );

	var options = new UI.Panel();
	options.setClass( 'options' );
	container.add( options );


	// Delete
	var option = new UI.Row();
	option.setClass( 'option' );
	option.setTextContent( 'delete selection' );
	option.onClick( function ()
	{
		//if ( confirm( 'Any unsaved data will be lost. Are you sure?' ) )
		//{
		//	editor.clear();
		//}
		if( editor.selected !== null )
		{
			editor.delete( editor.selected.name );
		}
		
	} );
	options.add( option );

	return container;
}