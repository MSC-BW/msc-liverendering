function createSidebarObject( editor )
{
	var signals = editor.signals;
	
	var container = new UI.Panel();
	container.setBorderTop( '0' );
	container.setPaddingTop( '20px' );
	container.setDisplay( 'none' );

	// ui elements for attributes ------------------------------
	// this is hardcoded for now but may be generated dynamically later on

	// object type
	var objectTypeRow = new UI.Row();
	var objectType = new UI.Text('test').setWidth( '90px' );

	objectTypeRow.add( new UI.Text( 'Type' ).setWidth( '90px' ) );
	objectTypeRow.add( objectType );
	container.add( objectTypeRow );

	var uiElements = {};
	var uiRows = {};


	// events ------------------------------
	signals.objectSelected.add( function ( object )
	{
		if ( object !== null )
		{
			container.setDisplay( 'block' );

			updateRows( object );
			updateUI( object );
		}else
		{
			container.setDisplay( 'none' );
		}

	} );


	// this function creates a row (dom element) for each attribute of each object type
	// these rows are created once on the first encounter and kept in a map
	// once they have been created, their visibiltiy is being updated according to the
	// selected object type
	function updateRows( object )
	{
		var objType = object.type;
		var attrNames = object.getAttrNames();
		var numAttributes = attrNames.length;

		// discover attributes and add rows as needed
		for (var i = 0; i < numAttributes; ++i)
		{
			var attrName = attrNames[i];
			var attr = object.getAttr(attrName);
			var attrType = attr.type();
			var uiInfo = object.getUIInfo(attrName);
			var rowId = objType + "." + attrName;

			// skip if row already exists
			if( rowId in uiRows )
				continue;

			// currently we only support single scalars and vectors
			if( !(attrType == Attribute.EType.EFloat ||
				  attrType == Attribute.EType.EV3f ||
				  attrType == Attribute.EType.EC3f ||
				  attrType == Attribute.EType.EString) )
				continue;



			var attrRow = new UI.Row();

			// add attribute name gui element
			var rowName = attrName;
			if(uiInfo)
				rowName = uiInfo.shortName;
			attrRow.add( new UI.Text( rowName ).setWidth( '90px' ) );

			// add edit elements
			if( attrType == Attribute.EType.EFloat )
			{
					var index = 0;
					var inputId = rowId + "." + index.toString();
					var value = attr.array()[index];
					var attrInput = new UI.Number(value).onChange( update ).setId( inputId );
					attrRow.add( attrInput );
					uiElements[inputId] = attrInput;
			}else
			if( attrType == Attribute.EType.EV3f )
			{
				for( var j=0;j<3;++j )
				{
					var index = j;
					var inputId = rowId + "." + index.toString();
					var value = attr.array()[index];
					var attrInput = new UI.Number(value).setWidth("50px").onChange( update ).setId( inputId );
					attrRow.add( attrInput );
					uiElements[inputId] = attrInput;
				}
			}else
			if( attrType == Attribute.EType.EC3f )
			{
				function rgbToHex(r, g, b)
				{
					return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
				}

				var color_hex = rgbToHex( parseInt(attr.array()[0]*255.0),
										  parseInt(attr.array()[1]*255.0),
										  parseInt(attr.array()[2]*255.0) );

				var inputId = rowId;
				var attrInput = new UI.Color().onChange( update ).setId( inputId ).setValue(color_hex);
				attrRow.add( attrInput );
				uiElements[inputId] = attrInput;
			}else
			if( attrType == Attribute.EType.EString )
			{
				var inputId = rowId;
				var attrInput = new UI.Input().setWidth( '150px' ).setFontSize( '12px' ).setValue(attr.m_strings[0]);
				attrRow.add( attrInput );
				uiElements[inputId] = attrInput;
			}


			
			container.add( attrRow );
			uiRows[rowId] = attrRow;
		}

		// first make all rows invisible
		for( var attrId in uiRows )
			uiRows[attrId].setDisplay( 'none' );

		// now make all rows visible, which belong to the type of the selected object
		for (var i = 0; i < numAttributes; ++i)
		{
			var attrName = attrNames[i];
			var attrId = objType + "." + attrName;
			if( attrId in uiRows )
				uiRows[attrId].setDisplay('');
		}
	}

	// updates ui values according to selected object
	function update( event )
	{
		var object = editor.selected;

		// update only makes sense if a valid object is being selected
		if( object !== null )
		{
			// now seperate target.id into objectType, attributename and optional index
			var split = event.target.id.split(".");
			var objectType = split[0];
			var attrName = split[1];
			var index = 0;

			if( split.length > 2 )
				index = parseInt(split[2]);

			// make sure that the objectType matches the type of selected object
			// this is a sanity check and should always be true since only rows
			// related to the given type are being made visible
			if( objectType == object.type )
			{
				//console.log(objectType, attrName, index);
				// instead of creating a new attribute for the setAttr command, we simply reuse
				// the attribute on the object and change it in place directly
				var attr = object.getAttr( attrName );
				var attrType = attr.type();
				// make sure we got a valid attribute
				if( attr !== null )
				{
					// now set the value
					if( attrType == Attribute.EType.EFloat ||
						attrType == Attribute.EType.EV3f)
					{
						attr.array()[index] = uiElements[event.target.id].getValue();

						var attr_list = [attr];
						editor.setAttr( object.name, attr_list )
					}else
					if(attrType == Attribute.EType.EC3f)
					{
						function hexToRgb(hex)
						{
							// Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
							var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
							hex = hex.replace(shorthandRegex, function(m, r, g, b)
							{
								return r + r + g + g + b + b;
							});

							var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
							return result ?
							{
								r: parseInt(result[1], 16),
								g: parseInt(result[2], 16),
								b: parseInt(result[3], 16)
							} : null;
						}

						var color_hex = uiElements[event.target.id].getValue();
						var color_rgb = hexToRgb(color_hex);
						attr.array()[0] = color_rgb.r / 255.0;
						attr.array()[1] = color_rgb.g / 255.0;
						attr.array()[2] = color_rgb.b / 255.0;

						var attr_list = [attr];
						editor.setAttr( object.name, attr_list )
					}
				}
			}
			

		}
	}

	function updateUI( object )
	{
		objectType.setValue( object.type );
	}

	return container;
}