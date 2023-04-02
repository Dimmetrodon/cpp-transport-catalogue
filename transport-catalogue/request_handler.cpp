#include "request_handler.h"

using namespace std;

namespace transport_catalogue
{
	namespace request_handler
	{
		RequestHandler::RequestHandler(TransportCatalogue& transport_catalogue)
			: transport_catalogue_(transport_catalogue)
			, json_reader_(transport_catalogue)
		{
		}

		void RequestHandler::LoadJsonDocument(std::istream& input) 
		{
			json_reader_.LoadJSON(input);
		}

		void RequestHandler::LoadDataIntoTC(std::istream& input)
		{
			json_reader_.LoadJSON(input);
			json_reader_.ProscessRoutingSettings();
			json_reader_.ProcessBaseRequests(); 
		}

		void RequestHandler::ProcessRequests(std::ostream& output)
		{
			json_reader_.ProcessStatRequests(output);
		}

		/*void RequestHandler::PrintResult()
		{
			json_reader_.PrintResult();
		}*/

		void RequestHandler::RenderMap(std::ostream& output)
		{
			map_renderer::MapRender map_render(json_reader_.GetRenderSettings(), transport_catalogue_.GetBusnamesToBuses());
			map_render.SetProjectorSettings(transport_catalogue_.GetBusesCoordinates());
			svg::Document doc = map_render.RenderMap();
			doc.Render(output);
		}

		std::string RequestHandler::GetSerializationFilename() const 
		{
			return json_reader_.GetSerializationFilename();
		}

	}
}