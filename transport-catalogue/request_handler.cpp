#include "request_handler.h"

using namespace std;

namespace transport_catalogue
{
	namespace request_handler
	{
		RequestHandler::RequestHandler(TransportCatalogue& transport_catalogue, istream& input, ostream& output)
			: transport_catalogue_(transport_catalogue)
			, input_(input)
			, output_(output)
			, json_reader_(transport_catalogue, input, output)
		{
		}

		void RequestHandler::LoadDataIntoTC()
		{
			json_reader_.LoadJSON();
			json_reader_.ProcessBaseRequests();
		}

		void RequestHandler::ProcessRequests()
		{
			json_reader_.ProcessStatRequests();
		}

		void RequestHandler::PrintResult()
		{
			json_reader_.PrintResult();
		}

		void RequestHandler::RenderMap()
		{
			map_renderer::MapRender map_render(json_reader_.GetRenderSettings(), transport_catalogue_.GetBusnamesToBuses());
			map_render.SetProjectorSettings(transport_catalogue_.GetBusesCoordinates());
			svg::Document doc = map_render.RenderMap();
			doc.Render(output_);
		}

	}
}