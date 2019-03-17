
#ifndef DZ_GAIKA_OBJ_IO_HPP
#define DZ_GAIKA_OBJ_IO_HPP

#include <vector>
#include <string>

#include "matrices.hpp"
#include "model.hpp"

namespace isg
{
namespace obj_io
{
    // TODO: handle multiple models per file?

    model read_obj_model(std::string path);
};
}

#endif //DZ_GAIKA_OBJ_IO_HPP
