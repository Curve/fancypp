#pragma once
#include <iomanip>
#include <string>
#include <chrono>
#include <iostream>

namespace Fancy
{
    struct Color
    {
        int r, g, b;
        Color() = default;
        Color(int r, int g, int b) : r(r), g(g), b(b) {}

        auto foreground() const noexcept
        {
            char buffer[32];
            snprintf(buffer, 32, "\033[38;2;%i;%i;%im", r, g, b);
            return std::string(buffer);
        }
        auto background() const noexcept
        {
            char buffer[32];
            snprintf(buffer, 32, "\033[48;2;%i;%i;%im", r, g, b);
            return std::string(buffer);
        }
        static constexpr auto bold() noexcept
        {
            return "\033[1m";
        }
        static constexpr auto italic() noexcept
        {
            return "\033[3m";
        }
        static constexpr auto striked() noexcept
        {
            return "\033[9m";
        }
        static constexpr auto underline() noexcept
        {
            return "\033[4m";
        }
        template <bool fast = false> static constexpr auto blinking() noexcept
        {
            if constexpr (fast)
                return "\033[6m";
            else
                return "\033[5m";
        }
        static constexpr auto reset() noexcept
        {
            return "\033[0m";
        }
    };

    class Stream
    {
        struct
        {
            const Color timepoint{80, 80, 80};
            const Color duration{155, 89, 182};

            const Color quotes{243, 156, 18};
            const Color braces{120, 120, 120};
            const Color curlybraces{160, 160, 160};

            const Color important{255, 255, 255};
            const Color arithmetic{142, 68, 173};

            const Color success{46, 204, 113};
            const Color failure{231, 76, 60};
            const Color warning{241, 196, 15};
        } colors;

        static constexpr std::string_view successStr = "success";
        static constexpr std::string_view failureStr = "failure";
        static constexpr std::string_view warningStr = "warning";

        static constexpr bool importantBold = true;
        static constexpr bool importantUnderline = true;
        static constexpr bool importantBackground = false;
        static constexpr std::string_view timeFormat = "%H:%M:%S";

        template <typename T> struct is_default_printable
        {
          private:
            static auto test(...) -> std::uint8_t;
            template <typename O>
            static auto test(O *) -> decltype(std::declval<std::ostream &>() << std::declval<O>(), std::uint16_t{});

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };
        template <typename T> struct is_time_point
        {
          private:
            static auto test(...) -> std::uint8_t;
            template <typename O> static auto test(std::chrono::time_point<O> *) -> std::uint16_t;

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };
        template <typename T> struct is_container
        {
          private:
            static auto test(...) -> std::uint8_t;
            template <typename O>
            static auto test(O *) -> decltype(std::declval<O &>().begin(), std::declval<O &>().end(), std::uint16_t{});

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };
        template <typename T> struct is_pair
        {
          private:
            static auto test(...) -> std::uint8_t;
            template <typename O>
            static auto test(O *) -> decltype(std::declval<O &>().first, std::declval<O &>().second, std::uint16_t{});

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };
        template <typename T> struct is_tuple
        {
          private:
            static auto test(...) -> std::uint8_t;
            template <typename... O> static auto test(std::tuple<O...> *) -> std::uint16_t;

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };
        template <typename T> struct is_duration
        {
          private:
            static auto test(...) -> std::uint8_t;
            template <typename O, typename L> static auto test(std::chrono::duration<O, L> *) -> std::uint16_t;

          public:
            static const bool value = sizeof(test(reinterpret_cast<T *>(0))) == sizeof(std::uint16_t);
        };

        template <typename T, std::enable_if_t<is_time_point<T>::value> * = nullptr>
        static auto toTimeStr(const T &what)
        {
            auto time_t = std::chrono::system_clock::to_time_t(what);
            auto tm = *std::gmtime(&time_t);

            std::stringstream ss;
            ss << std::put_time(&tm, timeFormat.data());

            return ss.str();
        }

