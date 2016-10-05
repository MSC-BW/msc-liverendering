#pragma once


// scene interface


#include <string>
#include <cstdlib>




struct Parameter
{
	enum EType
	{
		EUndefined,
		EFloat,
		EDouble,
		EInteger,
		EString,
		EColor,
		EPoint,
		EVector,
		ENormal,
		EMatrix44f,
		EMatrix44d,
		EPtr
	};

	Parameter():m_size(0), m_data(0), m_type(EUndefined), m_is_data_owner(false)
	{

	}

	~Parameter()
	{
		if(m_is_data_owner && (m_data!=0))
			free(m_data);
	}

	static std::string str( Parameter::EType type );
	static int size(Parameter::EType type);


	std::string m_name;
	EType m_type;
	int m_size;
	void* m_data;
	bool m_is_data_owner;

};





// marshalling and unmarshalling for czmq
#include <czmq.h>

zframe_t *zframe_from_string( const std::string string );
std::string string_from_zframe( zframe_t* frame );

zmsg_t* zmsg_from_param( const Parameter* param, zframe_t **content );
Parameter* param_from_zmsg( zmsg_t* msg );