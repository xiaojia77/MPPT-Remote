#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>
#include <vector>

namespace libdongle {

class auth_wrapper
{
    auth_wrapper();

public:
    static std::vector<uint8_t> get_random_auth_data();
    static std::vector<uint8_t> get_encrypted_auth_data(const std::vector<uint8_t> &data);
    static bool set_link_key(const std::vector<uint8_t> &data);
};

} // namespace libdongle

#endif // WRAPPER_H
