/**StreamFormat format.hpp
 * 
 * MIT License
 * 
 * Copyright (c) 2018 Berrysoft
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */
#ifndef SF_FORMAT_HPP
#define SF_FORMAT_HPP

#include <sf/utility.hpp>

#ifdef SF_CXX11

#include <functional>
#include <iostream>
#include <map>
#include <sf/string_view.hpp>
#include <vector>

namespace sf
{
    namespace internal
    {
        //For convinence.
        enum io_state : bool
        {
            input,
            output
        };

        //Define types.
        template <io_state IOState, typename Char, typename Traits>
        struct stream;
        template <typename Char, typename Traits>
        struct stream<input, Char, Traits>
        {
            typedef std::basic_istream<Char, Traits> type;
        };
        template <typename Char, typename Traits>
        struct stream<output, Char, Traits>
        {
            typedef std::basic_ostream<Char, Traits> type;
        };

        template <io_state IOState, typename Char, typename Traits>
        using stream_t = typename stream<IOState, Char, Traits>::type;

        template <typename Stream>
        struct arg
        {
            typedef std::function<Stream&(Stream&)> type;
            typedef std::vector<type> list_type;
        };

        template <typename Stream>
        using arg_t = typename arg<Stream>::type;
        template <typename Stream>
        using arg_list_t = typename arg<Stream>::list_type;

        template <io_state IOState, typename T, typename Stream>
        SF_CONSTEXPR typename std::enable_if<IOState == input, Stream>::type& operate_arg_io(Stream& stream, T&& arg)
        {
            return stream >> arg;
        }
        template <io_state IOState, typename T, typename Stream>
        SF_CONSTEXPR typename std::enable_if<IOState == output, Stream>::type& operate_arg_io(Stream& stream, T&& arg)
        {
            return stream << arg;
        }

        //A packed arg.
        template <io_state IOState, typename T, typename Char, typename Traits>
        class arg_io
        {
        public:
            typedef stream_t<IOState, Char, Traits> stream_type;

        private:
            T arg;

        public:
            arg_io(T&& arg) : arg(std::forward<T>(arg)) {}
            SF_CONSTEXPR stream_type& operator()(stream_type& stream)
            {
                //G++ will error with static_cast and -std=c++11.
                return operate_arg_io<IOState, T, stream_type>(stream, (T &&) arg);
            }
        };

        SF_CHAR_TEMPLATE(zero, '0')
        SF_CHAR_TEMPLATE(nine, '9')

#ifndef SF_USE_NO_EXCEPT
        template <typename Char>
        SF_CONSTEXPR bool isdigit(Char c)
        {
            return c >= zero<Char>() && c <= nine<Char>();
        }
#endif // !SF_USE_NO_EXCEPT
        template <typename Int, typename Char, typename Traits>
        SF_CONSTEXPR Int stou(const basic_string_view<Char, Traits>& str)
        {
            Int result(0);
            for (const Char& c : str)
            {
#ifndef SF_USE_NO_EXCEPT
                if (!isdigit(c))
                {
                    throw std::invalid_argument("Not digit.");
                }
#endif // !SF_USE_NO_EXCEPT
                result *= 10;
                result += c - zero<Char>();
            }
            return result;
        }

        SF_CHAR_TEMPLATE(space, ' ')
        SF_CHAR_TEMPLATE(tab, '\t')
        SF_CHAR_TEMPLATE(vtab, '\v')
        SF_CHAR_TEMPLATE(cr, '\r')
        SF_CHAR_TEMPLATE(lf, '\n')

        SF_CHAR_TEMPLATE(left_brace, '{')
        SF_CHAR_TEMPLATE(right_brace, '}')

        //Input/Output string slice.
        template <io_state, typename Char, typename Traits>
        class string_view_io;
        template <typename Char, typename Traits>
        class string_view_io<input, Char, Traits>
        {
        public:
            typedef stream_t<input, Char, Traits> stream_type;
            typedef basic_string_view<Char, Traits> string_view_type;

        private:
            string_view_type arg;

