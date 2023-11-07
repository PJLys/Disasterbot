#include "FreeRTOS.h"
#include "DriverVL53L0X.h"

#include <stdio.h>

#define VL53L0X_ADDR 0x29

#define VL53L0X_DEBUG 
//#define VL53L0X_ASSERT_COMMS_ERROR //If defined, assert communication success

#define decodeVcselPeriod(reg_val)      (((reg_val) + 1) << 1)
#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)

typedef enum  { VcselPeriodPreRange, VcselPeriodFinalRange } vcselPeriodType;

typedef enum 
    {
	    SYSRANGE_START                              = 0x00,

	    SYSTEM_THRESH_HIGH                          = 0x0C,
	    SYSTEM_THRESH_LOW                           = 0x0E,

	    SYSTEM_SEQUENCE_CONFIG                      = 0x01,
	    SYSTEM_RANGE_CONFIG                         = 0x09,
	    SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,

	    SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,

	    GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,

	    SYSTEM_INTERRUPT_CLEAR                      = 0x0B,

	    RESULT_INTERRUPT_STATUS                     = 0x13,
	    RESULT_RANGE_STATUS                         = 0x14,

	    RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
	    RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,
	    RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
	    RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
	    RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6,

	    ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,

	    I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,

	    MSRC_CONFIG_CONTROL                         = 0x60,

	    PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
	    PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
	    PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
	    PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,

	    FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
	    FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
	    FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
	    FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,

	    PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
	    PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,

	    PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
	    PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
	    PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,

	    SYSTEM_HISTOGRAM_BIN                        = 0x81,
	    HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
	    HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,

	    FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
	    FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
	    FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
	    CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,

	    MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,

	    SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
	    IDENTIFICATION_MODEL_ID                     = 0xC0,
	    IDENTIFICATION_REVISION_ID                  = 0xC2,

	    OSC_CALIBRATE_VAL                           = 0xF8,

	    GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
	    GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
	    GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
	    GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
	    GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
	    GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
	    GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,

	    GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
	    DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E,
	    DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F,
	    POWER_MANAGEMENT_GO1_POWER_FORCE            = 0x80,

	    VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           = 0x89,

	    ALGO_PHASECAL_LIM                           = 0x30,
	    ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30
    } regAddrEnum;

typedef struct 
{
	uint8_t tcc, msrc, dss, pre_range, final_range;
} SequenceStepEnables;

typedef struct 
{
	uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

	uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
	uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
}SequenceStepTimeouts;

static uint8_t setSignalRateLimit(float limit_Mcps);
static uint8_t getSpadInfo(uint8_t * count, uint8_t * type_is_aperture);
static void startTimeout(void);
static uint8_t checkTimeoutExpired();
static uint16_t getTimeout();
uint32_t static getMeasurementTimingBudget();
static void getSequenceStepEnables(SequenceStepEnables * enables);
static void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts);
static uint8_t getVcselPulsePeriod(vcselPeriodType type);
static uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);
static uint16_t decodeTimeout(uint16_t reg_val);
static uint16_t encodeTimeout(uint32_t timeout_mclks);
uint8_t setMeasurementTimingBudget(uint32_t budget_us);
static uint8_t performSingleRefCalibration(uint8_t vhv_init_byte);

uint32_t measurement_timing_budget_us;
static uint8_t stop_variable;
static uint32_t timeout_start_ticks;
static uint32_t io_timeout=0;
static uint8_t did_timeout=0;

// Write an 8-bit register
static uint8_t writeReg(uint8_t reg, uint8_t value)
{	
	uint8_t res;
	uint8_t Buffer[4];
	Buffer[0]=reg;
	Buffer[1]=value;
	res=TWIMWrite(VL53L0X_ADDR,Buffer,2);
	
	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif
	
	return res;
}

