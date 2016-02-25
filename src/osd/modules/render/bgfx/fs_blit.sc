$input v_color0, v_texcoord0

// license:BSD-3-Clause
// copyright-holders:Dario Manesku

#include "../../../../../3rdparty/bgfx/examples/common/common.sh"

SAMPLER2D(s_tex, 0);

void main()
{
	gl_FragColor = texture2D(s_tex,  v_texcoord0) * v_color0;
}
