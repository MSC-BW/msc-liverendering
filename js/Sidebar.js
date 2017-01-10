function createSidebar(editor)
{
	var container = new UI.Panel();
	container.setId( 'sidebar' );


	var sceneTab = new UI.Text( 'SCENE' );
	var settingsTab = new UI.Text( 'SETTINGS' );

	var tabs = new UI.Div();
	tabs.setId( 'tabs' );
	tabs.add( sceneTab, settingsTab );
	container.add( tabs );

	var scene = new UI.Span();
	scene.add( createSidebarScene(editor) );
	scene.add( createSidebarProperties(editor) );
	container.add(scene);
	var settings = new UI.Span();
	container.add(settings);


	// function for selecting a tab section ---
	function select( section )
	{
		sceneTab.setClass( '' );
		settingsTab.setClass( '' );

		scene.setDisplay( 'none' );
		settings.setDisplay( 'none' );

		switch ( section )
		{
			case 'SCENE':
				sceneTab.setClass( 'selected' );
				scene.setDisplay( '' );
				break;
			case 'SETTINGS':
				settingsTab.setClass( 'selected' );
				settings.setDisplay( '' );
				break;
		}
	}


	// set onClick behaviour for tabs ----
	function onClick( event )
	{
		select( event.target.textContent );
	}

	sceneTab.onClick( onClick );
	settingsTab.onClick( onClick )

	select( 'SCENE' );
	return container;
}