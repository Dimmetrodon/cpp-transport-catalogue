#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"

#include <string>
#include <vector>
#include <sstream>
namespace transport_catalogue
{
	namespace json_reader
	{
		class JsonReader
		{
		public:
			JsonReader(TransportCatalogue& transport_catalogue);

			void							LoadJSON(std::istream& input);
			void							ProcessBaseRequests();
			void							ProscessRoutingSettings();
			void							ProcessStatRequests(std::ostream& output);
			//void							PrintResult();

			ParsedStop						ParseStop(const json::Node& stop_node);
			ParsedDistance					ParseDistance(const json::Node& stop_node);
			ParsedBus						ParseBus(const json::Node& bus_node);

			json::Dict						ParseStopRequest(const json::Node& stop_node);
			json::Dict						ParseBusRequest(const json::Node& bus_node);
			json::Dict						ParseRouteRequest(const json::Node& route_node, const graph::Router<double>& router);

			map_renderer::RenderSettings	GetRenderSettings() const;
			RouteSettings					GetRoutingSettings() const;
			std::string						GetSerializationFilename() const;
		private:
			TransportCatalogue&				transport_catalogue_;
			json::Document					json_document_;
			json::Document					json_result_;
		};
	}
}