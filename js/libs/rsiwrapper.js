class PerspectiveCamera
{
	constructor()
	{
		this.type = "PerspectiveCamera";
		this.name = "camera";
		this.id = -1;

		this.attributes = {}
		this.attributes["fovy"] = new Attribute( "fovy", Attribute.EType.EFloat, 1 );
		this.attributes["fovy"].setFloat(60.0);
		this.attributes["apertureRadius"] = new Attribute( "apertureRadius", Attribute.EType.EFloat, 1 );
		this.attributes["apertureRadius"].setFloat(0.0);
		this.attributes["focusDistance"] = new Attribute( "focusDistance", Attribute.EType.EFloat, 1 );
		this.attributes["focusDistance"].setFloat(0.0);
		this.attributes["xform"] = new Attribute( "xform", Attribute.EType.EM44f, 1 );

		this.uiInfo = {};

		this.uiInfo["fovy"] = {shortName:"fov"};
		this.uiInfo["apertureRadius"] = {shortName:"aperture"};
		this.uiInfo["focusDistance"] = {shortName:"focal length"};
	}

	getAttr( name )
	{
		return this.attributes[name];
	}

	getAttrNames()
	{
		var attrNames = [];
		for( var attrName in this.attributes )
			attrNames.push(attrName);
		return attrNames;
	}

	getUIInfo( name )
	{
		return this.uiInfo[name];
	}
};

class DirectionalLight
{
  	constructor()
	{
		this.type = "DirectionalLight";
		this.name = "";
		this.id = -1;

		this.attributes = {}
		this.uiInfo = {};

		this.uiInfo["angularDiameter"] = {shortName:"spread"};
	}

	getAttr( name )
	{
		return this.attributes[name];
	}

	getAttrNames()
	{
		var attrNames = [];
		for( var attrName in this.attributes )
			attrNames.push(attrName);
		return attrNames;
	}

	getUIInfo( name )
	{
		return this.uiInfo[name];
	}
};


class HDRILight
{
	constructor()
	{
		this.type = "HDRILight";
		this.name = "";
		this.id = -1;

		this.attributes = {}
		this.uiInfo = {};
	}

	getAttr( name )
	{
		return this.attributes[name];
	}

	getAttrNames()
	{
		var attrNames = [];
		for( var attrName in this.attributes )
			attrNames.push(attrName);
		return attrNames;
	}

	getUIInfo( name )
	{
		return this.uiInfo[name];
	}
};


class Model
{
	constructor()
	{
		this.type = "Model";
		this.name = "";
		this.id = -1;

		this.attributes = {}
		this.uiInfo = {};
	}

	getAttr( name )
	{
		return this.attributes[name];
	}

	getAttrNames()
	{
		var attrNames = [];
		for( var attrName in this.attributes )
			attrNames.push(attrName);
		return attrNames;
	}

	getUIInfo( name )
	{
		return this.uiInfo[name];
	}
};