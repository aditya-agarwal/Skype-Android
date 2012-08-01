#include "bmcrypto/def.h"


// live primary
const uchar primary_rootkey[]= {0x41,0x01,0x04,0x01,0x82,0x01,0x52,0x4b,0xcd,0x66,0xec,0xff,0x41,0x97,
0x51,0xfa,0xcb,0x75,0xcc,0x6a,0xf9,0xa2,0x22,0x30,0xcf,0x4c,0x96,0xac,0x8e,0x2c,
0x7c,0x39,0x8d,0x2d,0xa8,0xf4,0x82,0x71,0x44,0xfd,0x9f,0x6a,0xbe,0xa9,0x0b,0xad,
0x00,0x5c,0xac,0xef,0x87,0x9e,0xeb,0x8e,0x3f,0xf9,0x7d,0x44,0x2a,0x60,0x15,0x4d,
0xdb,0xf5,0xa9,0x81,0x4d,0x49,0xba,0x0b,0x7e,0x2a,0xb3,0x07,0x98,0x7e,0xde,0xbc,
0x31,0x97,0x2d,0xdf,0x1b,0xbb,0x8f,0x97,0xf8,0x0a,0xd2,0x0a,0x0a,0x0f,0xc9,0xe8,
0xd6,0xba,0xcb,0xd8,0x8a,0x60,0xb7,0x33,0xb0,0x0d,0x5f,0xb4,0x55,0x2b,0xa3,0xe7,
0xed,0xc8,0xb3,0xf4,0xec,0x90,0x33,0x75,0xf6,0xf7,0x2d,0x4b,0xcd,0xbe,0x31,0x5d,
0xa0,0x41,0xfe,0x50,0xed,0xdc,0x4d,0x5c,0x9d,0x79
};

const uchar secondary_rootkey[] = {
0x41,0x01,0x04,0x01,0x82,0x01,0x52,0x4b,0xaf,0x0c,0x15,0xfa,0x48,0xad,
0xdb,0xd1,0xda,0x29,0xc6,0xbd,0xcc,0x1f,0x68,0x4e,0x90,0xaa,0x64,0xaf,0x94,0x87,
0xbb,0x73,0xa8,0x72,0xb5,0x5c,0xa1,0x03,0x09,0xbf,0xb3,0xe5,0x95,0x31,0x2d,0xe2,
0xb9,0xc8,0xbb,0xd6,0x2b,0xbd,0x0d,0x77,0xdb,0x72,0xdd,0xd2,0x5f,0xf5,0x79,0x6c,
0x96,0xc0,0x57,0xfb,0xaf,0xaa,0xe4,0x24,0x8d,0x03,0x39,0xdc,0xfe,0x02,0xdb,0xed,
0x4d,0x6e,0x46,0x8e,0x0f,0x7e,0x4e,0x95,0xbb,0x71,0xc0,0x5e,0x24,0xfc,0x92,0xad,
0x04,0xdf,0xfd,0x76,0x52,0xb9,0x9a,0xbb,0x53,0x48,0x15,0x94,0x4c,0x86,0xe7,0xd3,
0xbf,0xd0,0xb7,0xf9,0x83,0x86,0x6f,0xef,0xaa,0x12,0x6c,0x82,0x6b,0x00,0xa0,0x60,
0x3b,0x4e,0xfc,0xa4,0xc9,0xb8,0xe5,0x1e,0xed,0x85
};


