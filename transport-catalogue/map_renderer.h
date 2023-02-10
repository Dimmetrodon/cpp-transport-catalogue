#pragma once

#include "svg.h"
#include "geo.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>


namespace map_renderer
{
	struct RenderSettings
	{
		double							width;
		double							height;
		double							padding;
		double							line_width;
		double							stop_radius;
		int								bus_label_font_size;
		svg::Point						bus_label_offset;
		int								stop_label_font_size;
		svg::Point						stop_label_offset;
		svg::Color						underlayer_color;
		double							underlayer_width;
		std::vector<svg::Color>			color_palette;
	};

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector 
    {
    public:
        SphereProjector() = default;
        // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // ���� ����� ����������� ����� �� ������, ��������� ������
            if (points_begin == points_end) 
            {
                return;
            }

            // ������� ����� � ����������� � ������������ ��������
            const auto [left_it, right_it] = std::minmax_element
            (
                points_begin, points_end,
                [](auto lhs, auto rhs) 
                { return lhs.lng < rhs.lng; }
            );
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // ������� ����� � ����������� � ������������ �������
            const auto [bottom_it, top_it] = std::minmax_element
            (
                points_begin, points_end,
                [](auto lhs, auto rhs) 
                { return lhs.lat < rhs.lat; }
            );
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // ��������� ����������� ��������������� ����� ���������� x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) 
            {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // ��������� ����������� ��������������� ����� ���������� y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) 
            {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) 
            {
                // ������������ ��������������� �� ������ � ������ ���������,
                // ���� ����������� �� ���
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) 
            {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) 
            {
                // ����������� ��������������� �� ������ ���������, ���������� ���
                zoom_coeff_ = *height_zoom;
            }
        }

        // ���������� ������ � ������� � ���������� ������ SVG-�����������
        svg::Point operator()(transport_catalogue::coordinates::Coordinates coords) const 
        {
            return 
            {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class MapRender
    {
    public:
                                    MapRender(RenderSettings settings, std::map<std::string, const transport_catalogue::Bus*> bus_name_to_bus);
        void                        RenderRoutes(svg::Document& document) const;
        void                        RenderBusText(svg::Document& document) const;
        void                        RenderStopSymbols(svg::Document& document) const;
        void                        RenderStopNames(svg::Document& document) const;
        svg::Document               RenderMap() const;
        void                        SetProjectorSettings(std::vector<transport_catalogue::coordinates::Coordinates> all_coordinates);

    private:
        RenderSettings                                              settings_;
        std::map<std::string, const transport_catalogue::Bus*>      bus_name_to_bus_;
        SphereProjector                                             sphere_projector_;

    };
}