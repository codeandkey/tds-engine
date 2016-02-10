#pragma once

struct tds_vertex {
	float x, y;

	union {
		float z;
		float a;
	};

	union {
		float tx;
		float dx;
	};

	union {
		float ty;
		float dy;
	};
};
