
#ifndef PBL2_ID_GENERATOR_H
#define PBL2_ID_GENERATOR_H

#include <string>
#include <vector>

class IdGenerator
{
public:
    static std::string generateNextId(const std::string& entity,
                                      const std::vector<std::string>& existingIds);
};

#endif
