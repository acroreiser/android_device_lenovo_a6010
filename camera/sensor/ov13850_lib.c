/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"

#define SENSOR_MODEL_NO_SKUF_OV13850 "ov13850"
#define SKUF_OV13850_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_SKUF_OV13850"_"#n".so"

#define SENSOR_MODEL_NO_OV5670_Q5V41B "ov5670_q5v41b"
#define OV5670_Q5V41B_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_OV5670_Q5V41B"_"#n".so"

#undef DEBUG_INFO
//#define OV13850_DEBUG
#ifdef OV13850_DEBUG
#include <utils/Log.h>
#define SERR(fmt, args...) \
    ALOGE("%s:%d "fmt"\n", __func__, __LINE__, ##args)
#define DEBUG_INFO(fmt, args...) SERR(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = CAM_V_CUSTOM2,
    .config_val = 2,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VIO,
    .config_val = 0,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VANA,
    .config_val = 0,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_VREG,
    .seq_val = CAM_VAF,
    .config_val = 0,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = CAM_VIO,
    .config_val = 0,
    .delay = 1,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = CAM_VDIG,
    .config_val = 0,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = CAM_VANA,
    .config_val = 0,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = CAM_VIO,
    .config_val = 2,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = CAM_VDIG,
    .config_val = 2,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_GPIO,
    .seq_val = CAM_VANA,
    .config_val = 2,
    .delay = 5,
  },
  {
    .seq_type = SENSOR_CLK,
    .seq_val = CAM_VDIG,
    .config_val = 23880000,
    .delay = 10,
  },
  {
    .seq_type = SENSOR_I2C_MUX,
    .seq_val = CAM_VDIG,
    .config_val = 0,
    .delay = 0,
  },
};


static struct msm_camera_sensor_slave_info sensor_slave_info = {
  /* Camera slot where this camera is mounted */
  .camera_id = CAMERA_0,
  /* sensor slave address */
  .slave_addr = 0x6c,
  /* sensor i2c frequency*/
  .i2c_freq_mode = I2C_FAST_MODE,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x300a,
    /* sensor id */
    .sensor_id = 0xd850,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
  },
  .is_flash_supported = SENSOR_FLASH_SUPPORTED,
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 0,
  .position = 0,
  .sensor_mount_angle = 90,
};


static sensor_output_t sensor_output = {
  .output_format = SENSOR_BAYER,
  .connection_mode = 1,
  .raw_output = 1,
};

static struct msm_sensor_output_reg_addr_t output_reg_addr = {
  .x_output = 0x3808,
  .y_output = 0x380a,
  .line_length_pclk = 0x380c,
  .frame_length_lines = 0x380e,
};

static sensor_manual_exposure_info_t manual_exp_info = {
  .min_exposure_time = 100000,/*in nano sec = 1line*/
  .max_exposure_time = 34482759,/*in nano sec = FFFF lines*/
  .min_iso = 50,
  .max_iso = 1174,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x3500,
  .global_gain_addr = 0x350a,
  .vert_offset = 10,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 15.5,
  .max_linecount = 30834,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 4.92,
  .pix_size = 1.4,
  .f_number = 2.65,
  .total_f_dist = 1.97,
  .hor_view_angle = 64.2,
  .ver_view_angle = 42.7,
  .near_end_distance = 10, /*in cm*/
};

static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x1f,
  .csi_if = 1,
  .csid_core = {0},
};

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  {0x0103, 0x01},
};

