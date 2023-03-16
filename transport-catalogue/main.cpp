#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_builder.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

using namespace transport_catalogue;
using namespace std::literals;

int main() 
{
	setlocale(LC_ALL, "Russian");
	std::ifstream input;
	input.open("input.txt");
	std::ofstream output_txt;
	output_txt.open("output.txt");
	std::ofstream output_svg;
	output_svg.open("output.svg");


	transport_catalogue::TransportCatalogue catalogue;
	request_handler::RequestHandler request_handler(catalogue, input, output_txt, std::cout);
	request_handler.LoadDataIntoTC();
	request_handler.ProcessRequests();
	request_handler.PrintResult();
	//request_handler.RenderMap();
	//system("pause");
	return 0;
}