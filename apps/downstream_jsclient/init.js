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








var Canvas = function(width, height)
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
  this.azimuth = 0.0;
  this.elevation = 0.0;
  this.distance = 4.0;
  this.lookat = vec3.fromValues(0.0, 0.0, 0.0);
  this.camToWorld = mat4.create();

  this.onMouseDown = this.onMouseDown.bind(this)
  this.onMouseMove = this.onMouseMove.bind(this)
  this.onMouseUp = this.onMouseUp.bind(this)

  this.mouseMoveCallback = null;
  
  // add listener for capturing mouse interaction with the canvas
  this.canvas.addEventListener("mousedown", this.onMouseDown, false);
}

Canvas.prototype.onMouseDown = function(event)
{
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

Canvas.prototype.onMouseMove = function(event)
{
  // check of movecallback has been set
  if( typeof this.mouseMoveCallback == "function")
  {
    this.mouseMoveCallback(event);
  }
}

Canvas.prototype.onMouseUp = function(event)
{
  // start listening for mousedown events again
  this.canvas.addEventListener("mousedown", this.onMouseDown, false);
  // remove mouseup event listener
  window.removeEventListener("mouseup", this.onMouseUp, false);
  window.removeEventListener("mousemove", this.onMouseMove, false);
}


Canvas.prototype.loadImageFromPath = function(path)
{
  var img = document.createElement("img");
  var self = this;
  img.onload = function()
  {
    self.ctx.drawImage( this, 0, 0, 50, 150 );
  }
  img.src = path;
}

Canvas.prototype.loadImage = function(img)
{
  this.ctx.drawImage(img, 0, 0, 50, 150);
}


function download(data, name)
{
    var a = document.createElement("a");
    a.href = URL.createObjectURL(data);
    a.download = name;
    document.body.appendChild(a);
    //a.appendChild( document.createTextNode("Hello World") );
    a.click();
}

Canvas.prototype.loadImageFromBlob = function(data)
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
  img.src = URL.createObjectURL( data );
  //img.src = "data:image/jpeg;base64,"+window.btoa(data);
}


document.oncontextmenu = function() {
    return false;
}





class OBuffer
{
  constructor()
  {
    this.m_size = 0;
    this.m_data = new Array();
  }

  write_int( value )
  {
    this.m_data.push( {type:"int", value:value, offset:this.m_size} );
    this.m_size += 4;
  }

  write_float( value )
  {
    this.m_data.push( {type:"float", value:value, offset:this.m_size} );
    this.m_size += 4;
  }

  write_string( string )
  {
    this.m_data.push( {type:"string", value:string, offset:this.m_size} );
    this.m_size += 4 + string.length; // additional 4 bytes for size int
  }

  write_binary( arraybuffer )
  {
    this.m_data.push( {type:"binary", value:arraybuffer, offset:this.m_size} );
    this.m_size += 4 + arraybuffer.byteLength;
  }

  to_arraybuffer()
  {
    var littleEndian = true;
    var encoder = new TextEncoder();

    var buffer = new ArrayBuffer(this.m_size);
    var dataview = new DataView(buffer);

    var numItems = this.m_data.length;
    for (var i = 0; i < numItems; ++i)
    {
      var data = this.m_data[i];
      var type = data.type;

      if(type == "int")
      {
        dataview.setInt32( data.offset, data.value, littleEndian );
      }
      else
      if(type == "float")
      {
        dataview.setFloat32( data.offset, data.value, littleEndian );
      }
      else
      if(type == "string")
      {
        var size = data.value.length;
        dataview.setInt32( data.offset, size, littleEndian );
        var uint8array = encoder.encode(data.value);
        for( var j=0;j<size;++j )
          dataview.setUint8( data.offset+4+j, uint8array[j] );
      }
      else
      if(type == "binary")
      {
        var size = data.value.byteLength;
        dataview.setInt32( data.offset, size, littleEndian );

        var uint8_src = new Uint8Array(data.value);
        var uint8_dest = new Uint8Array(buffer, data.offset+4, data.value.byteLength);
        uint8_dest.set( uint8_src );
      }
    }
    return dataview.buffer;
  }
}




