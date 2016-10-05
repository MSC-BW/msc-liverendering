#include "si.h"




std::string Parameter::str( Parameter::EType type )
{
	switch(type)
	{
		case EUndefined:return "undefined";
		case EFloat:return "float";
		case EDouble:return "double";
		case EInteger:return "integer";
		case EString:return "string";
		case EColor:return "color";
		case EPoint:return "point";
		case EVector:return "vector";
		case ENormal:return "normal";
		case EMatrix44f:return "m44f";
		case EMatrix44d:return "m44d";
		case EPtr:return "void*";
	};
	return "undefined";
}


int Parameter::size( Parameter::EType type )
{
	switch(type)
	{
		case EUndefined:return -1;
		case EFloat:return sizeof(float);
		case EDouble:return sizeof(double);
		case EInteger:return sizeof(int);
		case EString:return -1;
		case EColor:return sizeof(float)*3;
		case EPoint:return sizeof(float)*3;
		case EVector:return sizeof(float)*3;
		case ENormal:return sizeof(float)*3;
		case EMatrix44f:return sizeof(float)*16;
		case EMatrix44d:return sizeof(double)*16;
		case EPtr:return sizeof(void*);
	};
	return -1;	
}


zframe_t *zframe_from_string( const std::string string )
{
	return zframe_new (&string[0], string.size());
}

std::string string_from_zframe( zframe_t* frame )
{
	return std::string( (char*)zframe_data(frame),  zframe_size (frame));
}

zmsg_t *zmsg_from_param( const Parameter* param, zframe_t **content )
{
	zmsg_t *m = zmsg_new ();

	// name
	zframe_t* name = zframe_from_string(param->m_name);
	zframe_set_more (name, 1);

	// type and size
	zframe_t* type_size = zframe_new (&param->m_type, sizeof(int)*2);
	zframe_set_more (type_size, 1);

	// data
	zframe_t* data = zframe_new (param->m_data, Parameter::size(param->m_type)*param->m_size);

	zmsg_append( m, &name );
	zmsg_append( m, &type_size );
	zmsg_append( m, &data );

	return m;
}

Parameter* param_from_zmsg( zmsg_t* msg )
{
	Parameter* param = new Parameter();

    // parameter name
    zframe_t* name = zmsg_first(msg);
    param->m_name = string_from_zframe(name);

    // parameter type and size
    zframe_t* type_size = zmsg_next(msg);
    memcpy( &param->m_type, zframe_data(type_size), sizeof(int)*2 );

    // parameter data
    zframe_t* data = zmsg_next(msg);
    param->m_data = zframe_data(data);
    param->m_size = zframe_size(data)/Parameter::size(param->m_type);


	return param;
}