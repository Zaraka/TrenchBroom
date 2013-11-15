/*
 Copyright (C) 2010-2013 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "GameImpl.h"

#include "Assets/Palette.h"
#include "IO/Bsp29Parser.h"
#include "IO/DefParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/FgdParser.h"
#include "IO/FileSystem.h"
#include "IO/Hexen2MapWriter.h"
#include "IO/MapParser.h"
#include "IO/MdlParser.h"
#include "IO/Md2Parser.h"
#include "IO/QuakeMapParser.h"
#include "IO/QuakeMapWriter.h"
#include "IO/Quake2MapWriter.h"
#include "IO/SystemPaths.h"
#include "IO/WadTextureLoader.h"
#include "IO/WalTextureLoader.h"
#include "Model/Map.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace Model {
        GameImpl::GameImpl(const GameConfig& config, const IO::Path& gamePath) :
        m_config(config),
        m_gamePath(gamePath),
        m_fs(m_config.fileSystemConfig().packageFormat,
             m_gamePath,
             m_config.fileSystemConfig().searchPath,
             m_additionalSearchPaths),
        m_palette(new Assets::Palette(config.findConfigFile(config.textureConfig().palette))) {}
        
        GameImpl::~GameImpl() {
            delete m_palette;
            m_palette = NULL;
        }

        const String& GameImpl::doGameName() const {
            return m_config.name();
        }

        void GameImpl::doSetGamePath(const IO::Path& gamePath) {
            m_gamePath = gamePath;
            m_fs = IO::GameFileSystem(m_config.fileSystemConfig().packageFormat,
                                      m_gamePath,
                                      m_config.fileSystemConfig().searchPath,
                                      m_additionalSearchPaths);
        }
        
        void GameImpl::doSetAdditionalSearchPaths(const IO::Path::List& searchPaths) {
            m_additionalSearchPaths = searchPaths;
            m_fs = IO::GameFileSystem(m_config.fileSystemConfig().packageFormat,
                                      m_gamePath,
                                      m_config.fileSystemConfig().searchPath,
                                      m_additionalSearchPaths);
        }

        Map* GameImpl::doNewMap(const MapFormat::Type format) const {
            return new Map(format);
        }
        
        Map* GameImpl::doLoadMap(const BBox3& worldBounds, const IO::Path& path) const {
            const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
            IO::QuakeMapParser parser(file->begin(), file->end());
            return parser.parseMap(worldBounds);
        }
        
        Model::EntityList GameImpl::doParseEntities(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseEntities(worldBounds);
        }
        
        Model::BrushList GameImpl::doParseBrushes(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseBrushes(worldBounds);
        }
        
        Model::BrushFaceList GameImpl::doParseFaces(const BBox3& worldBounds, const String& str) const {
            IO::QuakeMapParser parser(str);
            return parser.parseFaces(worldBounds);
        }
        
        void GameImpl::doWriteMap(Map& map, const IO::Path& path) const {
            mapWriter(map.format())->writeToFileAtPath(map, path, true);
        }
        
        void GameImpl::doWriteObjectsToStream(const MapFormat::Type format, const Model::ObjectList& objects, std::ostream& stream) const {
            mapWriter(format)->writeObjectsToStream(objects, stream);
        }
        
        void GameImpl::doWriteFacesToStream(const MapFormat::Type format, const Model::BrushFaceList& faces, std::ostream& stream) const {
            mapWriter(format)->writeFacesToStream(faces, stream);
        }
        
        IO::Path::List GameImpl::doFindBuiltinTextureCollections() const {
            const IO::Path& searchPath = m_config.textureConfig().builtinTexturesSearchPath;
            if (!searchPath.isEmpty() && m_fs.directoryExists(searchPath))
                return m_fs.findItems(searchPath, IO::FileSystem::TypeMatcher(false, true));
            return IO::Path::List();
        }
        
        IO::Path::List GameImpl::doExtractTexturePaths(const Map* map) const {
            IO::Path::List paths;
            
            const String& property = m_config.textureConfig().property;
            if (property.empty())
                return paths;
            
            Entity* worldspawn = map->worldspawn();
            if (worldspawn == NULL)
                return paths;
            
            const Model::PropertyValue& pathsValue = worldspawn->property(property);
            if (pathsValue.empty())
                return paths;
            
            const StringList pathStrs = StringUtils::split(pathsValue, ';');
            StringList::const_iterator it, end;
            for (it = pathStrs.begin(), end = pathStrs.end(); it != end; ++it) {
                const String pathStr = StringUtils::trim(*it);
                if (!pathStr.empty()) {
                    const IO::Path path(pathStr);
                    paths.push_back(path);
                }
            }
            
            return paths;
        }
        
        Assets::TextureCollection* GameImpl::doLoadTextureCollection(const IO::Path& path) const {
            const String& type = m_config.textureConfig().type;
            if (type == "wad")
                return loadWadTextureCollection(path);
            if (type == "wal")
                return loadWalTextureCollection(path);
            throw GameException("Unknown texture collection type '" + type + "'");
        }
        
        Assets::EntityDefinitionList GameImpl::doLoadEntityDefinitions(const IO::Path& path) const {
            const String extension = path.extension();
            const Color& defaultColor = m_config.entityConfig().defaultColor;
            
            if (StringUtils::caseInsensitiveEqual("fgd", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::FgdParser parser(file->begin(), file->end(), defaultColor);
                return parser.parseDefinitions();
            }
            if (StringUtils::caseInsensitiveEqual("def", extension)) {
                const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
                IO::DefParser parser(file->begin(), file->end(), defaultColor);
                return parser.parseDefinitions();
            }
            throw GameException("Unknown entity definition format: " + path.asString());
        }
        
        IO::Path GameImpl::doDefaultEntityDefinitionFile() const {
            return m_config.findConfigFile(m_config.entityConfig().defFilePath);
        }
        
        IO::Path GameImpl::doExtractEntityDefinitionFile(const Map* map) const {
            Entity* worldspawn = map->worldspawn();
            if (worldspawn == NULL)
                return defaultEntityDefinitionFile();
            
            const Model::PropertyValue& defValue = worldspawn->property(Model::PropertyKeys::EntityDefinitions);
            if (defValue.empty())
                return defaultEntityDefinitionFile();
            
            if (StringUtils::isPrefix(defValue, "external:"))
                return IO::Path(defValue.substr(9));
            if (StringUtils::isPrefix(defValue, "builtin:"))
                return IO::SystemPaths::resourceDirectory() + IO::Path(defValue.substr(8));
            
            const IO::Path defPath(defValue);
            if (defPath.isAbsolute())
                return defPath;
            return IO::SystemPaths::resourceDirectory() + defPath;
        }
        
        Assets::EntityModel* GameImpl::doLoadModel(const IO::Path& path) const {
            if (!m_fs.fileExists(path))
                return NULL;
            
            const IO::MappedFile::Ptr file = m_fs.openFile(path);
            assert(file != NULL);
            
            const String modelName = path.lastComponent().asString();
            const String extension = StringUtils::toLower(path.extension());
            const StringSet supported = m_config.entityConfig().modelFormats;
            
            if (extension == "mdl" && supported.count("mdl") > 0)
                return loadMdlModel(modelName, file);
            if (extension == "md2" && supported.count("md2") > 0)
                return loadMd2Model(modelName, file);
            if (extension == "bsp" && supported.count("bsp") > 0)
                return loadBspModel(modelName, file);
            throw GameException("Unsupported model format '" + path.asString() + "'");
        }
        
        GameImpl::MapWriterPtr GameImpl::mapWriter(const MapFormat::Type format) const {
            switch (format) {
                case MapFormat::Quake:
                    return MapWriterPtr(new IO::QuakeMapWriter());
                case MapFormat::Quake2:
                    return MapWriterPtr(new IO::Quake2MapWriter());
                case MapFormat::Hexen2:
                    return MapWriterPtr(new IO::Hexen2MapWriter());
                case MapFormat::Valve:
                    break;
            }
            throw GameException("Map format is not supported for writing");
        }
        
        Assets::TextureCollection* GameImpl::loadWadTextureCollection(const IO::Path& path) const {
            assert(m_palette != NULL);
            
            IO::WadTextureLoader loader(*m_palette);
            return loader.loadTextureCollection(path);
        }
        
        Assets::TextureCollection* GameImpl::loadWalTextureCollection(const IO::Path& path) const {
            assert(m_palette != NULL);
            
            if (path.isAbsolute()) {
                IO::DiskFileSystem diskFS(path.deleteLastComponent());
                IO::WalTextureLoader loader(diskFS, *m_palette);
                return loader.loadTextureCollection(path.lastComponent());
            } else {
                IO::WalTextureLoader loader(m_fs, *m_palette);
                return loader.loadTextureCollection(path);
            }
        }
        
        Assets::EntityModel* GameImpl::loadBspModel(const String& name, const IO::MappedFile::Ptr file) const {
            assert(m_palette != NULL);
            
            IO::Bsp29Parser parser(name, file->begin(), file->end(), *m_palette);
            return parser.parseModel();
        }
        
        Assets::EntityModel* GameImpl::loadMdlModel(const String& name, const IO::MappedFile::Ptr file) const {
            assert(m_palette != NULL);
            
            IO::MdlParser parser(name, file->begin(), file->end(), *m_palette);
            return parser.parseModel();
        }
        
        Assets::EntityModel* GameImpl::loadMd2Model(const String& name, const IO::MappedFile::Ptr file) const {
            assert(m_palette != NULL);
            
            IO::Md2Parser parser(name, file->begin(), file->end(), *m_palette, m_fs);
            return parser.parseModel();
        }
    }
}
