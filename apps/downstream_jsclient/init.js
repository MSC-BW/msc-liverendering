/*
 * Dies ist eine JavaScript-Umgebung.
 *
 * Geben Sie etwas JavaScript ein und führen Sie einen Rechtsklick aus oder wählen Sie aus dem Ausführen-Menü:
 * 1. Ausführen, um den ausgewählten Text zu evaluieren (Strg+R),
 * 2. Untersuchen, um den Objekt-Inspektor auf das Resultat anzuwenden (Strg+I), oder
 * 3. Anzeigen, um das Ergebnis in einem Kommentar hinter der Auswahl einzufügen. (Strg+L)
 */

//var test = mat4.create();
//alert(test[0]);



class RenderView
{
  constructor( width, height )
  {
    this.canvas = document.createElement("canvas");
    this.canvas.style.border = "solid #000000";
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


}



class ArcBall
{
	constructor()
	{
		this.azimuth = 0.0;
		this.elevation = 0.0;
		this.distance = 4.0;
		this.lookat = vec3.fromValues(0.0, 0.0, 0.0);
		//this.lookat = vec3.fromValues(1.0, 2.0, 3.0);
		this.localToWorld = mat4.create();

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



// disable context menus....
document.oncontextmenu = function()
{
    return false;
}






window.onload = function()
{
  // websocket test with rsi
/*
  var exampleSocket;

  exampleSocket = new WebSocket("ws://localhost:9002/test?id=test", "protocolOne");
  exampleSocket.binaryType = "arraybuffer";

  exampleSocket.onopen =
  function (event)
  {
    console.log("onopen");
    
    //var command = message( "hello compute node!" );

    var attr_list = new Array();
    var attr = new Attribute( "pos", Attribute.EType.EP3f, 1 );
    attr.array()[0] = 4.0;
    attr.array()[1] = 5.0;
    attr.array()[2] = 6.0;
    attr_list.push(attr);
    var command = setAttr( "test", attr_list );
    exampleSocket.send(command);


    // send json/text
    //var message = {command:"set"};
    //exampleSocket.send(JSON.stringify(message));
    
  }

  exampleSocket.onerror =
  function (event)
  {
    console.log("onerror");
  }
  */

  //message("hallo");
  //exampleSocket.close();








  // runs -----------
  var renderview = new RenderView(512, 512);
  var arcball = new ArcBall();
  document.body.appendChild(renderview.canvas);

  // load image from websocket
  var exampleSocket;
  try
  {
    exampleSocket = new WebSocket("ws://localhost:9002/test?id=test", "protocolOne");    
    exampleSocket.binaryType = "arraybuffer";
    exampleSocket.onmessage =
    function (event)
    {
      // convert received data from arraybuffer to blob
      var mimeString = "";
      var blob = new Blob([new DataView(event.data)], { type: mimeString });
      // set image in renderview
      renderview.loadImageFromBlob( blob );
    }


	renderview.mouseMoveCallback = function(event)
	{
		// getting mouse position correctly 
		var bRect = renderview.canvas.getBoundingClientRect();
		var mouseX = (event.clientX - bRect.left)*(renderview.canvas.width/bRect.width);
		var mouseY = (event.clientY - bRect.top)*(renderview.canvas.height/bRect.height);
		var buttons = event.buttons || event.which;
		var dx = mouseX - renderview.oldMouseX;
		var dy = mouseY - renderview.oldMouseY;

      //console.log("mouseMoveCallback");
      //console.log(mouseX);
      /*
      var rotate_x_sign = 1.0; // cycles
      //var rotate_x_sign = -1.0; // svpt

      var zoom_sign = -1.0; // svpt, cycles

      var pan_x_sign = -1.0; // cycles
      //var pan_x_sign = 1.0; // svpt


      // 1=lmb
      // 2=rmb
      // 4=mmb
      if( buttons == 1)
      {
        // orbit view
        canvas.azimuth += rotate_x_sign*dx;
        //canvas.elevation += -dy;
        canvas.elevation += dy;
      }else
      if( buttons == 4)
      {
        // pan
        var scale = 0.001;
        //var scale = 1.001;
        vec3.add( canvas.lookat, canvas.lookat, vec3.fromValues(pan_x_sign*dx*canvas.distance*scale*canvas.camToWorld[0], pan_x_sign*dx*canvas.distance*scale*canvas.camToWorld[1], pan_x_sign*dx*canvas.distance*scale*canvas.camToWorld[2]));
        vec3.add( canvas.lookat, canvas.lookat, vec3.fromValues(dy*canvas.distance*scale*canvas.camToWorld[4], dy*canvas.distance*scale*canvas.camToWorld[5], dy*canvas.distance*scale*canvas.camToWorld[6]));
      }else
      if( buttons == 2)
      {
        // zoom
        var scale = 0.005;
        canvas.distance += zoom_sign*dx*canvas.distance*scale;
      }

      // build transformation matrix and send to server
      //mat4.identity(canvas.camToWorld);
      //mat4.translate(canvas.camToWorld, canvas.camToWorld, canvas.lookat);
      //mat4.rotateY(canvas.camToWorld, canvas.camToWorld, canvas.azimuth*Math.PI/180.0);
      //mat4.rotateX(canvas.camToWorld, canvas.camToWorld, canvas.elevation*Math.PI/180.0);
      //mat4.translate(canvas.camToWorld, canvas.camToWorld, vec3.fromValues(0.0, 0.0, canvas.distance));
      
      // cycles / svpt
      mat4.identity(canvas.camToWorld);
      mat4.translate(canvas.camToWorld, canvas.camToWorld, canvas.lookat);
      mat4.rotateY(canvas.camToWorld, canvas.camToWorld, canvas.azimuth*Math.PI/180.0);
      mat4.rotateX(canvas.camToWorld, canvas.camToWorld, canvas.elevation*Math.PI/180.0);
      mat4.translate(canvas.camToWorld, canvas.camToWorld, vec3.fromValues(0.0, 0.0, -canvas.distance));

      //var message = {command:"set", properties:{cx:mouseX, cy:mouseY, azimuth:canvas.azimuth, elevation:canvas.elevation, lookat:canvas.lookat, camToWorld:mat4.str(canvas.camToWorld)}};
      var message = {command:"set", properties:{camToWorld:mat4.str(canvas.camToWorld)}};
      exampleSocket.send(JSON.stringify(message));
      */
      	var updateView = false;

		// 1=lmb
		// 2=rmb
		// 4=mmb
		if( buttons == 1)
		{
			// left mouse button pressed
			/*
			var attr_list = new Array();
			var attr = new Attribute( "position", Attribute.EType.EP3f, 1 );
			attr.array()[0] = mouseX;
			attr.array()[1] = mouseY;
			attr.array()[2] = 0.0;
			attr_list.push(attr);
			var command = setAttr( "sphere", attr_list );
			exampleSocket.send(command);
			*/
			// rotate
			arcball.elevation += dy;
			arcball.azimuth += -dx;
			arcball.build();
			updateView = true;
		}else
		if( buttons == 4)
		{
			var scale = 1.0;
			// middle mouse button pressed
			xform = arcball.getLocalToWorld();
			var right = vec3.fromValues(arcball.localToWorld[0]*scale*dx, arcball.localToWorld[1]*scale*dx, arcball.localToWorld[2]*scale*dx);
			var up = vec3.fromValues(arcball.localToWorld[4]*scale*dy, arcball.localToWorld[5]*scale*dy, arcball.localToWorld[6]*scale*dy);
			var dir = vec3.fromValues(arcball.localToWorld[8]*scale, arcball.localToWorld[9]*scale, arcball.localToWorld[10]*scale);

			vec3.add( arcball.lookat, arcball.lookat, up);
			vec3.add( arcball.lookat, arcball.lookat, right);
			arcball.build();
			updateView = true;
		}else
		if( buttons == 2)
		{
			// right mouse button pressed
			/*
			var attr_list = new Array();
			var attr = new Attribute( "radius", Attribute.EType.EFloat, 1 );
			attr.array()[0] = mouseX;
			attr_list.push(attr);
			var command = setAttr( "sphere", attr_list );
			exampleSocket.send(command);
			*/

			// zoom
			var zoom_sign = -1.0; //
			var scale = 0.005;
			//var scale = 1.0;
			arcball.distance += zoom_sign*dx*arcball.distance*scale;
			arcball.build();
			updateView = true;
		}

		if(updateView)
		{
			xform = arcball.getLocalToWorld();

			var attr_list = new Array();
			var attr = new Attribute( "xform", Attribute.EType.EM44f, 1 );
			for( i=0;i<16;++i )
				attr.array()[i] = xform[i];

			attr_list.push(attr);
			var command = setAttr( "camera", attr_list );
			exampleSocket.send(command);			
		}

/*
		xform = arcball.getLocalToWorld();
		var pos = vec3.fromValues(xform[12], xform[13], xform[14]);
    	console.log("pos=", pos[0], pos[1], pos[2]);

    	var dir = vec3.create();
    	vec3.sub( dir, arcball.lookat, pos );
    	vec3.normalize(dir, dir);
    	console.log("dir=", dir[0], dir[1], dir[2]);

    	//console.log( "localToWorld=", mat4.str(arcball.localToWorld) );
    	console.log( "localToWorld=", arcball.localToWorld[0], arcball.localToWorld[1], arcball.localToWorld[2], arcball.localToWorld[3]);
    	console.log( "             ", arcball.localToWorld[4], arcball.localToWorld[5], arcball.localToWorld[6], arcball.localToWorld[7]);
    	console.log( "             ", arcball.localToWorld[8], arcball.localToWorld[9], arcball.localToWorld[10], arcball.localToWorld[11]);
    	console.log( "             ", arcball.localToWorld[12], arcball.localToWorld[13], arcball.localToWorld[14], arcball.localToWorld[15]);
*/


		renderview.oldMouseX = mouseX;
		renderview.oldMouseY = mouseY;
    }
  }catch(e)
  {
  } 
}



//exampleSocket.close();
