#include "json_reader.h"

using namespace std;
/* Input
{
"base_requests": [
{
    "type": "Bus",
        "name" : "114",
        "stops" : ["Морской вокзал", "Ривьерский мост"] ,
        "is_roundtrip" : false
},
    {
      "type": "Stop",
      "name" : "Ривьерский мост",
      "latitude" : 43.587795,
      "longitude" : 39.716901,
      "road_distances" : {"Морской вокзал": 850}
    },
    {
      "type": "Stop",
      "name" : "Морской вокзал",
      "latitude" : 43.581969,
      "longitude" : 39.719848,
      "road_distances" : {"Ривьерский мост": 850}
    }
],
"stat_requests": [
    { "id": 1, "type" : "Stop", "name" : "Ривьерский мост" },
    { "id": 2, "type" : "Bus", "name" : "114" }
]
}
*/

/* Output
[
{
    "buses": [
        "114"
    ] ,
        "request_id" : 1
},
    {
        "curvature": 1.23199,
        "request_id" : 2,
        "route_length" : 1700,
        "stop_count" : 3,
        "unique_stop_count" : 2
    }
]
*/
namespace transport_catalogue
{
	namespace json_reader
	{
        JsonReader::JsonReader(TransportCatalogue& transport_catalogue, istream& input, ostream& output)
            : transport_catalogue_(transport_catalogue)
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
                stop_node.AsMap().at("longitude").AsDouble()
            };
        }

        ParsedDistance JsonReader::ParseDistance(const json::Node& stop_node)
        {
            /*string first_stop_name = stop_node.AsMap().at("name"s).AsString();
            json::Dict stop_names_and_distances = stop_node.AsMap().at("road_distances").AsMap();
            return {first_stop_name, stop_names_and_distances};*/
            return
            {
                stop_node.AsMap().at("name"s).AsString(),
                stop_node.AsMap().at("road_distances").AsMap()
            };
        }

        ParsedBus JsonReader::ParseBus(const json::Node& bus_node)
        {
            vector<string> stop_names;
            for (const json::Node& stop_node : bus_node.AsMap().at("stops").AsArray())
            {
                stop_names.push_back(stop_node.AsString());
            }
            return
            {
                bus_node.AsMap().at("name").AsString(),
                stop_names,
                bus_node.AsMap().at("is_roundtrip").AsBool()
            };
        }

		void JsonReader::ProcessBaseRequests()
		{
            json::Array requests_array = json_document_.GetRoot().AsMap().at("base_requests"s).AsArray();

            //Sort requests: buses or stops
            json::Array bus_requests, stop_requests;
            for (const json::Node& request : requests_array)
            {
                string request_type = request.AsMap().at("type").AsString();
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
                {"request_id"s,         json::Node(stop_node.AsMap().at("id").AsInt())}
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

        void JsonReader::ProcessStatRequests()
        {
            json::Array requests_array = json_document_.GetRoot().AsMap().at("stat_requests").AsArray();
            json::Array result;

            for (const json::Node& request : requests_array)
            {
                if (request.AsMap().at("type"s).AsString() == "Stop"s)
                {
                    result.push_back(ParseStopRequest(request));
                }
                else if (request.AsMap().at("type"s).AsString() == "Bus"s)
                {
                    result.push_back(ParseBusRequest(request));
                }
                else if (request.AsMap().at("type"s).AsString() == "Map"s)
                {
                    //!!
                    std::ostringstream output;
                    map_renderer::MapRender map_renderer(GetRenderSettings(), transport_catalogue_.GetBusnamesToBuses());
                    map_renderer.SetProjectorSettings(transport_catalogue_.GetBusesCoordinates());
                    map_renderer.RenderMap().Render(output);
                    result.push_back(json::Dict{ {"request_id"s, request.AsMap().at("id"s).AsInt()}, {"map"s, output.str()} });
                }
            }
            json_result_ = json::Document(json::Node(result));
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
            result.stop_label_font_size = render_settings_map.at("stop_label_font_size").AsInt();
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
	}
}