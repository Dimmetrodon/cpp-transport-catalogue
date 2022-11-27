#pragma once
#include "geo.h"

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
	struct Route
	{
		std::vector<std::string> stops;
		bool is_looped;
	};

	struct Bus
	{
		std::string name;
		Route route;
	};

	struct Stop
	{
		std::string name;
		coordinates::Coordinates coordinates;
	};

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
		void AddBus(const Bus bus);
		void AddStop(const Stop stop);
		void AddDistance(const std::string& stop1, const std::string& stop2, int distance);

		const Bus& FindBus(const std::string& name);
		const Stop& FindStop(const std::string& name);

		double GetGeoDistance(const std::string& stop1, const std::string& stop2);
		double GetRealDistance(const std::string& stop1, const std::string& stop2);
		std::set<std::string> GetBusesForStop(const Stop& stop);

	private:
		std::deque<Bus> buses_;
		std::deque<Stop> stops_;

		std::unordered_map<std::string, const Bus*> busnames_to_buses_;
		std::unordered_map<std::string, const Stop*> stopnames_to_stops_;
		std::unordered_map<std::string, std::set<std::string>> stopnames_to_busnames_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::StopsHasher> stops_distances_;
	};
}