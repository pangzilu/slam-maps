//
// Copyright (c) 2015-2017, Deutsches Forschungszentrum für Künstliche Intelligenz GmbH.
// Copyright (c) 2015-2017, University of Bremen
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include <vizkit3d/Vizkit3DPlugin.hpp>
#include "ContourMapVisualization.hpp"
#include "ElevationMapVisualization.hpp"
#include "GridMapVisualization.hpp"
#include "MLSMapVisualization.hpp"
#include "TraversabilityMap3dVisualization.hpp"
#include "OccupancyGridMapVisualization.hpp"

namespace vizkit3d {
    class QtPluginVizkitMaps : public vizkit3d::VizkitPluginFactory
    {
    public:

        QtPluginVizkitMaps() {}

        /**
        * Returns a list of all available visualization plugins.
        * @return list of plugin names
        */
        virtual QStringList* getAvailablePlugins() const
        {
            QStringList *pluginNames = new QStringList();
            pluginNames->push_back("ContourMapVisualization");
            pluginNames->push_back("ElevationMapVisualization");
            pluginNames->push_back("GridMapVisualization");
            pluginNames->push_back("MLSMapVisualization");
            pluginNames->push_back("TraversabilityMap3dVisualization");
            pluginNames->push_back("OccupancyGridMapVisualization");
            return pluginNames;
        }

        virtual QObject* createPlugin(QString const& pluginName)
        {
            vizkit3d::VizPluginBase* plugin = 0;
            if (pluginName == "ContourMapVisualization")
            {
                plugin = new vizkit3d::ContourMapVisualization();
            }
            else if (pluginName == "ElevationMapVisualization")
            {
                plugin = new vizkit3d::ElevationMapVisualization();
            }
            else if (pluginName == "GridMapVisualization")
            {
                plugin = new vizkit3d::GridMapVisualization();
            }
            else if (pluginName == "MLSMapVisualization")
            {
                plugin = new vizkit3d::MLSMapVisualization();
            }
            else if (pluginName == "TraversabilityMap3dVisualization")
            {
                plugin = new vizkit3d::TraversabilityMap3dVisualization();
            }
            else if (pluginName == "OccupancyGridMapVisualization")
            {
                plugin = new OccupancyGridMapVisualization();
            }

            if (plugin)
            {
                    return plugin;
            }
            return NULL;
        };
    };
    Q_EXPORT_PLUGIN2(QtPluginVizkitMaps, QtPluginVizkitMaps)
}
