#include <pwm.h>
// Assumes a 125MHz system clock.

const float PWM::DEFAULT_PWM_FREQ_HZ = 200.0f;

PWM::PWM(uint8_t pwm_pin)
:pwm_pin_{pwm_pin}
{
// inpired by:
// https://github.com/raspberrypi/pico-examples/blob/master/pwm/hello_pwm/hello_pwm.c#L14-L29
    /// Preemptively setup SIO to drive the pin LOW so that toggling pin
    /// function in disable_output() will immediately drive the pin low..
    gpio_set_dir(pwm_pin_, GPIO_OUT);
    gpio_put(pwm_pin_, 0);

    // Allocate gpio pin for pwm.
    gpio_set_function(pwm_pin_, GPIO_FUNC_PWM);

    // Find out (and save) which hardware (PWM slice & channel) are connected
    // to this GPIO pin.
    slice_num_ = pwm_gpio_to_slice_num(pwm_pin_);
    gpio_channel_ = pwm_gpio_to_channel(pwm_pin_);

    // Set period of 100 cycles (0 to 99 inclusive) (reg TOP value).
    set_pwm_step_increments(DEFAULT_PWM_STEP_INCREMENTS);
    // Clear output duty cycle on startup.
    set_duty_cycle(0);
    set_frequency(DEFAULT_PWM_FREQ_HZ); // 200Hz

    // Yield pin control to SIO.
    disable_output();

    // Enabling / Disabling PWM must be done by changing the duty cycle
    // and leaving the slice enabled bc disabling the slice leaves the GPIO
    // fixed in its current state.
    pwm_set_enabled(slice_num_, true);
}


PWM::~PWM()
{
    disable_output();
    pwm_set_chan_level(slice_num_, gpio_channel_, 0);
    // Don't call pwm_set_enabled because other pins may be configured to use
    // the same slice.
    gpio_deinit(pwm_pin_);
}

float PWM::set_duty_cycle(float normalized_duty_cycle)
{
    // Clamp output.
    if (normalized_duty_cycle > 1.0f)
        normalized_duty_cycle = 1.0f;
    else if (normalized_duty_cycle < 0.0f)
        normalized_duty_cycle = 0.0;
    duty_cycle_ = roundf(normalized_duty_cycle * pwm_step_increments_);
    pwm_set_chan_level(slice_num_, gpio_channel_, duty_cycle_);
    return float(duty_cycle_)/pwm_step_increments_;
}


float PWM::set_frequency(float freq_hz)
{
    uint32_t sys_clk_hz = clock_get_hz(clk_sys);
    // Configure for n[Hz] period broken down into PWM_STEP_INCREMENTS.
    // requested value must be within [0.0, 256.0]

    // FIXME: divide-by-zero edge case.
    float new_freq_div = sys_clk_hz / (freq_hz * pwm_step_increments_);
    pwm_set_clkdiv(slice_num_, new_freq_div);
    float actual_freq_hz = sys_clk_hz /( new_freq_div * pwm_step_increments_);
    return actual_freq_hz;
}

