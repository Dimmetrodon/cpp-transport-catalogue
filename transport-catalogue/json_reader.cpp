#include "json_reader.h"

using namespace std;

namespace transport_catalogue
{
	namespace json_reader
	{
        JsonReader::JsonReader(TransportCatalogue& transport_catalogue, istream& input, ostream& output)
            : transport_catalogue_(transport_catalogue)
            , router_(transport_catalogue_.GetGraph())
            , input_(input)
            , output_(output)
        {
        }

		void JsonReader::LoadJSON()
		{
			json_document_ = json::Load(input_);
		}

        ParsedStop JsonReader::ParseStop(const json::Node& stop_node)
        {
            return
            {
                stop_node.AsMap().at("name"s).AsString(),
                stop_node.AsMap().at("latitude"s).AsDouble(),
                stop_node.AsMap().at("longitude"s).AsDouble()
            };
        }

        ParsedDistance JsonReader::ParseDistance(const json::Node& stop_node)
        {
            return
            {
                stop_node.AsMap().at("name"s).AsString(),
                stop_node.AsMap().at("road_distances"s).AsMap()
            };
        }

        ParsedBus JsonReader::ParseBus(const json::Node& bus_node)
        {
            vector<string> stop_names;
            for (const json::Node& stop_node : bus_node.AsMap().at("stops"s).AsArray())
            {
                stop_names.push_back(stop_node.AsString());
            }
            return
            {
                bus_node.AsMap().at("name"s).AsString(),
                stop_names,
                bus_node.AsMap().at("is_roundtrip"s).AsBool()
            };
        }

		void JsonReader::ProcessBaseRequests()
		{
            json::Array requests_array = json_document_.GetRoot().AsMap().at("base_requests"s).AsArray();

            //Sort requests: buses or stops
            json::Array bus_requests, stop_requests;
            for (const json::Node& request : requests_array)
            {
                string request_type = request.AsMap().at("type"s).AsString();
                if (request_type == "Stop"s)
                {
                    stop_requests.push_back(request);
                }
                else
                {
                    bus_requests.push_back(request);
                }
            }

            //Process stops
            for (const json::Node& request : stop_requests)
            {
                ParsedStop parsed_stop = ParseStop(request);
                transport_catalogue_.AddStop(parsed_stop.stop_name, {parsed_stop.latitude, parsed_stop.longitude});
            }

            //Process distances
            for (const json::Node& request : stop_requests)
            {
                ParsedDistance parsed_distance = ParseDistance(request);
                for (const auto& [stop_name, distance] : parsed_distance.stop_names_and_distances)
                {
                    transport_catalogue_.AddDistance(parsed_distance.first_stop_name, stop_name, distance.AsInt());
                }
            }

            //Process buses
            for (const json::Node& request : bus_requests)
            {
                ParsedBus parsed_bus = ParseBus(request);
                transport_catalogue_.AddBus(parsed_bus.bus_name, parsed_bus.stop_names, parsed_bus.is_looped);
            }
            transport_catalogue_.BuildGraph();
            router_.RouterAfterInitialization();
		}

        void JsonReader::ProscessRoutingSettings()
        {
            json::Dict routing_settings_as_map = json_document_.GetRoot().AsMap().at("routing_settings"s).AsMap();
            int bus_wait_time = routing_settings_as_map.at("bus_wait_time"s).AsInt();
            double bus_velocity = routing_settings_as_map.at("bus_velocity"s).AsDouble(); 
            transport_catalogue_.AddRouteSettings({bus_wait_time, bus_velocity});
        }

        json::Dict JsonReader::ParseStopRequest(const json::Node& stop_node)
        {
            optional<Stop> stop_test = transport_catalogue_.FindStop(stop_node.AsMap().at("name"s).AsString());
            if (stop_test.value().name == ""s)
            {
                return { {"request_id"s, json::Node(stop_node.AsMap().at("id"s).AsInt())}, {"error_message"s, json::Node("not found"s)} };
            }
            set<string> stop_names_set = transport_catalogue_.GetBusesByStop(transport_catalogue_.FindStop(stop_node.AsMap().at("name"s).AsString()));
            json::Array stop_names_array;
            for (string stop_name : stop_names_set)
            {
                stop_names_array.push_back(stop_name);
            }
            return
            {
                {"buses"s,              json::Node(stop_names_array)},
                {"request_id"s,         json::Node(stop_node.AsMap().at("id"s).AsInt())}
            };
        }

