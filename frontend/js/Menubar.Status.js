function createMenubarStatus( editor )
{
	var container = new UI.Panel();
	container.setClass( 'menu right' );

	var connection_status = new UI.Text( 'not connected' );
	connection_status.setClass( 'title' );
	connection_status.setOpacity( 0.5 );
	container.add( connection_status );

/*
	var connection_button = new UI.Button( 'connect' );
	connection_button.onClick( function ()
	{
		console.log("button presses\n");
	} );
 
	//connection_button.setClass( 'title' );
	//connection_button.setOpacity( 0.5 );
	container.add( connection_button );
*/


	var version = new UI.Text( 'v0.1' );
	version.setClass( 'title' );
	version.setOpacity( 0.5 );
	container.add( version );


	editor.signals.connectionOpened.add( function ()
	{
		connection_status.setValue("connected");
	} );
	editor.signals.connectionClosed.add( function ()
	{
		connection_status.setValue("not connected");
	} );


	return container;
}