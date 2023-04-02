#pragma once
#include "geo.h"
#include "domain.h"
#include "router.h"
#include "graph.h"
#include "map_renderer.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <deque>
#include <vector>
#include <optional>

namespace transport_catalogue
{
	namespace detail
	{
		struct StopsHasher
		{
			size_t operator() (const std::pair<const Stop*, const Stop*>& stops) const
			{
				return stop_ptr(stops.first) * stops.first->name.size() + stop_ptr(stops.second);
			}

		private:
			std::hash<const void*> stop_ptr;
		};
	}


	class TransportCatalogue
	{
	public:
		using Stop_distances_Hash = std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::StopsHasher>;

		void										AddBus(const std::string& bus_name, const std::vector<std::string>& stop_names, bool is_looped);
		void                                        AddStop(const std::string& stop_name, coordinates::Coordinates coordinates);
		void                                        AddDistance(const std::string& stop1, const std::string& stop2, int distance);
		void										AddRouteSettings(RouteSettings route_settings);

		const Bus&                                  FindBus(const std::string& name);
		const Stop&                                 FindStop(const std::string& name);

		double                                      GetGeoDistance(const std::string& stop1, const std::string& stop2);
		double                                      GetRealDistance(const std::string& stop1, const std::string& stop2);
		double                                      GetGeoRouteDistance(const Bus& bus);
		double                                      GetRealRouteDistance(const Bus& bus);
		std::set<std::string>                       GetBusesByStop(const Stop& stop);
		int                                         GetBusStopCount(const std::string& bus_name);
		int                                         GetBusUniqueStopsCount(const std::string& bus_name);
		double                                      GetBusCurvature(const std::string& bus_name);

		void										BuildGraph();
		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		graph::VertexId								GetVertexId(std::string_view stop_name) const;
		const RouteSettings&						GetRouteSettings() const;

		const std::string&							GetFirstStopByEdgeId(graph::EdgeId id) const;
		const std::string							GetBusNameByEdgeId(graph::EdgeId id) const;
		double										GetEdgeWeightByEdgeId(graph::EdgeId id) const;
		uint32_t									GetSpanCountByEdgeId(graph::EdgeId id) const;

		std::vector<coordinates::Coordinates>       GetBusesCoordinates() const;
		const std::map<std::string, const Bus*>&    GetBusnamesToBuses() const;

		const std::deque<Stop>&						GetStops() const;
		const std::deque<Bus>&						GetBuses() const;
		const Stop_distances_Hash&					GetDistances() const;
		size_t										GetStopIndex(const Stop* stop) const;
		std::string									GetStopnameByIndex(size_t index) const;
		void 										SetRenderSettings(map_renderer::RenderSettings settings);
		const map_renderer::RenderSettings& 		GetRenderSettings() const;
		void 										SetGraph(graph::DirectedWeightedGraph<double> graph);

	private:
		std::deque<Bus>												buses_;
		std::deque<Stop>											stops_;
		RouteSettings												route_settings_;
		graph::DirectedWeightedGraph<double>						graph_;
		map_renderer::RenderSettings 								render_settings_;

		std::map<std::string, const Bus*>							busnames_to_buses_;
		std::unordered_map<std::string, const Stop*>				stopnames_to_stops_;
		std::unordered_map<std::string, std::set<std::string>>		stopnames_to_busnames_;
		Stop_distances_Hash											stops_distances_;
	};
}