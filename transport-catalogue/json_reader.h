#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "json.h"
#include "map_renderer.h"

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
			JsonReader(TransportCatalogue& transport_catalogue, std::istream& input, std::ostream& output);

			void							LoadJSON();
			void							ProcessBaseRequests();
			void							ProcessStatRequests();
			void							PrintResult();

			ParsedStop						ParseStop(const json::Node& stop_node);
			ParsedDistance					ParseDistance(const json::Node& stop_node);
			ParsedBus						ParseBus(const json::Node& bus_node);

			json::Dict						ParseStopRequest(const json::Node& stop_node);
			json::Dict						ParseBusRequest(const json::Node& bus_node);

			map_renderer::RenderSettings	GetRenderSettings() const;

		private:
			TransportCatalogue&				transport_catalogue_;
			std::istream&					input_;
			std::ostream&					output_;
			json::Document					json_document_;
			json::Document					json_result_;
		};
	}
}