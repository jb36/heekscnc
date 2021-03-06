
// Op.cpp
/*
 * Copyright (c) 2009, Dan Heeks
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */

#include <stdafx.h>
#include "Op.h"
#include "interface/PropertyInt.h"
#include "interface/PropertyString.h"
#include "interface/PropertyCheck.h"
#include "interface/PropertyChoice.h"
#include "interface/Tool.h"
#include "tinyxml/tinyxml.h"
#include "HeeksCNCTypes.h"
#include "CTool.h"
#include "PythonStuff.h"
#include "CNCConfig.h"
#include "MachineState.h"
#include "Program.h"
#ifdef HEEKSCNC
#define FIND_FIRST_TOOL CTool::FindFirstByType
#define FIND_ALL_TOOLS CTool::FindAllTools
#define MACHINE_STATE_TOOL(t) pMachineState->Tool(t)
#else
#define FIND_FIRST_TOOL heeksCNC->FindFirstToolByType
#define FIND_ALL_TOOLS heeksCNC->FindAllTools
#define MACHINE_STATE_TOOL(t) heeksCNC->MachineStateTool(pMachineState, t)
#endif

#include <iterator>
#include <vector>

const wxBitmap& COp::GetInactiveIcon()
{
	static wxBitmap* icon = NULL;
	if(icon == NULL)icon = new wxBitmap(wxImage(theApp.GetResFolder() + _T("/icons/noentry.png")));
	return *icon;
}

void COp::WriteBaseXML(TiXmlElement *element)
{
	if(m_comment.Len() > 0)element->SetAttribute( "comment", m_comment.utf8_str());
	element->SetAttribute( "active", m_active);
	element->SetAttribute( "title", m_title.utf8_str());
	element->SetAttribute( "tool_number", m_tool_number);
	element->SetAttribute( "coolant", m_coolant);

	ObjList::WriteBaseXML(element);
}

void COp::ReadBaseXML(TiXmlElement* element)
{
	const char* comment = element->Attribute("comment");
	if(comment)m_comment.assign(Ctt(comment));
	const char* active = element->Attribute("active");
	if(active)
	{
		int int_active;
		element->Attribute("active", &int_active);
		m_active = (int_active != 0);
	}
	else
	{
        m_active = true;
	}

	const char* title = element->Attribute("title");
	if(title)m_title = wxString(Ctt(title));

	if (element->Attribute("tool_number") != NULL)
	{
		m_tool_number = atoi(element->Attribute("tool_number"));
	} // End if - then
	else
	{
	    // The older files used to call this attribute 'cutting_tool_number'.  Look for this old
	    // name if we can't find the new one.
	    if (element->Attribute("cutting_tool_number") != NULL)
        {
            m_tool_number = atoi(element->Attribute("cutting_tool_number"));
        } // End if - then
        else
        {
            m_tool_number = 0;
        } // End if - else
	} // End if - else

	if (element->Attribute("coolant") != NULL)
	{
		m_coolant = atoi(element->Attribute("coolant"));
	} // End if - then
        else
        {
            m_coolant = 0;
        } // End if - else

	ObjList::ReadBaseXML(element);
}

static void on_set_comment(const wxChar* value, HeeksObj* object){((COp*)object)->m_comment = value;}
static void on_set_active(bool value, HeeksObj* object){((COp*)object)->m_active = value;heeksCAD->Changed();}

static void on_set_tool_number(int zero_based_choice, HeeksObj* object)
{
	if (zero_based_choice < 0) return;	// An error has occured.

	std::vector< std::pair< int, wxString > > tools = FIND_ALL_TOOLS();

	if ((zero_based_choice >= int(0)) && (zero_based_choice <= int(tools.size()-1)))
	{
                ((COp *)object)->m_tool_number = tools[zero_based_choice].first;	// Convert the choice offset to the tool number for that choice
	} // End if - then

	((COp*)object)->WriteDefaultValues();

} // End on_set_tool_number() routine

static void on_set_coolant(int zero_based_choice, HeeksObj* object)
{
	if (zero_based_choice < 0) return;	// An error has occured.
	if ((zero_based_choice >= 0) && (zero_based_choice <= 2))
	{
                ((COp *)object)->m_coolant = zero_based_choice;
	} // End if - then

	((COp*)object)->WriteDefaultValues();

} // End on_set_tool_number() routine


void COp::GetProperties(std::list<Property *> *list)
{
	list->push_back(new PropertyString(_("comment"), m_comment, this, on_set_comment));
	list->push_back(new PropertyCheck(_("active"), m_active, this, on_set_active));

	if(UsesTool()){
		std::vector< std::pair< int, wxString > > tools = FIND_ALL_TOOLS();

		int choice = 0;
                std::list< wxString > choices;
		for (std::vector< std::pair< int, wxString > >::size_type i=0; i<tools.size(); i++)
		{
                	choices.push_back(tools[i].second);

			if (m_tool_number == tools[i].first)
			{
                		choice = int(i);
			} // End if - then
		} // End for

		list->push_back(new PropertyChoice(_("tool"), choices, choice, this, on_set_tool_number));
	}
        
        {
          std::list< wxString > choices;
          choices.push_back(_("None"));
          choices.push_back(_("Mist"));
          choices.push_back(_("Flood"));
          list->push_back(new PropertyChoice(_("coolant"), choices, m_coolant, this, on_set_coolant));
        }

	ObjList::GetProperties(list);
}