class Attribute
{
  constructor( name, type, size )
  {
    this.m_name = name;
    this.m_type = type;
    this.m_size = size;
    // this is the underlying binary data
    this.m_buffer = new ArrayBuffer( this.memsize() );
    // we have this here for convienience...
    switch(type)
    {
      case Attribute.EType.EUndefined:break;
      case Attribute.EType.EDouble:break;
      case Attribute.EType.EInteger:break;
      case Attribute.EType.EString:break;
      case Attribute.EType.EFloat:
      case Attribute.EType.EC3f:
      case Attribute.EType.EP3f:
      case Attribute.EType.EV3f:
      case Attribute.EType.EN3f:
      case Attribute.EType.EM44f:this.m_arrayview = new Float32Array(this.m_buffer);break;
      case Attribute.EType.EM44d:break;
      case Attribute.EType.EPtr:break;
    }; 
  }

  array()
  {
    return this.m_arrayview;
  }

  memsize()
  {
    return this.m_size*Attribute.element_size(this.m_type);
  }


  static element_size( type )
  {
    switch(type)
    {
      case this.EType.EUndefined:return -1;
      case this.EType.EFloat:return 4;
      case this.EType.EDouble:return 8;
      case this.EType.EInteger:return 4;
      case this.EType.EString:return -1;
      case this.EType.EC3f:return 4*3;
      case this.EType.EP3f:return 4*3;
      case this.EType.EV3f:return 4*3;
      case this.EType.EN3f:return 4*3;
      case this.EType.EM44f:return 4*16;
      case this.EType.EM44d:return 8*16;
      case this.EType.EPtr:return 4;
    };
    return -1;  
  }
}

Attribute.EType = 
{
    EUndefined:0,
    EFloat:1,
    EDouble:2,
    EInteger:3,
    EString:4,
    EC3f:5, // color
    EP3f:6, // point
    EV3f:7, // vector
    EN3f:8, // normal
    EM44f:9,
    EM44d:10,
    EPtr:11  
}

EOpCode =
{
    ENOP : 0,
    EMessage : 1,
    ESetAttr : 2
}

function message( text )
{
  buf = new OBuffer();
  buf.write_int(EOpCode.EMessage);
  buf.write_string(text);
  return buf.to_arraybuffer();
}


function setAttr( object, attr_list )
{
  buf = new OBuffer();
  buf.write_int(EOpCode.ESetAttr);
  buf.write_string(object);

  var nattrs = attr_list.length;
  buf.write_int(nattrs);

  for( var i=0;i<nattrs;++i )
  {
    var attr = attr_list[i];
    buf.write_string(attr.m_name);
    buf.write_int(attr.m_type);
    buf.write_int(attr.m_size);
    buf.write_binary(attr.m_buffer);

  }

  return buf.to_arraybuffer();
}



window.onload = function()
{

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

  //message("hallo");
  //exampleSocket.close();








  // runs -----------
  //var canvas = new Canvas(512, 512);
  //document.body.appendChild(canvas.canvas);

  // load image from websocket
  //var exampleSocket;
  try
  {
    /*
    exampleSocket = new WebSocket("ws://localhost:9002/test?id=test", "protocolOne");    
    exampleSocket.binaryType = "arraybuffer";
    exampleSocket.onmessage =
    function (event)
    {
      canvas.loadImageFromBlob( event.data );
    }

    canvas.mouseMoveCallback = function(event)
    {
      // getting mouse position correctly 
      var bRect = canvas.canvas.getBoundingClientRect();
      var mouseX = (event.clientX - bRect.left)*(canvas.canvas.width/bRect.width);
      var mouseY = (event.clientY - bRect.top)*(canvas.canvas.height/bRect.height);
      var buttons = event.buttons || event.which;
      var dx = mouseX - canvas.oldMouseX;
      var dy = mouseY - canvas.oldMouseY;


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

      canvas.oldMouseX = mouseX;
      canvas.oldMouseY = mouseY;

    }
      */
    /*
    canvas.canvas.onclick = function()
    {
      var message = {command:"set", properties:{cx:0.5, cy:0.5}};
      //exampleSocket.send("Here's some text that the server is urgently awaiting!");
      exampleSocket.send(JSON.stringify(message));
    };
    */

  }catch(e)
  {
  } 
}



//exampleSocket.close();
