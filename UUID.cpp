#include "UUID.h"

/*boost::uuids::uuid UUID::getUUID() {
	return boost::uuids::random_generator()();
}*/
std::string UUIDcr::sgetUUID()
{
    // Use the random generator to generate a UUID
    boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}
