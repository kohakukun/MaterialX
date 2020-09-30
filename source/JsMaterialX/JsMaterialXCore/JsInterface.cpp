#include "../helpers.h"
#include <MaterialXCore/Interface.h>
#include <MaterialXCore/Node.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

#define BIND_INTERFACE_TYPE_INSTANCE(NAME, T)                                 \
.function("setParameterValue" #NAME, &mx::InterfaceElement::setParameterValue<T>) \
.function("setInputValue" #NAME, &mx::InterfaceElement::setInputValue<T>)

extern "C"
{
    EMSCRIPTEN_BINDINGS(interface)
    {

        ems::class_<mx::Parameter, ems::base<mx::ValueElement>>("Parameter")
            .smart_ptr_constructor("Parameter", &std::make_shared<mx::Parameter, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Parameter>>("Parameter")
            .class_property("CATEGORY", &mx::Parameter::CATEGORY);

        ems::class_<mx::PortElement, ems::base<mx::ValueElement>>("PortElement")
            .smart_ptr<std::shared_ptr<mx::PortElement>>("PortElement")
            .smart_ptr<std::shared_ptr<const mx::PortElement>>("PortElement")
            .function("setNodeName", &mx::PortElement::setNodeName)
            .function("getNodeName", &mx::PortElement::getNodeName)
            .function("setChannels", &mx::PortElement::setChannels)
            .function("getChannels", &mx::PortElement::getChannels)
            .function("setConnectedNode", &mx::PortElement::setConnectedNode)
            .function("getConnectedNode", &mx::PortElement::getConnectedNode);

        ems::class_<mx::Input, ems::base<mx::PortElement>>("Input")
            .smart_ptr_constructor("Input", &std::make_shared<mx::Input, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Input>>("Input")
            .function("setDefaultGeomPropString", &mx::Input::setDefaultGeomPropString)
            .function("hasDefaultGeomPropString", &mx::Input::hasDefaultGeomPropString)
            .function("getDefaultGeomPropString", &mx::Input::getDefaultGeomPropString)
            .function("getDefaultGeomProp", &mx::Input::getDefaultGeomProp)
            .class_property("CATEGORY", &mx::Input::CATEGORY);

        ems::class_<mx::Output, ems::base<mx::PortElement>>("Output")
            .smart_ptr_constructor("Output", &std::make_shared<mx::Output, mx::ElementPtr, const std::string &>)
            .smart_ptr<std::shared_ptr<const mx::Output>>("Output")
            .function("hasUpstreamCycle", &mx::Output::hasUpstreamCycle)
            .class_property("CATEGORY", &mx::Output::CATEGORY)
            .class_property("DEFAULT_INPUT_ATTRIBUTE", &mx::Output::DEFAULT_INPUT_ATTRIBUTE);

        ems::class_<mx::InterfaceElement, ems::base<mx::TypedElement>>("InterfaceElement")
            .smart_ptr<std::shared_ptr<mx::InterfaceElement>>("InterfaceElement")
            .smart_ptr<std::shared_ptr<const mx::InterfaceElement>>("InterfaceElement")
            .function("setNodeDefString", &mx::InterfaceElement::setNodeDefString)
            .function("hasNodeDefString", &mx::InterfaceElement::hasNodeDefString)
            .function("getNodeDefString", &mx::InterfaceElement::getNodeDefString)
            .function("addParameter", &mx::InterfaceElement::addParameter)

            .function("getParameter", &mx::InterfaceElement::getParameter)
            .function("getParameters", &mx::InterfaceElement::getParameters)
            .function("getParameterCount", &mx::InterfaceElement::getParameterCount)
            .function("removeParameter", &mx::InterfaceElement::removeParameter)
            .function("getActiveParameter", &mx::InterfaceElement::getActiveParameter)
            .function("getActiveParameters", &mx::InterfaceElement::getActiveParameters)
            .function("addInput", &mx::InterfaceElement::addInput)
            .function("getInput", &mx::InterfaceElement::getInput)
            .function("getInputs", &mx::InterfaceElement::getInputs)
            .function("getInputCount", &mx::InterfaceElement::getInputCount)
            .function("removeInput", &mx::InterfaceElement::removeInput)
            .function("getActiveInput", &mx::InterfaceElement::getActiveInput)
            .function("getActiveInputs", &mx::InterfaceElement::getActiveInputs)
            .function("addOutput", &mx::InterfaceElement::addOutput)
            .function("getOutput", &mx::InterfaceElement::getOutput)
            .function("getOutputs", &mx::InterfaceElement::getOutputs)
            .function("getOutputCount", &mx::InterfaceElement::getOutputCount)
            .function("removeOutput", &mx::InterfaceElement::removeOutput)
            .function("getActiveOutput", &mx::InterfaceElement::getActiveOutput)
            .function("getActiveOutputs", &mx::InterfaceElement::getActiveOutputs)

            .function("addToken", &mx::InterfaceElement::addToken)
            .function("getToken", &mx::InterfaceElement::getToken)
            .function("getTokens", &mx::InterfaceElement::getTokens)
            .function("removeToken", &mx::InterfaceElement::removeToken)
            .function("getActiveToken", &mx::InterfaceElement::getActiveToken)
            .function("getActiveTokens", &mx::InterfaceElement::getActiveTokens)

            .function("getActiveValueElement", &mx::InterfaceElement::getActiveValueElement)
            .function("getActiveValueElements", &mx::InterfaceElement::getActiveValueElements)
            .function("getParameterValue", &mx::InterfaceElement::getParameterValue)
            .function("getInputValue", &mx::InterfaceElement::getInputValue)

            .function("setTokenValue", &mx::InterfaceElement::setTokenValue)
            .function("getTokenValue", &mx::InterfaceElement::getTokenValue)
            .function("getDeclaration", &mx::InterfaceElement::getDeclaration)
            .function("isTypeCompatible", &mx::InterfaceElement::isTypeCompatible)
            BIND_INTERFACE_TYPE_INSTANCE(integer, int)
            BIND_INTERFACE_TYPE_INSTANCE(boolean, bool)
            BIND_INTERFACE_TYPE_INSTANCE(float, float)
            BIND_INTERFACE_TYPE_INSTANCE(color2, mx::Color2)
            BIND_INTERFACE_TYPE_INSTANCE(color3, mx::Color3)
            BIND_INTERFACE_TYPE_INSTANCE(color4, mx::Color4)
            BIND_INTERFACE_TYPE_INSTANCE(vector2, mx::Vector2)
            BIND_INTERFACE_TYPE_INSTANCE(vector3, mx::Vector3)
            BIND_INTERFACE_TYPE_INSTANCE(vector4, mx::Vector4)
            BIND_INTERFACE_TYPE_INSTANCE(matrix33, mx::Matrix33)
            BIND_INTERFACE_TYPE_INSTANCE(matrix44, mx::Matrix44)
            BIND_INTERFACE_TYPE_INSTANCE(string, std::string)
            BIND_INTERFACE_TYPE_INSTANCE(integerarray, mx::IntVec)
            BIND_INTERFACE_TYPE_INSTANCE(booleanarray, mx::BoolVec)
            BIND_INTERFACE_TYPE_INSTANCE(floatarray, mx::FloatVec)
            BIND_INTERFACE_TYPE_INSTANCE(stringarray, mx::StringVec)

            .class_property("NODE_DEF_ATTRIBUTE", &mx::InterfaceElement::NODE_DEF_ATTRIBUTE);
    }
}