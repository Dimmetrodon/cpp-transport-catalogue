#include "input_reader.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace transport_catalogue
{
	namespace input_reader
	{
		Reader::Reader(TransportCatalogue& transport_catalogue)
			:transport_catalogue_(transport_catalogue)

		{
		}

		void Reader::Read(istream& input)
		{
			string line;
			vector<string> buses, stops;
			getline(input, line);
			size_t input_number = stoi(line);  //Кол-во запросов

			//Запись автобусов
			for (size_t i = 0; i != input_number; ++i)
			{
				getline(input, line);
				if (line.substr(0, 3) == "Bus"s)
				{
					buses.push_back(line);
					continue;
				}
				//Запись остановок
				//transport_catalogue_.AddStop(ParseStop(line));
				stops.push_back(line);
			}

			for (auto& stop : stops)
			{
				transport_catalogue_.AddStop(ParseStop(stop));
			}

			for (auto& bus : buses)
			{
				transport_catalogue_.AddBus(ParseBus(bus));
			}

			for (auto& stop : stops)
			{
				if (stop.find(',') == stop.find_last_of(','))
				{
					continue;
				}
				for (const auto& [stop_name, distance] : ParseDistance(stop))
				{
					string start_stop = stop.substr(5, stop.find(':') - 5);
					transport_catalogue_.AddDistance(start_stop, stop_name, distance);
				}
			}
		}

		Stop Reader::ParseStop(string& text)
		{
			size_t colon = text.find(':');
			size_t comma = text.find(',');

			string name = text.substr(5, colon - 5);

			double lat = stod(text.substr(colon + 2, comma - colon - 2));
			double lng = stod(text.substr(comma + 2, text.size() - 3));
			
			return { name, {lat, lng} };
		}

		Route Reader::ParseRoute(string text)
		{
			text.erase(text.begin());
			bool looped = (text.find('>') != string::npos ? true : false);
			vector<string> stops;
			char limiter_char = (looped ? '>' : '-');
			text += " "s + limiter_char;

			while (!text.empty())
			{
				stops.push_back(text.substr(0, text.find(limiter_char) - 1));
				text.erase(0, text.find(limiter_char) + 2);
			}

			return {stops, looped};
		}

		Bus Reader::ParseBus(string text)
		{
			size_t colon = text.find(':');
			string name = text.substr(4, colon - 4);
			text.erase(0, colon + 1);
			
			return {name, ParseRoute(text)};
		}
		
		unordered_map<string, int> Reader::ParseDistance(string stop)
		{
			//Stop Biryulyovo Zapadnoye : 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
			unordered_map<string, int> result;
			stop = stop.substr(stop.find(',') + 1);
			stop = stop.substr(stop.find(',') + 1); //Осталась строка с путями и пробелом в начале
			// 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam

			int distance;
			while (!stop.empty())
			{
				stop = stop.substr(1);
				distance = stoi(stop.substr(0, stop.find('m')));
				stop = stop.substr(stop.find("to"s));
				stop = stop.substr(3);
				//Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam

				if (stop.find(',') != string::npos)
				{
					result.insert({ stop.substr(0, stop.find(',')), distance});
					stop = stop.substr(stop.find(','));
					stop = stop.substr(1);
				}
				else
				{
					result.insert({ stop.substr(0, stop.size()), distance });
					stop = ""s;
				}
			}
			return result;
		}
	}
}