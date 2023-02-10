#include "map_renderer.h"

namespace map_renderer
{
	bool IsZero(double value)
	{
		return std::abs(value) < EPSILON;
	}

	MapRender::MapRender(RenderSettings settings, std::map<std::string, const transport_catalogue::Bus*> bus_name_to_bus)
		: settings_(settings)
		, bus_name_to_bus_(bus_name_to_bus)
	{
	}

	void MapRender::SetProjectorSettings(std::vector<transport_catalogue::coordinates::Coordinates> all_coordinates)
	{
		sphere_projector_ = SphereProjector(all_coordinates.begin(), all_coordinates.end(), settings_.width, settings_.height, settings_.padding);
	}

	void MapRender::RenderRoutes(svg::Document& document) const
	{
		size_t polyline_num_ = 0;
		for (const auto& [bus_name, bus] : bus_name_to_bus_)
		{
			if (bus->stops.empty())
			{
				continue;
			}
			svg::Polyline polyline;
			polyline.SetFillColor("none");
			polyline.SetStrokeWidth(settings_.line_width);
			polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			if (polyline_num_ == settings_.color_palette.size())
			{
				polyline_num_ = 0;
			}
			polyline.SetStrokeColor(settings_.color_palette[polyline_num_++]);
			for (size_t i = 0; i < bus->stops.size(); ++i)
			{
				const transport_catalogue::Stop* stop = bus->stops[i];
				svg::Point p = sphere_projector_(stop->coordinates);
				polyline.AddPoint(p);
			}
			if (!bus->is_looped)
			{
				for (int i = bus->stops.size() - 2; i >= 0; i--)
				{
					polyline.AddPoint(sphere_projector_(bus->stops[i]->coordinates));
				}
			}
			document.Add(polyline);
		}

	}

	void MapRender::RenderBusText(svg::Document& document) const
	{
		size_t polyline_num_ = 0;
		for (const auto& [bus_name, bus] : bus_name_to_bus_)
		{
			if (bus->stops.empty())
			{
				continue;
			}
			svg::Text base;
			base.SetPosition(sphere_projector_(bus->stops[0]->coordinates));
			base.SetOffset(settings_.bus_label_offset);
			base.SetFontSize(settings_.bus_label_font_size);
			base.SetFontFamily("Verdana");
			base.SetFontWeight("bold");
			base.SetData(bus_name);

			svg::Text stop_text = base, stop_base = base;
			if (polyline_num_ == settings_.color_palette.size())
			{
				polyline_num_ = 0;
			}
			stop_text.SetFillColor(settings_.color_palette[polyline_num_++]);

			stop_base.SetFillColor(settings_.underlayer_color);
			stop_base.SetStrokeColor(settings_.underlayer_color);
			stop_base.SetStrokeWidth(settings_.underlayer_width);
			stop_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			stop_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			document.Add(stop_base);
			document.Add(stop_text);

			if (!(bus->is_looped) && bus->stops[0] != bus->stops[(bus->stops).size() - 1])
			{
				document.Add(stop_base.SetPosition(sphere_projector_(bus->stops[bus->stops.size() - 1]->coordinates)));
				document.Add(stop_text.SetPosition(sphere_projector_(bus->stops[bus->stops.size() - 1]->coordinates)));
			}

		}
	}

	void MapRender::RenderStopSymbols(svg::Document& document) const
	{
		std::map<std::string, const transport_catalogue::Stop*> unique_stops;
		for (const auto& [bus_name, bus] : bus_name_to_bus_)
		{
			for (const auto& stop : bus->stops)
			{
				unique_stops.insert({ stop->name, stop });
			}
		}
		for (const auto& [stop_name, stop]:unique_stops)
		{
			svg::Circle circle;
			circle.SetCenter(sphere_projector_(stop->coordinates));
			circle.SetRadius(settings_.stop_radius);
			circle.SetFillColor("white");
			document.Add(circle);
		}
	}

	void MapRender::RenderStopNames(svg::Document& document) const
	{
		std::map<std::string, const transport_catalogue::Stop*> unique_stops;
		for (const auto& [bus_name, bus] : bus_name_to_bus_)
		{
			for (const auto& stop : bus->stops)
			{
				unique_stops.insert({ stop->name, stop });
			}
		}
		for (const auto& [stop_name, stop] : unique_stops)
		{
			svg::Text base;
			base.SetPosition(sphere_projector_(stop->coordinates));
			base.SetOffset(settings_.stop_label_offset);
			base.SetFontSize(settings_.stop_label_font_size);
			base.SetFontFamily("Verdana");
			base.SetData(stop_name);

			svg::Text bus_base = base, bus_text = base;

			bus_base.SetFillColor(settings_.underlayer_color);
			bus_base.SetStrokeColor(settings_.underlayer_color);
			bus_base.SetStrokeWidth(settings_.underlayer_width);
			bus_base.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			bus_base.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			document.Add(bus_base);
			
			bus_text.SetFillColor("black");
			document.Add(bus_text);
		}
	}

	svg::Document MapRender::RenderMap() const
	{
		svg::Document result;
		RenderRoutes(result);
		RenderBusText(result);
		RenderStopSymbols(result);
		RenderStopNames(result);
		return result;
	}
}