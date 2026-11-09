
#ifndef PBL2_ID_GENERATOR_H
#define PBL2_ID_GENERATOR_H

#include <string>

class IdGenerator {
public:
    static void generateNextId(
        const std::string& entity,
        const std::string& sequencesPath,
        std::string& id,
        std::string& updatedSequences
    );
};

#endif
