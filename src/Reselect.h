// Reselect.h
// Copyright (c) 2010, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "interface/Tool.h"

class ReselectSymbols: public Tool{
public:
        std::list< std::pair<int,unsigned int> > *m_symbols;
	HeeksObj* m_object;
	ReselectSymbols(): m_symbols(NULL), m_object(NULL){}

	// Tool's virtual functions
	const wxChar* GetTitle(){return _("Re-select sketches");}
	void Run();
	wxString BitmapPath(){ return _T("selsketch");}
};

class ReselectSketches: public Tool{
public:
	std::list<int> *m_sketches;
	HeeksObj* m_object;
	ReselectSketches(): m_sketches(NULL), m_object(NULL){}

	// Tool's virtual functions
	const wxChar* GetTitle(){return _("Re-select sketches");}
	void Run();
	wxString BitmapPath(){ return _T("selsketch");}
};

class ReselectSolids: public Tool{
public:
	std::list<int> *m_solids;
	HeeksObj* m_object;
	ReselectSolids(): m_solids(NULL), m_object(NULL){}

	// Tool's virtual functions
	const wxChar* GetTitle(){return _("Re-select solids");}
	void Run();
	wxString BitmapPath(){ return _T("selsolid");}
};

#ifdef OP_SKETCHES_AS_CHILDREN
void AddSolidsProperties(std::list<Property *> *list, HeeksObj* object);
void AddSketchesProperties(std::list<Property *> *list, HeeksObj* object);
#else
void AddSolidsProperties(std::list<Property *> *list, const std::list<int> &sketches);
void AddSketchesProperties(std::list<Property *> *list, const std::list<int> &sketches);
#endif
