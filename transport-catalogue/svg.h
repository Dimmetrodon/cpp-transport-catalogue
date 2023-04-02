#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg
{
    using namespace std::literals;
    struct Rgb
    {
        Rgb() = default;
        Rgb(uint8_t red, uint8_t green, uint8_t blue);

        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba : public Rgb
    {
        Rgba() = default;
        Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity);

        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    std::ostream& operator<<(std::ostream& out, const Color& color);

    struct ColorPrinter
    {
        std::ostream& out;

        void operator()(std::monostate) const;
        void operator()(std::string color_string) const;
        void operator()(Rgb rgb) const;
        void operator()(Rgba rgba) const;
    };
    inline const Color NoneColor{ "none" };

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };
    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);
    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);


    template <typename Owner>
    class PathProps
    {
    public:
        Owner& SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width)
        {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap)
        {
            line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_)
            {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (line_cap_)
            {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }
            if (line_join_)
            {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner()
        {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };


    struct Point
    {
        Point() = default;

        Point(double x, double y)
            : x(x)
            , y(y)
        {
        }
        double x = 0;
        double y = 0;
    };


    struct RenderContext
    {
        RenderContext(std::ostream& out)
            : out(out)
        {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent)
        {
        }

        RenderContext Indented() const
        {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };


    class Object
    {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };


    class ObjectContainer
    {
    public:
        template <typename Obj>
        void Add(Obj obj)
        {
            std::unique_ptr<Obj> obj1 = std::make_unique<Obj>(std::move(obj));
            AddPtr(std::move(obj1));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;
    };


    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;

        virtual ~Drawable() = default;
    };


    class Circle final : public Object, public PathProps<Circle>
    {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };


    class Polyline : public Object, public PathProps<Polyline>
    {
    public:

        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };


    class Text : public Object, public PathProps<Text>
    {
    public:

        Text& SetPosition(Point pos);

        Text& SetOffset(Point offset);

        Text& SetFontSize(uint32_t size);

        Text& SetFontFamily(std::string font_family);

        Text& SetFontWeight(std::string font_weight);

        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        void CleanUpString(std::string& data) const;

        Point position_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };


    class Document : public ObjectContainer
    {
    public:

        void AddPtr(std::unique_ptr<Object>&& obj);

        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };
}  // namespace svg