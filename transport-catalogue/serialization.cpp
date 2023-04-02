#include "serialization.h"

using namespace std;

namespace transport_catalogue 
{
    TC_color PackColor(SVG_color svg_color) 
    {
        TC_color serialization_color;
        if (holds_alternative<string>(svg_color)) 
        {
            transport_catalogue_serialize::StringColor serialization_string_color;
            serialization_string_color.set_color(get<string>(svg_color));

            *serialization_color.mutable_string_color() = serialization_string_color;
            serialization_color.clear_rgb();
            serialization_color.clear_rgba();
        }
        else if (holds_alternative<svg::Rgb>(svg_color)) 
        {
            transport_catalogue_serialize::Rgb serialization_rgb_color;
            svg::Rgb rgb_color = get<svg::Rgb>(svg_color);
            serialization_rgb_color.set_r(rgb_color.red);
            serialization_rgb_color.set_g(rgb_color.green);
            serialization_rgb_color.set_b(rgb_color.blue);

            *serialization_color.mutable_rgb() = serialization_rgb_color;
            serialization_color.clear_string_color();
            serialization_color.clear_rgba();
        }
        else 
        {
            transport_catalogue_serialize::Rgba serialization_rgba_color;
            svg::Rgba rgba_color = get<svg::Rgba>(svg_color);
            transport_catalogue_serialize::Rgb serialization_rgb_color;
            serialization_rgb_color.set_r(rgba_color.red);
            serialization_rgb_color.set_g(rgba_color.green);
            serialization_rgb_color.set_b(rgba_color.blue);
            *serialization_rgba_color.mutable_rgb() = serialization_rgb_color;
            serialization_rgba_color.set_opacity(rgba_color.opacity);

            *serialization_color.mutable_rgba() = serialization_rgba_color;
            serialization_color.clear_string_color();
            serialization_color.clear_rgb();
        }
        return serialization_color;
    }

    SVG_color UnpackColor(TC_color serialization_color) 
    {
        SVG_color result;
        if (serialization_color.has_string_color()) 
        {
            return {serialization_color.string_color().color()};
        }
        else if (serialization_color.has_rgb()) 
        {
            return {svg::Rgb(
                            serialization_color.rgb().r(),
                            serialization_color.rgb().g(),
                            serialization_color.rgb().b()
                            )};
        }
        else 
        {
            return {svg::Rgba(
                            serialization_color.rgba().rgb().r(),
                            serialization_color.rgba().rgb().g(),
                            serialization_color.rgba().rgb().b(),
                            serialization_color.rgba().opacity()
                            )};
        }
    }

    TC_render_settings PackRenderSettings(const MP_render_settings& render_settings)
    {
        TC_render_settings serialization_render_settings;
        serialization_render_settings.set_width(render_settings.width);
        serialization_render_settings.set_height(render_settings.height);
        serialization_render_settings.set_padding(render_settings.padding);
        serialization_render_settings.set_line_width(render_settings.line_width);
        serialization_render_settings.set_stop_radius(render_settings.stop_radius);
        serialization_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
        serialization_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);

        transport_catalogue_serialize::Point serialization_point;
        serialization_point.set_x(render_settings.stop_label_offset.x);
        serialization_point.set_y(render_settings.stop_label_offset.y);
        *serialization_render_settings.mutable_stop_label_offset() = serialization_point;

        serialization_point.set_x(render_settings.bus_label_offset.x);
        serialization_point.set_y(render_settings.bus_label_offset.y);
        *serialization_render_settings.mutable_bus_label_offset() = serialization_point;

        *serialization_render_settings.mutable_underlayer_color() = PackColor(render_settings.underlayer_color);

        serialization_render_settings.set_underlayer_width(render_settings.underlayer_width);

        for (const SVG_color& svg_color : render_settings.color_palette) 
        {
            *serialization_render_settings.mutable_color_palette()->Add() = PackColor(svg_color);
        }

