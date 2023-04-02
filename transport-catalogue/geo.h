#pragma once
#define _USE_MATH_DEFINES

#include <cmath>
//#include <corecrt_math_defines.h>

namespace transport_catalogue
{
    namespace coordinates
    {
        struct Coordinates
        {
            double                  lat;
            double                  lng;

            bool operator==(const Coordinates& other) const
            {
                return lat == other.lat && lng == other.lng;
            }
            bool operator!=(const Coordinates& other) const
            {
                return !(*this == other);
            }
        };


        //inline double               ComputeDistance(Coordinates from, Coordinates to);
        inline double ComputeDistance(Coordinates from, Coordinates to)
        {
            const int EARTH_RADIUS = 6371000;
            using namespace std;
            if (from == to)
            {
                return 0;
            }
            //static const double dr = 3.1415926535 / 180.;
            static const double dr = M_PI / 180.;
            return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
                * EARTH_RADIUS;

        }
    }
}

