#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue
{
	void TransportCatalogue::AddBus(string& bus_name, const vector<string>& stop_names, bool is_looped)
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

	void TransportCatalogue::AddStop(string& stop_name, coordinates::Coordinates coordinates)
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
			//auto test = unique_bus_names.find(bus_stop->name); // !!
			//if (unique_bus_names.find(bus_stop->name) != unique_bus_names.end())
			//{
			//	unique_bus_names.insert(bus_stop->name);
			//}
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

}