// Write a 16-bit register
uint8_t writeReg16Bit(uint8_t reg, uint16_t value)
{
	uint8_t res;
	uint8_t Buffer[4];
	Buffer[0]=reg,
	Buffer[1]=value>>8;
	Buffer[2]=value;
	res=TWIMWrite(VL53L0X_ADDR,Buffer,3);

	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif

	return res;
}

// Write a 32-bit register
uint8_t writeReg32Bit(uint8_t reg, uint32_t value)
{
	uint8_t res;
	uint8_t Buffer[5];
	Buffer[0]=reg,
	Buffer[1]=value>>24;
	Buffer[2]=value>>16;
	Buffer[3]=value>>8;
	Buffer[4]=value;
	res=TWIMWrite(VL53L0X_ADDR,Buffer,5);

	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif

	return res;
}

// Read an 8-bit register
uint8_t readReg(uint8_t reg,uint8_t *data)
{	
	uint8_t res;
	uint8_t Buffer[4];
	
	Buffer[0]=reg;
	
	res=TWIMWriteRead(VL53L0X_ADDR,Buffer,1,Buffer,1);
	if (data!=NULL) data[0]=Buffer[0];
	
	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif

	return res;
}

// Read a 16-bit register
uint8_t readReg16Bit(uint8_t reg,uint16_t *data)
{
	
	uint8_t res;
	uint8_t Buffer[4];
	
	Buffer[0]=reg;
	
	res=TWIMWriteRead(VL53L0X_ADDR,Buffer,1,Buffer,2);
	if (data!=NULL) 
	{
		*data=(uint16_t) Buffer[0]<<8 + (uint16_t) Buffer[1];
	}
	
	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif
	return res;
}

// Read a 32-bit register
uint8_t readReg32Bit(uint8_t reg,uint32_t *data)
{
	uint8_t res;
	uint8_t Buffer[4];
	
	Buffer[0]=reg;
	
	res=TWIMWriteRead(VL53L0X_ADDR,Buffer,1,Buffer,4);
	if (data!=NULL)
	{
		*data=(uint32_t) Buffer[0]<<24 + (uint32_t) Buffer[1]<<16 + (uint32_t) Buffer[2]<<8 + (uint32_t) Buffer[3];
	}
	
	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif
	return res;
}

// Write an arbitrary number of bytes from the given array to the sensor,
// starting at the given register
uint8_t writeMulti(uint8_t reg, uint8_t const * src, uint8_t count)
{
	uint8_t res;
	uint8_t Buffer[16];
	
	if (count>15) return -2;
	
	Buffer[0]=reg;
	memcpy(&Buffer[1],src,count);
	
	res=TWIMWrite(VL53L0X_ADDR,Buffer,count+1);
	
	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif
	
	return res;
}

// Read an arbitrary number of bytes from the sensor, starting at the given
// register, into the given array
uint8_t readMulti(uint8_t reg, uint8_t * dst, uint8_t count)
{
	uint8_t res;
	uint8_t Buffer[1];
	Buffer[0]=reg;
	
	res=TWIMWriteRead(VL53L0X_ADDR,Buffer,1,dst,count);
	
	#ifdef VL53L0X_ASSERT_COMMS_ERROR
		configASSERT(res);
	#endif
	
	return res;
}


