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


	transport_catalogue::TransportCatalogue cat;
	request_handler::RequestHandler request_handler(cat, input, std::cout);
	request_handler.LoadDataIntoTC();
	request_handler.ProcessRequests();
	request_handler.PrintResult();
	request_handler.RenderMap();
	system("pause");

    /*json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartArray()
                    .Value(456)
                    .StartDict().EndDict()
                    .StartDict()
                        .Key(""s)
                        .Value(nullptr)
                    .EndDict()
                    .Value(""s)
                .EndArray()
            .EndDict()
            .Build()
        },
        std::cout
    );
    std::cout << std::endl;

    json::Print(
        json::Document{
            json::Builder{}
            .Value("just a string"s)
            .Build()
        },
        std::cout
    );
    std::cout << std::endl;*/

	return 0;
}