static struct msm_camera_i2c_reg_array init_reg_array1[] = {
  {0x300, 0x6},
  {0x301, 0x0},
  {0x302, 0x13},
  {0x303, 0x0},
  {0x30a, 0x0},
  {0x300f, 0x11},
  {0x3010, 0x1},
  {0x3011, 0x76},
  {0x3012, 0x41},
  {0x3013, 0x12},
  {0x3014, 0x11},
  {0x301f, 0x3},
  {0x3106, 0x0},
  {0x3210, 0x47},
  {0x3500, 0x0},
  {0x3501, 0xc0},
  {0x3502, 0x0},
  {0x3506, 0x0},
  {0x3507, 0x2},
  {0x3508, 0x0},
  {0x350a, 0x1},
  {0x350b, 0x0},
  {0x350e, 0x0},
  {0x350f, 0x10},
  {0x351a, 0x0},
  {0x351b, 0x10},
  {0x351c, 0x0},
  {0x351d, 0x20},
  {0x351e, 0x0},
  {0x351f, 0x40},
  {0x3520, 0x0},
  {0x3521, 0x80},
  {0x3600, 0xc0},
  {0x3601, 0xfc},
  {0x3602, 0x2},
  {0x3603, 0x78},
  {0x3604, 0xb1},
  {0x3605, 0x95},
  {0x3606, 0x73},
  {0x3607, 0x7},
  {0x3609, 0x40},
  {0x360a, 0x30},
  {0x360b, 0x91},
  {0x360c, 0x9},
  {0x360f, 0x2},
  {0x3611, 0x10},
  {0x3612, 0x27},
  {0x3613, 0x33},
  {0x3615, 0x8},
  {0x3616, 0xe},
  {0x3641, 0x2},
  {0x3660, 0x82},
  {0x3668, 0x54},
  {0x3669, 0x0},
  {0x366a, 0x3f},
  {0x3667, 0xa0},
  {0x3702, 0x40},
  {0x3703, 0x44},
  {0x3704, 0x2c},
  {0x3705, 0x1},
  {0x3706, 0x15},
  {0x3707, 0x44},
  {0x3708, 0x3c},
  {0x3709, 0x1f},
  {0x370a, 0x24},
  {0x370b, 0x3c},
  {0x3720, 0x55},
  {0x3722, 0x84},
  {0x3728, 0x40},
  {0x372a, 0x4},
  {0x372b, 0x2},
  {0x372e, 0x22},
  {0x372f, 0xa0},
  {0x3730, 0x0},
  {0x3731, 0x0},
  {0x3732, 0x0},
  {0x3733, 0x0},
  {0x3710, 0x28},
  {0x3716, 0x3},
  {0x3718, 0x10},
  {0x3719, 0xc},
  {0x371a, 0x8},
  {0x371c, 0xfc},
  {0x3748, 0x0},
  {0x3760, 0x13},
  {0x3761, 0x33},
  {0x3762, 0x86},
  {0x3763, 0x16},
  {0x3767, 0x24},
  {0x3768, 0x6},
  {0x3769, 0x45},
  {0x376c, 0x23},
  {0x376f, 0x80},
  {0x3773, 0x6},
  {0x3d84, 0x0},
  {0x3d85, 0x17},
  {0x3d8c, 0x73},
  {0x3d8d, 0xbf},
  {0x3800, 0x0},
  {0x3801, 0x14},
  {0x3802, 0x0},
  {0x3803, 0xc},
  {0x3804, 0x10},
  {0x3805, 0x8b},
  {0x3806, 0xc},
  {0x3807, 0x43},
  {0x3808, 0x10},
  {0x3809, 0x70},
  {0x380a, 0xc},
  {0x380b, 0x30},
  {0x380c, 0x12},
  {0x380d, 0xc0},
  {0x380e, 0xd},
  {0x380f, 0x0},
  {0x3810, 0x0},
  {0x3811, 0x4},
  {0x3812, 0x0},
  {0x3813, 0x4},
  {0x3814, 0x11},
  {0x3815, 0x11},
  {0x3820, 0x0},
  {0x3821, 0x4},
  {0x3823, 0x0},
  {0x3826, 0x0},
  {0x3827, 0x2},
  {0x3834, 0x0},
  {0x3835, 0x1c},
  {0x3836, 0x4},
  {0x3837, 0x1},
  {0x4000, 0xf1},
  {0x4001, 0x0},
  {0x400b, 0xc},
  {0x4011, 0x0},
  {0x401a, 0x0},
  {0x401b, 0x0},
  {0x401c, 0x0},
  {0x401d, 0x0},
  {0x4020, 0x2},
  {0x4021, 0x3c},
  {0x4022, 0xe},
  {0x4023, 0x37},
  {0x4024, 0xf},
  {0x4025, 0x1c},
  {0x4026, 0xf},
  {0x4027, 0x1f},
  {0x4028, 0x0},
  {0x4029, 0x2},
  {0x402a, 0x4},
  {0x402b, 0x8},
  {0x402c, 0x2},
  {0x402d, 0x2},
  {0x402e, 0xc},
  {0x402f, 0x8},
  {0x403d, 0x2c},
  {0x403f, 0x7f},
  {0x4041, 0x7},
  {0x4500, 0x82},
  {0x4501, 0x38},
  {0x458b, 0x0},
  {0x459c, 0x0},
  {0x459d, 0x0},
  {0x459e, 0x0},
  {0x4601, 0x4},
  {0x4602, 0x22},
  {0x4603, 0x1},
  {0x4837, 0xf},
  {0x4d00, 0x4},
  {0x4d01, 0x42},
  {0x4d02, 0xd1},
  {0x4d03, 0x90},
  {0x4d04, 0x66},
  {0x4d05, 0x65},
  {0x4d0b, 0x0},
  {0x5000, 0xf},
  {0x5001, 0x3},
  {0x5002, 0x7},
  {0x5013, 0x40},
  {0x501c, 0x0},
  {0x501d, 0x10},
  {0x510f, 0xfc},
  {0x5110, 0xf0},
  {0x5111, 0x10},
  {0x536d, 0x2},
  {0x536e, 0x67},
  {0x536f, 0x1},
  {0x5370, 0x4c},
  {0x5400, 0x0},
  {0x5400, 0x0},
  {0x5401, 0x71},
  {0x5402, 0x0},
  {0x5403, 0x0},
  {0x5404, 0x0},
  {0x5405, 0x80},
  {0x540c, 0x5},
  {0x5501, 0x0},
  {0x5b00, 0x0},
  {0x5b01, 0x0},
  {0x5b02, 0x1},
  {0x5b03, 0xff},
  {0x5b04, 0x2},
  {0x5b05, 0x6c},
  {0x5b09, 0x2},
  {0x5e00, 0x0},
  {0x5e10, 0x1c},
};

