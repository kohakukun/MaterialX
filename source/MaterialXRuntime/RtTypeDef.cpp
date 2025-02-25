//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTypeDef.h>

#include <MaterialXRuntime/Private/PvtTypeDef.h>

namespace MaterialX
{

const RtIdentifier RtTypeDef::BASETYPE_NONE("none");
const RtIdentifier RtTypeDef::BASETYPE_BOOLEAN("boolean");
const RtIdentifier RtTypeDef::BASETYPE_FLOAT("float");
const RtIdentifier RtTypeDef::BASETYPE_INTEGER("integer");
const RtIdentifier RtTypeDef::BASETYPE_STRING("string");
const RtIdentifier RtTypeDef::BASETYPE_STRUCT("struct");

const RtIdentifier RtTypeDef::SEMANTIC_NONE("none");
const RtIdentifier RtTypeDef::SEMANTIC_COLOR("color");
const RtIdentifier RtTypeDef::SEMANTIC_VECTOR("vector");
const RtIdentifier RtTypeDef::SEMANTIC_MATRIX("matrix");
const RtIdentifier RtTypeDef::SEMANTIC_FILENAME("filename");
const RtIdentifier RtTypeDef::SEMANTIC_CLOSURE("closure");
const RtIdentifier RtTypeDef::SEMANTIC_SHADER("shader");
const RtIdentifier RtTypeDef::SEMANTIC_MATERIAL("material");

RtTypeDef::RtTypeDef(const RtIdentifier& name, const RtIdentifier& basetype, const RtValueFuncs& funcs, const RtIdentifier& semantic, size_t size) :
    _ptr(new PvtTypeDef(name, basetype, funcs, semantic, size))
{
}

RtTypeDef::~RtTypeDef()
{
    delete static_cast<PvtTypeDef*>(_ptr);
}

RtValue RtTypeDef::createValue(RtPrim& owner) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().create(owner);
}

void RtTypeDef::copyValue(const RtValue& src, RtValue& dest) const
{
    static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().copy(src, dest);
}

bool RtTypeDef::compareValue(const RtValue& a, const RtValue& b) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().compare(a, b);
}

void RtTypeDef::toStringValue(const RtValue& src, string& dest) const
{
    static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().toString(src, dest);
}

void RtTypeDef::fromStringValue(const string& src, RtValue& dest) const
{
    static_cast<PvtTypeDef*>(_ptr)->getValueFuncs().fromString(src, dest);
}

const RtIdentifier& RtTypeDef::getName() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getName();
}

const RtIdentifier& RtTypeDef::getBaseType() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getBaseType();
}

const RtIdentifier& RtTypeDef::getSemantic() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getSemantic();
}

size_t RtTypeDef::getSize() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getSize();
}

void RtTypeDef::setComponent(size_t index, const RtIdentifier& name, const RtIdentifier& basetype)
{
    static_cast<PvtTypeDef*>(_ptr)->setComponent(index, name, basetype);
}

size_t RtTypeDef::getComponentIndex(const RtIdentifier& name) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentIndex(name);
}

const RtIdentifier& RtTypeDef::getComponentName(size_t index) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentName(index);
}

const RtIdentifier& RtTypeDef::getComponentBaseType(size_t index) const
{
    return static_cast<PvtTypeDef*>(_ptr)->getComponentBaseType(index);
}

const RtIdentifierSet& RtTypeDef::getValidConnectionTypes() const
{
    return static_cast<PvtTypeDef*>(_ptr)->getValidConnectionTypes();
}

RtTypeDef* RtTypeDef::registerType(const RtIdentifier& name, const RtIdentifier& basetype, const RtValueFuncs& funcs,
                                   const RtIdentifier& semantic, size_t size)
{
    if (PvtTypeDefRegistry::get().findType(name))
    {
        throw ExceptionRuntimeError("A type named '" + name.str() + "' is already registered");
    }
    return PvtTypeDefRegistry::get().newType(name, basetype, funcs, semantic, size);
}

size_t RtTypeDef::numTypes()
{
    return PvtTypeDefRegistry::get().numTypes();
}

const RtTypeDef* RtTypeDef::getType(size_t index)
{
    return PvtTypeDefRegistry::get().getType(index);
}

const RtTypeDef* RtTypeDef::findType(const RtIdentifier& name)
{
    return PvtTypeDefRegistry::get().findType(name);
}

}
