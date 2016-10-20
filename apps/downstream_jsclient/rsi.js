



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

// CLIENT API functions ======================================================

// here we define the client api functions which produce and return command objects.
// Command objects are binary representation of scene edits and can be used directly
// for sending it over the wire.

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




// SERIALIZATION HELPERS =====================================================

// here we give very simple binary buffers for serialization and deserialization
// these are used to pack scene manipulation commands into binary messages which
// can go over the wire

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
