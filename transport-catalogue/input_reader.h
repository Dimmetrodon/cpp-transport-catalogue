#pragma once
#include "geo.h"
#include "transport_catalogue.h"

#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

namespace transport_catalogue
{
	namespace input_reader
	{
		class Reader
		{
		public:
			Reader(TransportCatalogue& transport_catalogue);
			void Read(std::istream& input);

			Stop ParseStop(std::string& text);
			Bus ParseBus(std::string text);
			Route ParseRoute(std::string text);
			std::unordered_map<std::string, int> ParseDistance(std::string stop);

		private:
			TransportCatalogue& transport_catalogue_;
		};
	}
	
}
