#pragma once
#include "geo.h"
#include "json.h"

#include <vector>
#include <string>

namespace transport_catalogue
{
	struct Stop
	{
		std::string name;
		coordinates::Coordinates coordinates;
	};

	inline bool operator==(const Stop& lhs, const Stop& rhs)
	{
		return lhs.name == rhs.name && lhs.coordinates == rhs.coordinates;
	}

	struct Route
	{
		std::vector<std::string> stops;
		bool is_looped;
	};

	struct RouteSettings
	{
		int bus_wait_time;
		double bus_velocity;
	};

	struct Bus
	{
		std::string name;
		std::vector<const Stop*> stops;
		bool is_looped;
	};

	namespace json_reader
	{
		struct ParsedStop
		{
			std::string					stop_name;
			double						latitude;
			double						longitude;
		};
		struct ParsedDistance
		{
			std::string					first_stop_name;
			json::Dict					stop_names_and_distances;
		};
		struct ParsedBus
		{
			std::string					bus_name;
			std::vector<std::string>	stop_names;
			bool						is_looped;
		};
	}
}