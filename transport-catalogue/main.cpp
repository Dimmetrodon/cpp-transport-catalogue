#include "transport_catalogue.h"
#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace transport_catalogue;

int main()
 {
    /*{
        std::istringstream input
        {
            "10\n"
            "Stop Tolstopaltsevo: 55.611087, 37.208290, 9900m to Rasskazovka, 100m to Marushkino\n"
            "Stop Marushkino: 55.595884, 37.209755\n"
            "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
            "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
            "Stop Rasskazovka: 55.632761, 37.333324\n"
            "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700\n"
            "Stop Biryusinka: 55.581065, 37.648390\n"
            "Stop Universam: 55.587655, 37.645687\n"
            "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
            "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
            "3\n"
            "Bus 256\n"
            "Bus 750\n"
            "Bus 751\n"
        };

        TransportCatalogue transport_catalogue;
        input_reader::Reader reader(transport_catalogue);
        reader.Read(input);
        stat_reader::StatReader stat_reader(transport_catalogue, input, cout);
        stat_reader.Read();
    }*/

    {
         ifstream myfile;
         ofstream out_file;
         myfile.open("input.txt");
         out_file.open("output.txt");

        TransportCatalogue transport_catalogue;
        input_reader::Reader reader(transport_catalogue);
        reader.Read(myfile);
        stat_reader::StatReader stat_reader(transport_catalogue, myfile, out_file);
        stat_reader.Read();

        myfile.close();
        out_file.close();
    }
}