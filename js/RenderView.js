


class ArcBall
{
	constructor()
	{
		this.azimuth = 0.0;
		this.elevation = 0.0;
		this.distance = 4.0;
		this.lookat = vec3.fromValues(0.0, 0.0, 0.0);
		this.localToWorld = mat4.create();

		this.sensitivity_pan = 1.0;
		this.sensitivity_zoom = 0.005;
		this.sensitivity_rotate = 1.0;

		this.build();
	}

	build()
	{
		mat4.identity(this.localToWorld);
		mat4.translate(this.localToWorld, this.localToWorld, this.lookat);
		mat4.rotateY(this.localToWorld, this.localToWorld, this.azimuth*Math.PI/180.0);
		mat4.rotateX(this.localToWorld, this.localToWorld, this.elevation*Math.PI/180.0);
		mat4.translate(this.localToWorld, this.localToWorld, vec3.fromValues(0.0, 0.0, -this.distance));
	}

	getLocalToWorld()
	{
		return this.localToWorld;
	}
}


class RenderView
{
	constructor( editor, width, height )
	{
		this.editor = editor;
		var signals = editor.signals;

		var container = new UI.Panel();
		container.setId( 'viewport' );
		container.setPosition( 'absolute' );
		this.dom = container;


		this.canvas = document.createElement("canvas");
		this.canvas.style.border = "solid #000000";
		this.canvas.style.borderWidth = "thin";
		this.canvas.style.padding = 0;
		this.canvas.style.margin = "auto";
		this.canvas.style.display = "block";
		this.canvas.width = width;
		this.canvas.height = height;
		this.ctx = this.canvas.getContext("2d");


		this.oldMouseX = 0;
		this.oldMouseY = 0;

		this.onMouseDown = this.onMouseDown.bind(this)
		this.onMouseMove = this.onMouseMove.bind(this)
		this.onMouseUp = this.onMouseUp.bind(this)

		this.mouseMoveCallback = null;

		// add listener for capturing mouse interaction with the canvas
		this.canvas.addEventListener("mousedown", this.onMouseDown, false);

		container.add(new UI.Element( this.canvas ));



		var self = this;
		signals.imageReceived.add( function ( blob )
		{
			self.loadImageFromBlob(blob);
		} );


		// camera control
		this.arcball = new ArcBall();
		this.mouseMoveCallback = function(event)
		{
			// getting mouse position correctly 
			var bRect = self.canvas.getBoundingClientRect();
			var mouseX = (event.clientX - bRect.left)*(self.canvas.width/bRect.width);
			var mouseY = (event.clientY - bRect.top)*(self.canvas.height/bRect.height);
			var buttons = event.buttons || event.which;
			var dx = mouseX - self.oldMouseX;
			var dy = mouseY - self.oldMouseY;


			// 1=lmb
			// 2=rmb
			// 4=mmb
			if( buttons == 1)
			{
				// left mouse button pressed - rotate
				var scale = self.arcball.sensitivity_rotate;
				self.arcball.elevation += dy*scale;
				self.arcball.azimuth += -dx*scale;
				self.updateCamera();
			}else
			if( buttons == 4)
			{
				var scale = self.arcball.sensitivity_pan;
				// middle mouse button pressed - pan
				var xform = self.arcball.getLocalToWorld();
				var right = vec3.fromValues(self.arcball.localToWorld[0]*scale*dx, self.arcball.localToWorld[1]*scale*dx, self.arcball.localToWorld[2]*scale*dx);
				var up = vec3.fromValues(self.arcball.localToWorld[4]*scale*dy, self.arcball.localToWorld[5]*scale*dy, self.arcball.localToWorld[6]*scale*dy);
				//var dir = vec3.fromValues(self.arcball.localToWorld[8]*scale, self.arcball.localToWorld[9]*scale, self.arcball.localToWorld[10]*scale);

				vec3.add( self.arcball.lookat, self.arcball.lookat, up);
				vec3.add( self.arcball.lookat, self.arcball.lookat, right);
				self.updateCamera();
			}else
			if( buttons == 2)
			{
				// right mouse button pressed - zoom
				var zoom_sign = -1.0; //
				var scale = self.arcball.sensitivity_zoom;
				//var scale = 1.0;
				self.arcball.distance += zoom_sign*dx*self.arcball.distance*scale;
				self.updateCamera();
			}

			//console.log( "azimuth", this.arcball.azimuth );
			//console.log( "elevation", this.arcball.elevation );
			//console.log( "distance", this.arcball.distance );
			//console.log( "lookat", this.arcball.lookat );


			self.oldMouseX = mouseX;
			self.oldMouseY = mouseY;
	    }

	    this.updateCamera();
	}

	updateCamera()
	{
		this.arcball.build();
		var xform = this.arcball.getLocalToWorld();

		// TODO: get attribute from camera object
		var attr_list = new Array();
		var attr = new Attribute( "xform", Attribute.EType.EM44f, 1 );
		for( var i=0;i<16;++i )
			attr.array()[i] = xform[i];

		attr_list.push(attr);

		this.editor.setAttr( "camera", attr_list );
	}


	loadImageFromBlob(data)
	{
		var img = new Image;
		var self = this;
		//document.body.appendChild(img);
		img.onload = function()
		{
		  //alert(img.width);
		  //alert(img.height);
		  //self.ctx.drawImage( img, 0, 0, img.width, image.height );
		  // adjust size of canvas according to image
		  //self.canvas.width = img.width;
		  //self.canvas.height = img.height;
		  //self.ctx.drawImage( img, 0, 0, img.width, img.height );
		  self.ctx.drawImage( img, 0, 0, self.canvas.width, self.canvas.height );
		}
		img.src = URL.createObjectURL(data);
		//img.src = "data:image/jpeg;base64,"+window.btoa(data);
	}



	onMouseDown(event)
	{
		//console.log("mousedown");
		var bRect = this.canvas.getBoundingClientRect();
		var mouseX = (event.clientX - bRect.left)*(this.canvas.width/bRect.width);
		var mouseY = (event.clientY - bRect.top)*(this.canvas.height/bRect.height);
		this.oldMouseX = mouseX;
		this.oldMouseY = mouseY;

		// start tracking mouse movement once the canvas has been clicked
		window.addEventListener("mousemove", this.onMouseMove, false);
		// ignore mouse down events while tracking mouse movement
		this.canvas.removeEventListener("mousedown", this.onMouseDown, false);
		// start listening to mouseup events which will stop tracking
		window.addEventListener("mouseup", this.onMouseUp, false);
	}

	onMouseMove(event)
	{
		//console.log("mousemove");
		// check of movecallback has been set
		if( typeof this.mouseMoveCallback == "function")
		{
		  this.mouseMoveCallback(event);
		}
	}

	onMouseUp(event)
	{
		//console.log("mouseup");
		// start listening for mousedown events again
		this.canvas.addEventListener("mousedown", this.onMouseDown, false);
		// remove mouseup event listener
		window.removeEventListener("mouseup", this.onMouseUp, false);
		window.removeEventListener("mousemove", this.onMouseMove, false);
	}


};