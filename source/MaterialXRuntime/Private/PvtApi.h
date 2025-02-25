//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTAPI_H
#define MATERIALX_PVTAPI_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtTargetDef.h>

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtCommand.h>
#include <MaterialXRuntime/Private/PvtMessage.h>

namespace MaterialX
{

class PvtApi
{
public:
    PvtApi()
    {
        reset();
    }

    static PvtApi* cast(RtApi& api)
    {
        return reinterpret_cast<PvtApi*>(api._ptr);
    }

    void reset();

    PvtCommandEngine& getCommandEngine()
    {
        return _commandEngine;
    }

    PvtMessageHandler& getMessageHandler()
    {
        return _messageHandler;
    }

    void registerLogger(RtLoggerPtr logger)
    {
        _loggers.push_back(logger);
    }

    void unregisterLogger(RtLoggerPtr logger)
    {
        static_cast<void>(std::remove(_loggers.begin(), _loggers.end(), logger));
    }

    void log(RtLogger::MessageType type, const string& msg)
    {
        for (RtLoggerPtr logger : _loggers)
        {
            if (logger)
            {
                logger->log(type, msg);
            }
        }
    }

    void registerCreateFunction(const RtIdentifier& typeName, RtPrimCreateFunc creator)
    {
        if (getCreateFunction(typeName))
        {
            throw ExceptionRuntimeError("A create function for type name '" + typeName.str() + "' is already registered");
        }
        _createFunctions[typeName] = creator;
    }

    void unregisterCreateFunction(const RtIdentifier& typeName)
    {
        _createFunctions.erase(typeName);
    }

    bool hasCreateFunction(const RtIdentifier& typeName)
    {
        return _createFunctions.count(typeName) > 0;
    }

    RtPrimCreateFunc getCreateFunction(const RtIdentifier& typeName)
    {
        auto it = _createFunctions.find(typeName);
        return it != _createFunctions.end() ? it->second : nullptr;
    }

    void registerNodeDef(const RtPrim& prim)
    {
        if (!prim.hasApi<RtNodeDef>())
        {
            throw ExceptionRuntimeError("Given prim '" + prim.getName().str() + "' is not a valid nodedef");
        }
        if (_nodedefs.count(prim.getName()))
        {
            _nodedefs.remove(prim.getName());
        }
        _nodedefs.add(PvtObject::cast(prim));
    }

    void unregisterNodeDef(const RtIdentifier& name)
    {
        _nodedefs.remove(name);
    }

    bool hasNodeDef(const RtIdentifier& name)
    {
        return _nodedefs.count(name);
    }

    size_t numNodeDefs() const
    {
        return _nodedefs.size();
    }

    PvtObject* getNodeDef(const RtIdentifier& name)
    {
        return _nodedefs.find(name);
    }

    PvtObject* getNodeDef(size_t index)
    {
        return _nodedefs[index];
    }

    void registerNodeGraph(const RtPrim& prim)
    {
        if (!prim.hasApi<RtNodeGraph>())
        {
            throw ExceptionRuntimeError("Given prim '" + prim.getName().str() + "' is not a valid nodegraph");
        }
        if (_nodegraphs.count(prim.getName()))
        {
            _nodegraphs.remove(prim.getName());
        }
        _nodegraphs.add(PvtObject::cast(prim));
    }

    void unregisterNodeGraph(const RtIdentifier& name)
    {
        _nodegraphs.remove(name);
    }

    bool hasNodeGraph(const RtIdentifier& name)
    {
        return _nodegraphs.count(name);
    }

    size_t numNodeGraphs() const
    {
        return _nodegraphs.size();
    }

    PvtObject* getNodeGraph(const RtIdentifier& name)
    {
        return _nodegraphs.find(name);
    }

    PvtObject* getNodeGraph(size_t index)
    {
        return _nodegraphs[index];
    }

    void registerNodeImpl(const RtPrim& prim)
    {
        if (!prim.hasApi<RtNodeImpl>())
        {
            throw ExceptionRuntimeError("Given prim '" + prim.getName().str() + "' is not a valid nodeimpl");
        }
        if (_nodeimpls.count(prim.getName()))
        {
            _nodeimpls.remove(prim.getName());
        }
        _nodeimpls.add(PvtObject::cast(prim));
    }

    void unregisterNodeImpl(const RtIdentifier& name)
    {
        _nodeimpls.remove(name);
    }

    bool hasNodeImpl(const RtIdentifier& name)
    {
        return _nodeimpls.count(name);
    }

    size_t numNodeImpls() const
    {
        return _nodeimpls.size();
    }

    PvtObject* getNodeImpl(const RtIdentifier& name)
    {
        return _nodeimpls.find(name);
    }

    PvtObject* getNodeImpl(size_t index)
    {
        return _nodeimpls[index];
    }

    void registerTargetDef(const RtPrim& prim)
    {
        if (!prim.hasApi<RtTargetDef>())
        {
            throw ExceptionRuntimeError("Given prim '" + prim.getName().str() + "' is not a valid targetdef");
        }
        if (_targetdefs.count(prim.getName()))
        {
            _targetdefs.remove(prim.getName());
        }
        _targetdefs.add(PvtObject::cast(prim));
    }

