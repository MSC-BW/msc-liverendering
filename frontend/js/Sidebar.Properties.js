function createSidebarProperties( editor )
{
	var container = new UI.Span();

	var objectTab = new UI.Text( 'OBJECT' );
	objectTab.onClick( onClick );

	var tabs = new UI.Div();
	tabs.setId( 'tabs' );
	tabs.add( objectTab );
	container.add( tabs );

	function onClick( event )
	{
		select( event.target.textContent );
	}

	var object = new UI.Span();
	object.add( createSidebarObject( editor ) );
	container.add( object );

	function select( section )
	{
		objectTab.setClass( '' );
		
		object.setDisplay( 'none' );

		switch ( section )
		{
			case 'OBJECT':
				objectTab.setClass( 'selected' );
				object.setDisplay( '' );
				break;
		}
	}

	select( 'OBJECT' );
	return container;
}