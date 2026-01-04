#include "hal_adc.h"
#include "hal_adc_mock.h"
#include <assert.h>
#include <stdio.h>

void test_adc_read_write(void) {
    printf("Running test: %s\n", __func__);
    hal_adc_init();

    mock_adc_set_value(ADC_CHANNEL_0, 1234);
    assert(hal_adc_read(ADC_CHANNEL_0) == 1234);

    mock_adc_set_value(ADC_CHANNEL_3, 4321);
    assert(hal_adc_read(ADC_CHANNEL_3) == 4321);

    // test overwrite
    mock_adc_set_value(ADC_CHANNEL_0, 555);
    assert(hal_adc_read(ADC_CHANNEL_0) == 555);

    printf("Test passed.\n");
}

void test_adc_out_of_bounds(void) {
    printf("Running test: %s\n", __func__);
    hal_adc_init();

    // these calls should not crash and should return 0
    uint16_t value = hal_adc_read(ADC_CHANNEL_MAX);
    assert(value == 0);

    // this should not crash
    mock_adc_set_value(ADC_CHANNEL_MAX, 1234);

    // reading it again should still be 0
    value = hal_adc_read(ADC_CHANNEL_MAX);
    assert(value == 0);

    printf("Test passed.\n");
}

int main(void) {
    test_adc_read_write();
    test_adc_out_of_bounds();

    return 0;
}