uint8_t DriverVL53L0XInit(void)
{
	uint8_t Data;
	uint8_t res;
	int a;
	
	//Check Model ID
	res=readReg(IDENTIFICATION_MODEL_ID,&Data);
	if (!res)
	{
		#ifdef VL53L0X_DEBUG
			printf ("VL53L0X:Cannot access.\r\n");
		#endif
		return false;
	}
	if (Data != 0xEE)
	{
		#ifdef VL53L0X_DEBUG
			printf ("VL53L0X:Incorrect ID_MODEL_ID\r\n");
		#endif
		return false;
	}

	// "Set I2C standard mode"
	writeReg(0x88, 0x00);

	writeReg(0x80, 0x01);
	writeReg(0xFF, 0x01);
	writeReg(0x00, 0x00);
	readReg(0x91,&stop_variable);
	writeReg(0x00, 0x01);
	writeReg(0xFF, 0x00);
	writeReg(0x80, 0x00);

	// disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
	readReg(MSRC_CONFIG_CONTROL,&Data);
	writeReg(MSRC_CONFIG_CONTROL, Data | 0x12);

	// set final range signal rate limit to 0.25 MCPS (million counts per second)
	setSignalRateLimit(0.25);

	writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF);

	// VL53L0X_DataInit() end

	// VL53L0X_StaticInit() begin

	uint8_t spad_count;
	uint8_t spad_type_is_aperture;
	if (!getSpadInfo(&spad_count, &spad_type_is_aperture)) { return false; }

	// The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
	// the API, but the same data seems to be more easily readable from
	// GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
	uint8_t ref_spad_map[6];
	readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
	

	// -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)

	writeReg(0xFF, 0x01);
	writeReg(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
	writeReg(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
	writeReg(0xFF, 0x00);
	writeReg(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

	uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
	uint8_t spads_enabled = 0;

	for (uint8_t i = 0; i < 48; i++)
	{
		if (i < first_spad_to_enable || spads_enabled == spad_count)
		{
			// This bit is lower than the first one that should be enabled, or
			// (reference_spad_count) bits have already been enabled, so zero this bit
			ref_spad_map[i / 8] &= ~(1 << (i % 8));
		}
		else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
		{
			spads_enabled++;
		}
	}

	writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

	// -- VL53L0X_set_reference_spads() end

	// -- VL53L0X_load_tuning_settings() begin
	// DefaultTuningSettings from vl53l0x_tuning.h

	writeReg(0xFF, 0x01);
	writeReg(0x00, 0x00);

	writeReg(0xFF, 0x00);
	writeReg(0x09, 0x00);
	writeReg(0x10, 0x00);
	writeReg(0x11, 0x00);

	writeReg(0x24, 0x01);
	writeReg(0x25, 0xFF);
	writeReg(0x75, 0x00);

	writeReg(0xFF, 0x01);
	writeReg(0x4E, 0x2C);
	writeReg(0x48, 0x00);
	writeReg(0x30, 0x20);

	writeReg(0xFF, 0x00);
	writeReg(0x30, 0x09);
	writeReg(0x54, 0x00);
	writeReg(0x31, 0x04);
	writeReg(0x32, 0x03);
	writeReg(0x40, 0x83);
	writeReg(0x46, 0x25);
	writeReg(0x60, 0x00);
	writeReg(0x27, 0x00);
	writeReg(0x50, 0x06);
	writeReg(0x51, 0x00);
	writeReg(0x52, 0x96);
	writeReg(0x56, 0x08);
	writeReg(0x57, 0x30);
	writeReg(0x61, 0x00);
	writeReg(0x62, 0x00);
	writeReg(0x64, 0x00);
	writeReg(0x65, 0x00);
	writeReg(0x66, 0xA0);

	writeReg(0xFF, 0x01);
	writeReg(0x22, 0x32);
	writeReg(0x47, 0x14);
	writeReg(0x49, 0xFF);
	writeReg(0x4A, 0x00);

	writeReg(0xFF, 0x00);
	writeReg(0x7A, 0x0A);
	writeReg(0x7B, 0x00);
	writeReg(0x78, 0x21);

	writeReg(0xFF, 0x01);
	writeReg(0x23, 0x34);
	writeReg(0x42, 0x00);
	writeReg(0x44, 0xFF);
	writeReg(0x45, 0x26);
	writeReg(0x46, 0x05);
	writeReg(0x40, 0x40);
	writeReg(0x0E, 0x06);
	writeReg(0x20, 0x1A);
	writeReg(0x43, 0x40);

	writeReg(0xFF, 0x00);
	writeReg(0x34, 0x03);
	writeReg(0x35, 0x44);

	writeReg(0xFF, 0x01);
	writeReg(0x31, 0x04);
	writeReg(0x4B, 0x09);
	writeReg(0x4C, 0x05);
	writeReg(0x4D, 0x04);

	writeReg(0xFF, 0x00);
	writeReg(0x44, 0x00);
	writeReg(0x45, 0x20);
	writeReg(0x47, 0x08);
	writeReg(0x48, 0x28);
	writeReg(0x67, 0x00);
	writeReg(0x70, 0x04);
	writeReg(0x71, 0x01);
	writeReg(0x72, 0xFE);
	writeReg(0x76, 0x00);
	writeReg(0x77, 0x00);

	writeReg(0xFF, 0x01);
	writeReg(0x0D, 0x01);

	writeReg(0xFF, 0x00);
	writeReg(0x80, 0x01);
	writeReg(0x01, 0xF8);

	writeReg(0xFF, 0x01);
	writeReg(0x8E, 0x01);
	writeReg(0x00, 0x01);
	writeReg(0xFF, 0x00);
	writeReg(0x80, 0x00);

	// -- VL53L0X_load_tuning_settings() end

	// "Set interrupt config to new sample ready"
	// -- VL53L0X_SetGpioConfig() begin

	writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
	readReg(GPIO_HV_MUX_ACTIVE_HIGH,&Data);
	writeReg(GPIO_HV_MUX_ACTIVE_HIGH, Data & ~0x10); // active low
	writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

	// -- VL53L0X_SetGpioConfig() end

	measurement_timing_budget_us = getMeasurementTimingBudget();
	

	// "Disable MSRC and TCC by default"
	// MSRC = Minimum Signal Rate Check
	// TCC = Target CentreCheck
	// -- VL53L0X_SetSequenceStepEnable() begin

	writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);

	// -- VL53L0X_SetSequenceStepEnable() end

	// "Recalculate timing budget"
	setMeasurementTimingBudget(measurement_timing_budget_us);

	// VL53L0X_StaticInit() end

	// VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())

	// -- VL53L0X_perform_vhv_calibration() begin

	writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01);
	if (!performSingleRefCalibration(0x40)) { return false; }

	// -- VL53L0X_perform_vhv_calibration() end

	// -- VL53L0X_perform_phase_calibration() begin

	writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
	if (!performSingleRefCalibration(0x00)) { return false; }

	// -- VL53L0X_perform_phase_calibration() end

	// "restore the previous Sequence Config"
	writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);

	// VL53L0X_PerformRefCalibration() end

	return true;
}

