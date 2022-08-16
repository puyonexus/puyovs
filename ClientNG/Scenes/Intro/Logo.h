#pragma once

extern const unsigned char kLogoPng[10287];

// Size of the texture.
constexpr int kLetterTexSize[2] = { 512, 256 };

// Offsets and sizes for each letter.
constexpr int kLetterTexCoords[9][4] = {
	{ 5, 5, 84, 122 },
	{ 94, 5, 77, 89 },
	{ 176, 5, 85, 122 },
	{ 266, 5, 81, 91 },
	{ 352, 5, 92, 122 },
	{ 5, 132, 76, 91 },
	{ 86, 132, 88, 87 },
	{ 179, 132, 77, 89 },
	{ 261, 132, 64, 91 },
};

// Final positions for each letter.
constexpr int kLetterPositions[9][2] = {
	{ 383, 521 },
	{ 474, 557 },
	{ 556, 557 },
	{ 643, 555 },
	{ 783, 522 },
	{ 890, 555 },
	{ 969, 557 },
	{ 1066, 557 },
	{ 1153, 555 },
};
