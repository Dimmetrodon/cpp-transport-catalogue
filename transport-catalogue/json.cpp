#include "json.h"

using namespace std;

namespace json
{
    Node LoadArray(istream& input)
    {
        Array result;
        bool met_the_end = false;

        for (char c; input >> c;)
        {
            if (c == ']')
            {
                met_the_end = true;
                break;
            }
            if (c != ',')
            {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }

        if (!met_the_end)
        {
            throw ParsingError("Array parsing error"s);
        }

        return Node(move(result));
    }

    Node LoadString(istream& input)
    {
        using namespace literals;

        auto it = istreambuf_iterator<char>(input);
        auto end = istreambuf_iterator<char>();
        string s;
        while (true)
        {
            if (it == end)
            {
                throw ParsingError("String parsing error"s);
            }
            const char ch = *it;
            if (ch == '"')
            {
                ++it;
                break;
            }
            else if (ch == '\\')
            {
                ++it;
                if (it == end)
                {
                    throw ParsingError("String parsing error"s);
                }
                const char escaped_char = *(it);
                switch (escaped_char)
                {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                throw ParsingError("Unexpected end of line"s);
            }
            else
            {
                s.push_back(ch);
            }
            ++it;
        }

        return Node(s);
    }

    Node LoadDict(istream& input)
    {
        Dict result;
        bool met_the_end = false;

        for (char c; input >> c;)
        {
            if (c == '}')
            {
                met_the_end = true;
                break;
            }
            if (c == ',')
            {
                input >> c;
            }

            string key = LoadString(input).AsString();
            input >> c;
            result.insert({ move(key), LoadNode(input) });
        }
        if (!met_the_end)
        {
            throw ParsingError("");
        }
        return Node(move(result));
    }

    Node LoadBool(istream& input)
    {
        string s;
        for (char c; input >> c;)
        {
            if (c == EOF || c == ',' || c == ']' || c == '}')
            {
                input.putback(c);
                break;
            }
            s += c;
        }

        if (s == "true"s)
        {
            return Node(true);
        }
        else if (s == "false"s)
        {
            return Node(false);
        }
        else
        {
            throw ParsingError("");
        }
    }

    Node LoadNull(istream& input)
    {
        string s;
        for (char c; input >> c;)
        {
            if (c == EOF || c == ',' || c == ']' || c == '}')
            {
                input.putback(c);
                break;
            }
            s += c;
        }

        if (s == "null"s)
        {
            return Node(nullptr);
        }
        else
        {
            throw ParsingError("Couldn't parse null value");
        }
    }

    using Number = variant<int, double>;
    Node LoadNumber(istream& input)
    {
        using namespace literals;

        string parsed_num;

        auto read_char = [&parsed_num, &input]
        {
            parsed_num += static_cast<char>(input.get());
            if (!input)
            {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        auto read_digits = [&input, read_char]
        {
            if (!isdigit(input.peek()))
            {
                throw ParsingError("A digit is expected"s);
            }
            while (isdigit(input.peek()))
            {
                read_char();
            }
        };

        if (input.peek() == '-')
        {
            read_char();
        }
        if (input.peek() == '0')
        {
            read_char();
        }
        else
        {
            read_digits();
        }

        bool is_int = true;
        if (input.peek() == '.')
        {
            read_char();
            read_digits();
            is_int = false;
        }

        if (int ch = input.peek(); ch == 'e' || ch == 'E')
        {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-')
            {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try
        {
            if (is_int)
            {
                try
                {
                    return Node(stoi(parsed_num));
                }
                catch (...)
                {
                }
            }
            //parsed_num[parsed_num.find('.')] = ','; //setlocale dependent
            return Node(stod(parsed_num));
        }
        catch (...)
        {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }

    Node LoadNode(istream& input)
    {
        char c;
        input >> c;

        switch (c)
        {
        case EOF:
            throw ParsingError("End of file");
            break;
        case ('['):
            return LoadArray(input);
            break;
        case ('{'):
            return LoadDict(input);
            break;
        case ('"'):
            return LoadString(input);
            break;
        case ('t'):
            input.putback(c);
            return LoadBool(input);
            break;
        case ('f'):
            input.putback(c);
            return LoadBool(input);
            break;
        case ('n'):
            input.putback(c);
            return LoadNull(input);
            break;
        default:
            input.putback(c);
            return LoadNumber(input);
        }
    }


    bool Node::IsInt() const
    {
        return holds_alternative<int>(*this);
    }
    bool Node::IsDouble() const
    {
        return holds_alternative<int>(*this) || holds_alternative<double>(*this);
    }
    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(*this);
    }
    bool Node::IsBool() const
    {
        return holds_alternative<bool>(*this);
    }
    bool Node::IsString() const
    {
        return holds_alternative<string>(*this);
    }
    bool Node::IsNull() const
    {
        return holds_alternative<nullptr_t>(*this);
    }
    bool Node::IsArray() const
    {
        return holds_alternative<Array>(*this);
    }
    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(*this);
    }


    int Node::AsInt() const
    {
        if (!(IsInt()))
        {
            throw logic_error("Check for int failed");
        }
        return get<int>(*this);
    }
    bool Node::AsBool() const
    {
        if (!(IsBool()))
        {
            throw logic_error("Check for bool failed");
        }
        return get<bool>(*this);
    }
    double Node::AsDouble() const
    {
        if (IsInt())
        {
            return get<int>(*this) * 1.0;
        }
        else if (IsPureDouble())
        {
            return get<double>(*this);
        }
        throw logic_error("Check for pure double failed");
    }
    const string& Node::AsString() const
    {
        if (!(IsString()))
        {
            throw logic_error("Check for string failed");
        }
        return get<string>(*this);
    }
    const Array& Node::AsArray() const
    {
        if (!(IsArray()))
        {
            throw logic_error("Check for array failed");
        }
        return get<Array>(this->GetValue());
    }
    const Dict& Node::AsMap() const
    {
        if (!(IsMap()))
        {
            throw logic_error("Check for map failed");
        }
        return get<Dict>(*this);
    }

    string& Node::AsString()
    {
        if (!(IsString()))
        {
            throw logic_error("Check for string failed");
        }
        return get<string>(*this);
    }
    Array& Node::AsArray()
    {
        if (!(IsArray()))
        {
            throw logic_error("Check for array failed");
        }
        return get<Array>(this->GetValueNonConst());
    }
    Dict& Node::AsMap()
    {
        if (!(IsMap()))
        {
            throw logic_error("Check for map failed");
        }
        return get<Dict>(*this);
    }

    Document::Document(Node root)
        :root_(root)
    {
    }

    const Node& Document::GetRoot() const
    {
        return root_;
    }

    Document Load(istream& input)
    {
        return Document(LoadNode(input));
    }

    void PrintNode(const Node& value, const PrintContext& ctx);

    template <typename Value>
    void PrintValue(const Value& value, const PrintContext& ctx)
    {
        ctx.out << value;
    }

    void PrintString(const std::string& value, std::ostream& out)
    {
        out.put('"');
        for (const char c : value)
        {
            switch (c)
            {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }
        out.put('"');
    }

    template <>
    void PrintValue<std::string>(const std::string& value, const PrintContext& ctx)
    {
        PrintString(value, ctx.out);
    }

    template <>
    void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx)
    {
        ctx.out << "null"sv;
    }

    template <>
    void PrintValue<bool>(const bool& value, const PrintContext& ctx)
    {
        if (value)
        {
            ctx.out << "true"sv;
        }
        else
        {
            ctx.out << "false"sv;
        }
    }

    template <>
    void PrintValue<Array>(const Array& nodes, const PrintContext& ctx)
    {
        std::ostream& out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node& node : nodes)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    template <>
    void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx)
    {
        std::ostream& out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto& [key, node] : nodes)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }

    void PrintNode(const Node& node, const PrintContext& ctx)
    {
        visit(
            [&ctx](const auto& value)
            {
                PrintValue(value, ctx);
            },
            node.GetValue());
    }

    void Print(const Document& doc, std::ostream& out)
    {
        PrintNode(doc.GetRoot(), PrintContext{ out });
    }
}