        json::Dict JsonReader::ParseBusRequest(const json::Node& bus_node)
        {
            optional<Bus> bus_test = transport_catalogue_.FindBus(bus_node.AsMap().at("name"s).AsString());
            if (bus_test.value().name == ""s)
            {
                return { {"request_id"s, json::Node(bus_node.AsMap().at("id"s).AsInt())}, {"error_message"s, json::Node("not found"s)} };
            }

            string bus_name = bus_node.AsMap().at("name"s).AsString();
            int real_distance = transport_catalogue_.GetRealRouteDistance(transport_catalogue_.FindBus(bus_name));
            double curvature = transport_catalogue_.GetBusCurvature(bus_name);
            return
            {
                {"request_id"s,         json::Node(bus_node.AsMap().at("id"s).AsInt())},
                {"stop_count"s,         json::Node(transport_catalogue_.GetBusStopCount(bus_name))},
                {"unique_stop_count"s,  json::Node(transport_catalogue_.GetBusUniqueStopsCount(bus_name))},
                {"route_length"s,       json::Node(real_distance)},
                {"curvature"s,          json::Node(curvature)}
            };
        }

        json::Dict JsonReader::ParseRouteRequest(const json::Node& route_node)
        {
            int request_id = route_node.AsMap().at("id"s).AsInt();
            std::string from_string = route_node.AsMap().at("from"s).AsString();
            int from_int = transport_catalogue_.GetVertexId(from_string);
            std::string to_string = route_node.AsMap().at("to"s).AsString();
            int to_int = transport_catalogue_.GetVertexId(to_string);

            std::optional<graph::Router<double>::RouteInfo> route = router_.BuildRoute(from_int, to_int);
            //check
            if (!route.has_value())
            {
                return { {"request_id"s, request_id}, {"error_message"s, "not found"s}};
            }
            else if ((*route).edges.empty())
            {
                return { {"request_id"s, request_id}, {"total_time"s, 0}, {"items"s, json::Array(0)} };
            }

            json::Builder builder{};
            json::ArrayContext array_result = builder.StartDict().Key("request_id"s).Value(request_id).Key("total_time"s).Value((*route).weight).Key("items"s).StartArray();
            int bus_waiting_time = transport_catalogue_.GetRouteSettings().bus_wait_time;
            
            std::string waiting_stop, bus_name;
            double travel_time;
            int span_count;

            for (auto edge_id : (*route).edges)
            {
                waiting_stop = transport_catalogue_.GetFirstStopByEdgeId(edge_id);
                bus_name = transport_catalogue_.GetBusNameByEdgeId(edge_id);
                travel_time = transport_catalogue_.GetEdgeWeightByEdgeId(edge_id) - bus_waiting_time;
                span_count = transport_catalogue_.GetSpanCountByEdgeId(edge_id);
                array_result.StartDict().Key("type"s).Value("Wait"s).Key("stop_name"s).Value(waiting_stop).Key("time"s).Value(bus_waiting_time).EndDict()
                            .StartDict().Key("type"s).Value("Bus"s).Key("bus"s).Value(std::string(bus_name)).Key("span_count"s).Value(span_count).Key("time"s).Value(travel_time).EndDict();
            }
            return array_result.EndArray().Build().AsMap();
        }

        void JsonReader::ProcessStatRequests()
        {
            json::Array requests_array = json_document_.GetRoot().AsMap().at("stat_requests"s).AsArray();
            //json::Array result;
            json::Builder builder;
            json::ArrayContext array_result = builder.StartArray();

            for (const json::Node& request : requests_array)
            {
                if (request.AsMap().at("type"s).AsString() == "Stop"s)
                {
                    //result.push_back(ParseStopRequest(request));
                    array_result.Value(ParseStopRequest(request));
                }
                else if (request.AsMap().at("type"s).AsString() == "Bus"s)
                {
                    //result.push_back(ParseBusRequest(request));
                    array_result.Value(ParseBusRequest(request));
                }
                else if (request.AsMap().at("type"s).AsString() == "Map"s)
                {
                    std::ostringstream output;
                    map_renderer::MapRender map_renderer(GetRenderSettings(), transport_catalogue_.GetBusnamesToBuses());
                    map_renderer.SetProjectorSettings(transport_catalogue_.GetBusesCoordinates());
                    map_renderer.RenderMap().Render(output);
                    array_result.Value(json::Dict{ {"request_id"s, request.AsMap().at("id"s).AsInt()}, {"map"s, output.str()} });
                }
                else if (request.AsMap().at("type"s).AsString() == "Route"s)
                {
                    array_result.Value(ParseRouteRequest(request));
                }
            }
            json::Builder result = array_result.EndArray();
            json_result_ = json::Document(result.Build());
        }