// Returns a range reading in millimeters when continuous mode is active
// (readRangeSingleMillimeters() also calls this function after starting a
// single-shot range measurement)
uint16_t DriverVL53L0XReadContinuous()
{
	uint16_t Data16;
	uint8_t Data;
	startTimeout();
	
	
	while (1)
	{
		readReg(RESULT_INTERRUPT_STATUS,&Data);
		if ((Data & 0x07)!=0) break;
		if (checkTimeoutExpired())
		{
			did_timeout = true;
			return 65535;
		}		
	}
	// assumptions: Linearity Corrective Gain is 1000 (default);
	// fractional ranging is not enabled
	readReg16Bit(RESULT_RANGE_STATUS+10,&Data16);
	uint16_t range = Data16;

	writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

	return range;
}

// Performs a single-shot range measurement and returns the reading in
// millimeters
// based on VL53L0X_PerformSingleRangingMeasurement()
uint16_t DriverVL53L0XReadSingle()
{
	uint8_t Data;
	
	writeReg(0x80, 0x01);
	writeReg(0xFF, 0x01);
	writeReg(0x00, 0x00);
	writeReg(0x91, stop_variable);
	writeReg(0x00, 0x01);
	writeReg(0xFF, 0x00);
	writeReg(0x80, 0x00);

	writeReg(SYSRANGE_START, 0x01);
	

	// "Wait until start bit has been cleared"
	startTimeout();
	
	while (1)
	{
		readReg(SYSRANGE_START,&Data);
		if (! (Data & 0x01)) break;
		if (checkTimeoutExpired())
		{
			did_timeout = true;
			return 65535;
		}
	}

	return DriverVL53L0XReadContinuous();
}

