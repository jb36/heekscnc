// Op.h
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#ifndef OP_HEADER
#define OP_HEADER

#include "interface/ObjList.h"
#include "PythonStuff.h"

// #define OP_SKETCHES_AS_CHILDREN

class CMachineState;

class COp : public ObjList
{
public:
	wxString m_comment;
	bool m_active; // don't make NC code, if this is not active
	wxString m_title;
	int m_tool_number;	// joins the m_tool_number in one of the CTool objects in the tools list.
	int m_operation_type; // Type of operation (because GetType() overloading does not allow this class to call the parent's method)
        int m_coolant; // 0 = no coolant, 1 = mist, 2 = flood

 COp(const wxString& title, const int tool_number = 0, const int operation_type = UnknownType, const int coolant = 0 )
            :m_active(true), m_title(title), m_tool_number(tool_number),
          m_operation_type(operation_type), m_coolant(coolant)
    {
        ReadDefaultValues();
    }

	COp & operator= ( const COp & rhs );
	COp( const COp & rhs );

	// HeeksObj's virtual functions
	void GetProperties(std::list<Property *> *list);
	void WriteBaseXML(TiXmlElement *element);
	void ReadBaseXML(TiXmlElement* element);
	const wxBitmap& GetInactiveIcon();
	const wxChar* GetShortString(void)const{return m_title.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
	void GetTools(std::list<Tool*>* t_list, const wxPoint* p);
	void glCommands(bool select, bool marked, bool no_color);

	virtual void WriteDefaultValues();
	virtual void ReadDefaultValues();
	virtual Python AppendTextToProgram( CMachineState *pMachineState );
	virtual bool UsesTool(){return true;} // some operations don't use the tool number

	void ReloadPointers() { ObjList::ReloadPointers(); }

	// The DesignRulesAdjustment() method is the opportunity for all Operations objects to
	// adjust their parameters to values that 'make sense'.  eg: If a drilling cycle has a
	// profile operation as a reference then it should not have a depth value that is deeper
	// than the profile operation.
	// The list of strings provides a description of what was changed.
	virtual std::list<wxString> DesignRulesAdjustment(const bool apply_changes) { std::list<wxString> empty; return(empty); }

	bool operator==(const COp & rhs) const;
	bool operator!=(const COp & rhs) const { return(! (*this == rhs)); }
	bool IsDifferent(HeeksObj *other) { return( *this != (*((COp *)other)) ); }
};

#endif

