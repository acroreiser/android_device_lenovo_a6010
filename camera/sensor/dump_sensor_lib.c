/*
  Copyright (c) 2022 @acroreiser
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include "sensor_lib.h"


typedef struct {
	void *sensor_lib_handle;
	void *module_lib_handle;
	sensor_lib_t *sensor_lib_ptr;
} sensor_lib_params_t;

static sensor_lib_params_t sensor_lib_params_0;

#define BUFF_SIZE_255 255
#define SENSOR_SUCCESS 0
#define SENSOR_FAILURE -1
#define SENSOR_ERROR_IO -2
#define SENSOR_ERROR_NOMEM -3
#define SENSOR_ERROR_INVAL -4

int32_t sensor_load_library(const char *name, void *data)
{
	char lib_name[BUFF_SIZE_255] = {0};
	char open_lib_str[BUFF_SIZE_255] = {0};
	void *(*sensor_open_lib_l)(void) = NULL;
	void *handle;
	sensor_lib_params_t *sensor_lib_params = (sensor_lib_params_t *)data; 
	snprintf(lib_name, BUFF_SIZE_255, "./libmmcamera_%s.so", name);

	sensor_lib_params->sensor_lib_handle = handle = dlopen(lib_name, RTLD_NOW); //RTLD_NOW
	if (!sensor_lib_params->sensor_lib_handle && !handle)
		return -EINVAL;

	snprintf(open_lib_str, BUFF_SIZE_255, "%s_open_lib", name);
	*(void **)&sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
	open_lib_str);
	if (!sensor_open_lib_l) {
		*(void **)&sensor_open_lib_l = dlsym(sensor_lib_params->sensor_lib_handle,
			"sensor_open_lib");

		if (!sensor_open_lib_l)
			return -EINVAL;
	}
	sensor_lib_params->sensor_lib_ptr = (sensor_lib_t *)sensor_open_lib_l();
	if (!sensor_lib_params->sensor_lib_ptr)
		return -EINVAL;

	return 0;
}

int32_t sensor_unload_library(sensor_lib_params_t *sensor_lib_params)
{
	if (!sensor_lib_params)
		return SENSOR_FAILURE;

	if (sensor_lib_params->sensor_lib_handle) {
		dlclose(sensor_lib_params->sensor_lib_handle);
		sensor_lib_params->sensor_lib_handle = NULL;
		sensor_lib_params->sensor_lib_ptr = NULL;
	}
  
	if (sensor_lib_params->module_lib_handle)
		dlclose(sensor_lib_params->module_lib_handle);

	return SENSOR_SUCCESS;
}

void __attach(void) __attribute__((constructor));
void __detach(void) __attribute__((destructor));


void __attach(void)
{
	printf("__attach()\n");
}

void __detach(void)
{
	printf("__detach()\n");
	sensor_unload_library(&sensor_lib_params_0);
}


static char * camera_power_seq_type_str[10];
static char* camera_vreg_name_str[8];

void print_power_setting_array(struct msm_sensor_power_setting_array *power_setting_array)
{
	int i;
		
	camera_power_seq_type_str[SENSOR_CLK] = "SENSOR_CLK";
	camera_power_seq_type_str[SENSOR_GPIO] = "SENSOR_GPIO";
	camera_power_seq_type_str[SENSOR_VREG]="SENSOR_VREG";
	camera_power_seq_type_str[SENSOR_I2C_MUX]="SENSOR_I2C_MUX";
	camera_power_seq_type_str[SENSOR_I2C]="SENSOR_I2C";

	camera_vreg_name_str[CAM_VDIG]="CAM_VDIG";
	camera_vreg_name_str[CAM_VIO]= "CAM_VIO";
	camera_vreg_name_str[CAM_VANA]=  "CAM_VANA";
	camera_vreg_name_str[CAM_VAF]= "CAM_VAF";
	camera_vreg_name_str[CAM_V_CUSTOM1]="CAM_V_CUSTOM1";
	camera_vreg_name_str[CAM_V_CUSTOM2]="CAM_V_CUSTOM2";
	camera_vreg_name_str[CAM_VREG_MAX]="CAM_VREG_MAX";
	
	if(power_setting_array->size)
	{
		printf("BEGIN POWER SETTINGS\n");
		printf(".size = %d,\n", power_setting_array->size);
		for(i = 0;i < power_setting_array->size; i++)
		{
			printf("  {\n");
			printf("    .seq_type = %s,\n", camera_power_seq_type_str[power_setting_array->power_setting[i].seq_type]);
			printf("    .seq_val = %s,\n", camera_vreg_name_str[power_setting_array->power_setting[i].seq_val]);
			printf("    .config_val = %ld,\n", power_setting_array->power_setting[i].config_val);
			printf("    .delay = %hu,\n", power_setting_array->power_setting[i].delay);
			printf("  },\n");
		}
		printf("END POWER SETTINGS\n");
	} else
		printf("power_setting not found!\n");
}

void print_msm_camera_i2c_reg_setting(struct msm_camera_i2c_reg_setting_5 *reg_settings)
{
	int i, k;
	
	printf("msm_camera_i2c_reg_setting =");
	printf("  {\n");
	printf("    .reg_setting = init_reg_array,\n");
	printf("    .size = %d,\n", reg_settings->size);
	printf("    .addr_type = %d,\n",  reg_settings->addr_type);
	printf("    .data_type = %d,\n", reg_settings->data_type);
	printf("    .delay = %d,\n", reg_settings->delay);
	printf("  },\n");

	printf("reg_settings array =\n");

	printf("init_reg_array = \n");
	printf("{\n");
	for(k = 0; k < reg_settings->size; k++)
		printf("  {0x%x, 0x%x},\n", reg_settings->reg_setting[k].reg_addr, reg_settings->reg_setting[k].reg_data);

	printf("};\n");
}


void print_sensor_lib_reg_settings_array(struct sensor_lib_reg_settings_array *init_settings_array)
{
	int i;

	if(init_settings_array->size)
	{
		printf("BEGIN\n");
		printf("sensor_lib_reg_settings_array array =\n");
		printf(".size = %d,\n", init_settings_array->size);
		for(i = 0; i < init_settings_array->size; i++)
			print_msm_camera_i2c_reg_setting(&init_settings_array->reg_settings[i]);

		printf("END\n");
	} else {
		printf("init_setting not found!\n");
	}
}

void print_sensor_slave_info(sensor_lib_t *info)
{
							
	if(!info) {
		printf("info not found\n");
		return;
	} 

	print_power_setting_array(&info->sensor_slave_info->power_setting_array);

	printf("slave_addr = 0x%x\n", info->sensor_slave_info->slave_addr);
	printf("addr_type = %u\n", info->sensor_slave_info->addr_type);
	printf("i2c_freq_mode = %u\n", info->sensor_slave_info->i2c_freq_mode);
	printf("sensor_id_reg_addr = 0x%x\n", info->sensor_slave_info->sensor_id_info.sensor_id_reg_addr);
	printf("sensor_id = 0x%x\n", info->sensor_slave_info->sensor_id_info.sensor_id);
}

void print_msm_sensor_exp_gain_info_t(struct msm_sensor_exp_gain_info_t *gain_info)
{
	printf("BEGIN\n");
	printf("msm_sensor_exp_gain_info_t gain_info =\n");
	printf("{\n");
	printf("  .coarse_int_time_addr = 0x%x,\n", gain_info->coarse_int_time_addr);
	printf("  .global_gain_addr = 0x%x,\n", gain_info->global_gain_addr);
	printf("  .vert_offset = %d,\n", gain_info->vert_offset);
	printf("};\n");
	printf("END\n");
}

void print_msm_sensor_output_reg_addr_t(struct msm_sensor_output_reg_addr_t *output_reg_addr)
{
	printf("BEGIN\n");
	printf("msm_sensor_output_reg_addr_t output_reg_addr =\n");
	printf("{\n");
	printf("  .x_output = 0x%x,\n", output_reg_addr->x_output);
	printf("  .y_output = 0x%x,\n", output_reg_addr->y_output);
	printf("  .line_length_pclk = 0x%x,\n", output_reg_addr->line_length_pclk);
	printf("  .frame_length_lines = 0x%x,\n", output_reg_addr->frame_length_lines);
	printf("};\n");
	printf("END\n");
}


void print_sensor_manual_exposure_info_t(sensor_manual_exposure_info_t *manual_exposure_info)
{
		if(!manual_exposure_info)
		{
				printf("Sensor library doesn't contain manual_exposure_info!\n");
				return;
		}
		printf("BEGIN\n");
		printf("sensor_manual_exposure_info_t manual_exposure_info =\n");
		printf("{\n");
		printf("  .min_exposure_time = %llu,\n", manual_exposure_info->min_exposure_time);
		printf("  .max_exposure_time = %llu,\n", manual_exposure_info->max_exposure_time);
		printf("  .min_iso = %u,\n", manual_exposure_info->min_iso);
		printf("  .max_iso = %u,\n", manual_exposure_info->max_iso);
		printf("};\n");
		printf("END\n");
}

void print_sensor_lens_info_t(sensor_lens_info_t *lens_info)
{
		printf("BEGIN\n");
		printf("sensor_lens_info_t lens_info =\n");
		printf("{\n");
		printf("  .focal_length = %f,\n", lens_info->focal_length);
		printf("  .pix_size = %f,\n", lens_info->pix_size);
		printf("  .f_number = %f,\n", lens_info->f_number);
		printf("  .total_f_dist = %f,\n", lens_info->total_f_dist);
		printf("  .hor_view_angle = %f,\n", lens_info->hor_view_angle);
		printf("  .ver_view_angle = %f,\n", lens_info->ver_view_angle);
		printf("  .um_per_dac = %f,\n", lens_info->um_per_dac);
		printf("  .dac_offset = %d,\n", lens_info->dac_offset);
		printf("  .near_end_distance = %u,\n", lens_info->near_end_distance);
		printf("  .sensing_method = %d,\n", lens_info->sensing_method);
		printf("  .crop_factor = %f,\n", lens_info->crop_factor);
		printf("};\n");
		printf("END\n");
}

void print_csi_lane_params_t(struct csi_lane_params_t *csi_lane_params)
{
		printf("BEGIN\n");
		printf("csi_lane_params_t csi_lane_params =\n");
		printf("{\n");
		printf("  .csi_lane_assign = 0x%x,\n", csi_lane_params->csi_lane_assign);
		printf("  .csi_lane_mask = 0x%x,\n", csi_lane_params->csi_lane_mask);
		printf("  .csi_if = %u,\n", csi_lane_params->csi_if);
		printf("  .csid_core = %u,\n", csi_lane_params->csid_core[0]);
		printf("};\n");
		printf("END\n");
}

void print_sensor_output_t(sensor_output_t *sensor_output)
{
		printf("BEGIN\n");
		printf("sensor_output_t sensor_output =\n");
		printf("{\n");
		printf("  .output_format = 0x%x,\n", sensor_output->output_format);
		printf("  .connection_mode = 0x%x,\n", sensor_output->connection_mode);
		printf("  .raw_output = %u,\n", sensor_output->raw_output);
		printf("};\n");
		printf("END\n");
}

void print_sensor_lib_out_info_array(struct sensor_lib_out_info_array *out_info_array)
{
	int i;

	printf("BEGIN\n");
	printf("sensor_output_t sensor_output =\n");
	for(i = 0; i < out_info_array->size; i++)
	{
		printf("{\n");
		printf("  .x_output = %u,\n", out_info_array->out_info[i].x_output);
		printf("  .y_output = %u,\n", out_info_array->out_info[i].y_output);
		printf("  .line_length_pclk = %u,\n", out_info_array->out_info[i].line_length_pclk);
		printf("  .frame_length_lines = %u,\n", out_info_array->out_info[i].frame_length_lines);
		printf("  .vt_pixel_clk = %u,\n", out_info_array->out_info[i].vt_pixel_clk);
		printf("  .op_pixel_clk = %u,\n", out_info_array->out_info[i].op_pixel_clk);
		printf("  .binning_factor = %u,\n", out_info_array->out_info[i].binning_factor);
		printf("  .max_fps = %f,\n", out_info_array->out_info[i].max_fps);
		printf("  .min_fps = %f,\n", out_info_array->out_info[i].min_fps);
		printf("  .mode = %u,\n", out_info_array->out_info[i].mode);
		printf("};\n");
	}
	printf("END\n");
}

void print_sensor_lib_chromatix_array(struct sensor_lib_chromatix_array *chromatix_array)
{
	int i;

	printf("BEGIN\n");
	printf("sensor_lib_chromatix_array chromatix_array =\n");
	for(i = 0; i < chromatix_array->size; i++)
	{
		printf("{\n");
		printf("  .common_chromatix = %s,\n", chromatix_array->sensor_lib_chromatix[i].common_chromatix);
		printf("  .camera_preview_chromatix = %s,\n", chromatix_array->sensor_lib_chromatix[i].camera_preview_chromatix);
		printf("  .camera_snapshot_chromatix = %s,\n", chromatix_array->sensor_lib_chromatix[i].camera_snapshot_chromatix);
		printf("  .camcorder_chromatix = %s,\n", chromatix_array->sensor_lib_chromatix[i].camcorder_chromatix);
		printf("  .liveshot_chromatix = %s,\n", chromatix_array->sensor_lib_chromatix[i].liveshot_chromatix);
		printf("};\n");
	}
	printf("END\n");
}

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
	printf("Caught segfault at address %p\n", si->si_addr);
	sensor_unload_library(&sensor_lib_params_0);
	exit(0);
}

int main(int argc, char *argv[]){
	int i;
	static char *sensor;
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags   = SA_SIGINFO;
	
	sigaction(SIGSEGV, &sa, NULL);

	if (argc != 2) {
		printf("usage: %s [ov13850]\n", argv[0]);
		printf("for example, 'ov13850' for libmmcamera_ov13850.so\n");
		printf("e.g. %s s5k3p3sm for libmmcamera_s5k3p3sm.so\n", argv[0]);
		exit(1);
	}

	sensor = argv[1];

	i = sensor_load_library(sensor, &sensor_lib_params_0);
	printf("sensor_load_library, return %d\n", i);

	if(!i) {
		if (sensor_lib_params_0.sensor_lib_ptr)
		{
			print_sensor_slave_info(sensor_lib_params_0.sensor_lib_ptr);
		
			printf("BEGIN init_settings_array\n");
			print_sensor_lib_reg_settings_array(sensor_lib_params_0.sensor_lib_ptr->init_settings_array);

			printf("BEGIN res_settings_array\n");
			print_sensor_lib_reg_settings_array(sensor_lib_params_0.sensor_lib_ptr->res_settings_array);

			printf("BEGIN start_settings\n");
			print_msm_camera_i2c_reg_setting(sensor_lib_params_0.sensor_lib_ptr->start_settings);

			printf("BEGIN stop_settings\n");
			print_msm_camera_i2c_reg_setting(sensor_lib_params_0.sensor_lib_ptr->stop_settings);	

			printf("BEGIN groupon_settings\n");
			print_msm_camera_i2c_reg_setting(sensor_lib_params_0.sensor_lib_ptr->groupon_settings);	

			printf("BEGIN groupoff_settings\n");
			print_msm_camera_i2c_reg_setting(sensor_lib_params_0.sensor_lib_ptr->groupoff_settings);

			printf("BEGIN exp_gain_info\n");
			print_msm_sensor_exp_gain_info_t(sensor_lib_params_0.sensor_lib_ptr->exp_gain_info);

			printf("BEGIN output_reg_addr\n");
			print_msm_sensor_output_reg_addr_t(sensor_lib_params_0.sensor_lib_ptr->output_reg_addr);
	
			printf("BEGIN sensor_lens_info_t\n");
			print_sensor_lens_info_t(sensor_lib_params_0.sensor_lib_ptr->default_lens_info);

			printf("BEGIN sensor_manual_exposure_info_t\n");
			print_sensor_manual_exposure_info_t(sensor_lib_params_0.sensor_lib_ptr->manual_exp_info);

			printf("BEGIN sensor_output_t\n");
			print_sensor_output_t(sensor_lib_params_0.sensor_lib_ptr->sensor_output);

			printf("BEGIN csi_lane_params_t\n");
			print_csi_lane_params_t(sensor_lib_params_0.sensor_lib_ptr->csi_lane_params);

			printf("BEGIN sensor_lib_out_info_array\n");
			print_sensor_lib_out_info_array(sensor_lib_params_0.sensor_lib_ptr->out_info_array);

			printf("BEGIN sensor_lib_chromatix_array\n");
			print_sensor_lib_chromatix_array(sensor_lib_params_0.sensor_lib_ptr->chromatix_array);
		}
	}
	else
		printf("no info\n");
	
	sensor_unload_library(&sensor_lib_params_0);
	printf("sensor_unload_library\n");

	return 0;
}