// Start continuous ranging measurements. If period_ms (optional) is 0 or not
// given, continuous back-to-back mode is used (the sensor takes measurements as
// often as possible); otherwise, continuous timed mode is used, with the given
// inter-measurement period in milliseconds determining how often the sensor
// takes a measurement.
// based on VL53L0X_StartMeasurement()
void DriverVL53L0XStartContinuous(uint32_t period_ms)
{
	uint16_t osc_calibrate_val;
	
	writeReg(0x80, 0x01);
	writeReg(0xFF, 0x01);
	writeReg(0x00, 0x00);
	writeReg(0x91, stop_variable);
	writeReg(0x00, 0x01);
	writeReg(0xFF, 0x00);
	writeReg(0x80, 0x00);

	if (period_ms != 0)
	{
		// continuous timed mode

		// VL53L0X_SetInterMeasurementPeriodMilliSeconds() begin

		readReg16Bit(OSC_CALIBRATE_VAL,&osc_calibrate_val);

		if (osc_calibrate_val != 0)
		{
			period_ms *= osc_calibrate_val;
		}

		writeReg32Bit(SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);

		// VL53L0X_SetInterMeasurementPeriodMilliSeconds() end

		writeReg(SYSRANGE_START, 0x04); // VL53L0X_REG_SYSRANGE_MODE_TIMED
	}
	else
	{
		// continuous back-to-back mode
		writeReg(SYSRANGE_START, 0x02); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
	}
}

// Stop continuous measurements
// based on VL53L0X_StopMeasurement()
void DriverVL53L0XStopContinuous()
{
	writeReg(SYSRANGE_START, 0x01); // VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT

	writeReg(0xFF, 0x01);
	writeReg(0x00, 0x00);
	writeReg(0x91, 0x00);
	writeReg(0x00, 0x01);
	writeReg(0xFF, 0x00);
}



// Set the measurement timing budget in microseconds, which is the time allowed
// for one measurement; the ST API and this library take care of splitting the
// timing budget among the sub-steps in the ranging sequence. A longer timing
// budget allows for more accurate measurements. Increasing the budget by a
// factor of N decreases the range measurement standard deviation by a factor of
// sqrt(N). Defaults to about 33 milliseconds; the minimum is 20 ms.
// based on VL53L0X_set_measurement_timing_budget_micro_seconds()
uint8_t setMeasurementTimingBudget(uint32_t budget_us)
{
	SequenceStepEnables enables;
	SequenceStepTimeouts timeouts;

	uint16_t const StartOverhead     = 1910;
	uint16_t const EndOverhead        = 960;
	uint16_t const MsrcOverhead       = 660;
	uint16_t const TccOverhead        = 590;
	uint16_t const DssOverhead        = 690;
	uint16_t const PreRangeOverhead   = 660;
	uint16_t const FinalRangeOverhead = 550;

	uint32_t const MinTimingBudget = 20000;

	if (budget_us < MinTimingBudget) { return false; }

	uint32_t used_budget_us = StartOverhead + EndOverhead;

	getSequenceStepEnables(&enables);
	getSequenceStepTimeouts(&enables, &timeouts);

	if (enables.tcc)
	{
		used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
	}

	if (enables.dss)
	{
		used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
	}
	else if (enables.msrc)
	{
		used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
	}

	if (enables.pre_range)
	{
		used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
	}

	if (enables.final_range)
	{
		used_budget_us += FinalRangeOverhead;

		// "Note that the final range timeout is determined by the timing
		// budget and the sum of all other timeouts within the sequence.
		// If there is no room for the final range timeout, then an error
		// will be set. Otherwise the remaining time will be applied to
		// the final range."

		if (used_budget_us > budget_us)
		{
			// "Requested timeout too big."
			return false;
		}

		uint32_t final_range_timeout_us = budget_us - used_budget_us;

		// set_sequence_step_timeout() begin
		// (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

		// "For the final range timeout, the pre-range timeout
		//  must be added. To do this both final and pre-range
		//  timeouts must be expressed in macro periods MClks
		//  because they have different vcsel periods."

		uint32_t final_range_timeout_mclks =
		timeoutMicrosecondsToMclks(final_range_timeout_us,
		timeouts.final_range_vcsel_period_pclks);

		if (enables.pre_range)
		{
			final_range_timeout_mclks += timeouts.pre_range_mclks;
		}

		writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
		encodeTimeout(final_range_timeout_mclks));

		// set_sequence_step_timeout() end

		measurement_timing_budget_us = budget_us; // store for internal reuse
	}
	return true;
}

