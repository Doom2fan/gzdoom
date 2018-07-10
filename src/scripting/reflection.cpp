/*
** reflection.cpp
** Implements reflection to ZScript
**
**---------------------------------------------------------------------------
** Copyright 1998-2016 Randy Heit
** Copyright 2005-2016 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

// HEADER FILES ------------------------------------------------------------

#include <float.h>
#include <limits>

#include "dobject.h"
#include "i_system.h"
#include "templates.h"
#include "doomerrors.h"
#include "vm.h"
#include "types.h"

enum RuntimeTypeInfo
{
	// Type
	RTI_Class = 1 << 0,
	RTI_Struct = 1 << 1,
	RTI_Method = 1 << 2,
	RTI_Field = 1 << 3,

	// Access
	RTI_Public = 1 << 4,
	RTI_Private = 1 << 5,
	RTI_Protected = 1 << 6,
	RTI_Native = 1 << 7,

	// Class
	RTI_Abstract = 1 << 8,

	// Method
	RTI_Action = 1 << 9,

	// other stuff
};

class DTypeInfo : public DObject
{
	DECLARE_ABSTRACT_CLASS (DTypeInfo, DObject)

	public:
	virtual FName GetName () = 0;
};

IMPLEMENT_CLASS (DTypeInfo, true, false);

class DClassInfo : public DTypeInfo
{
	DECLARE_ABSTRACT_CLASS (DClassInfo, DTypeInfo)

	public:
	PClass *cInfo;

	DClassInfo (PClass *classInfo)
	{
		cInfo = classInfo;
	}

	FName GetName ()
	{
		return cInfo->TypeName;
	}
};

IMPLEMENT_CLASS (DClassInfo, true, false);

class DFieldInfo : public DTypeInfo
{
	DECLARE_ABSTRACT_CLASS (DFieldInfo, DTypeInfo)

public:
	PField * fInfo;

	DFieldInfo (PField *fieldInfo)
	{
		fInfo = fieldInfo;
	}

	FName GetName ()
	{
		return fInfo->SymbolName;
	}
};

IMPLEMENT_CLASS (DFieldInfo, true, false);

DEFINE_ACTION_FUNCTION (_Reflection, GetClassInfo)
{
	PARAM_PROLOGUE;
	PARAM_CLASS_DEF (type, DObject);

	ACTION_RETURN_OBJECT (Create<DClassInfo> (type));
}

DEFINE_ACTION_FUNCTION (DTypeInfo, GetName)
{
	PARAM_SELF_PROLOGUE (DTypeInfo);

	ACTION_RETURN_INT (self->GetName ());
}

DEFINE_ACTION_FUNCTION (DClassInfo, GetFields)
{
	PARAM_SELF_PROLOGUE (DClassInfo);
	PARAM_POINTER (fields, TArray<DObject*>);

	auto it = self->cInfo->VMType->Symbols.GetIterator ();
	PSymbolTable::MapType::Pair *pair;
	while (it.NextPair (pair))
	{
		auto field = dyn_cast<PField>(pair->Value);
		if (field)
		{
			fields->Push (Create<DFieldInfo> (field));
		}
	}

	return 0;
}

DEFINE_ACTION_FUNCTION (DFieldInfo, GetFieldType)
{
	PARAM_SELF_PROLOGUE (DFieldInfo);

	if (self->fInfo->Type->isObjectPointer ()) {
		PClassPointer *classType = PType::toClassPointer (self->fInfo->Type);
		if (classType)
		{
			ACTION_RETURN_OBJECT (Create<DClassInfo> (classType->ClassRestriction));
		}
	}

	assert (false && "Unhandled type in GetFieldType");
	ACTION_RETURN_OBJECT (nullptr); // Only here to shut the compiler up
}
