#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue
{
	void TransportCatalogue::AddBus(const string& bus_name, const vector<string>& stop_names, bool is_looped)
	{
		vector<const Stop*> stops_ptrs;
		for (const string& stop_name : stop_names)
		{
			if (stopnames_to_stops_.find(stop_name) == stopnames_to_stops_.end())
			{
				continue;
			}
			stops_ptrs.push_back(stopnames_to_stops_.at(stop_name));
		}
		Bus& deque_bus = *(buses_.insert(buses_.end(), { bus_name, stops_ptrs, is_looped }));
		busnames_to_buses_.insert({ deque_bus.name, &deque_bus });

		for (const auto& stop : deque_bus.stops)
		{
			stopnames_to_busnames_[stop->name].insert(bus_name);
		}
	}

	void TransportCatalogue::AddStop(const string& stop_name, coordinates::Coordinates coordinates)
	{
		Stop& deque_stop = *(stops_.insert(stops_.end(), { stop_name, coordinates }));
		stopnames_to_stops_.insert({ deque_stop.name, &deque_stop });
	}

	void TransportCatalogue::AddDistance(const string& stop1, const string& stop2, int distance)
	{
		stops_distances_.insert({ {stopnames_to_stops_.at(stop1), stopnames_to_stops_.at(stop2)}, distance * 1.0 });
	}

	double TransportCatalogue::GetGeoDistance(const string& stop1, const string& stop2)
	{
		if (stop1 == stop2)
		{
			return 0;
		}
		return coordinates::ComputeDistance(stopnames_to_stops_.at(stop1)->coordinates, stopnames_to_stops_.at(stop2)->coordinates);
	}

	double TransportCatalogue::GetRealDistance(const string& stop1, const string& stop2)
	{
		if (stops_distances_.count({ stopnames_to_stops_.at(stop1), stopnames_to_stops_.at(stop2) }))
		{
			return stops_distances_.at({ stopnames_to_stops_.at(stop1), stopnames_to_stops_.at(stop2) });
		}
		return stops_distances_.at({ stopnames_to_stops_.at(stop2), stopnames_to_stops_.at(stop1) });
	}

	double TransportCatalogue::GetGeoRouteDistance(const Bus& bus)
	{
		double result = 0.0;
		for (size_t i = 0; i < bus.stops.size() - 1; ++i)
		{
			result += coordinates::ComputeDistance(bus.stops[i]->coordinates, bus.stops[i + 1]->coordinates);
		}
		if (!bus.is_looped)
		{
			result *= 2;
		}
		return result;
	}

	double TransportCatalogue::GetRealRouteDistance(const Bus& bus)
	{
		double result = 0;
		for (size_t i = 0; i < bus.stops.size() - 1; ++i)
		{
			result += GetRealDistance(bus.stops[i]->name, bus.stops[i + 1]->name);
		}
		if (!bus.is_looped)
		{
			for (size_t i = 0; i < bus.stops.size() - 1; ++i)
			{
				result += GetRealDistance(bus.stops[i + 1]->name, bus.stops[i]->name);
			}
		}
		return result;
	}

	set<string> TransportCatalogue::GetBusesByStop(const Stop& stop)
	{
		if (stopnames_to_busnames_.count(stop.name))
		{
			return stopnames_to_busnames_.at(stop.name);
		}

		return {};
	}

	const Bus& TransportCatalogue::FindBus(const string& name)
	{
		if (busnames_to_buses_.count(name))
		{
			return *(busnames_to_buses_.at(name));
		}
		static Bus empty;
		return empty;
	}

	const Stop& TransportCatalogue::FindStop(const string& name)
	{
		if (stopnames_to_stops_.count(name))
		{
			return *(stopnames_to_stops_.at(name));
		}
		static Stop empty;
		return empty;
	}

	int TransportCatalogue::GetBusStopCount(const string& bus_name)
	{
		const Bus& bus = this->FindBus(bus_name);
		int one_way_stop_count = bus.stops.size();
		if (!bus.is_looped)
		{
			return one_way_stop_count * 2 - 1;
		}
		else
		{
			return one_way_stop_count;
		}
	}

	int	TransportCatalogue::GetBusUniqueStopsCount(const string& bus_name)
	{
		const vector<const Stop*> bus_stops = this->FindBus(bus_name).stops;
		unordered_set<string> unique_bus_names;
		for (const Stop* bus_stop : bus_stops)
		{
			unique_bus_names.insert(bus_stop->name);
		}
		return unique_bus_names.size();
	}

	double TransportCatalogue::GetBusCurvature(const string& bus_name)
	{
		const Bus& bus = this->FindBus(bus_name);
		double geo_distance = 0;
		int real_distance = 0;

		for (size_t i = 0; i < bus.stops.size() - 1; ++i)
		{
			geo_distance += coordinates::ComputeDistance(bus.stops[i]->coordinates, bus.stops[i + 1]->coordinates);
			real_distance += this->GetRealDistance(bus.stops[i]->name, bus.stops[i + 1]->name);
		}
		if (!bus.is_looped)
		{
			for (size_t i = 0; i < bus.stops.size() - 1; ++i)
			{
				real_distance += this->GetRealDistance(bus.stops[i + 1]->name, bus.stops[i]->name);
			}
			geo_distance *= 2;
		}
		return real_distance / geo_distance;
	}

	const map<string, const Bus*>& TransportCatalogue::GetBusnamesToBuses() const
	{
		return busnames_to_buses_;
	}

	vector<coordinates::Coordinates> TransportCatalogue::GetBusesCoordinates() const
	{
		vector<coordinates::Coordinates> result;
		for (const auto& [bus_name, bus] : busnames_to_buses_)
		{
			for (const Stop* stop : bus->stops)
			{
				result.push_back(stop->coordinates);
			}
		}
		return result;
	}

	void TransportCatalogue::BuildGraph()
	{
		graph_ = graph::DirectedWeightedGraph<double>(stops_.size());
		for (const Bus& bus : buses_)
		{
			size_t left_bus_stop_count = bus.stops.size();
			for (size_t i = 0; i != left_bus_stop_count; ++i)
			{
				const Stop* first_stop_iter = bus.stops[i];
				graph::EdgeId first_stop_id = distance(stops_.begin(), find(stops_.begin(), stops_.end(), *first_stop_iter));
				double forward_distance = 0;
				double backwards_distance = 0;
				for (size_t j = i + 1; j != left_bus_stop_count; ++j)
				{
					const Stop* second_stop_iter = bus.stops[j];
					const Stop* prev_second_stop_iter = bus.stops[j - 1];
					forward_distance += GetRealDistance(prev_second_stop_iter->name, second_stop_iter->name);
					graph::EdgeId second_stop_id = distance(stops_.begin(), find(stops_.begin(), stops_.end(), *second_stop_iter));

					graph_.AddEdge
						({ first_stop_id, second_stop_id, (static_cast<uint32_t>(j) - static_cast<uint32_t>(i)), bus.name, route_settings_.bus_wait_time + forward_distance / route_settings_.bus_velocity * 60 / 1000 });
					if (!bus.is_looped)
					{
						backwards_distance += GetRealDistance(second_stop_iter->name, prev_second_stop_iter->name);
						graph_.AddEdge
							({ second_stop_id, first_stop_id, (static_cast<uint32_t>(j) - static_cast<uint32_t>(i)), bus.name, route_settings_.bus_wait_time + backwards_distance / route_settings_.bus_velocity * 60 / 1000 });
					}
				}
			}
		}
	}

	void TransportCatalogue::AddRouteSettings(RouteSettings route_settings)
	{
		route_settings_ = route_settings;
	}

	graph::VertexId TransportCatalogue::GetVertexId(std::string_view stop_name) const
	{
		std::deque<transport_catalogue::Stop>::const_iterator needed_stop = find_if(stops_.begin(), stops_.end(), [stop_name](const Stop& stop) {return stop.name == stop_name; });
		return distance(stops_.begin(), needed_stop);
	}

	const graph::DirectedWeightedGraph<double>& TransportCatalogue::GetGraph() const
	{
		return graph_;
	}

	const RouteSettings& TransportCatalogue::GetRouteSettings() const
	{
		return route_settings_;
	}

	const string& TransportCatalogue::GetFirstStopByEdgeId(graph::EdgeId id) const
	{
		graph::Edge edge = graph_.GetEdge(id);
		return stops_.at(edge.from).name;
	}

	const string TransportCatalogue::GetBusNameByEdgeId(graph::EdgeId id) const
	{
		graph::Edge edge = graph_.GetEdge(id);
		return edge.bus_name;
	}

	double TransportCatalogue::GetEdgeWeightByEdgeId(graph::EdgeId id) const
	{
		graph::Edge edge = graph_.GetEdge(id);
		return edge.weight;
	}

	uint32_t TransportCatalogue::GetSpanCountByEdgeId(graph::EdgeId id) const
	{
		graph::Edge edge = graph_.GetEdge(id);
		return edge.span_count;
	}

	const std::deque<Stop>& TransportCatalogue::GetStops() const
	{
		return stops_; 
	}

	const std::deque<Bus>& TransportCatalogue::GetBuses() const 
	{
		return buses_;
	}

	const std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::StopsHasher>& TransportCatalogue::GetDistances() const
	{
		return stops_distances_;
	}

	size_t TransportCatalogue::GetStopIndex(const Stop* stop) const
	{
		auto it = find(stops_.begin(), stops_.end(), *stop);
		return distance(stops_.begin(), it);
	}

	std::string TransportCatalogue::GetStopnameByIndex(size_t index) const
	{
		return stops_.at(index).name;
	}

	void TransportCatalogue::SetRenderSettings(map_renderer::RenderSettings settings)
	{
		render_settings_ = settings;
	}

	const map_renderer::RenderSettings&	TransportCatalogue::GetRenderSettings() const
	{
		return render_settings_;
	}

	void TransportCatalogue::SetGraph(graph::DirectedWeightedGraph<double> graph)
	{
		graph_ = graph;
	}
}// namespace transport_catalogue