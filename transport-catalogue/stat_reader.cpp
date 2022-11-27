#include "stat_reader.h"

using namespace std;

namespace transport_catalogue
{
	namespace stat_reader
	{
		StatReader::StatReader(TransportCatalogue& transport_catalogue, istream& input, ostream& out)
			:transport_catalogue_(transport_catalogue)
			,input_(input)
			,output_(out)
		{
		}

		void StatReader::Read()
		{
			string line;
			getline(input_, line); // Кол-во запросов
			size_t request_number = stoi(line);

			for (size_t i = 0; i != request_number; ++i)
			{
				getline(input_, line);
				if (line.substr(0, 3) == "Bus"s)
				{
					PrintBusInfo(line.substr(4));
				}
				else if (line.substr(0, 4) == "Stop"s)
				{
					PrintStopInfo(line.substr(5));
				}
			}
		}

		void StatReader::PrintBusInfo(const string& bus_name)
		{
			const Bus& bus = transport_catalogue_.FindBus(bus_name);
			if (bus.name.empty())
			{
				output_ << "Bus "s << bus_name << ": not found"s << endl;
				return;
			}

			output_ << "Bus "s << bus.name << ": "s 
					<< StopCount(bus) << " stops on route, "s 
					<< UniqueStopCount(bus) << " unique stops, " 
					<< setprecision(6) << CalculateRouteLengthReal(bus) << " route length, "s
					<< setprecision(6) << CalculateRouteLengthReal(bus)/CalculateRouteLengthGeo(bus) << " curvature"s << endl;
		}

		size_t StatReader::StopCount(const Bus& bus)
		{
			if (!bus.route.is_looped)
			{
				return bus.route.stops.size() * 2 - 1;
			}
			return bus.route.stops.size();
		}

		size_t StatReader::UniqueStopCount(const Bus& bus)
		{
			unordered_set<string> unique_stops;
			for (const auto& stop : bus.route.stops)
			{
				unique_stops.insert(stop);
			}

			return unique_stops.size();
		}

		double StatReader::CalculateRouteLengthGeo(const Bus& bus)
		{
			double result = 0;
			string first_stop = *bus.route.stops.begin();
			for (const string& second_stop : bus.route.stops)
			{
				result += transport_catalogue_.GetGeoDistance(first_stop, second_stop);
				first_stop = second_stop;
			}

			result = (bus.route.is_looped ? result : result * 2);
			return result;
		}

		int StatReader::CalculateRouteLengthReal(const Bus& bus)
		{
			double result = 0;
			string first_stop, second_stop;
			for (size_t i = 1; i != bus.route.stops.size(); ++i)
			{
				first_stop = bus.route.stops[i - 1];
				second_stop = bus.route.stops[i];
				result += transport_catalogue_.GetRealDistance(first_stop, second_stop);
			}
			if (!bus.route.is_looped)
			{
				for (size_t i = bus.route.stops.size() - 1; i != 0; --i)
				{
					first_stop = bus.route.stops[i];
					second_stop = bus.route.stops[i - 1];
					result += transport_catalogue_.GetRealDistance(first_stop, second_stop);
				}
			}

			return result;
		}

		void StatReader::PrintStopInfo(const string& stop_name)
		{
			const Stop& stop = transport_catalogue_.FindStop(stop_name);

			if (stop.name.empty())
			{
				output_ << "Stop "s << stop_name << ": not found"s << endl;
			}
			else if (transport_catalogue_.GetBusesForStop(stop).empty())
			{
				output_ << "Stop "s << stop_name << ": no buses"s << endl;
			}
			else
			{
				output_ << "Stop "s << stop_name << ": buses"s;
				for (const auto& bus : transport_catalogue_.GetBusesForStop(stop))
				{
					output_ << " "s << bus;
				}
				output_ << endl;
			}
		}
	}
}