static struct msm_camera_i2c_reg_setting_5 init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 50,
  },
  {
    .reg_setting = init_reg_array1,
    .size = ARRAY_SIZE(init_reg_array1),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 2,
};

static struct msm_camera_i2c_reg_array start_reg_array[] = {
  {0x0100,0x01},
};

static  struct msm_camera_i2c_reg_setting_5 start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x0100,0x00},
};

static struct msm_camera_i2c_reg_setting_5 stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupon_reg_array[] = {
  {0x3208,0x00},
};

static struct msm_camera_i2c_reg_setting_5 groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0x3208,0x10},
  {0x3208,0xA0},
};

static struct msm_camera_i2c_reg_setting_5 groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg ov13850_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params ov13850_csi_params = {
  .csid_params = {
    .lane_cnt = 4,
    .lut_params = {
      .num_cid = 2,
      .vc_cfg = {
         &ov13850_cid_cfg[0],
         &ov13850_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 4,
    .settle_cnt = 24,
  },
};

static struct msm_camera_csi2_params *csi_params[] = {
  &ov13850_csi_params, /* RES 0*/
  &ov13850_csi_params, /* RES 1*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_pix_fmt_info_t ov13850_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SBGGR10 },
};

static struct sensor_pix_fmt_info_t ov13850_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t ov13850_stream_info[] = {
  {1, &ov13850_cid_cfg[0], ov13850_pix_fmt0_fourcc},
  {1, &ov13850_cid_cfg[1], ov13850_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t ov13850_stream_info_array = {
  .sensor_stream_info = ov13850_stream_info,
  .size = ARRAY_SIZE(ov13850_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
  {0x300, 0x4},
  {0x301, 0x0},
  {0x302, 0x8a},
  {0x3501, 0xc0},
  {0x3611, 0x15},
  {0x3612, 0x7},
  {0x3614, 0xa1},
  {0x370a, 0x24},
  {0x372a, 0x4},
  {0x372f, 0xa0},
  {0x3780, 0x90},
  {0x3781, 0x2},
  {0x3782, 0x44},
  {0x3800, 0x0},
  {0x3801, 0x14},
  {0x3802, 0x0},
  {0x3803, 0xc},
  {0x3804, 0x10},
  {0x3805, 0x8b},
  {0x3806, 0xc},
  {0x3807, 0x43},
  {0x3808, 0x10},
  {0x3809, 0x70},
  {0x380a, 0xc},
  {0x380b, 0x30},
  {0x380c, 0x14},
  {0x380d, 0xe4},
  {0x380e, 0xd},
  {0x380f, 0x0},
  {0x3813, 0x4},
  {0x3814, 0x11},
  {0x3815, 0x11},
  {0x3820, 0x0},
  {0x3821, 0x4},
  {0x3836, 0x4},
  {0x3837, 0x1},
  {0x4020, 0x2},
  {0x4021, 0x3c},
  {0x4022, 0xe},
  {0x4023, 0x37},
  {0x4024, 0xf},
  {0x4025, 0x1c},
  {0x4026, 0xf},
  {0x4027, 0x1f},
  {0x4501, 0x38},
  {0x4601, 0x4},
  {0x4603, 0x1},
  {0x4837, 0xf},
  {0x5401, 0x71},
  {0x5405, 0x80},
  {0x3718, 0x10},
  {0x3811, 0x4},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
  {0x300, 0x4},
  {0x301, 0x0},
  {0x302, 0x8a},
  {0x3501, 0x60},
  {0x3611, 0x15},
  {0x3612, 0x27},
  {0x3614, 0xa1},
  {0x370a, 0x27},
  {0x372a, 0x0},
  {0x372f, 0x90},
  {0x3780, 0x10},
  {0x3781, 0x0},
  {0x3782, 0x0},
  {0x3800, 0x0},
  {0x3801, 0x8},
  {0x3802, 0x0},
  {0x3803, 0x4},
  {0x3804, 0x10},
  {0x3805, 0x97},
  {0x3806, 0xc},
  {0x3807, 0x4b},
  {0x3808, 0x8},
  {0x3809, 0x40},
  {0x380a, 0x6},
  {0x380b, 0x20},
  {0x380c, 0x9},
  {0x380d, 0x60},
  {0x380e, 0xd},
  {0x380f, 0x0},
  {0x3813, 0x2},
  {0x3814, 0x31},
  {0x3815, 0x31},
  {0x3820, 0x2},
  {0x3821, 0x6},
  {0x3836, 0x8},
  {0x3837, 0x2},
  {0x4020, 0x0},
  {0x4021, 0xe4},
  {0x4022, 0x4},
  {0x4023, 0xd7},
  {0x4024, 0x5},
  {0x4025, 0xbc},
  {0x4026, 0x5},
  {0x4027, 0xbf},
  {0x4501, 0x3c},
  {0x4601, 0x83},
  {0x4603, 0x1},
  {0x4837, 0xf},
  {0x5401, 0x61},
  {0x5405, 0x40},
  {0x372f, 0xa0},
  {0x3718, 0x1c},
  {0x3801, 0x0},
  {0x3805, 0x9f},
  {0x3811, 0x8},
  {0x3820, 0x1},
};


static struct msm_camera_i2c_reg_setting_5 res_settings[] = {
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct sensor_crop_parms_t crop_params[] = {
  {0, 0, 0, 0}, /* RES 0 */
  {0, 0, 0, 0}, /* RES 1 */
};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  {
  .x_output = 4208,
  .y_output = 3120,
  .line_length_pclk = 5348,
  .frame_length_lines = 3328,
  .vt_pixel_clk = 480585000,
  .op_pixel_clk = 439392000,
  .binning_factor = 1,
  .max_fps = 27,
  .min_fps = 12,
  .mode = 1,
  },
  {
  .x_output = 2112,
  .y_output = 1568,
  .line_length_pclk = 2400,
  .frame_length_lines = 3328,
  .vt_pixel_clk = 240292500,
  .op_pixel_clk = 439392000,
  .binning_factor = 1,
  .max_fps = 30,
  .min_fps = 12,
  .mode = 1,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t ov13850_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t ov13850_res_table = {
  .res_cfg_type = ov13850_res_cfg,
  .size = ARRAY_SIZE(ov13850_res_cfg),
};

static struct sensor_lib_chromatix_t ov13850_chromatix[] = {
  {
    .common_chromatix = SKUF_OV13850_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = OV5670_Q5V41B_LOAD_CHROMATIX(snapshot),
    .camera_snapshot_chromatix = OV5670_Q5V41B_LOAD_CHROMATIX(snapshot),
    .camcorder_chromatix = SKUF_OV13850_LOAD_CHROMATIX(default_video),
    .liveshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(snapshot),
  },
  {
    .common_chromatix = SKUF_OV13850_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = SKUF_OV13850_LOAD_CHROMATIX(preview),
    .camera_snapshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(preview),
    .camcorder_chromatix = SKUF_OV13850_LOAD_CHROMATIX(default_video),
    .liveshot_chromatix = SKUF_OV13850_LOAD_CHROMATIX(preview),
  },
};

static struct sensor_lib_chromatix_array ov13850_lib_chromatix_array = {
  .sensor_lib_chromatix = ov13850_chromatix,
  .size = ARRAY_SIZE(ov13850_chromatix),
};

/*===========================================================================
 * FUNCTION    - ov13850_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t ov13850_real_to_register_gain(float gain)
{
  uint16_t reg_gain, reg_temp;
  if (gain < 1.0) {
      gain = 1.0;
  } else if (gain > 15.5) {
      gain = 15.5;
  }
  gain = (gain) * 16.0;
  reg_gain = (uint16_t) gain;
  return reg_gain;
}


/*===========================================================================
 * FUNCTION    - ov13850_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float ov13850_register_to_real_gain(uint16_t reg_gain)
{
  float real_gain;

  if (reg_gain < 0x10) {
      reg_gain = 0x10;
  } else if (reg_gain >= 0xf8) {
      reg_gain = 0xf8;
  }
  real_gain = (float) reg_gain / 16.0;
  return real_gain;
}


/*===========================================================================
 * FUNCTION    - ov13850_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov13850_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = ov13850_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain =
    ov13850_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
  exp_info->line_count = line_count;
  exp_info->sensor_digital_gain = 0x1;
  return 0;
}

/*===========================================================================
 * FUNCTION    - ov13850_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t ov13850_fill_exposure_array(uint16_t gain,
        uint32_t line,uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
        struct msm_camera_i2c_reg_setting_5 *reg_setting)
{
  int32_t rc = 0;
  uint16_t reg_count = 0;
  uint16_t i = 0;
  DEBUG_INFO("%s:gain=%d,line=%d,fl_lines=%d",__func__,
    gain,line,fl_lines) ;

  if (!reg_setting) {
    return -1;
  }

  for (i = 0; i < sensor_lib_ptr.groupon_settings->size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupon_settings->reg_setting[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupon_settings->reg_setting[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
  DEBUG_INFO("%s:fl_lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines + 1;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
  DEBUG_INFO("%s:fl_lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line&0xffff) >> 12;
  DEBUG_INFO("%s:lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line&0xfff) >> 4;
  DEBUG_INFO("%s:lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 2;
  reg_setting->reg_setting[reg_count].reg_data = (line&0x0f) << 4;
  DEBUG_INFO("%s:lines:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF) >> 8;
  DEBUG_INFO("%s:gain:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  DEBUG_INFO("%s:gain:addr=0x%x,data=0x%x",__func__,
    reg_setting->reg_setting[reg_count].reg_addr,
    reg_setting->reg_setting[reg_count].reg_data) ;
  reg_count++;

  for (i = 0; i < sensor_lib_ptr.groupoff_settings->size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->size = reg_count;
  reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;

  return rc;
}

static sensor_exposure_table_t ov13850_expsoure_tbl = {
  .sensor_calculate_exposure = ov13850_calculate_exposure,
  .sensor_fill_exposure_array = ov13850_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor eeprom name */
  .eeprom_name = "wingtech_ov13850",
  /* sensor actuator name */
  .actuator_name = "dw9761_ofilm",
  /* sensor output settings */
  .sensor_output = &sensor_output,
  /* sensor output register address */
  .output_reg_addr = &output_reg_addr,
  /* sensor exposure gain register address */
  .exp_gain_info = &exp_gain_info,
  /* manual_exp_info */
  .manual_exp_info = &manual_exp_info,
  /* sensor aec info */
  .aec_info = &aec_info,
  /* sensor snapshot exposure wait frames info */
  .snapshot_exp_wait_frames = 2,
  /* number of frames to skip after start stream */
  .sensor_num_frame_skip = 2,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 2,
  /* sensor exposure table size */
  .exposure_table_size = 10,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = ov13850_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(ov13850_cid_cfg),
  /* init settings */
  .init_settings_array = &init_settings_array,
  /* start settings */
  .start_settings = &start_settings,
  /* stop settings */
  .stop_settings = &stop_settings,
  /* group on settings */
  .groupon_settings = &groupon_settings,
  /* group off settings */
  .groupoff_settings = &groupoff_settings,
  /* resolution cfg table */
  .sensor_res_cfg_table = &ov13850_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &ov13850_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &ov13850_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &ov13850_lib_chromatix_array,
  /* sensor pipeline immediate delay */
  .sensor_max_immediate_frame_delay = 2,
};

/*===========================================================================
 * FUNCTION    - ov13850_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *ov13850_open_lib(void)
{
  return &sensor_lib_ptr;
}
