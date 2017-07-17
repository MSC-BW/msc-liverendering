#include <rsi/rsi.h>
#include <stdexcept>
#include <vector>




std::string Attribute::type_str( int type )
{
	switch(type)
	{
		case EUndefined:return "undefined";
		case EFloat:return "float";
		case EDouble:return "double";
		case EInteger:return "integer";
		case EString:return "string";
		case EC3f:return "C3f";
		case EP3f:return "P3f";
		case EV3f:return "V3f";
		case EN3f:return "N3f";
		case EM44f:return "M44f";
		case EM44d:return "M44d";
		case EPtr:return "void*";
	};
	return "undefined";
}


int Attribute::element_size( int type )
{
	switch(type)
	{
		case EUndefined:return -1;
		case EFloat:return sizeof(float);
		case EDouble:return sizeof(double);
		case EInteger:return sizeof(int);
		case EString:return -1;
		case EC3f:return sizeof(float)*3;
		case EP3f:return sizeof(float)*3;
		case EV3f:return sizeof(float)*3;
		case EN3f:return sizeof(float)*3;
		case EM44f:return sizeof(float)*16;
		case EM44d:return sizeof(double)*16;
		case EPtr:return sizeof(void*);
	};
	return -1;	
}


enum EOpCode
{
	ENOP = 0,
	EMessage = 1,
	ESetAttr = 2,
	ECreate = 3,
	ERemove = 4
};

const std::string getString(const Attribute& attr, int index )
{
	int size = 0;
	const char* ptr = (char*)attr.m_data.get();
	for( int i=0;i<=index;++i )
	{
		 size = *((int*)ptr);
		 ptr += 4 + size;
	}

	ptr -= size;
	return std::string(ptr, size);
}



// SERIALIZATION HELPERS =====================================================

// here we give very simple binary buffers for serialization and deserialization
// these are used to pack scene manipulation commands into binary messages which
// can go over the wire

struct OBuffer
{
    OBuffer():message(std::stringstream::out | std::stringstream::binary)
    {
    }

    void write( const int& value )
    {
        message.write( reinterpret_cast<const char*>(&value), sizeof(int) );
    }

    void write( const float& value )
    {
        message.write( reinterpret_cast<const char*>(&value), sizeof(float) );
    }

    void write( const std::string& str )
    {
        int size = str.size();
        write( size );
        message.write( reinterpret_cast<const char*>(&str[0]), size );
    }

    void write( const char* ptr, int size )
    {
        write( size );
        message.write( ptr, size );
    }

    // returns a copy of the stream
    std::string to_string()
    {
        return message.str();
    }

 private:
    std::ostringstream message;
};


struct IBuffer
{
    IBuffer( char* data, int size ):message(std::stringstream::in | std::stringstream::binary)
    {
    	message.rdbuf()->pubsetbuf(data,size);
    }

    int read_int()
    {
    	int value = -1;
    	message.read( (char*)&value, sizeof(int));
    	return value;
    }

    int read_float()
    {
    	float value = -1.0f;
    	message.read( (char*)&value, sizeof(float));
    	return value;
    }

    std::string read_string()
    {
    	int size = read_int();
    	std::string str(size, ' ');

    	message.read( (char*)&str[0], size);

    	return str;
    }

    std::shared_ptr<void> read_data()
    {
    	int size = read_int();
    	std::shared_ptr<void> data( malloc(size), free );
    	message.read( (char*)data.get(), size );
    	return data;
    }

private:
	std::istringstream message;
};


// SERVER ===============================================

void execute( IScene* si, Command command )
{
	IBuffer buf((char*)command.data.get(), command.size);
	int op = buf.read_int();

	switch(op)
	{
		case EOpCode::EMessage:
		{
			si->message(buf.read_string());
		}break;
		case EOpCode::ESetAttr:
		{
			// number of attributes
			std::string handle = buf.read_string();
			int nattr = buf.read_int();

			std::vector<Attribute> attr_list;
			for( int i=0;i<nattr;++i )
			{
				// read attribute...
				Attribute attr;

				attr.m_name = buf.read_string();
				attr.m_type = buf.read_int();
				attr.m_size = buf.read_int();
				attr.m_data = buf.read_data();

				attr_list.push_back(attr);
			}

			si->setAttr( handle, &attr_list[0], attr_list.size() );
		}break;
		case EOpCode::ECreate:
		{
			std::string type = buf.read_string();
			std::string handle = buf.read_string();
			si->create( type, handle );
		}break;
		case EOpCode::ERemove:
		{
			std::string handle = buf.read_string();
			si->remove( handle );
		}break;
		default:
			//throw std::runtime_error("execute: unknown opcode");
			std::cout << "execute: unknown opcode\n";
	};
}


// CLIENT ================================================================

Command message( const std::string& text )
{
    OBuffer buf;

    buf.write( int(EOpCode::EMessage) );
    buf.write( text );

    Command cmd;
    cmd.data_alt = buf.to_string();
    cmd.size = cmd.data_alt.size();

    return cmd;
}

Command setAttr( const std::string& object, const Attribute* attr_list, int nattrs )
{
    OBuffer buf;

    buf.write( int(EOpCode::ESetAttr) );

    // serialize object handle ---
    buf.write( object );

    // serialize number of attributes ---
    buf.write( nattrs );

    // now for each attribute: expand the data buffer and serialize attribute ---
    for( int i=0;i<nattrs;++i )
    {
        // serialize attribute
        const Attribute& attr = attr_list[i];

        buf.write(attr.m_name);
        buf.write(int(attr.m_type));
        buf.write(attr.m_size);
        buf.write(attr.ptr<char>(), attr.memsize());
    }

    Command cmd;
    cmd.data_alt = buf.to_string();
    cmd.size = cmd.data_alt.size();

    return cmd;
}


