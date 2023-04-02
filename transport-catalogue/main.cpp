#include "transport_catalogue.h"
#include "json_reader.h"
#include "request_handler.h"
#include "serialization.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

using namespace transport_catalogue;
using namespace std::literals;

int main(int argc, const char** argv)
{
	//setlocale(LC_ALL, "Russian");
	std::ifstream base_input;
	base_input.open("/home/dmitry/source/repos/Project5/base_input.txt");
	std::ifstream stat_input;
	stat_input.open("/home/dmitry/source/repos/Project5/stat_input.txt");
	std::ofstream output_txt;
	output_txt.open("/home/dmitry/source/repos/Project5/output.txt");
	// std::ofstream output_svg;
	// output_svg.open("output.svg");


	//transport_catalogue::TransportCatalogue catalogue;
	//request_handler::RequestHandler request_handler(catalogue, input, output_txt, std::cout);
	//request_handler.LoadDataIntoTC();
	//request_handler.ProcessRequests();
	//request_handler.PrintResult();
	////request_handler.RenderMap();
	////system("pause");
	//return 0;

	if (argv[1] == "make_base"s) 
	{
		transport_catalogue::TransportCatalogue catalogue;
		request_handler::RequestHandler request_handler(catalogue);

		request_handler.LoadDataIntoTC(base_input);
		std::string filename = request_handler.GetSerializationFilename();
		Serialize(catalogue, filename);
	}
	else if (argv[1] == "process_requests"s) 
	{
		transport_catalogue::TransportCatalogue catalogue;
		request_handler::RequestHandler request_handler(catalogue);
		request_handler.LoadJsonDocument(stat_input);

		std::string filename = request_handler.GetSerializationFilename();
		Deserialize(filename, catalogue);

		request_handler.ProcessRequests(output_txt);
	}
	return 0;
}