COp & COp::operator= ( const COp & rhs )
{
	if (this != &rhs)
	{
		// In the case of machine operations, the child objects are all used
		// for reference (position etc.) only.  When we duplicate the machine
		// operation, we don't want to duplicate these reference (child)
		// objects too.
		// To this end, we want to copy the m_objects list without duplicating the
		// objects they point to.  i.e. don't call the ObjList::operator=( rhs ) method.

		m_objects.clear();
		for (HeeksObj *child = ((ObjList &)rhs).GetFirstChild(); child != NULL; child = ((ObjList &)rhs).GetNextChild())
		{
			m_objects.push_back( child );
		} // End for

		HeeksObj::operator=(rhs);	// We need to call this as we've skipped over the ObjList::operator=() method
									// which would normally have called it for us.

		m_comment = rhs.m_comment;
		m_active = rhs.m_active;
		m_title = rhs.m_title;
		m_tool_number = rhs.m_tool_number;
                m_coolant = rhs.m_coolant;
		m_operation_type = rhs.m_operation_type;
	}

	return(*this);
}

// Don't call the ObjList() constructor here as the duplication is
// handled in an unusual way by the assignment operator.
COp::COp( const COp & rhs ) // : ObjList(rhs)
{
	*this = rhs;	// Call the assignment operator.
}

void COp::glCommands(bool select, bool marked, bool no_color)
{
	ObjList::glCommands(select, marked, no_color);
}


void COp::WriteDefaultValues()
{
	CNCConfig config(GetTypeString());
	config.Write(_T("Tool"), m_tool_number);
	config.Write(_T("Coolant"), m_coolant);
}

void COp::ReadDefaultValues()
{
	if (m_tool_number <= 0)
	{
		// The tool number hasn't been assigned from above.  Set some reasonable
		// defaults.

		CNCConfig config(GetTypeString());

		// assume that default.tooltable contains tools with IDs:
		// 1 drill
		// 2 centre drill
		// 3 end mill
		// 4 slot drill
		// 5 ball end mill
		// 6 chamfering bit
		// 7 turn tool

		int default_tool = 0;
		switch(m_operation_type)
		{
		case DrillingType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eDrill );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eCentreDrill );
			break;
		case AdaptiveType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eEndmill );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eSlotCutter );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eBallEndMill );
			break;
		case ProfileType:
		case PocketType:
		case RaftType:
		case CounterBoreType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eEndmill );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eSlotCutter );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eBallEndMill );
			break;
		case ZigZagType:
		case WaterlineType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eEndmill );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eBallEndMill );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eSlotCutter );
			break;
		case TurnRoughType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eTurningTool );
			break;
        case PositioningType:
		case ProbeCentreType:
		case ProbeEdgeType:
		case ProbeGridType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eTouchProbe );
			break;
        case ChamferType:
        case InlayType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eChamfer );
			break;

        case TappingType:
			default_tool = FIND_FIRST_TOOL( CToolParams::eTapTool );
			break;

		default:
			default_tool = FIND_FIRST_TOOL( CToolParams::eEndmill );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eSlotCutter );
			if (default_tool <= 0) default_tool = FIND_FIRST_TOOL( CToolParams::eBallEndMill );
			if (default_tool <= 0) default_tool = 4;
			break;
		}
		config.Read(_T("Tool"), &m_tool_number, default_tool);
	} // End if - then

	if (m_coolant <= 0) m_coolant = 0;
}

/**
    Change tools (if necessary) and assign any private fixtures.
 */
Python COp::AppendTextToProgram(CMachineState *pMachineState )
{
    Python python;

	if(m_comment.Len() > 0)
	{
		python << _T("comment(") << PythonString(m_comment) << _T(")\n");
	}

	if(UsesTool())python << MACHINE_STATE_TOOL(m_tool_number);  // Select the correct tool.

        python << _T("coolant(mode=") << m_coolant << _T(")\n");

	return(python);
}

void COp::OnEditString(const wxChar* str){
	m_title.assign(str);
	heeksCAD->Changed();
}

void COp::GetTools(std::list<Tool*>* t_list, const wxPoint* p)
{
    ObjList::GetTools( t_list, p );
}

bool COp::operator==(const COp & rhs) const
{
	if (m_comment != rhs.m_comment) return(false);
	if (m_active != rhs.m_active) return(false);
	if (m_title != rhs.m_title) return(false);
	if (m_tool_number != rhs.m_tool_number) return(false);
	if (m_operation_type != rhs.m_operation_type) return(false);
        if (m_coolant != rhs.m_coolant) return(false);

	return(ObjList::operator==(rhs));
}

