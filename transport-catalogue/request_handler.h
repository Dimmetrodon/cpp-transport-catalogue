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
			RequestHandler(TransportCatalogue& transport_catalogue, std::istream& input, std::ostream& output);

			void						LoadDataIntoTC();
			void						ProcessRequests();
			void						PrintResult();
			void						RenderMap();

		private:
			TransportCatalogue&			transport_catalogue_;
			std::istream&				input_;
			std::ostream&				output_;
			json_reader::JsonReader		json_reader_;
		};
	}
}