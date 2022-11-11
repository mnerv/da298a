/**
 * @file   common.hpp
 * @author Pratchaya Khansomboon (me@mononerv.dev)
 * @brief  Common includes
 * @date   2022-11-08
 *
 * @copyright Copyright (c) 2022
 */
#ifndef SHELTER_COMMON_HPP
#define SHELTER_COMMON_HPP

#include "utility.hpp"

namespace shelter {
// Forward declaration
using window_ref_t           = ref<class window>;
using graphics_context_ref_t = ref<class graphics_context>;
using shader_ref_t           = ref<class shader>;
using vertex_buffer_ref_t    = ref<class vertex_buffer>;
using index_buffer_ref_t     = ref<class index_buffer>;
using renderer_ref_t         = ref<class renderer>;
using camera_ref_t           = ref<class camera>;

using shader_local_t           = local<class shader>;
using vertex_buffer_local_t    = local<class vertex_buffer>;
using index_buffer_local_t     = local<class index_buffer>;

} // namespace shelter

#endif  // SHELTER_COMMON_HPP
