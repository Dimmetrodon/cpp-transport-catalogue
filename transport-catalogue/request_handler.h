#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"

#include <vector>
#include <map>

namespace transport_catalogue
{
	namespace request_handler
	{
		class RequestHandler
		{
		public:
			RequestHandler(TransportCatalogue& transport_catalogue);

			void						LoadDataIntoTC(std::istream& input);
			void						ProcessRequests(std::ostream& output);
			//void						PrintResult();
			void						LoadJsonDocument(std::istream& input);
			void						RenderMap(std::ostream& output);
			std::string					GetSerializationFilename() const;

		private:
			TransportCatalogue&			transport_catalogue_;
			json_reader::JsonReader		json_reader_;
		};
	}
}