#include "transport_catalogue.h"

using namespace std;

namespace transport_catalogue
{
	void TransportCatalogue::AddBus(const Bus bus)
	{
		Bus& deque_bus = *(buses_.insert(buses_.end(), bus));
		busnames_to_buses_.insert({deque_bus.name, &deque_bus});
		
		for (const auto& stop : deque_bus.route.stops)
		{
			stopnames_to_busnames_[stop].insert(deque_bus.name);
		}
	}

	void TransportCatalogue::AddStop(const Stop stop)
	{
		Stop& deque_stop = *(stops_.insert(stops_.end(), stop));
		stopnames_to_stops_.insert({deque_stop.name, &deque_stop});
	}

	void TransportCatalogue::AddDistance(const std::string& stop1, const std::string& stop2, int distance)
	{
		stops_distances_.insert({ {stopnames_to_stops_.at(stop1), stopnames_to_stops_.at(stop2)}, distance * 1.0});
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


	set<string> TransportCatalogue::GetBusesForStop(const Stop& stop)
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

}