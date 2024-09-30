#include "UID.h"

#include <cassert>

std::mt19937 Generator()
{
    std::random_device rd;
    auto seed_data = std::array<int, std::mt19937::state_size> {};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 randomGenerator(seq);

    return randomGenerator;
}
std::mt19937 UID::randomGenerator = Generator();

UID::UID()
{
    uuids::uuid_random_generator generator{randomGenerator};
    this->id = generator();
    this->str = uuids::to_string(id);
}

UID::UID(std::string str)
{
    auto stringID = uuids::uuid::from_string(str);
    assert(stringID.has_value());

    this->id = stringID.value();
    this->str = str;
}

UID UID::Empty()
{
    UID uid = UID();
    uid.id = uuids::uuid();
    uid.str = uuids::to_string(uid.id);

    return uid;
}