static uint8_t setSignalRateLimit(float limit_Mcps)
{
	if (limit_Mcps < 0 || limit_Mcps > 511.99) { return false; }

	// Q9.7 fixed point format (9 integer bits, 7 fractional bits)
	writeReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, limit_Mcps * (1 << 7));
	return true;
}

static uint8_t getSpadInfo(uint8_t * count, uint8_t * type_is_aperture)
{
	uint8_t tmp;
	uint8_t Data;

	writeReg(0x80, 0x01);
	writeReg(0xFF, 0x01);
	writeReg(0x00, 0x00);

	writeReg(0xFF, 0x06);
	readReg(0x83,&Data);
	writeReg(0x83, Data | 0x04);
	writeReg(0xFF, 0x07);
	writeReg(0x81, 0x01);

	writeReg(0x80, 0x01);

	writeReg(0x94, 0x6b);
	writeReg(0x83, 0x00);
	startTimeout();
	
	while (1)
	{
		readReg(0x83,&Data);
		if (Data!=0x00) break;
		if (checkTimeoutExpired()) { return false; }
	} 

	writeReg(0x83, 0x01);
	readReg(0x92,&tmp);

	*count = tmp & 0x7f;
	*type_is_aperture = (tmp >> 7) & 0x01;

	writeReg(0x81, 0x00);
	writeReg(0xFF, 0x06);
	readReg(0x83,&Data);
	writeReg(0x83, Data  & ~0x04);
	writeReg(0xFF, 0x01);
	writeReg(0x00, 0x01);

	writeReg(0xFF, 0x00);
	writeReg(0x80, 0x00);

	return true;
}


uint32_t static getMeasurementTimingBudget()
{
	SequenceStepEnables enables;
	SequenceStepTimeouts timeouts;

	uint16_t const StartOverhead     = 1910;
	uint16_t const EndOverhead        = 960;
	uint16_t const MsrcOverhead       = 660;
	uint16_t const TccOverhead        = 590;
	uint16_t const DssOverhead        = 690;
	uint16_t const PreRangeOverhead   = 660;
	uint16_t const FinalRangeOverhead = 550;

	// "Start and end overhead times always present"
	uint32_t budget_us = StartOverhead + EndOverhead;
	getSequenceStepEnables (&enables);
	getSequenceStepTimeouts(&enables, &timeouts);

	if (enables.tcc)
	{
		budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
	}

	if (enables.dss)
	{
		budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
	}
	else if (enables.msrc)
	{
		budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
	}

	if (enables.pre_range)
	{
		budget_us += (timeouts.pre_range_us + PreRangeOverhead);
	}

	if (enables.final_range)
	{
		budget_us += (timeouts.final_range_us + FinalRangeOverhead);
	}

	measurement_timing_budget_us = budget_us; // store for internal reuse
	return budget_us;
}

// Get sequence step enables
// based on VL53L0X_GetSequenceStepEnables()
static void getSequenceStepEnables(SequenceStepEnables * enables)
{
	uint8_t sequence_config;
	readReg(SYSTEM_SEQUENCE_CONFIG,&sequence_config);

	enables->tcc          = (sequence_config >> 4) & 0x1;
	enables->dss          = (sequence_config >> 3) & 0x1;
	enables->msrc         = (sequence_config >> 2) & 0x1;
	enables->pre_range    = (sequence_config >> 6) & 0x1;
	enables->final_range  = (sequence_config >> 7) & 0x1;
}

