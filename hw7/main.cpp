#include "mbed.h"

#include "arm_math.h"
#include "math_helper.h"
#include "stm32l475e_iot01_accelero.h"

/* ----------------------------------------------------------------------
** Macro Defines
** ------------------------------------------------------------------- */

#define TEST_LENGTH_SAMPLES 320
/*

This SNR is a bit small. Need to understand why
this example is not giving better SNR ...

*/
#define SNR_THRESHOLD_F32 75.0f
#define BLOCK_SIZE 32

#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
/* Must be a multiple of 16 */
#define NUM_TAPS_ARRAY_SIZE 32
#else
#define NUM_TAPS_ARRAY_SIZE 29
#endif

#define NUM_TAPS 29

/* -------------------------------------------------------------------
 * The input signal and reference output (computed with MATLAB)
 * are defined externally in arm_fir_lpf_data.c.
 * ------------------------------------------------------------------- */

extern float32_t testInput_f32_1kHz_15kHz[TEST_LENGTH_SAMPLES];
extern float32_t refOutput[TEST_LENGTH_SAMPLES];

/* -------------------------------------------------------------------
 * Declare Test output buffer
 * ------------------------------------------------------------------- */

static float32_t testOutput[TEST_LENGTH_SAMPLES];

/* -------------------------------------------------------------------
 * Declare State buffer of size (numTaps + blockSize - 1)
 * ------------------------------------------------------------------- */
#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
static float32_t firStateF32[2 * BLOCK_SIZE + NUM_TAPS - 1];
#else
static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
#endif

/* ----------------------------------------------------------------------
** FIR Coefficients buffer generated using fir1() MATLAB function.
** fir1(28, 6/24)
** ------------------------------------------------------------------- */
#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
const float32_t firCoeffs32[NUM_TAPS_ARRAY_SIZE] = {
    -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
    -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
    +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
    +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f, 0.0f, 0.0f, 0.0f};
#else
const float32_t firCoeffs32[NUM_TAPS_ARRAY_SIZE] = {
    -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
    -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
    +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
    +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f};
#endif

/* ------------------------------------------------------------------
 * Global variables for FIR LPF Example
 * ------------------------------------------------------------------- */

uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = TEST_LENGTH_SAMPLES / BLOCK_SIZE;

float32_t snr;

/* ----------------------------------------------------------------------
 * FIR LPF Example
 * ------------------------------------------------------------------- */

int32_t main(void)
{
  // * Test data
  uint32_t i;
  arm_fir_instance_f32 S;
  arm_status status;
  float32_t *inputF32, *outputF32;

  /* Initialize input and output buffer pointers */
  inputF32 = &testInput_f32_1kHz_15kHz[0];
  outputF32 = &testOutput[0];

  /* Call FIR init function to initialize the instance structure. */
  arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);
  /* ----------------------------------------------------------------------
  ** Call the FIR process function for every blockSize samples
  ** ------------------------------------------------------------------- */

  for (i = 0; i < numBlocks; i++)
  {
    arm_fir_f32(&S, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
  }

  /* ----------------------------------------------------------------------
  ** Compare the generated output against the reference output computed
  ** in MATLAB.
  ** ------------------------------------------------------------------- */

  snr = arm_snr_f32(&refOutput[0], &testOutput[0], TEST_LENGTH_SAMPLES);

  status = (snr < SNR_THRESHOLD_F32) ? ARM_MATH_TEST_FAILURE : ARM_MATH_SUCCESS;
  if (status != ARM_MATH_SUCCESS)
  {
    printf("FAILURE\n");
    while (1)
      ; /* main function does not return */
  }
  else
  {
    printf("SUCCESS\n");

    // TODO: When button pressed => Read data from accelerometer and apply FIR filter

    // Initialize the accelerometer
    BSP_ACCELERO_Init();

    // Variables to store the acceleration data
    float32_t buffer[TEST_LENGTH_SAMPLES];
    float32_t output[TEST_LENGTH_SAMPLES];

    int16_t pDataXYZ[3];

    // Read the acceleration data
    for (int i = 0; i < TEST_LENGTH_SAMPLES; i++)
    {
      BSP_ACCELERO_AccGetXYZ(pDataXYZ);
      buffer[i] = float32_t(pDataXYZ[0]);
      // wait
      ThisThread::sleep_for(10ms);
    }

    // Call FIR init function to initialize the instance structure.
    arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], blockSize);

    // Call the FIR process function for every blockSize samples
    for (i = 0; i < numBlocks; i++)
    {
      arm_fir_f32(&S, buffer + (i * blockSize), output + (i * blockSize), blockSize);
    }

    // Print the input
    printf("\n================INPUT================\n");
    for (int i = 0; i < TEST_LENGTH_SAMPLES; i++)
    {
      printf("%d, \t", int16_t(buffer[i]));
    }

    printf("\n================OUTPUT================\n");
    // Print the output
    for (int i = 0; i < TEST_LENGTH_SAMPLES; i++)
    {
      printf("%d, \t", int16_t(output[i]));
    }
    printf("\n================END================\n");

    while (1)
      ;
  }
}

/** \endlink */