        void JsonReader::PrintResult()
        {
            json::Print(json_result_, output_);
        }

        map_renderer::RenderSettings JsonReader::GetRenderSettings() const
        {
            map_renderer::RenderSettings result;
            json::Dict render_settings_map  = json_document_.GetRoot().AsMap().at("render_settings"s).AsMap();

            result.width                    = render_settings_map.at("width"s).AsDouble();
            result.height                   = render_settings_map.at("height"s).AsDouble();
            result.padding                  = render_settings_map.at("padding"s).AsDouble();
            result.line_width               = render_settings_map.at("line_width"s).AsDouble();
            result.stop_radius              = render_settings_map.at("stop_radius"s).AsDouble();
            result.bus_label_font_size      = render_settings_map.at("bus_label_font_size"s).AsInt();

            result.bus_label_offset =
            {
                render_settings_map.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                render_settings_map.at("bus_label_offset"s).AsArray()[1].AsDouble()
            };
            result.stop_label_font_size = render_settings_map.at("stop_label_font_size"s).AsInt();
            result.stop_label_offset =
            {
                render_settings_map.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                render_settings_map.at("stop_label_offset"s).AsArray()[1].AsDouble()
            };

            json::Node color_node = render_settings_map.at("underlayer_color"s);
            if (color_node.IsString())
            {
                result.underlayer_color = svg::Color(color_node.AsString());
            }
            else
            {
                if (color_node.AsArray().size() == 3)
                {
                    json::Array color_array = color_node.AsArray();
                    result.underlayer_color = svg::Color
                    (
                        svg::Rgb
                        {
                            static_cast<uint8_t>(color_array[0].AsInt()),
                            static_cast<uint8_t>(color_array[1].AsInt()),
                            static_cast<uint8_t>(color_array[2].AsInt())
                        }
                    );
                }
                else
                {
                    json::Array color_array = color_node.AsArray();
                    result.underlayer_color = svg::Color
                    (
                        svg::Rgba
                        {
                            static_cast<uint8_t>(color_array[0].AsInt()),
                            static_cast<uint8_t>(color_array[1].AsInt()),
                            static_cast<uint8_t>(color_array[2].AsInt()),
                                                (color_array[3].AsDouble())
                        }
                    );
                }
            }

            result.underlayer_width = render_settings_map.at("underlayer_width"s).AsDouble();

            for (const json::Node& color_node : render_settings_map.at("color_palette"s).AsArray())
            {
                if (color_node.IsString()) 
                {
                    result.color_palette.push_back(color_node.AsString());
                }
                else if (color_node.IsArray()) 
                {
                    if (color_node.AsArray().size() == 3) 
                    {
                        json::Array color_array = color_node.AsArray();
                        result.color_palette.push_back( 
                            svg::Color(
                            svg::Rgb
                            {
                                static_cast<uint8_t>(color_array[0].AsInt()),
                                static_cast<uint8_t>(color_array[1].AsInt()),
                                static_cast<uint8_t>(color_array[2].AsInt())
                            }
                        ));
                    }
                    else 
                    {
                        json::Array color_array = color_node.AsArray();
                        result.color_palette.push_back(
                            svg::Color(
                            svg::Rgba
                            {
                                static_cast<uint8_t>(color_array[0].AsInt()),
                                static_cast<uint8_t>(color_array[1].AsInt()),
                                static_cast<uint8_t>(color_array[2].AsInt()),
                                                    (color_array[3].AsDouble())
                            }
                        ));
                    }
                }
            }
            return result;
        }
	}//namespace json_reader
}//namespace transport_catalogue