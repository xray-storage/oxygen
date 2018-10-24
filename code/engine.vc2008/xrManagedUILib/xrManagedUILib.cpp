#include "stdafx.h"
#include "xrManagedUILib.h"

XRay::xrManagedUILib::UIXMLParser::UIXMLParser() : NativeParserObject(new CUIXml())
{
}

XRay::xrManagedUILib::UIXMLParser::~UIXMLParser()
{
	delete NativeParserObject;
}
