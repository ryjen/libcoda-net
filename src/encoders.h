#ifndef RJ_NET_ENCODERS_H
#define RJ_NET_ENCODERS_H

#include <map>
#include <string>
#include <vector>

namespace rj
{
    namespace net
    {
        namespace encoding
        {
            namespace input
            {
                // when encoding we expect two types of inputs:
                // a standard string content
                typedef std::string str;
                // and a map of key-value strings
                typedef std::map<str, str> map;

                // util to assert input type
                template <typename T>
                struct is_type : std::integral_constant<bool, std::is_same<T, input::map>::value ||
                                                                  std::is_same<T, input::str>::value> {
                };
            }

            // a base encoder class, T is the type after encoding (and thus the type being decoded from)
            template <typename T>
            class encoder
            {
               public:
                typedef T encoded_type;

                // encode a input
                template <typename I, typename = std::enable_if<encoding::input::is_type<I>::value>>
                encoded_type encode(const I &value) const
                {
                    encoded_type output;
                    encode(value, output);
                    return output;
                }

                // decode to an input type
                template <typename I, typename = std::enable_if<encoding::input::is_type<I>::value>>
                I decode(const encoded_type &value) const
                {
                    T output;
                    decode(value, output);
                    return output;
                }

               protected:
                virtual void encode(const encoding::input::map &from, encoded_type &to) const = 0;
                virtual void encode(const encoding::input::str &from, encoded_type &to) const = 0;
                virtual void decode(const encoded_type &from, encoding::input::map &to) const = 0;
                virtual void decode(const encoded_type &from, encoding::input::str &to) const = 0;
            };

            // form based url-encoding
            class url : public encoder<std::string>
            {
               public:
                using encoder::encode;

               private:
                void encode(const encoding::input::map &values, encoded_type &to) const;
                void encode(const encoding::input::str &value, encoded_type &to) const;
                void decode(const encoded_type &from, encoding::input::map &to) const;
                void decode(const encoded_type &from, encoding::input::str &to) const;
            };

            class json : public encoder<std::string>
            {
               public:
                using encoder::encode;

               private:
                void encode(const encoding::input::map &values, encoded_type &to) const;
                void encode(const encoding::input::str &value, encoded_type &to) const;
                void decode(const encoded_type &from, encoding::input::map &to) const;
                void decode(const encoded_type &from, encoding::input::str &to) const;
            };


            class xml : public encoder<std::string>
            {
               public:
                using encoder::encode;

               private:
                void encode(const encoding::input::map &values, encoded_type &to) const;
                void encode(const encoding::input::str &value, encoded_type &to) const;
                void decode(const encoded_type &from, encoding::input::map &to) const;
                void decode(const encoded_type &from, encoding::input::str &to) const;
            };
        }
    }
}

#endif