        return serialization_render_settings;
    }

    MP_render_settings UnpackRenderSettings(const TC_render_settings& serialization_render_settings) 
    {
        MP_render_settings render_settings;
        
        render_settings.width = serialization_render_settings.width();
        render_settings.height = serialization_render_settings.height();
        render_settings.padding = serialization_render_settings.padding();
        render_settings.line_width = serialization_render_settings.line_width();
        render_settings.stop_radius = serialization_render_settings.stop_radius();
        render_settings.bus_label_font_size = serialization_render_settings.bus_label_font_size();
        render_settings.stop_label_font_size = serialization_render_settings.stop_label_font_size();
        render_settings.stop_label_offset = 
        {
            serialization_render_settings.stop_label_offset().x(),
            serialization_render_settings.stop_label_offset().y()
        };
        render_settings.bus_label_offset = 
        {
            serialization_render_settings.bus_label_offset().x(),
            serialization_render_settings.bus_label_offset().y()
        };

        TC_color serialization_color = serialization_render_settings.underlayer_color();
        render_settings.underlayer_color = UnpackColor(serialization_color);

        render_settings.underlayer_width = serialization_render_settings.underlayer_width();

        size_t color_palette_size = serialization_render_settings.color_palette_size();
        for (size_t i = 0; i != color_palette_size; ++i) 
        {
            render_settings.color_palette.push_back(UnpackColor(serialization_render_settings.color_palette(i)));
        }

        return render_settings;
    }

    TC_route_settings PackRoutingSettings(const RouteSettings& routing_settings) 
    {
        TC_route_settings serialization_routing_settings;
        serialization_routing_settings.set_bus_wait_time(routing_settings.bus_wait_time);
        serialization_routing_settings.set_bus_velocity(routing_settings.bus_velocity);
        
        return serialization_routing_settings;
    }

    RouteSettings UnpackRoutingSettings(const TC_route_settings& serialization_routing_settings) 
    {
        RouteSettings routing_settings;
        routing_settings.bus_wait_time = serialization_routing_settings.bus_wait_time();
        routing_settings.bus_velocity = serialization_routing_settings.bus_velocity();
        
        return routing_settings;
    }

    TC_graph PackGraph(const Graph& graph) 
    {
        TC_graph serialization_graph;
        transport_catalogue_serialize::Edge serialization_edge;
        for (const graph::Edge<double>& edge : graph) 
        {
            serialization_edge.set_from_id(edge.from);
            serialization_edge.set_to_id(edge.to);
            serialization_edge.set_span_count(edge.span_count);
            serialization_edge.set_bus_name(string(edge.bus_name));
            serialization_edge.set_weight(edge.weight);

            *serialization_graph.mutable_edges()->Add() = serialization_edge;
        }

        return serialization_graph;
    }

    Graph UnpackGraph(const TC_graph& serialization_graph, size_t vertex_count) 
    {
        size_t edges_count = serialization_graph.edges_size();
        Graph graph(vertex_count);  
        for (size_t i = 0; i != edges_count; ++i) 
        {
            const transport_catalogue_serialize::Edge& serialization_edge = serialization_graph.edges(i);
            graph.AddEdge
            ({
                serialization_edge.from_id(),
                serialization_edge.to_id(),
                serialization_edge.span_count(),
                serialization_edge.bus_name(),
                serialization_edge.weight()
            });
        }
        
        return graph;
    }

    transport_catalogue_serialize::Stop PackStop(const Stop& stop) 
    {
        transport_catalogue_serialize::Coordinates serialization_coords;
        transport_catalogue_serialize::Stop serialization_stop;

        serialization_coords.set_lat(stop.coordinates.lat);
        serialization_coords.set_lng(stop.coordinates.lng);

        serialization_stop.set_name(stop.name);
        *serialization_stop.mutable_coordinates() = serialization_coords;

        return serialization_stop;
    }

    transport_catalogue_serialize::Bus PackBus(const Bus& bus, const TransportCatalogue& catalogue) 
    {
        transport_catalogue_serialize::Bus serialization_bus;
        serialization_bus.set_name(bus.name);
        serialization_bus.set_is_roundtrip(bus.is_looped);

        for (const Stop* stop_ptr : bus.stops) 
        {
            *serialization_bus.mutable_stop_index()->Add() = catalogue.GetStopIndex(stop_ptr);
        }

        return serialization_bus;
    }

    transport_catalogue_serialize::StopPairPlusDistance PackDistance(const std::pair<const Stop*, const Stop*>& stop_ptr_pair, double distance, const TransportCatalogue& catalogue)
    {
        size_t stop1_idx = catalogue.GetStopIndex(stop_ptr_pair.first);
        size_t stop2_idx = catalogue.GetStopIndex(stop_ptr_pair.second);

        transport_catalogue_serialize::StopPairPlusDistance pair_dist;
        pair_dist.set_stop1_index(stop1_idx);
        pair_dist.set_stop2_index(stop2_idx);
        pair_dist.set_distance(distance);

        return pair_dist;
    }

    void Serialize(const TransportCatalogue& catalogue, const string& filename) 
    {
        transport_catalogue_serialize::TransportCatalogue transport_catalogue_to_serialize;
        const std::deque<Stop>& stops = catalogue.GetStops();
        const std::deque<Bus>& buses = catalogue.GetBuses();
        
        for (const Stop& stop : stops) 
        {
            *transport_catalogue_to_serialize.mutable_stops()->Add() = PackStop(stop);
        }

        for (const Bus& bus : buses) 
        {
            *transport_catalogue_to_serialize.mutable_buses()->Add() = PackBus(bus, catalogue);
        }

        for (const auto& [stop_ptr_pair, distance] : catalogue.GetDistances()) 
        {
            *transport_catalogue_to_serialize.mutable_distances()->Add() = PackDistance(stop_ptr_pair, distance, catalogue);
        }

        const MP_render_settings& render_settings = catalogue.GetRenderSettings();
        *transport_catalogue_to_serialize.mutable_render_settings() = PackRenderSettings(render_settings);


        const transport_catalogue::RouteSettings& routing_settings = catalogue.GetRouteSettings();
        *transport_catalogue_to_serialize.mutable_route_settings() = PackRoutingSettings(routing_settings);


        const Graph& gr = catalogue.GetGraph();
        *transport_catalogue_to_serialize.mutable_graph() = PackGraph(gr);

        std::ofstream ofs(filename, ios::binary);
        transport_catalogue_to_serialize.SerializeToOstream(&ofs);
        ofs.close();
    }

    void Deserialize(const string& filename, TransportCatalogue& catalogue) 
    {
        transport_catalogue_serialize::TransportCatalogue transport_catalogue_serialized;
        ifstream ifs(filename, ios::binary);
        transport_catalogue_serialized.ParseFromIstream(&ifs);

        for (size_t i = 0; i != transport_catalogue_serialized.stops_size(); ++i) 
        {
            transport_catalogue_serialize::Stop serialization_stop = transport_catalogue_serialized.stops(i);
            catalogue.AddStop( serialization_stop.name(), {serialization_stop.coordinates().lat(), serialization_stop.coordinates().lng()} );
        }

        for (size_t i = 0; i != transport_catalogue_serialized.buses_size(); ++i) 
        {
            transport_catalogue_serialize::Bus serialization_bus = transport_catalogue_serialized.buses(i);
            std::vector<std::string> stops;
            size_t stops_count = serialization_bus.stop_index_size();
            for (size_t j = 0; j != stops_count; ++j) 
            {
                stops.push_back(catalogue.GetStopnameByIndex(serialization_bus.stop_index(j)));
            }
            catalogue.AddBus(serialization_bus.name(), stops, serialization_bus.is_roundtrip());
        }

        for (size_t i = 0; i != transport_catalogue_serialized.distances_size(); ++i) 
        {
            transport_catalogue_serialize::StopPairPlusDistance stop_pair_distance = transport_catalogue_serialized.distances(i);
            std::string stop1_name = catalogue.GetStopnameByIndex(stop_pair_distance.stop1_index());
            std::string stop2_name = catalogue.GetStopnameByIndex(stop_pair_distance.stop2_index());

            catalogue.AddDistance(stop1_name, stop2_name, stop_pair_distance.distance()); 
        }

        TC_render_settings serialization_render_settings = transport_catalogue_serialized.render_settings();
        catalogue.SetRenderSettings(UnpackRenderSettings(serialization_render_settings));
        TC_route_settings serialization_routing_settings = transport_catalogue_serialized.route_settings();
        catalogue.AddRouteSettings(UnpackRoutingSettings(serialization_routing_settings));
        TC_graph serialization_graphaph = transport_catalogue_serialized.graph();
        catalogue.SetGraph(UnpackGraph(serialization_graphaph, catalogue.GetStops().size()));

        ifs.close();
    }

}