    void unregisterTargetDef(const RtIdentifier& name)
    {
        _targetdefs.remove(name);
    }

    bool hasTargetDef(const RtIdentifier& name)
    {
        return _targetdefs.count(name);
    }

    size_t numTargetDefs() const
    {
        return _targetdefs.size();
    }

    PvtObject* getTargetDef(const RtIdentifier& name)
    {
        return _targetdefs.find(name);
    }

    PvtObject* getTargetDef(size_t index)
    {
        return _targetdefs[index];
    }

    void clearSearchPath()
    {
        _searchPaths.clear();
    }

    void clearTextureSearchPath()
    {
        _textureSearchPaths.clear();
    }

    void clearImplementationSearchPath()
    {
        _implementationSearchPaths.clear();
    }

    void setSearchPath(const FileSearchPath& searchPath)
    {
        _searchPaths.append(searchPath);
    }

    void setTextureSearchPath(const FileSearchPath& searchPath)
    {
        _textureSearchPaths.append(searchPath);
    }

    void setImplementationSearchPath(const FileSearchPath& searchPath)
    {
        _implementationSearchPaths.append(searchPath);
    }

    const FileSearchPath& getSearchPath() const
    {
        return _searchPaths;
    }

    const FileSearchPath& getTextureSearchPath() const
    {
        return _textureSearchPaths;
    }

    const FileSearchPath& getImplementationSearchPath() const
    {
        return _implementationSearchPaths;
    }

    RtStagePtr loadLibrary(const RtIdentifier& name, const FilePath& path, const RtReadOptions* options = nullptr, bool forceReload = false);
    void unloadLibrary(const RtIdentifier& name);
    void unloadLibraries();

    size_t numLibraries() const
    {
        return _librariesOrder.size();
    }

    RtStagePtr getLibrary(const RtIdentifier& name) const
    {
        auto it = _libraries.find(name);
        return it != _libraries.end() ? it->second : nullptr;
    }

    RtStagePtr getLibrary(size_t index) const
    {
        return _librariesOrder[index];
    }

    void setupNodeImplRelationships();

    RtIdentifier makeUniqueStageName(const RtIdentifier& name) const;

    RtStagePtr createStage(const RtIdentifier& name)
    {
        const RtIdentifier newName = makeUniqueStageName(name);
        RtStagePtr stage = PvtStage::createNew(newName);
        _stages[newName] = stage;
        _stagesOrder.push_back(stage);
        return stage;
    }

    void deleteStage(const RtIdentifier& name)
    {
        auto it = _stages.find(name);
        if (it != _stages.end())
        {
            RtStagePtr stage = it->second;
            unregisterPrims(stage);

            _stages.erase(it);
            _stagesOrder.erase(std::find(_stagesOrder.begin(), _stagesOrder.end(), stage));
        }
    }

    void deleteStages()
    {
        for (auto stage : _stagesOrder)
        {
            unregisterPrims(stage);
        }
        _stages.clear();
        _stagesOrder.clear();
    }

    void registerPrims(RtStagePtr stage);
    void unregisterPrims(RtStagePtr stage);

    size_t numStages() const
    {
        return _stagesOrder.size();
    }

    RtStagePtr getStage(const RtIdentifier& name) const
    {
        auto it = _stages.find(name);
        return it != _stages.end() ? it->second : RtStagePtr();
    }

    RtStagePtr getStage(size_t index) const
    {
        return _stagesOrder[index];
    }

    RtIdentifier renameStage(const RtIdentifier& name, const RtIdentifier& newName)
    {
        RtStagePtr stage = getStage(name);
        if (!stage)
        {
            throw ExceptionRuntimeError("Can't find a stage named '" + name.str() + "' to rename");
        }
        const RtIdentifier uniqueName = makeUniqueStageName(newName);
        stage->setName(uniqueName);
        _stages[uniqueName] = stage;
        _stages.erase(name);
        return uniqueName;
    }

    UnitConverterRegistryPtr& getUnitDefinitions()
    {
        return _unitDefinitions;
    }

    vector<RtLoggerPtr> _loggers;

    PvtCommandEngine _commandEngine;
    PvtMessageHandler _messageHandler;

    FileSearchPath _searchPaths;
    FileSearchPath _implementationSearchPaths;
    FileSearchPath _textureSearchPaths;
    FilePath _userDefinitionPath;

    UnitConverterRegistryPtr _unitDefinitions;

    RtIdentifierMap<RtPrimCreateFunc> _createFunctions;

    RtIdentifierMap<RtStagePtr> _libraries;
    vector<RtStagePtr> _librariesOrder;
    RtIdentifierMap<RtStagePtr> _stages;
    vector<RtStagePtr> _stagesOrder;
    PvtObjectList _nodedefs;
    PvtObjectList _nodegraphs;
    PvtObjectList _nodeimpls;
    PvtObjectList _targetdefs;
};

}

#endif