// Get sequence step timeouts
// based on get_sequence_step_timeout(),
// but gets all timeouts instead of just the requested one, and also stores
// intermediate values
static void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts)
{
	uint16_t Data16;
	uint8_t Data;
	timeouts->pre_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodPreRange);

	readReg(MSRC_CONFIG_TIMEOUT_MACROP,&Data);
	timeouts->msrc_dss_tcc_mclks = Data + 1;
	timeouts->msrc_dss_tcc_us =
	timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
	timeouts->pre_range_vcsel_period_pclks);

	readReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,&Data16);
	timeouts->pre_range_mclks =decodeTimeout(Data16);
	timeouts->pre_range_us =
	timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
	timeouts->pre_range_vcsel_period_pclks);

	timeouts->final_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodFinalRange);

	readReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,&Data16);
	timeouts->final_range_mclks =decodeTimeout(Data16);

	if (enables->pre_range)
	{
		timeouts->final_range_mclks -= timeouts->pre_range_mclks;
	}

	timeouts->final_range_us = timeoutMclksToMicroseconds(timeouts->final_range_mclks,timeouts->final_range_vcsel_period_pclks);
}

static uint16_t decodeTimeout(uint16_t reg_val)
{
	// format: "(LSByte * 2^MSByte) + 1"
	return (uint16_t)((reg_val & 0x00FF) <<
	(uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L0X_encode_timeout()
static uint16_t encodeTimeout(uint32_t timeout_mclks)
{
	// format: "(LSByte * 2^MSByte) + 1"

	uint32_t ls_byte = 0;
	uint16_t ms_byte = 0;

	if (timeout_mclks > 0)
	{
		ls_byte = timeout_mclks - 1;

		while ((ls_byte & 0xFFFFFF00) > 0)
		{
			ls_byte >>= 1;
			ms_byte++;
		}

		return (ms_byte << 8) | (ls_byte & 0xFF);
	}
	else { return 0; }
}


static uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

	return ((timeout_period_mclks * macro_period_ns) + 500) / 1000;
}

uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

	return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

static uint8_t getVcselPulsePeriod(vcselPeriodType type)
{
	uint8_t Data;
	if (type == VcselPeriodPreRange)
	{
		readReg(PRE_RANGE_CONFIG_VCSEL_PERIOD,&Data);
		return decodeVcselPeriod(Data);
	}
	else if (type == VcselPeriodFinalRange)
	{
		readReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD,&Data);
		return decodeVcselPeriod(Data);
	}
	else { return 255; }
}

// based on VL53L0X_perform_single_ref_calibration()
static uint8_t performSingleRefCalibration(uint8_t vhv_init_byte)
{
	uint8_t Data;
	writeReg(SYSRANGE_START, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP

	startTimeout();
	
	while (1)
	{
		readReg(RESULT_INTERRUPT_STATUS,&Data);
		if ((Data & 0x07)!=0) break;	
		if (checkTimeoutExpired()) 
		{ 
			return false; 
		}
	}


	writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

	writeReg(SYSRANGE_START, 0x00);

	return true;
}

static void startTimeout(void)
{
	timeout_start_ticks=portGET_RUN_TIME_COUNTER_VALUE();
}

static uint8_t checkTimeoutExpired()
{
	if (io_timeout>0 && portGET_RUN_TIME_COUNTER_VALUE() >= timeout_start_ticks+io_timeout) 
		return true;
	else
		return false;
}

static void setTimeout(uint16_t timeout) 
{ 
	io_timeout = timeout*(F_CPU/1000); 
}

static uint16_t getTimeout() 
{ 	
	return io_timeout/(F_CPU/1000); 
}