        public:
            string_view_io(string_view_type&& arg) : arg(std::move(arg)) {}
            stream_type& operator()(stream_type& is)
            {
                if (is)
                {
                    for (const Char& c : arg)
                    {
                        if (Traits::eq(c, space<Char>()))
                        {
                            while (true)
                            {
                                auto t = is.peek();
                                if (t != Traits::eof() &&
                                    (Traits::eq(t, space<Char>()) || Traits::eq(t, tab<Char>()) || Traits::eq(t, vtab<Char>()) || Traits::eq(t, cr<Char>()) || Traits::eq(t, lf<Char>())))
                                {
                                    is.get();
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                        else
                        {
                            while (true)
                            {
                                auto t = is.get();
                                if (t == Traits::eof() || Traits::eq(t, c))
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
                return is;
            }
        };
        template <typename Char, typename Traits>
        class string_view_io<output, Char, Traits>
        {
        public:
            typedef stream_t<output, Char, Traits> stream_type;
            typedef basic_string_view<Char, Traits> string_view_type;
            typedef typename string_view_type::size_type int_type;

        private:
            string_view_type arg;

        public:
            string_view_io(string_view_type&& arg) : arg(std::move(arg)) {}
            SF_CONSTEXPR stream_type& operator()(stream_type& os)
            {
                return os << arg;
            }
        };

        SF_CHAR_TEMPLATE(colon, ':')
        SF_CHAR_TEMPLATE(comma, ',')

        SF_CHAR_TEMPLATE(cbla, 'b') //boolalpha
        SF_CHAR_TEMPLATE(cdec, 'd') //dec
        SF_CHAR_TEMPLATE(csci, 'e') //scientific
        SF_CHAR_TEMPLATE(cfix, 'f') //fixed
        SF_CHAR_TEMPLATE(cgen, 'g') //general
        SF_CHAR_TEMPLATE(citn, 'i') //internal
        SF_CHAR_TEMPLATE(clft, 'l') //left
        SF_CHAR_TEMPLATE(coct, 'o') //oct
        SF_CHAR_TEMPLATE(crit, 'r') //right
        SF_CHAR_TEMPLATE(cupc, 'u') //uppercase
        SF_CHAR_TEMPLATE(chex, 'x') //hex

        template <io_state IOState, typename Char, typename Traits, std::ios_base::fmtflags Flag, std::ios_base::fmtflags Base, Char Fill>
        std::ios_base::fmtflags stream_setf_w(stream_t<IOState, Char, Traits>& stream, int fmtf)
        {
            std::ios_base::fmtflags oldf = stream.setf(Flag, Base);
            stream.fill(Fill);
            stream.width(fmtf);
            return oldf;
        }
        template <io_state IOState, typename Char, typename Traits, std::ios_base::fmtflags Flag>
        std::ios_base::fmtflags stream_setf_f(stream_t<IOState, Char, Traits>& stream, int)
        {
            return stream.setf(Flag);
        }
        template <io_state IOState, typename Char, typename Traits>
        std::ios_base::fmtflags stream_setf(stream_t<IOState, Char, Traits>&, int)
        {
            return static_cast<std::ios_base::fmtflags>(0);
        }
        template <io_state IOState, typename Char, typename Traits, std::ios_base::fmtflags Flag, std::ios_base::fmtflags Base>
        std::ios_base::fmtflags stream_setf_p(stream_t<IOState, Char, Traits>& stream, int fmtf)
        {
            std::ios_base::fmtflags oldf = stream.setf(Flag, Base);
            stream.precision(fmtf);
            return oldf;
        }

        template <io_state IOState, typename Char, typename Traits>
        struct format_setf
        {
            static const std::map<Char, std::function<std::ios_base::fmtflags(stream_t<IOState, Char, Traits>&, int)>> methods;
        };
        template <io_state IOState, typename Char, typename Traits>
        const std::map<Char, std::function<std::ios_base::fmtflags(stream_t<IOState, Char, Traits>&, int)>> format_setf<IOState, Char, Traits>::methods =
            {
                { cdec<Char>(), stream_setf_w<IOState, Char, Traits, std::ios_base::dec, std::ios_base::basefield, '0'> },
                { coct<Char>(), stream_setf_w<IOState, Char, Traits, std::ios_base::oct, std::ios_base::basefield, '0'> },
                { chex<Char>(), stream_setf_w<IOState, Char, Traits, std::ios_base::hex, std::ios_base::basefield, '0'> },
                { csci<Char>(), stream_setf_p<IOState, Char, Traits, std::ios_base::scientific, std::ios_base::floatfield> },
                { cfix<Char>(), stream_setf_p<IOState, Char, Traits, std::ios_base::fixed, std::ios_base::floatfield> },
                { clft<Char>(), stream_setf_w<IOState, Char, Traits, std::ios_base::left, std::ios_base::adjustfield, ' '> },
                { crit<Char>(), stream_setf_w<IOState, Char, Traits, std::ios_base::right, std::ios_base::adjustfield, ' '> },
                { citn<Char>(), stream_setf_w<IOState, Char, Traits, std::ios_base::internal, std::ios_base::adjustfield, ' '> },
                { cbla<Char>(), stream_setf_f<IOState, Char, Traits, std::ios_base::boolalpha> },
                { cupc<Char>(), stream_setf_f<IOState, Char, Traits, std::ios_base::uppercase> },
                { cgen<Char>(), stream_setf<IOState, Char, Traits> }
            };

        template <io_state IOState, typename Char, typename Traits>
        class format_arg_io
        {
        public:
            typedef stream_t<IOState, Char, Traits> stream_type;
            typedef arg_t<stream_type> arg_type;
            typedef basic_string_view<Char, Traits> string_view_type;
            typedef typename string_view_type::size_type int_type;
            typedef format_setf<IOState, Char, Traits> fsetf_type;

        private:
            arg_type& ori;
            const string_view_type& fmts;

        public:
            format_arg_io(arg_type& ori, const string_view_type& fmts) : ori(ori), fmts(fmts) {}
            stream_type& operator()(stream_type& stream)
            {
                std::ios_base::fmtflags oldf = static_cast<std::ios_base::fmtflags>(0);
                int_type length = fmts.length();
                int_type offset = 0, index = 0;
                for (; index <= length; index++)
                {
                    if (index == length || Traits::eq(fmts[index], comma<Char>()))
                    {
                        Char fmtc = fmts[offset];
                        int fmtf = 0;
                        int_type len = index - offset - 1;
                        if (len > 0)
                        {
                            fmtf = stou<int, Char, Traits>(fmts.substr(offset + 1, len));
                        }
                        auto it = fsetf_type::methods.find(fmtc);
                        if (it != fsetf_type::methods.end())
                        {
                            oldf |= (it->second)(stream, fmtf);
                        }
#ifndef SF_USE_NO_EXCEPT
                        else
                        {
                            throw std::logic_error("Invalid format character.");
                        }
#endif // !SF_USE_NO_EXCEPT
                        offset = index + 1;
                    }
                }
                ori(stream);
                if (oldf)
                    stream.setf(oldf);
                return stream;
            }
        };

        //A pack of format string and arguments.
        template <io_state IOState, typename Char, typename Traits>
        class format_string_view
        {
        public:
            typedef stream_t<IOState, Char, Traits> stream_type;
            typedef arg_t<stream_type> arg_type;
            typedef arg_list_t<stream_type> arg_list_type;
            typedef basic_string_view<Char, Traits> string_view_type;
            typedef typename string_view_type::size_type int_type;
            typedef string_view_io<IOState, Char, Traits> string_view_io_type;

        private:
            const string_view_type& fmt;
            arg_list_type args;

        public:
            SF_CONSTEXPR format_string_view(const string_view_type& fmt, arg_list_type&& args)
                : fmt(fmt), args(std::move(args))
            {
            }
            stream_type& operator()(stream_type& stream)
            {
                int_type offset = 0, index = 0;
                const int_type length = fmt.length();
                bool in_number = false;
                std::size_t arg_index = 0;
                while (offset < length)
                {
                    if (!in_number)
                    {
                        int_type off = offset;
                        for (index = offset; index < length; index++)
                        {
                            if (Traits::eq(fmt[index], left_brace<Char>()))
                            {
                                index++;
                                if (!(index < length && Traits::eq(fmt[index], left_brace<Char>())))
                                {
                                    in_number = true;
                                    index--;
                                }
                            }
                            else if (Traits::eq(fmt[index], right_brace<Char>()))
                            {
                                index++;
                                if (!(index < length && Traits::eq(fmt[index], right_brace<Char>())))
                                {
#ifndef SF_USE_NO_EXCEPT
                                    throw std::logic_error("No \"{\" matches \"}\".");
#else
                                    index--;
                                    continue;
#endif // !SF_USE_NO_EXCEPT
                                }
                            }
                            else
                            {
                                continue;
                            }
                            break;
                        }
                        int_type len = index - offset;
                        offset = index + 1;
                        if (len <= 0)
                            continue;
                        string_view_io_type(fmt.substr(off, len))(stream);
                    }
                    else
                    {
                        for (index = offset; index < length; index++)
                        {
                            if (Traits::eq(fmt[index], right_brace<Char>()))
                                break;
                        }
                        if (index == length)
                        {
#ifndef SF_USE_NO_EXCEPT
                            throw std::logic_error("No \"}\" was found after \"{\".");
#else
                            string_view_io_type(fmt.substr(offset - 1))(stream);
                            offset = index + 1;
#endif // !SF_USE_NO_EXCEPT
                        }
                        else
                        {
                            in_number = false;
                            int_type ci;
                            for (ci = offset; ci < index; ci++)
                            {
                                if (Traits::eq(fmt[ci], colon<Char>()))
                                {
                                    break;
                                }
                            }
                            if (ci > offset)
                            {
                                arg_index = stou<std::size_t, Char, Traits>(fmt.substr(offset, ci - offset));
                            }
                            if (arg_index >= args.size())
                            {
                                string_view_io_type(fmt.substr(offset - 1, index - offset + 2))(stream);
                            }
                            else if (index == ci)
                            {
                                args[arg_index](stream);
                            }
                            else
                            {
                                format_arg_io<IOState, Char, Traits>(args[arg_index], fmt.substr(ci + 1, index - ci - 1))(stream);
                            }
                            offset = index + 1;
                            arg_index++;
                        }
                    }
                }
                return stream;
            }
        };

        template <io_state IOState, typename Char, typename Traits, typename... Args>
        SF_CONSTEXPR stream_t<IOState, Char, Traits>& format(stream_t<IOState, Char, Traits>& stream, const basic_string_view<Char, Traits>& fmt, Args&&... args)
        {
            return format_string_view<IOState, Char, Traits>(fmt, arg_list_t<stream_t<IOState, Char, Traits>>{ arg_io<IOState, Args, Char, Traits>(std::forward<Args>(args))... })(stream);
        }

        template <io_state IOState, typename Char, typename Traits, typename T>
        SF_CONSTEXPR stream_t<IOState, Char, Traits>& format(stream_t<IOState, Char, Traits>& stream, T&& arg)
        {
            return arg_io<IOState, T, Char, Traits>(std::forward<T>(arg))(stream);
        }
    } // namespace internal

    //template IO
    template <typename Char, typename Traits = std::char_traits<Char>, typename... Args>
    SF_CONSTEXPR std::basic_istream<Char, Traits>& scan(std::basic_istream<Char, Traits>& stream, basic_string_view<Char, Traits> fmt, Args&&... args)
    {
        return internal::format<internal::input>(stream, fmt, std::forward<Args>(args)...);
    }
    template <typename Char, typename Traits = std::char_traits<Char>, typename... Args>
    SF_CONSTEXPR std::basic_ostream<Char, Traits>& print(std::basic_ostream<Char, Traits>& stream, basic_string_view<Char, Traits> fmt, Args&&... args)
    {
        return internal::format<internal::output>(stream, fmt, std::forward<Args>(args)...);
    }
    template <typename Char, typename Traits = std::char_traits<Char>, typename... Args>
    SF_CONSTEXPR std::basic_ostream<Char, Traits>& println(std::basic_ostream<Char, Traits>& stream, basic_string_view<Char, Traits> fmt, Args&&... args)
    {
        return internal::format<internal::output>(stream, fmt, std::forward<Args>(args)...) << std::endl;
    }

    //char IO
    template <typename... Args>
    SF_CONSTEXPR std::istream& scan(std::istream& stream, string_view fmt, Args&&... args)
    {
        return internal::format<internal::input>(stream, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::istream& scan(string_view fmt, Args&&... args)
    {
        return scan(std::cin, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::ostream& print(std::ostream& stream, string_view fmt, Args&&... args)
    {
        return internal::format<internal::output>(stream, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::ostream& print(string_view fmt, Args&&... args)
    {
        return print(std::cout, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::ostream& println(std::ostream& stream, string_view fmt, Args&&... args)
    {
        return internal::format<internal::output>(stream, fmt, std::forward<Args>(args)...) << std::endl;
    }
    template <typename... Args>
    SF_CONSTEXPR std::ostream& println(string_view fmt, Args&&... args)
    {
        return println(std::cout, fmt, std::forward<Args>(args)...);
    }

    //wchar_t IO
    template <typename... Args>
    SF_CONSTEXPR std::wistream& scan(std::wistream& stream, wstring_view fmt, Args&&... args)
    {
        return internal::format<internal::input>(stream, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::wistream& scan(wstring_view fmt, Args&&... args)
    {
        return scan(std::wcin, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::wostream& print(std::wostream& stream, wstring_view fmt, Args&&... args)
    {
        return internal::format<internal::output>(stream, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::wostream& print(wstring_view fmt, Args&&... args)
    {
        return print(std::wcout, fmt, std::forward<Args>(args)...);
    }
    template <typename... Args>
    SF_CONSTEXPR std::wostream& println(std::wostream& stream, wstring_view fmt, Args&&... args)
    {
        return internal::format<internal::output>(stream, fmt, std::forward<Args>(args)...) << std::endl;
    }
    template <typename... Args>
    SF_CONSTEXPR std::wostream& println(wstring_view fmt, Args&&... args)
    {
        return println(std::wcout, fmt, std::forward<Args>(args)...);
    }

    //Simple IO functions for convinence.

    //template IO
    template <typename Char, typename Traits = std::char_traits<Char>, typename T>
    SF_CONSTEXPR std::basic_istream<Char, Traits>& scan(std::basic_istream<Char, Traits>& stream, T&& arg)
    {
        return internal::format<internal::input, Char, Traits, T>(stream, std::forward<T>(arg));
    }
    template <typename Char, typename Traits = std::char_traits<Char>, typename T>
    SF_CONSTEXPR std::basic_ostream<Char, Traits>& print(std::basic_ostream<Char, Traits>& stream, T&& arg)
    {
        return internal::format<internal::output, Char, Traits, T>(stream, std::forward<T>(arg));
    }
    template <typename Char, typename Traits = std::char_traits<Char>, typename T>
    SF_CONSTEXPR std::basic_ostream<Char, Traits>& println(std::basic_ostream<Char, Traits>& stream, T&& arg)
    {
        return print<Char, Traits, T>(stream, std::forward<T>(arg)) << std::endl;
    }
    template <typename Char, typename Traits = std::char_traits<Char>>
    SF_CONSTEXPR std::basic_ostream<Char, Traits>& println(std::basic_ostream<Char, Traits>& stream)
    {
        return print<Char, Traits>(stream, std::endl<Char, Traits>);
    }
#ifndef SF_FORCE_WIDE_IO
    //char IO
    template <typename T>
    SF_CONSTEXPR std::istream& scan(T&& arg)
    {
        return scan(std::cin, std::forward<T>(arg));
    }
    template <typename T>
    SF_CONSTEXPR std::ostream& print(T&& arg)
    {
        return print(std::cout, std::forward<T>(arg));
    }
    template <typename T>
    SF_CONSTEXPR std::ostream& println(T&& arg)
    {
        return println(std::cout, std::forward<T>(arg));
    }
    SF_CONSTEXPR std::ostream& println()
    {
        return println<char>(std::cout);
    }
#else
    //wchar_t IO
    template <typename T>
    SF_CONSTEXPR std::wistream& scan(T&& arg)
    {
        return scan(std::wcin, std::forward<T>(arg));
    }
    template <typename T>
    SF_CONSTEXPR std::wostream& print(T&& arg)
    {
        return print(std::wcout, std::forward<T>(arg));
    }
    template <typename T>
    SF_CONSTEXPR std::wostream& println(T&& arg)
    {
        return println(std::wcout, std::forward<T>(arg));
    }
    SF_CONSTEXPR std::wostream& println()
    {
        return println<wchar_t>(std::wcout);
    }
#endif // !SF_FORCE_WIDE_IO
} // namespace sf
#endif // SF_CXX11

#endif // !SF_FORMAT_HPP