      public:
        template <bool isContainerItem = false, typename T = void> const Stream &operator<<(const T &what) const
        {
            using rawType = std::remove_cv_t<
                std::remove_pointer_t<std::remove_const_t<std::remove_reference_t<std::remove_all_extents_t<T>>>>>;

            if constexpr (isContainerItem &&
                          (std::is_same_v<rawType, std::string> || std::is_same_v<rawType, std::string_view> ||
                           std::is_same_v<rawType, char>))
            {
                if constexpr (std::is_same_v<T, char>)
                {
                    std::cout << colors.quotes.foreground() << "'" << what << colors.quotes.foreground() << "'";
                }
                else
                {
                    std::cout << colors.quotes.foreground() << "\"" << what << colors.quotes.foreground() << "\"";
                }
            }
            else if constexpr (std::is_arithmetic_v<T>)
            {
                std::cout << colors.arithmetic.foreground() << what;
            }
            else if constexpr (is_default_printable<T>::value)
            {
                std::cout << what;
            }
            else if constexpr (is_duration<T>::value)
            {
                std::cout << colors.duration.foreground() << what.count();
                if constexpr (std::is_same_v<T, std::chrono::hours>)
                {
                    std::cout << "h";
                }
                else if constexpr (std::is_same_v<T, std::chrono::minutes>)
                {
                    std::cout << "m";
                }
                else if constexpr (std::is_same_v<T, std::chrono::seconds>)
                {
                    std::cout << "s";
                }
                else if constexpr (std::is_same_v<T, std::chrono::milliseconds>)
                {
                    std::cout << "ms";
                }
                else if constexpr (std::is_same_v<T, std::chrono::nanoseconds>)
                {
                    std::cout << "ns";
                }
            }
            else if constexpr (is_time_point<T>::value)
            {
                std::cout << colors.timepoint.foreground() << toTimeStr(what);
            }
            else if constexpr (is_tuple<T>::value)
            {
                static const auto unpackLambda = [this](auto... args) {
                    static const auto unpackArgs = [this](auto arg) {
                        this->operator<<<true>(arg);
                        *this << ", ";
                    };
                    (unpackArgs(args), ...);
                };
                std::cout << colors.curlybraces.foreground() << "{";
                std::apply(unpackLambda, what);
                std::cout << "\b\b" << colors.curlybraces.foreground() << "}";
            }
            else if constexpr (is_pair<T>::value)
            {
                std::cout << colors.curlybraces.foreground() << "{";
                this->operator<<<true>(what.first);
                *this << ", ";
                this->operator<<<true>(what.second);
                std::cout << colors.curlybraces.foreground() << "}";
            }
            else if constexpr (is_container<T>::value)
            {
                std::cout << colors.braces.foreground() << "[";
                for (auto it = what.begin(); it != what.end(); it++)
                {
                    this->operator<<<true>(*it);
                    if (std::distance(it, what.end()) > 1)
                        *this << ", ";
                }
                std::cout << colors.braces.foreground() << "]";
            }
            else
            {
                static_assert(is_default_printable<T>::value, "Type is not supported!");
            }

            if constexpr (!is_default_printable<T>::value || isContainerItem)
            {
                std::cout << Color::reset();
            }

            return *this; // NOLINT
        }
        template <typename T> auto &operator>>(const T &what) const
        {
            if constexpr (importantBold)
            {
                std::cout << Color::bold();
            }
            if constexpr (importantUnderline)
            {
                std::cout << Color::underline();
            }
            if constexpr (importantBackground)
            {
                std::cout << colors.important.background();
            }
            else
            {
                std::cout << colors.important.foreground();
            }

            *this << what;

            std::cout << Color::reset();
            return *this;
        }
        auto &operator<<(std::ostream &(*var)(std::ostream &)) const
        {
            std::cout << Color::reset();
            std::cout << var;
            return *this;
        }
        auto &logTime()
        {
            auto &self = *this;
            self << colors.braces.foreground() << "[" << std::chrono::system_clock::now() << colors.braces.foreground()
                 << "] " << Color::reset();

            return self;
        }
        auto &success()
        {
            auto &self = *this;
            self << colors.braces.foreground() << "[" << colors.success.foreground() << successStr
                 << colors.braces.foreground() << "] " << Color::reset();

            return self;
        }
        auto &warning()
        {
            auto &self = *this;
            self << colors.braces.foreground() << "[" << colors.warning.foreground() << warningStr
                 << colors.braces.foreground() << "] " << Color::reset();

            return self;
        }
        auto &failure()
        {
            auto &self = *this;
            self << colors.braces.foreground() << "[" << colors.failure.foreground() << failureStr
                 << colors.braces.foreground() << "] " << Color::reset();

            return self;
        }
    };
    inline Stream fancy;
} // namespace Fancy