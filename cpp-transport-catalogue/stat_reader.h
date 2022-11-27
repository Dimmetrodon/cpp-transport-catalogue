#pragma once
#include "transport_catalogue.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
namespace transport_catalogue
{
	namespace stat_reader
	{
		class StatReader
		{
		public:
			StatReader(TransportCatalogue& transport_catalogue, std::istream& input, std::ostream& out);
			void Read();
			size_t UniqueStopCount(const Bus& bus);
			size_t StopCount(const Bus& bus);
			double CalculateRouteLengthGeo(const Bus& bus);
			int CalculateRouteLengthReal(const Bus& bus);

			void PrintBusInfo(const std::string& bus_name);
			void PrintStopInfo(const std::string& stop_name);

		private:
			TransportCatalogue& transport_catalogue_;
			std::istream& input_;
			std::ostream& output_;
		};
	}
}