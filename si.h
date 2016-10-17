#pragma once

#include <string>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <sstream>




struct Attribute
{
	enum EType
	{
		EUndefined,
		EFloat,
		EDouble,
		EInteger,
		EString,
		EC3f, // color
		EP3f, // point
		EV3f, // vector
		EN3f, // normal
		EM44f,
		EM44d,
		EPtr
	};

	Attribute(const std::string& name, 
			  EType type,
			  int size ):
				m_name(name),
				m_size(size),
				m_data(malloc(size*element_size(type)), free),
				m_type(type)
	{
	}


	Attribute():
		m_name(),
		m_size(0),
		m_data(),
		m_type(EUndefined)
	{
	}

	~Attribute()
	{
	}

	void print()
	{
		std::cout << "Attribute:\n";
		std::cout << "\tname=" << m_name << std::endl;
		std::cout << "\ttype=" << type_str(m_type) << std::endl;
		std::cout << "\tsize=" << m_size << std::endl;
	}

	static std::string type_str( int type );
	static int element_size(int type); // returns size of a single element in bytes


	template<typename T=void>
	const T* ptr()const
	{
		return reinterpret_cast<T*>(m_data.get());
	}

	template<typename T=void>
	T* ptr()
	{
		return reinterpret_cast<T*>(m_data.get());
	}

	int memsize()const
	{
		return m_size*element_size(m_type);
	}


	std::string m_name;
	int m_type;
	int m_size; // number of elements of type m_type (i.e. number of floats, v3fs etc.)
	std::shared_ptr<void> m_data;
};




// CLIENT ====================================================
struct Command
{
	std::shared_ptr<void> data;
	std::string data_alt;
	int size;
};


Command message( const std::string& text );
Command setAttr( const std::string& object, const Attribute* attr_list, int nattrs );



// SERVER ====================================================
struct IScene
{
	virtual void message( const std::string& msg )=0;
	virtual void setAttr( const std::string& object_handle, const Attribute* attr_list, int nattrs )=0;
};


void execute( IScene* si, Command command );