/*
// live secondary
uchar secondary_rootkey[]= {0x41,0x01,0x04,0x01,0x82,0x01,0x52,0x4b,0xae,0x0a,0x50,0xa2,0xfd,0x3e,
0xb0,0xd3,0xdb,0x8b,0xf1,0xa5,0xd8,0x5a,0xf4,0x0c,0xf1,0x9c,0x5c,0xc3,0x86,0x55,
0xd7,0x06,0xc9,0x14,0xb5,0xf6,0x26,0x7c,0xa5,0x71,0xe9,0x34,0xb4,0xba,0xf8,0x19,
0x7b,0xfc,0x01,0xbd,0xa5,0xfe,0x01,0x07,0x0d,0x28,0xaf,0x68,0xb3,0x80,0x66,0x2a,
0x11,0xc7,0xe0,0x2f,0x87,0x71,0x8d,0x99,0x44,0x5b,0xdf,0x5c,0xa4,0x2a,0x01,0x6b,
0x2e,0x96,0x1c,0x16,0x3f,0x51,0x03,0xde,0x6d,0xe5,0xde,0x2a,0xe6,0x28,0x7a,0x93,
0xcd,0x47,0x4f,0xb9,0xa3,0x36,0xd1,0x01,0x4a,0xdc,0xde,0x65,0xe4,0x2e,0xc3,0x43,
0x78,0x3e,0x26,0x75,0x26,0x82,0x11,0x2a,0xb9,0x25,0xeb,0x00,0x23,0x9a,0xa6,0xd8,
0x19,0x05,0xf9,0x8a,0x9c,0x63,0x5c,0x7d,0x05,0x91
};

*/

/*

// test primary

uchar primary_rootkey[] = {
0x41,0x01,0x04,0x01,0x82,0x01,0x52,0x4b,0xaf,0x0c,0x15,0xfa,0x48,0xad,
0xdb,0xd1,0xda,0x29,0xc6,0xbd,0xcc,0x1f,0x68,0x4e,0x90,0xaa,0x64,0xaf,0x94,0x87,
0xbb,0x73,0xa8,0x72,0xb5,0x5c,0xa1,0x03,0x09,0xbf,0xb3,0xe5,0x95,0x31,0x2d,0xe2,
0xb9,0xc8,0xbb,0xd6,0x2b,0xbd,0x0d,0x77,0xdb,0x72,0xdd,0xd2,0x5f,0xf5,0x79,0x6c,
0x96,0xc0,0x57,0xfb,0xaf,0xaa,0xe4,0x24,0x8d,0x03,0x39,0xdc,0xfe,0x02,0xdb,0xed,
0x4d,0x6e,0x46,0x8e,0x0f,0x7e,0x4e,0x95,0xbb,0x71,0xc0,0x5e,0x24,0xfc,0x92,0xad,
0x04,0xdf,0xfd,0x76,0x52,0xb9,0x9a,0xbb,0x53,0x48,0x15,0x94,0x4c,0x86,0xe7,0xd3,
0xbf,0xd0,0xb7,0xf9,0x83,0x86,0x6f,0xef,0xaa,0x12,0x6c,0x82,0x6b,0x00,0xa0,0x60,
0x3b,0x4e,0xfc,0xa4,0xc9,0xb8,0xe5,0x1e,0xed,0x85
};

uchar secondary_rootkey[] = {
0x41,0x01,0x04,0x01,0x82,0x01,0x52,0x4b,0xb5,0x4c,0x01,0x2e,0xa3,0x19,
0xa5,0x4f,0xfb,0x5f,0x14,0xaa,0xe0,0x41,0x2f,0xce,0x9c,0x27,0xe2,0x2d,0x4f,0x87,
0x5e,0x38,0x9f,0x7d,0x27,0xc5,0x0b,0x64,0x2e,0xf8,0xf7,0xb5,0x4b,0x12,0xf4,0xe2,
0x3f,0xa6,0x70,0x6f,0x60,0xcb,0xd4,0xb7,0xea,0xba,0xb1,0xb9,0x85,0xe4,0xe6,0x4d,
0x4e,0x76,0x83,0xea,0x21,0x6c,0x65,0x9b,0xdf,0xfa,0x3b,0x82,0x1b,0x76,0xa3,0xe9,
0xcc,0x95,0x53,0x1c,0x17,0x7a,0xae,0xe3,0x60,0x32,0x97,0x5b,0x0d,0x16,0x4c,0xdf,
0x1f,0xdc,0xef,0x7e,0x42,0x69,0x0c,0x92,0x6f,0xfc,0xd3,0xd7,0x67,0xce,0x7f,0xe8,
0xe9,0x80,0x12,0x6a,0x20,0x62,0x5c,0x19,0xd3,0x62,0xbc,0xca,0xb2,0x90,0x81,0x66,
0xde,0xfc,0x32,0xb3,0xc1,0xd1,0xde,0x21,0x84,0x39
};
  
*/