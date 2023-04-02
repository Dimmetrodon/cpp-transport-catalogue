#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "graph.h"

#include <transport_catalogue.pb.h>

#include <string>
#include <fstream>

namespace transport_catalogue 
{
    using TC_color = transport_catalogue_serialize::Color;
    using SVG_color = svg::Color;

    using TC_render_settings = transport_catalogue_serialize::RenderSettings;
    using MP_render_settings = map_renderer::RenderSettings;

    using TC_route_settings = transport_catalogue_serialize::RouteSettings;

    using TC_graph = transport_catalogue_serialize::DirectedWeightedGraph;
    using Graph = graph::DirectedWeightedGraph<double>;

    void                Serialize(const TransportCatalogue& transport_catalogue, const std::string& filename);
    void                Deserialize(const std::string& filename,  TransportCatalogue& transport_catalogue);

    TC_color            PackColor(SVG_color svg_color);
    SVG_color           UnpackColor(TC_color ser_color);

    TC_render_settings  PackRenderSettings(const MP_render_settings& render_settings);
    MP_render_settings  UnpackRenderSettings(const TC_render_settings& ser_render_settings);

    TC_route_settings   PackRouteSettings(const RouteSettings& routing_settings);
    RouteSettings       UnpackRouteSettings(const TC_route_settings& ser_routing_settings);

    TC_graph            PackGraph(const Graph& gr);
    Graph               UnpackGraph(const TC_graph& ser_gr, size_t vertex_count);

    transport_catalogue_serialize::Stop                     PackStop(const Stop& stop);
    transport_catalogue_serialize::Bus                      PackBus(const Bus& bus, const TransportCatalogue& catalogue);
    transport_catalogue_serialize::StopPairPlusDistance         PackDistance(const std::pair<const Stop*, const Stop*>& stop_ptr_pair,double distance, const TransportCatalogue& catalogue);
}