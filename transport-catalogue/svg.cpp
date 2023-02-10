#include "svg.h"

namespace svg
{

    Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue)
        : red(red)
        , green(green)
        , blue(blue)
    {
    }

    Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
        : Rgb(red, green, blue)
        , opacity(opacity)
    {
    }

    void ColorPrinter::operator()(std::monostate) const
    {
        out << "none"sv;
    }
    void ColorPrinter::operator()(std::string color_string) const
    {
        out << color_string;
    }
    void ColorPrinter::operator()(Rgb rgb) const
    {
        out << "rgb("sv
            << (int)rgb.red << ","sv
            << (int)rgb.green << ","sv
            << (int)rgb.blue
            << ")"sv;
    }
    void ColorPrinter::operator()(Rgba rgba) const
    {
        out << "rgba("sv
            << (int)rgba.red << ","sv
            << (int)rgba.green << ","sv
            << (int)rgba.blue << ","
            << rgba.opacity
            << ")"sv;
    }

    std::ostream& operator<<(std::ostream& out, const Color& color)
    {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap)
    {
        if (line_cap == StrokeLineCap::BUTT)
        {
            out << "butt"s;
        }
        if (line_cap == StrokeLineCap::ROUND)
        {
            out << "round"s;
        }
        if (line_cap == StrokeLineCap::SQUARE)
        {
            out << "square"s;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join)
    {
        if (line_join == StrokeLineJoin::ARCS)
        {
            out << "arcs"s;
        }
        if (line_join == StrokeLineJoin::BEVEL)
        {
            out << "bevel"s;
        }
        if (line_join == StrokeLineJoin::MITER)
        {
            out << "miter"s;
        }
        if (line_join == StrokeLineJoin::MITER_CLIP)
        {
            out << "miter-clip"s;
        }
        if (line_join == StrokeLineJoin::ROUND)
        {
            out << "round"s;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const
    {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ----------------

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (const auto& point : points_)
        {
            if (!first)
            {
                out << " "sv;
            }
            out << point.x << "," << point.y;
            first = false;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text --------------------

    Text& Text::SetPosition(Point pos)
    {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t font_size)
    {
        font_size_ = font_size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }


    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<text"s;
        RenderAttrs(out);
        out << " x=\"" << position_.x << "\" y=\"" << position_.y << "\" dx=\"" << offset_.x
            << "\" dy=\"" << offset_.y << "\" font-size=\"" << font_size_;

        if (!font_family_.empty())
        {
            out << "\" font-family=\"" << font_family_;
        }
        if (!font_weight_.empty())
        {
            out << "\" font-weight=\"" << font_weight_;
        }

        out << "\">" << data_ << "</text>";
    }

    void Text::CleanUpString(std::string& data) const
    {
        size_t pos = 0;

        while ((pos = data.find('&', pos + 1)) != data.npos)
        {
            data.insert(pos + 1, "&amp;");
            data.erase(pos, 1);
        }

        while ((pos = data.find_first_of(R"("'<>)")) != data.npos)
        {
            switch (data[pos])
            {
            case ('\"'):
                data.insert(pos + 1, "&quot;");
                break;

            case ('\''):
                data.insert(pos + 1, "&apos;");
                break;

            case ('<'):
                data.insert(pos + 1, "&lt;");
                break;

            case ('>'):
                data.insert(pos + 1, "&gt;");
                break;
            }
            data.erase(pos, 1);
        }
    }

    // ---------- Document -----------------

    void Document::AddPtr(std::unique_ptr<Object>&& object_ptr)
    {
        objects_.emplace_back(std::move(object_ptr));
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
        for (const auto& obj : objects_)
        {
            obj->Render({ out, 2, 2 });
        }
        out << "</svg>";
    }

}  // namespace svg