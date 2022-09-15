//
// Created by Zero on 06/09/2022.
//

#include "scene_desc.h"
#include "scene_parser.h"


namespace vision {

unique_ptr<SceneDesc> SceneDesc::from_json(const fs::path &path) {
    SceneParser parser(path);
    return parser.parse();
}

}// namespace vision